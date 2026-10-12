/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

int Engine::box_for_node(int node) const
{
    if (node < 0 || static_cast<std::size_t>(node) >= node_box.size())
    {
        return -1;
    }
    return node_box[static_cast<std::size_t>(node)];
}

int Engine::alloc_box(int node, int parent)
{
    Box b;
    b.node = node;
    b.parent = parent;
    int idx = static_cast<int>(result->boxes.size());
    result->boxes.push_back(b);
    if (parent >= 0)
    {
        Box& p = result->boxes[static_cast<std::size_t>(parent)];
        if (p.first_child < 0)
        {
            p.first_child = idx;
        }
        else
        {
            int sib = p.first_child;
            while (result->boxes[static_cast<std::size_t>(sib)].next_sibling >= 0)
            {
                sib = result->boxes[static_cast<std::size_t>(sib)].next_sibling;
            }
            result->boxes[static_cast<std::size_t>(sib)].next_sibling = idx;
        }
    }
    if (node >= 0 && static_cast<std::size_t>(node) < node_box.size())
    {
        node_box[static_cast<std::size_t>(node)] = idx;
    }
    return idx;
}

void Engine::pseudo_segments(int box_idx, std::vector<Segment>& before, std::vector<Segment>& after)
{
    bool first = true;
    for (int c = result->boxes[static_cast<std::size_t>(box_idx)].first_child; c >= 0;
         c = result->boxes[static_cast<std::size_t>(c)].next_sibling)
    {
        const Box& child = result->boxes[static_cast<std::size_t>(c)];
        if (child.node >= 0)
        {
            continue;
        }
        Segment seg;
        seg.style = child.style;
        seg.text = child.style.content;
        if (first)
        {
            before.push_back(seg);
        }
        else
        {
            after.push_back(seg);
        }
        first = false;
    }
}

bool Engine::has_pseudo_rule(int node, const std::string& pe, const ElementState& state)
{
    for (const auto& rule : sheet->rules)
    {
        if (!rule_applies(rule, ctx.viewport_width))
        {
            continue;
        }
        for (const auto& sel : rule.selectors)
        {
            std::string found;
            for (const auto& part : sel.parts)
            {
                if (!part.pseudo_element.empty())
                {
                    found = part.pseudo_element;
                }
            }
            if (found == pe && match_selector(*doc, node, sel, state, pe))
            {
                return true;
            }
        }
    }
    return false;
}

int Engine::build_boxes(int node, int parent_box, const ElementState& state)
{
    const Node& n = doc->node(node);
    if (n.kind == NodeKind::k_text)
    {
        return -1;
    }
    const ComputedStyle& st = result->styles[static_cast<std::size_t>(node)];
    if (st.display == Display::k_none)
    {
        return -1;
    }
    int idx = alloc_box(node, parent_box);
    result->boxes[static_cast<std::size_t>(idx)].style = st;
    for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
    {
        build_boxes(c, idx, state);
    }
    for (const char* pe : {"before", "after"})
    {
        if (!has_pseudo_rule(node, pe, state))
        {
            continue;
        }
        ComputedStyle pst = default_style_for_tag("span");
        pst.display = Display::k_inline;
        pst.color = st.color;
        pst.font_family = st.font_family;
        pst.font_size = st.font_size;
        pst.font_weight = st.font_weight;
        pst.font_italic = st.font_italic;
        const std::vector<std::pair<std::string, std::string>>& pvars = result->vars[static_cast<std::size_t>(node)];
        for (const auto& rule : sheet->rules)
        {
            if (!rule_applies(rule, ctx.viewport_width))
            {
                continue;
            }
            for (const auto& sel : rule.selectors)
            {
                if (!match_selector(*doc, node, sel, state, pe))
                {
                    continue;
                }
                for (const auto& d : rule.declarations)
                {
                    if (d.property.rfind("--", 0) == 0)
                    {
                        continue;
                    }
                    apply_declaration(pst, d.property, detail::substitute_vars(d.value, pvars), &st, ctx.viewport_width, viewport_h);
                }
            }
        }
        if (pst.has_content && !pst.content.empty())
        {
            int gidx = alloc_box(-1, idx);
            result->boxes[static_cast<std::size_t>(gidx)].style = pst;
        }
    }
    return idx;
}

