/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "wview/layout.hpp"

namespace wv
{

int find_box_for_node(const LayoutResult& layout, int node)
{
    if (node < 0)
    {
        return -1;
    }
    for (std::size_t i = 0; i < layout.boxes.size(); ++i)
    {
        if (layout.boxes[i].node == node)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void measure_scroll_extents(LayoutResult& layout, int box_idx)
{
    Box& box = layout.boxes[static_cast<std::size_t>(box_idx)];
    float max_right = 0.0f;
    float max_bottom = 0.0f;
    for (const auto& line : box.lines)
    {
        for (const auto& run : line.runs)
        {
            float right = run.x + run.width - box.x;
            if (right > max_right)
            {
                max_right = right;
            }
        }
        float bottom = line.y + line.height - box.y;
        if (bottom > max_bottom)
        {
            max_bottom = bottom;
        }
    }
    for (int c = box.first_child; c >= 0;
         c = layout.boxes[static_cast<std::size_t>(c)].next_sibling)
    {
        const Box& child = layout.boxes[static_cast<std::size_t>(c)];
        float right = child.x + child.w - box.x;
        float bottom = child.y + child.h - box.y;
        if (right > max_right)
        {
            max_right = right;
        }
        if (bottom > max_bottom)
        {
            max_bottom = bottom;
        }
    }
    box.scroll_w = max_right > 0.0f ? max_right : 0.0f;
    box.scroll_h = max_bottom > 0.0f ? max_bottom : 0.0f;
}

bool set_scroll(LayoutResult& layout, int box_idx, float x, float y)
{
    if (box_idx < 0 || static_cast<std::size_t>(box_idx) >= layout.boxes.size())
    {
        return false;
    }
    Box& box = layout.boxes[static_cast<std::size_t>(box_idx)];
    bool scroll_x = (box.style.overflow_x == Overflow::k_scroll ||
                     box.style.overflow_x == Overflow::k_auto);
    bool scroll_y = (box.style.overflow_y == Overflow::k_scroll ||
                     box.style.overflow_y == Overflow::k_auto);
    float want_x = x;
    float want_y = y;
    if (!scroll_x)
    {
        want_x = 0.0f;
    }
    else
    {
        float max_x = box.scroll_w - box.w;
        if (max_x < 0.0f)
        {
            max_x = 0.0f;
        }
        if (want_x < 0.0f)
        {
            want_x = 0.0f;
        }
        if (want_x > max_x)
        {
            want_x = max_x;
        }
    }
    if (!scroll_y)
    {
        want_y = 0.0f;
    }
    else
    {
        float max_y = box.scroll_h - box.h;
        if (max_y < 0.0f)
        {
            max_y = 0.0f;
        }
        if (want_y < 0.0f)
        {
            want_y = 0.0f;
        }
        if (want_y > max_y)
        {
            want_y = max_y;
        }
    }
    if (box.scroll_x == want_x && box.scroll_y == want_y)
    {
        return false;
    }
    box.scroll_x = want_x;
    box.scroll_y = want_y;
    return true;
}

bool ensure_visible(LayoutResult& layout, int box_idx)
{
    if (box_idx < 0 || static_cast<std::size_t>(box_idx) >= layout.boxes.size())
    {
        return false;
    }
    bool changed = false;
    const Box& target = layout.boxes[static_cast<std::size_t>(box_idx)];
    float top = target.y;
    float bottom = target.y + target.h;
    float left = target.x;
    float right = target.x + target.w;
    for (int p = target.parent; p >= 0;
         p = layout.boxes[static_cast<std::size_t>(p)].parent)
    {
        Box& ancestor = layout.boxes[static_cast<std::size_t>(p)];
        bool scroll_x = (ancestor.style.overflow_x == Overflow::k_scroll ||
                         ancestor.style.overflow_x == Overflow::k_auto);
        bool scroll_y = (ancestor.style.overflow_y == Overflow::k_scroll ||
                         ancestor.style.overflow_y == Overflow::k_auto);
        if (!scroll_x && !scroll_y)
        {
            continue;
        }
        if (scroll_y)
        {
            float view_top = ancestor.y + ancestor.scroll_y;
            float view_bottom = view_top + ancestor.h;
            if (top < view_top)
            {
                if (set_scroll(layout, p, ancestor.scroll_x, top - ancestor.y))
                {
                    changed = true;
                }
            }
            else if (bottom > view_bottom)
            {
                if (set_scroll(layout, p, ancestor.scroll_x, bottom - ancestor.y - ancestor.h))
                {
                    changed = true;
                }
            }
        }
        if (scroll_x)
        {
            float view_left = ancestor.x + ancestor.scroll_x;
            float view_right = view_left + ancestor.w;
            if (left < view_left)
            {
                if (set_scroll(layout, p, left - ancestor.x, ancestor.scroll_y))
                {
                    changed = true;
                }
            }
            else if (right > view_right)
            {
                if (set_scroll(layout, p, right - ancestor.x - ancestor.w, ancestor.scroll_y))
                {
                    changed = true;
                }
            }
        }
    }
    return changed;
}

} // namespace wv
