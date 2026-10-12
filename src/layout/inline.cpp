/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

void Engine::collect_inline(int node, const ComputedStyle& st, std::vector<Segment>& out)
{
    const Node& n = doc->node(node);
    if (n.kind == NodeKind::k_text)
    {
        if (!n.text.empty())
        {
            Segment seg;
            seg.node = node;
            seg.style = st;
            seg.text = transform_text(std::string(n.text), st.text_transform);
            out.push_back(seg);
        }
        return;
    }
    if (n.tag == "br")
    {
        Segment seg;
        seg.node = node;
        seg.style = st;
        seg.break_before = true;
        out.push_back(seg);
        return;
    }
    if (n.tag == "img" || n.tag == "input")
    {
        Segment seg;
        seg.node = node;
        seg.style = st;
        seg.replaced = true;
        if (n.tag == "img")
        {
            float iw = 320.0f;
            float ih = 180.0f;
            float tw = 0.0f;
            float th = 0.0f;
            if (image_metrics && image_metrics->size_of(doc->attr(node, "src"), tw, th) && tw > 0.0f && th > 0.0f)
            {
                iw = tw;
                ih = th;
            }
            seg.replaced_w = st.width.is_auto() ? iw : resolve_len(st.width, iw, st.font_size, viewport_w, viewport_h);
            seg.replaced_h = st.height.is_auto() ? ih : resolve_len(st.height, ih, st.font_size, viewport_w, viewport_h);
        }
        else if (n.tag == "input")
        {
            seg.replaced_w = st.width.is_auto() ? 120.0f : resolve_len(st.width, 120.0f, st.font_size, viewport_w, viewport_h);
            seg.replaced_h = line_height_of(st) + 8.0f;
        }
        else
        {
            std::vector<Segment> inner;
            for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
            {
                collect_inline(c, result->styles[static_cast<std::size_t>(c)], inner);
            }
            float w = 0.0f;
            float h = 0.0f;
            for (const auto& s2 : inner)
            {
                if (s2.break_before)
                {
                    continue;
                }
                if (s2.replaced)
                {
                    w += s2.replaced_w;
                    if (s2.replaced_h > h)
                    {
                        h = s2.replaced_h;
                    }
                }
                else
                {
                    w += text_width(s2.text, s2.style);
                    float lh = line_height_of(s2.style);
                    if (lh > h)
                    {
                        h = lh;
                    }
                }
            }
            seg.replaced_w = w + resolve_len(st.padding[1], 0.0f, st.font_size, viewport_w, viewport_h) +
                resolve_len(st.padding[3], 0.0f, st.font_size, viewport_w, viewport_h);
            seg.replaced_h = h + resolve_len(st.padding[0], 0.0f, st.font_size, viewport_w, viewport_h) +
                resolve_len(st.padding[2], 0.0f, st.font_size, viewport_w, viewport_h);
        }
        out.push_back(seg);
        return;
    }
    for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
    {
        collect_inline(c, result->styles[static_cast<std::size_t>(c)], out);
    }
}