void Engine::finalize(int box_idx, float ox, float oy)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    box.x += ox;
    box.y += oy;
    for (auto& line : box.lines)
    {
        line.y += box.y;
        for (auto& r : line.runs)
        {
            r.x += box.x;
        }
    }
    for (int c = box.first_child; c >= 0; c = result->boxes[static_cast<std::size_t>(c)].next_sibling)
    {
        finalize(c, box.x, box.y);
    }
}

void Engine::layout_box(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle& st = box.style;
    if (st.display == Display::k_none)
    {
        box.w = 0.0f;
        box.h = 0.0f;
        return;
    }
    int my_abs = abs_cb;
    if (st.position == Position::k_relative || st.position == Position::k_absolute || st.position == Position::k_fixed)
    {
        my_abs = box_idx;
    }
    if (box.node >= 0)
    {
        const Node& n = doc->node(box.node);
        if (n.kind == NodeKind::k_element && (n.tag == "img" || n.tag == "input"))
        {
            layout_replaced(box_idx, cb_w, my_abs);
            return;
        }
    }
    if (st.display == Display::k_flex || st.display == Display::k_inline_flex)
    {
        layout_flex(box_idx, cb_w, my_abs);
        return;
    }
    if (st.display == Display::k_grid)
    {
        layout_grid(box_idx, cb_w, my_abs);
        return;
    }
    if (st.display == Display::k_table)
    {
        layout_table(box_idx, cb_w, my_abs);
        return;
    }
    if (is_inline_display(st.display))
    {
        layout_inline_block(box_idx, cb_w, my_abs);
        return;
    }
    layout_block(box_idx, cb_w, my_abs);
}

bool rule_applies(const Rule& rule, float viewport_w)
{
    if (rule.media_max_width >= 0.0f && viewport_w > rule.media_max_width)
    {
        return false;
    }
    return rule.media_min_width < 0.0f || viewport_w >= rule.media_min_width;
}

LayoutResult layout_document(
    const Document& doc,
    const Stylesheet& sheet,
    TextMetrics& text_metrics,
    ImageMetrics* image_metrics,
    const ResolveContext& ctx,
    float viewport_w,
    float viewport_h)
{
    Engine e;
    e.doc = &doc;
    e.sheet = &sheet;
    e.text_metrics = &text_metrics;
    e.image_metrics = image_metrics;
    e.ctx = ctx;
    e.viewport_w = viewport_w;
    e.viewport_h = viewport_h;
    LayoutResult result;
    e.result = &result;
    result.styles.resize(doc.node_count());
    result.vars.resize(doc.node_count());
    e.node_box.assign(doc.node_count(), -1);
    ElementState state;
    for (std::size_t i = 0; i < doc.node_count(); ++i)
    {
        const Node& n = doc.node(static_cast<int>(i));
        if (n.kind != NodeKind::k_element)
        {
            continue;
        }
        const ComputedStyle* parent = nullptr;
        const std::vector<std::pair<std::string, std::string>>* pvars = nullptr;
        if (n.parent >= 0)
        {
            parent = &result.styles[static_cast<std::size_t>(n.parent)];
            pvars = &result.vars[static_cast<std::size_t>(n.parent)];
        }
        result.styles[i] = resolve_style(doc, static_cast<int>(i), parent, pvars, sheet, state, ctx, result.vars[i]);
    }
    int html = doc.root();
    if (html < 0)
    {
        return result;
    }
    int root_box = e.build_boxes(html, -1, state);
    result.root_box = root_box;
    if (root_box >= 0)
    {
        e.layout_box(root_box, viewport_w, root_box);
        Box& root = result.boxes[static_cast<std::size_t>(root_box)];
        root.w = viewport_w;
        root.x = 0.0f;
        root.y = 0.0f;
        e.finalize(root_box, 0.0f, 0.0f);
    }
    return result;
}

} // namespace wv
