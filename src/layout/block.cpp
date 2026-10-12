/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

void Engine::layout_block(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle st = box.style;
    float avail = content_width(st, cb_w);
    float ox = pad_left(st, cb_w);
    float y = pad_top(st, cb_w);
    box.w = box_width(st, cb_w);
    const Node& n = (box.node >= 0) ? doc->node(box.node) : doc->node(doc->root());
    std::vector<Segment> pending;
    std::vector<Segment> before;
    std::vector<Segment> after;
    pseudo_segments(box_idx, before, after);
    for (const auto& s : before)
    {
        pending.push_back(s);
    }
    if (box.node < 0 && st.has_content && !st.content.empty())
    {
        Segment seg;
        seg.style = st;
        seg.text = st.content;
        pending.push_back(seg);
    }
    auto flush = [&]()
    {
        if (pending.empty())
        {
            return;
        }
        layout_lines(box, pending, avail, ox, y);
        y += box.h - pad_bottom(st, avail);
        box.h = 0.0f;
        pending.clear();
    };
    if (box.node >= 0)
    {
        for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
        {
            const Node& cn = doc->node(c);
            if (cn.kind == NodeKind::k_text)
            {
                if (!cn.text.empty())
                {
                    collect_inline(c, result->styles[static_cast<std::size_t>(c)], pending);
                }
                continue;
            }
            int cb2 = box_for_node(c);
            if (cb2 < 0)
            {
                continue;
            }
            Box& child = result->boxes[static_cast<std::size_t>(cb2)];
            if (child.style.position == Position::k_absolute || child.style.position == Position::k_fixed)
            {
                flush();
                child.x = ox;
                child.y = y;
                layout_box(cb2, avail, abs_cb);
                continue;
            }
            if (is_inline_display(child.style.display))
            {
                collect_inline(c, child.style, pending);
                continue;
            }
            flush();
            float mt = resolve_len(child.style.margin[0], avail, child.style.font_size, viewport_w, viewport_h);
            float mb = resolve_len(child.style.margin[2], avail, child.style.font_size, viewport_w, viewport_h);
            y += mt;
            child.x = ox + resolve_len(child.style.margin[3], avail, child.style.font_size, viewport_w, viewport_h);
            child.y = y;
            layout_box(cb2, avail, abs_cb);
            y += result->boxes[static_cast<std::size_t>(cb2)].h + mb;
        }
    }
    flush();
    if (!after.empty())
    {
        layout_lines(box, after, avail, ox, y);
        y += box.h - pad_bottom(st, avail);
        box.h = 0.0f;
    }
    float h = y + pad_bottom(st, avail);
    if (!st.height.is_auto())
    {
        float fixed = resolve_len(st.height, cb_w, st.font_size, viewport_w, viewport_h);
        h = (st.box_border ? fixed : fixed + pad_top(st, cb_w) + pad_bottom(st, avail));
    }
    box.h = h;
    measure_scroll_extents(*result, box_idx);
    apply_position(box_idx, cb_w, abs_cb);
}

void Engine::apply_position(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle& st = box.style;
    if (st.position == Position::k_absolute || st.position == Position::k_fixed)
    {
        const Box& cb = (st.position == Position::k_fixed || abs_cb < 0)
            ? result->boxes[static_cast<std::size_t>(result->root_box)]
            : result->boxes[static_cast<std::size_t>(abs_cb)];
        float cbw = cb.w;
        if (!st.left.is_auto())
        {
            box.x = cb.x + resolve_len(st.left, cbw, st.font_size, viewport_w, viewport_h);
        }
        else if (!st.right.is_auto())
        {
            box.x = cb.x + cbw - resolve_len(st.right, cbw, st.font_size, viewport_w, viewport_h) - box.w;
        }
        if (!st.top.is_auto())
        {
            box.y = cb.y + resolve_len(st.top, cbw, st.font_size, viewport_w, viewport_h);
        }
        else if (!st.bottom.is_auto())
        {
            box.y = cb.y + cb.h - resolve_len(st.bottom, cbw, st.font_size, viewport_w, viewport_h) - box.h;
        }
        if (st.has_translate)
        {
            box.x += st.translate_x;
            box.y += st.translate_y;
        }
        (void)cb_w;
        return;
    }
    if (st.position == Position::k_relative)
    {
        if (!st.left.is_auto())
        {
            box.x += resolve_len(st.left, cb_w, st.font_size, viewport_w, viewport_h);
        }
        if (!st.top.is_auto())
        {
            box.y += resolve_len(st.top, cb_w, st.font_size, viewport_w, viewport_h);
        }
    }
}