void Engine::layout_lines(Box& box, std::vector<Segment> segs, float avail_w, float ox, float start_y)
{
    float y = start_y;
    std::size_t i = 0;
    while (i < segs.size())
    {
        while (i < segs.size() && segs[i].text.empty() && !segs[i].replaced && !segs[i].break_before)
        {
            ++i;
        }
        if (i >= segs.size())
        {
            break;
        }
        if (segs[i].break_before)
        {
            Line line;
            line.y = y;
            line.height = line_height_of(box.style);
            y += line.height;
            box.lines.push_back(line);
            ++i;
            continue;
        }
        bool no_wrap = (segs[i].style.white_space == 1 || segs[i].style.white_space == 2);
        float line_h = 0.0f;
        float used = 0.0f;
        std::size_t j = i;
        std::vector<Segment> placed;
        while (j < segs.size() && !segs[j].break_before)
        {
            if (segs[j].replaced)
            {
                if (used > 0.0f && used + segs[j].replaced_w > avail_w)
                {
                    break;
                }
                used += segs[j].replaced_w;
                if (segs[j].replaced_h > line_h)
                {
                    line_h = segs[j].replaced_h;
                }
                placed.push_back(segs[j]);
                ++j;
                continue;
            }
            std::vector<std::string> words = split_words(segs[j].text, segs[j].style.white_space);
            std::string acc;
            bool consumed_all = true;
            for (std::size_t w = 0; w < words.size(); ++w)
            {
                if (words[w] == "\n")
                {
                    consumed_all = false;
                    segs[j].text = segs[j].text.substr(acc.size() + 1);
                    break;
                }
                std::string trial = acc + words[w];
                float tw = text_width(trial, segs[j].style);
                if (used + tw > avail_w && (!acc.empty() || used > 0.0f))
                {
                    consumed_all = false;
                    segs[j].text = segs[j].text.substr(acc.size());
                    break;
                }
                acc = trial;
                float th = line_height_of(segs[j].style);
                if (th > line_h)
                {
                    line_h = th;
                }
            }
            if (acc.empty() && used == 0.0f && !words.empty() && words[0] != " " && words[0] != "\n")
            {
                acc = words[0];
                segs[j].text = segs[j].text.substr(acc.size());
                consumed_all = segs[j].text.empty();
                float th = line_height_of(segs[j].style);
                if (th > line_h)
                {
                    line_h = th;
                }
            }
            if (!acc.empty())
            {
                Segment p = segs[j];
                p.text = acc;
                placed.push_back(p);
                used += text_width(acc, segs[j].style);
            }
            if (consumed_all)
            {
                ++j;
            }
            if (!consumed_all || no_wrap)
            {
                if (no_wrap)
                {
                    ++j;
                    while (j < segs.size() && !segs[j].break_before)
                    {
                        if (segs[j].replaced)
                        {
                            placed.push_back(segs[j]);
                            used += segs[j].replaced_w;
                            ++j;
                            continue;
                        }
                        Segment p = segs[j];
                        placed.push_back(p);
                        used += text_width(segs[j].text, segs[j].style);
                        ++j;
                    }
                }
                break;
            }
        }
        if (placed.empty())
        {
            ++i;
            continue;
        }
        if (line_h <= 0.0f)
        {
            line_h = line_height_of(box.style);
        }
        if (box.style.ellipsis && box.style.white_space == 1 && used > avail_w)
        {
            float over = used - avail_w + 20.0f;
            for (std::size_t k = placed.size(); k-- > 0;)
            {
                if (placed[k].replaced || placed[k].text.empty())
                {
                    continue;
                }
                while (!placed[k].text.empty() && over > 0.0f)
                {
                    float cw = text_width(placed[k].text, placed[k].style) -
                        text_width(placed[k].text.substr(0, placed[k].text.size() - 1), placed[k].style);
                    placed[k].text.pop_back();
                    over -= (cw > 0.0f ? cw : 1.0f);
                }
                placed[k].text += "…";
                break;
            }
        }
        Line line;
        line.y = y;
        line.height = line_h;
        float cx = ox;
        for (const auto& p : placed)
        {
            Run run;
            run.node = p.node;
            run.style = p.style;
            if (p.replaced)
            {
                run.width = p.replaced_w;
            }
            else
            {
                run.text = p.text;
                run.width = text_width(p.text, p.style);
            }
            run.x = cx;
            cx += run.width;
            line.runs.push_back(run);
        }
        float total = cx - ox;
        if (box.style.text_align == 1)
        {
            float shift = (avail_w - total) / 2.0f;
            for (auto& r : line.runs)
            {
                r.x += shift;
            }
        }
        else if (box.style.text_align == 2)
        {
            float shift = avail_w - total;
            for (auto& r : line.runs)
            {
                r.x += shift;
            }
        }
        box.lines.push_back(line);
        y += line_h;
        i = j;
    }
    box.h = (y - start_y) + pad_bottom(box.style, avail_w);
}

} // namespace wv
