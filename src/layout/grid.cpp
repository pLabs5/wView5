/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

void Engine::layout_grid(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle st = box.style;
    float avail = content_width(st, cb_w);
    box.w = box_width(st, cb_w);
    float ox = pad_left(st, cb_w);
    float oy = pad_top(st, cb_w);
    std::size_t ncols = st.grid_columns.size();
    if (ncols == 0)
    {
        layout_block(box_idx, cb_w, abs_cb);
        return;
    }
    std::vector<float> col_w(ncols, 0.0f);
    float fixed_sum = 0.0f;
    float fr_sum = 0.0f;
    for (std::size_t i = 0; i < ncols; ++i)
    {
        const Track& t = st.grid_columns[i];
        if (t.is_fr)
        {
            fr_sum += t.fr;
        }
        else
        {
            col_w[i] = resolve_len(t.fixed, avail, st.font_size, viewport_w, viewport_h);
            fixed_sum += col_w[i];
        }
    }
    float gaps = st.gap_col * static_cast<float>(ncols - 1);
    float fr_unit = (fr_sum > 0.0f) ? (avail - fixed_sum - gaps) / fr_sum : 0.0f;
    if (fr_unit < 0.0f)
    {
        fr_unit = 0.0f;
    }
    for (std::size_t i = 0; i < ncols; ++i)
    {
        if (st.grid_columns[i].is_fr)
        {
            col_w[i] = fr_unit * st.grid_columns[i].fr;
            if (st.grid_columns[i].has_min)
            {
                float mn = resolve_len(st.grid_columns[i].min, avail, st.font_size, viewport_w, viewport_h);
                if (col_w[i] < mn)
                {
                    col_w[i] = mn;
                }
            }
        }
    }
    float row_y = oy;
    float row_h = 0.0f;
    std::size_t col = 0;
    const Node& n = (box.node >= 0) ? doc->node(box.node) : doc->node(doc->root());
    if (box.node >= 0)
    {
        for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
        {
            const Node& cn = doc->node(c);
            if (cn.kind == NodeKind::k_text)
            {
                continue;
            }
            int cb2 = box_for_node(c);
            if (cb2 < 0)
            {
                continue;
            }
            Box& child = result->boxes[static_cast<std::size_t>(cb2)];
            if (child.style.display == Display::k_none)
            {
                continue;
            }
            if (child.style.position == Position::k_absolute || child.style.position == Position::k_fixed)
            {
                child.x = ox;
                child.y = row_y;
                layout_box(cb2, avail, abs_cb);
                continue;
            }
            if (col >= ncols)
            {
                row_y += row_h + st.gap_row;
                row_h = 0.0f;
                col = 0;
            }
            ComputedStyle adj = child.style;
            adj.width = Length::px(col_w[col]);
            child.style = adj;
            float cx = ox;
            for (std::size_t k = 0; k < col; ++k)
            {
                cx += col_w[k] + st.gap_col;
            }
            child.x = cx;
            child.y = row_y;
            layout_box(cb2, col_w[col], abs_cb);
            float ch = result->boxes[static_cast<std::size_t>(cb2)].h;
            if (ch > row_h)
            {
                row_h = ch;
            }
            ++col;
        }
    }
    float h = (row_y - oy) + row_h + pad_bottom(st, avail);
    if (!st.height.is_auto())
    {
        float fixed = resolve_len(st.height, cb_w, st.font_size, viewport_w, viewport_h);
        h = st.box_border ? fixed : fixed + pad_top(st, cb_w) + pad_bottom(st, avail);
    }
    box.h = h;
    measure_scroll_extents(*result, box_idx);
    apply_position(box_idx, cb_w, abs_cb);
}

} // namespace wv