void Engine::layout_inline_block(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle st = box.style;
    std::vector<Segment> segs;
    std::vector<Segment> before;
    std::vector<Segment> after;
    pseudo_segments(box_idx, before, after);
    for (const auto& s : before)
    {
        segs.push_back(s);
    }
    if (box.node >= 0)
    {
        collect_inline(box.node, st, segs);
    }
    else if (st.has_content && !st.content.empty())
    {
        Segment seg;
        seg.style = st;
        seg.text = st.content;
        segs.push_back(seg);
    }
    for (const auto& s : after)
    {
        segs.push_back(s);
    }
    float natural = 0.0f;
    for (const auto& s : segs)
    {
        if (s.break_before)
        {
            continue;
        }
        if (s.replaced)
        {
            natural += s.replaced_w;
        }
        else
        {
            natural += text_width(s.text, s.style);
        }
    }
    float avail = st.width.is_auto() ? natural : content_width(st, cb_w);
    if (cb_w < k_inf / 2.0f && avail > content_width(st, cb_w) && st.width.is_auto())
    {
        avail = content_width(st, cb_w);
    }
    float ox = pad_left(st, cb_w);
    float oy = pad_top(st, cb_w);
    box.w = (st.width.is_auto() ? avail : content_width(st, cb_w)) + pad_x(st, cb_w);
    if (st.display == Display::k_inline && st.width.is_auto())
    {
        box.w = avail + pad_x(st, cb_w);
    }
    layout_lines(box, segs, avail, ox, oy);
    float h = box.h;
    if (!st.height.is_auto())
    {
        float fixed = resolve_len(st.height, cb_w, st.font_size, viewport_w, viewport_h);
        h = (st.box_border ? fixed : fixed + pad_top(st, cb_w) + pad_bottom(st, avail));
    }
    box.h = h;
    measure_scroll_extents(*result, box_idx);
    apply_position(box_idx, cb_w, abs_cb);
}

void Engine::layout_replaced(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle& st = box.style;
    const Node& n = doc->node(box.node);
    float iw = 300.0f;
    float ih = 150.0f;
    if (n.tag == "img")
    {
        iw = 320.0f;
        ih = 180.0f;
        float tw = 0.0f;
        float th = 0.0f;
        if (image_metrics && image_metrics->size_of(doc->attr(box.node, "src"), tw, th) && tw > 0.0f && th > 0.0f)
        {
            iw = tw;
            ih = th;
        }
    }
    else
    {
        iw = 120.0f;
        ih = line_height_of(st) + 8.0f;
    }
    if (!st.width.is_auto())
    {
        iw = resolve_len(st.width, cb_w, st.font_size, viewport_w, viewport_h);
    }
    if (!st.height.is_auto())
    {
        ih = resolve_len(st.height, cb_w, st.font_size, viewport_w, viewport_h);
    }
    else if (!st.width.is_auto() && n.tag == "img")
    {
        float tw = 0.0f;
        float th = 0.0f;
        if (image_metrics && image_metrics->size_of(doc->attr(box.node, "src"), tw, th) && tw > 0.0f && th > 0.0f)
        {
            ih = iw * th / tw;
        }
    }
    box.w = iw + pad_x(st, cb_w);
    box.h = ih + pad_top(st, cb_w) + pad_bottom(st, cb_w);
    if (n.tag == "img")
    {
        box.intrinsic_image = true;
        box.intrinsic_w = iw;
        box.intrinsic_h = ih;
    }
    measure_scroll_extents(*result, box_idx);
    apply_position(box_idx, cb_w, abs_cb);
}

} // namespace wv
