/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

void Engine::layout_flex(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle st = box.style;
    bool column = (st.flex_direction == FlexDirection::k_column);
    float avail = content_width(st, cb_w);
    float gap = column ? st.gap_row : st.gap_col;
    float ox = pad_left(st, cb_w);
    float oy = pad_top(st, cb_w);
    box.w = box_width(st, cb_w);
    if (st.display == Display::k_inline_flex)
    {
        box.w = 0.0f;
    }
    struct Item
    {
        int box = -1;
        float base = 0.0f;
        float grow = 0.0f;
        float shrink = 1.0f;
    };
    std::vector<Item> items;
    std::vector<int> abs_items;
    const Node& n = (box.node >= 0) ? doc->node(box.node) : doc->node(doc->root());
    if (box.node >= 0)
    {
        for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
        {
            const Node& cn = doc->node(c);
            if (cn.kind == NodeKind::k_text)
            {
                bool spaces = true;
                for (char ch : cn.text)
                {
                    if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r')
                    {
                        spaces = false;
                    }
                }
                if (spaces)
                {
                    continue;
                }
                int ab = alloc_box(c, box_idx);
                ComputedStyle ast = st;
                ast.display = Display::k_inline_block;
                result->boxes[static_cast<std::size_t>(ab)].style = ast;
                Item it;
                it.box = ab;
                items.push_back(it);
                continue;
            }
            int cb2 = box_for_node(c);
            if (cb2 < 0)
            {
                continue;
            }
            const Box& child = result->boxes[static_cast<std::size_t>(cb2)];
            if (child.style.position == Position::k_absolute || child.style.position == Position::k_fixed)
            {
                abs_items.push_back(cb2);
                continue;
            }
            Item it;
            it.box = cb2;
            it.grow = child.style.flex_grow;
            it.shrink = child.style.flex_shrink;
            items.push_back(it);
        }
    }
    for (auto& it : items)
    {
        Box& child = result->boxes[static_cast<std::size_t>(it.box)];
        if (!child.style.flex_basis.is_auto())
        {
            it.base = resolve_len(child.style.flex_basis, avail, child.style.font_size, viewport_w, viewport_h);
        }
        else if (column ? !child.style.height.is_auto() : !child.style.width.is_auto())
        {
            it.base = resolve_len(column ? child.style.height : child.style.width, avail, child.style.font_size, viewport_w, viewport_h);
        }
        else
        {
            int tmp = alloc_box(-1, -1);
            result->boxes[static_cast<std::size_t>(tmp)].style = child.style;
            result->boxes[static_cast<std::size_t>(tmp)].node = child.node;
            if (column)
            {
                e_layout_box_measure(tmp, avail, abs_cb);
                it.base = result->boxes[static_cast<std::size_t>(tmp)].h;
            }
            else
            {
                e_layout_box_measure(tmp, k_inf, abs_cb);
                it.base = result->boxes[static_cast<std::size_t>(tmp)].w;
            }
            result->boxes.pop_back();
        }
    }
    float total_base = 0.0f;
    for (const auto& it : items)
    {
        total_base += it.base;
    }
    if (!items.empty())
    {
        total_base += gap * static_cast<float>(items.size() - 1);
    }
    float free_space = column ? 0.0f : avail - total_base;
    float total_grow = 0.0f;
    float total_shrink = 0.0f;
    for (const auto& it : items)
    {
        total_grow += it.grow;
        total_shrink += it.shrink * it.base;
    }
    float cursor = 0.0f;
    float cross_max = 0.0f;
    for (auto& it : items)
    {
        Box& child = result->boxes[static_cast<std::size_t>(it.box)];
        float main_size = it.base;
        if (!column)
        {
            if (free_space > 0.0f && total_grow > 0.0f)
            {
                main_size += free_space * it.grow / total_grow;
            }
            else if (free_space < 0.0f && total_shrink > 0.0f)
            {
                main_size += free_space * (it.shrink * it.base) / total_shrink;
            }
            if (main_size < 0.0f)
            {
                main_size = 0.0f;
            }
            ComputedStyle adj = child.style;
            adj.width = Length::px(main_size);
            adj.flex_basis = Length::auto_();
            child.style = adj;
            child.x = ox + cursor;
            child.y = oy;
            layout_box(it.box, avail, abs_cb);
            cursor += main_size + gap;
            if (child.h > cross_max)
            {
                cross_max = child.h;
            }
        }
        else
        {
            child.x = ox;
            child.y = oy + cursor;
            layout_box(it.box, avail, abs_cb);
            cursor += result->boxes[static_cast<std::size_t>(it.box)].h + gap;
            if (child.w > cross_max)
            {
                cross_max = child.w;
            }
        }
    }
    if (!items.empty())
    {
        cursor -= gap;
    }
    for (int ab : abs_items)
    {
        Box& child = result->boxes[static_cast<std::size_t>(ab)];
        child.x = ox;
        child.y = oy;
        layout_box(ab, avail, abs_cb);
    }
    float content_h = column ? cursor : cross_max;
    if (!st.height.is_auto())
    {
        float fixed = resolve_len(st.height, cb_w, st.font_size, viewport_w, viewport_h);
        content_h = st.box_border ? fixed - pad_top(st, cb_w) - pad_bottom(st, avail) : fixed;
    }
    if (!column)
    {
        for (auto& it : items)
        {
            Box& child = result->boxes[static_cast<std::size_t>(it.box)];
            Align al = (child.style.align_self != Align::k_auto) ? child.style.align_self : st.align_items;
            if (al == Align::k_center)
            {
                child.y = oy + (content_h - child.h) / 2.0f;
            }
            else if (al == Align::k_flex_end)
            {
                child.y = oy + (content_h - child.h);
            }
            if (al == Align::k_stretch && child.style.height.is_auto())
            {
                child.h = content_h;
            }
        }
        float used_main = cursor;
        float joff = 0.0f;
        if (st.justify_content == Align::k_center)
        {
            joff = (avail - used_main) / 2.0f;
        }
        else if (st.justify_content == Align::k_flex_end)
        {
            joff = avail - used_main;
        }
        else if ((st.justify_content == Align::k_space_between || st.justify_content == Align::k_space_around) && items.size() > 1)
        {
            float extra = avail - used_main;
            if (extra > 0.0f)
            {
                float step = (st.justify_content == Align::k_space_between)
                    ? extra / static_cast<float>(items.size() - 1)
                    : extra / static_cast<float>(items.size());
                float shift = (st.justify_content == Align::k_space_around) ? step / 2.0f : 0.0f;
                for (auto& it : items)
                {
                    result->boxes[static_cast<std::size_t>(it.box)].x += shift;
                    shift += step;
                }
            }
        }
        if (joff != 0.0f)
        {
            for (auto& it : items)
            {
                result->boxes[static_cast<std::size_t>(it.box)].x += joff;
            }
        }
    }
    if (st.display == Display::k_inline_flex)
    {
        box.w = cursor + pad_x(st, cb_w);
    }
    box.h = content_h + pad_top(st, cb_w) + pad_bottom(st, avail);
    measure_scroll_extents(*result, box_idx);
    apply_position(box_idx, cb_w, abs_cb);
}

void Engine::e_layout_box_measure(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle keep = box.style;
    ComputedStyle probe = keep;
    if (cb_w >= k_inf / 2.0f)
    {
        probe.width = Length::auto_();
    }
    box.style = probe;
    layout_box(box_idx, cb_w, abs_cb);
}

} // namespace wv
