/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

namespace
{
int span_attr(const Document& doc, int node, const char* name)
{
    std::string_view value = doc.attr(node, name);
    if (value.empty() || value.size() > 2)
    {
        return 1;
    }
    int span = 0;
    for (char c : value)
    {
        if (c < '0' || c > '9')
        {
            return 1;
        }
        span = span * 10 + (c - '0');
    }
    if (span < 1)
    {
        return 1;
    }
    if (span > 32)
    {
        return 32;
    }
    return span;
}
} // namespace

void Engine::layout_table(int box_idx, float cb_w, int abs_cb)
{
    Box& box = result->boxes[static_cast<std::size_t>(box_idx)];
    const ComputedStyle st = box.style;
    float avail = content_width(st, cb_w);
    box.w = box_width(st, cb_w);
    float ox = pad_left(st, cb_w);
    float y = pad_top(st, cb_w);
    const Node& n = (box.node >= 0) ? doc->node(box.node) : doc->node(doc->root());
    int caption = -1;
    std::vector<int> rows;
    if (box.node >= 0)
    {
        for (int c = n.first_child; c >= 0; c = doc->node(c).next_sibling)
        {
            int cb2 = box_for_node(c);
            if (cb2 < 0)
            {
                continue;
            }
            const Box& child = result->boxes[static_cast<std::size_t>(cb2)];
            if (child.style.display == Display::k_none)
            {
                continue;
            }
            if (child.style.position == Position::k_absolute ||
                child.style.position == Position::k_fixed)
            {
                Box& abs_child = result->boxes[static_cast<std::size_t>(cb2)];
                abs_child.x = ox;
                abs_child.y = y;
                layout_box(cb2, avail, abs_cb);
                continue;
            }
            if (child.style.display == Display::k_table_caption && caption < 0)
            {
                caption = cb2;
                continue;
            }
            if (child.style.display == Display::k_table_row)
            {
                rows.push_back(cb2);
                continue;
            }
            if (child.style.display == Display::k_table_row_group)
            {
                const Node& group = doc->node(child.node);
                for (int r = group.first_child; r >= 0; r = doc->node(r).next_sibling)
                {
                    int rb = box_for_node(r);
                    if (rb < 0)
                    {
                        continue;
                    }
                    const Box& row = result->boxes[static_cast<std::size_t>(rb)];
                    if (row.style.display == Display::k_table_row)
                    {
                        rows.push_back(rb);
                    }
                }
                continue;
            }
        }
    }
    if (caption >= 0)
    {
        Box& cap = result->boxes[static_cast<std::size_t>(caption)];
        cap.x = ox;
        cap.y = y;
        layout_box(caption, avail, abs_cb);
        y += result->boxes[static_cast<std::size_t>(caption)].h;
    }
    struct Cell
    {
        int box = -1;
        int col = 0;
        int colspan = 1;
        int rowspan = 1;
        float natural = 0.0f;
    };
    std::vector<std::vector<Cell>> grid;
    std::size_t ncols = 0;
    for (int row_box : rows)
    {
        const Node& row_node = doc->node(result->boxes[static_cast<std::size_t>(row_box)].node);
        std::vector<Cell> row_cells;
        std::size_t col = 0;
        auto occupied = [&](std::size_t r, std::size_t c) -> bool {
            if (r >= grid.size())
            {
                return false;
            }
            for (const auto& cell : grid[r])
            {
                if (c >= static_cast<std::size_t>(cell.col) &&
                    c < static_cast<std::size_t>(cell.col + cell.colspan))
                {
                    return true;
                }
            }
            return false;
        };
        std::size_t current_row = grid.size();
        for (int c = row_node.first_child; c >= 0; c = doc->node(c).next_sibling)
        {
            int cb2 = box_for_node(c);
            if (cb2 < 0)
            {
                continue;
            }
            const Box& child = result->boxes[static_cast<std::size_t>(cb2)];
            if (child.style.display != Display::k_table_cell)
            {
                continue;
            }
            while (occupied(current_row, col))
            {
                ++col;
            }
            Cell cell;
            cell.box = cb2;
            cell.col = static_cast<int>(col);
            cell.colspan = span_attr(*doc, c, "colspan");
            cell.rowspan = span_attr(*doc, c, "rowspan");
            {
                int tmp = alloc_box(-1, -1);
                result->boxes[static_cast<std::size_t>(tmp)].style = child.style;
                result->boxes[static_cast<std::size_t>(tmp)].node = child.node;
                e_layout_box_measure(tmp, k_inf, abs_cb);
                cell.natural = result->boxes[static_cast<std::size_t>(tmp)].w;
                result->boxes.pop_back();
            }
            row_cells.push_back(cell);
            col += static_cast<std::size_t>(cell.colspan);
        }
        if (col > ncols)
        {
            ncols = col;
        }
        grid.push_back(row_cells);
    }
    std::vector<float> col_nat(ncols, 0.0f);
    std::vector<float> col_fixed(ncols, -1.0f);
    std::vector<bool> col_has_fixed(ncols, false);
    for (const auto& row_cells : grid)
    {
        for (const auto& cell : row_cells)
        {
            const Box& child = result->boxes[static_cast<std::size_t>(cell.box)];
            float each = cell.natural / static_cast<float>(cell.colspan);
            for (int k = 0; k < cell.colspan; ++k)
            {
                std::size_t cc = static_cast<std::size_t>(cell.col + k);
                if (cc < ncols && each > col_nat[cc])
                {
                    col_nat[cc] = each;
                }
            }
            if (!child.style.width.is_auto() && cell.colspan == 1)
            {
                float fixed = detail::resolve_len(child.style.width, avail, child.style.font_size,
                                                  viewport_w, viewport_h);
                std::size_t cc = static_cast<std::size_t>(cell.col);
                if (cc < ncols && (!col_has_fixed[cc] || fixed > col_fixed[cc]))
                {
                    col_fixed[cc] = fixed;
                    col_has_fixed[cc] = true;
                }
            }
        }
    }
    std::vector<float> col_w(ncols, 0.0f);
    float fixed_sum = 0.0f;
    float nat_sum = 0.0f;
    for (std::size_t i = 0; i < ncols; ++i)
    {
        if (col_has_fixed[i])
        {
            col_w[i] = col_fixed[i];
            fixed_sum += col_w[i];
        }
        else
        {
            nat_sum += col_nat[i];
        }
    }
    float free_space = avail - fixed_sum;
    if (free_space < 0.0f)
    {
        free_space = 0.0f;
    }
    float scale = 1.0f;
    if (nat_sum > free_space && nat_sum > 0.0f)
    {
        scale = free_space / nat_sum;
    }
    float used = fixed_sum;
    for (std::size_t i = 0; i < ncols; ++i)
    {
        if (!col_has_fixed[i])
        {
            col_w[i] = col_nat[i] * scale;
            used += col_w[i];
        }
    }
    if (used < avail && ncols > 0)
    {
        float extra = (avail - used) / static_cast<float>(ncols);
        for (std::size_t i = 0; i < ncols; ++i)
        {
            col_w[i] += extra;
        }
    }
    float row_y = y;
    for (std::size_t r = 0; r < grid.size(); ++r)
    {
        float row_h = 0.0f;
        for (const auto& cell : grid[r])
        {
            float w = 0.0f;
            for (int k = 0; k < cell.colspan; ++k)
            {
                w += col_w[static_cast<std::size_t>(cell.col + k)];
            }
            Box& child = result->boxes[static_cast<std::size_t>(cell.box)];
            ComputedStyle adj = child.style;
            adj.width = Length::px(w);
            child.style = adj;
            float cx = ox;
            for (int k = 0; k < cell.col; ++k)
            {
                cx += col_w[static_cast<std::size_t>(k)];
            }
            child.x = cx;
            child.y = row_y;
            layout_box(cell.box, w, abs_cb);
            float ch = result->boxes[static_cast<std::size_t>(cell.box)].h;
            if (ch > row_h)
            {
                row_h = ch;
            }
        }
        int row_box = rows[r];
        Box& row = result->boxes[static_cast<std::size_t>(row_box)];
        row.x = ox;
        row.y = row_y;
        row.w = avail;
        row.h = row_h;
        row_y += row_h;
    }
    float h = (row_y - pad_top(st, cb_w)) + pad_bottom(st, avail);
    if (!st.height.is_auto())
    {
        float fixed = detail::resolve_len(st.height, cb_w, st.font_size, viewport_w, viewport_h);
        h = st.box_border ? fixed : fixed + pad_top(st, cb_w) + pad_bottom(st, avail);
    }
    box.h = h;
    measure_scroll_extents(*result, box_idx);
    apply_position(box_idx, cb_w, abs_cb);
}

} // namespace wv
