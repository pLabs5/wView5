/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/paint.hpp"

namespace wv
{

namespace
{

bool has_radius(const float r[4])
{
    return r[0] > 0.0f || r[1] > 0.0f || r[2] > 0.0f || r[3] > 0.0f;
}

bool has_border(const ComputedStyle& st)
{
    return st.border_width[0].value > 0.0f || st.border_width[1].value > 0.0f ||
        st.border_width[2].value > 0.0f || st.border_width[3].value > 0.0f;
}

void paint_background(const Box& box, Painter& painter)
{
    const ComputedStyle& st = box.style;
    if (st.shadow.has)
    {
        painter.draw_shadow(box.x, box.y, box.w, box.h, st.radius, st.shadow);
    }
    if (st.background.has_gradient)
    {
        painter.fill_gradient(box.x, box.y, box.w, box.h, st.radius, st.background);
        return;
    }
    if (!st.background.has_color || st.background.color.a == 0)
    {
        return;
    }
    if (has_radius(st.radius))
    {
        painter.fill_rrect(box.x, box.y, box.w, box.h, st.radius, st.background.color);
    }
    else
    {
        painter.fill_rect(box.x, box.y, box.w, box.h, st.background.color);
    }
}

void paint_borders(const Box& box, Painter& painter)
{
    const ComputedStyle& st = box.style;
    if (!has_border(st) || st.border_color.a == 0)
    {
        return;
    }
    float t = st.border_width[0].value;
    float r = st.border_width[1].value;
    float b = st.border_width[2].value;
    float l = st.border_width[3].value;
    if (t > 0.0f)
    {
        painter.fill_rect(box.x, box.y, box.w, t, st.border_color);
    }
    if (b > 0.0f)
    {
        painter.fill_rect(box.x, box.y + box.h - b, box.w, b, st.border_color);
    }
    if (l > 0.0f)
    {
        painter.fill_rect(box.x, box.y + t, l, box.h - t - b, st.border_color);
    }
    if (r > 0.0f)
    {
        painter.fill_rect(box.x + box.w - r, box.y + t, r, box.h - t - b, st.border_color);
    }
}

void paint_box(const Document& doc, const LayoutResult& layout, int box_idx, Painter& painter, float ox, float oy)
{
    const Box& box = layout.boxes[static_cast<std::size_t>(box_idx)];
    if (box.style.display == Display::k_none)
    {
        return;
    }
    float bx = box.x + ox;
    float by = box.y + oy;
    bool clip = (box.style.overflow_x != Overflow::k_visible ||
                 box.style.overflow_y != Overflow::k_visible);
    if (clip)
    {
        painter.push_clip(bx, by, box.w, box.h);
    }
    {
        Box shifted = box;
        shifted.x = bx;
        shifted.y = by;
        paint_background(shifted, painter);
    }
    for (const auto& line : box.lines)
    {
        for (const auto& run : line.runs)
        {
            if (run.node >= 0)
            {
                const Node& n = doc.node(run.node);
                if (n.kind == NodeKind::k_element && n.tag == "img")
                {
                    painter.draw_image(run.x + ox, line.y + oy, run.width, line.height,
                                       run.node, doc.attr(run.node, "src"));
                    continue;
                }
            }
            if (!run.text.empty())
            {
                Color c = run.style.color;
                c.a = static_cast<std::uint8_t>(static_cast<float>(c.a) * box.style.opacity);
                painter.draw_text(run.x + ox, line.y + oy, run.text, run.style, c);
            }
        }
    }
    if (box.node >= 0)
    {
        const Node& n = doc.node(box.node);
        if (n.kind == NodeKind::k_element && (n.tag == "img" || n.tag == "input") && box.lines.empty())
        {
            if (n.tag == "img")
            {
                painter.draw_image(bx, by, box.intrinsic_w, box.intrinsic_h, box.node, doc.attr(box.node, "src"));
            }
        }
    }
    for (int c = box.first_child; c >= 0; c = layout.boxes[static_cast<std::size_t>(c)].next_sibling)
    {
        const Box& child = layout.boxes[static_cast<std::size_t>(c)];
        if (child.style.position == Position::k_absolute || child.style.position == Position::k_fixed)
        {
            continue;
        }
        if (child.node < 0)
        {
            continue;
        }
        if (child.style.display == Display::k_inline && child.lines.empty())
        {
            continue;
        }
        paint_box(doc, layout, c, painter, ox - box.scroll_x, oy - box.scroll_y);
    }
    {
        Box shifted = box;
        shifted.x = bx;
        shifted.y = by;
        paint_borders(shifted, painter);
    }
    for (int c = box.first_child; c >= 0; c = layout.boxes[static_cast<std::size_t>(c)].next_sibling)
    {
        const Box& child = layout.boxes[static_cast<std::size_t>(c)];
        if (child.style.position == Position::k_fixed)
        {
            paint_box(doc, layout, c, painter, 0.0f, 0.0f);
        }
        else if (child.style.position == Position::k_absolute)
        {
            paint_box(doc, layout, c, painter, ox - box.scroll_x, oy - box.scroll_y);
        }
    }
    if (clip)
    {
        painter.pop_clip();
    }
}

}

void paint_tree(const Document& doc, const LayoutResult& layout, int box_idx, Painter& painter)
{
    if (box_idx < 0)
    {
        return;
    }
    paint_box(doc, layout, box_idx, painter, 0.0f, 0.0f);
}

}
