/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "css/internal.hpp"

#include "wview/css.hpp"

namespace wv
{
namespace detail
{

bool apply_box(ComputedStyle& st, const std::string& prop,
                 const std::string& value, const ComputedStyle* parent,
                 float, float)
{
    Color current = parent ? parent->color : Color{0, 0, 0, 255};
    if (prop == "display")
    {
        if (value == "none") { st.display = Display::k_none; }
        else if (value == "block") { st.display = Display::k_block; }
        else if (value == "flex") { st.display = Display::k_flex; }
        else if (value == "inline-flex") { st.display = Display::k_inline_flex; }
        else if (value == "grid") { st.display = Display::k_grid; }
        else if (value == "table") { st.display = Display::k_table; }
        else if (value == "table-row-group" || value == "table-header-group" ||
                 value == "table-footer-group") { st.display = Display::k_table_row_group; }
        else if (value == "table-row") { st.display = Display::k_table_row; }
        else if (value == "table-cell") { st.display = Display::k_table_cell; }
        else if (value == "table-caption") { st.display = Display::k_table_caption; }
        else if (value == "inline-block") { st.display = Display::k_inline_block; }
        else if (value == "inline") { st.display = Display::k_inline; }
        return true;
    }
    if (prop == "position")
    {
        if (value == "absolute") { st.position = Position::k_absolute; }
        else if (value == "relative" || value == "sticky") { st.position = Position::k_relative; }
        else if (value == "fixed") { st.position = Position::k_fixed; }
        else { st.position = Position::k_static; }
        return true;
    }
    if (prop == "top" || prop == "right" || prop == "bottom" || prop == "left")
    {
        Length l;
        if (detail::parse_length_value(value, l))
        {
            if (prop == "top") { st.top = l; }
            else if (prop == "right") { st.right = l; }
            else if (prop == "bottom") { st.bottom = l; }
            else { st.left = l; }
        }
        return true;
    }
    if (prop == "width" || prop == "height" || prop == "min-width" || prop == "min-height" || prop == "max-width" || prop == "max-height")
    {
        Length l;
        if (detail::parse_length_value(value, l))
        {
            if (prop == "width") { st.width = l; }
            else if (prop == "height") { st.height = l; }
            else if (prop == "min-width") { st.min_width = l; }
            else if (prop == "min-height") { st.min_height = l; }
            else if (prop == "max-width") { st.max_width = l; }
            else { st.max_height = l; }
        }
        return true;
    }
    if (prop == "margin" || prop == "padding")
    {
        Length tmp[4] = {st.margin[0], st.margin[1], st.margin[2], st.margin[3]};
        if (prop == "padding")
        {
            tmp[0] = st.padding[0];
            tmp[1] = st.padding[1];
            tmp[2] = st.padding[2];
            tmp[3] = st.padding[3];
        }
        detail::apply_box_sides(detail::split_ws(value), tmp);
        for (int i = 0; i < 4; ++i)
        {
            if (prop == "margin") { st.margin[i] = tmp[i]; }
            else { st.padding[i] = tmp[i]; }
        }
        return true;
    }
    if (prop.rfind("margin-", 0) == 0 || prop.rfind("padding-", 0) == 0)
    {
        bool is_margin = (prop[0] == 'm');
        Length l;
        if (detail::parse_length_value(value, l))
        {
            std::string naming = prop.substr(is_margin ? 7 : 8);
            int idx = -1;
            if (naming == "top") { idx = 0; }
            else if (naming == "right") { idx = 1; }
            else if (naming == "bottom") { idx = 2; }
            else if (naming == "left") { idx = 3; }
            if (idx >= 0)
            {
                if (is_margin) { st.margin[idx] = l; }
                else { st.padding[idx] = l; }
            }
        }
        return true;
    }
    if (prop == "border" || prop == "border-top" || prop == "border-right" || prop == "border-bottom" || prop == "border-left")
    {
        int first = 0;
        int last = 3;
        if (prop == "border-top") { first = last = 0; }
        else if (prop == "border-right") { first = last = 1; }
        else if (prop == "border-bottom") { first = last = 2; }
        else if (prop == "border-left") { first = last = 3; }
        std::vector<std::string> bits = detail::split_ws(value);
        for (const auto& b : bits)
        {
            Length l;
            if (detail::parse_length_value(b, l))
            {
                for (int i = first; i <= last; ++i)
                {
                    st.border_width[i] = l;
                }
            }
            else
            {
                Color c;
                if (detail::parse_color_value(b, current, c))
                {
                    st.border_color = c;
                }
            }
        }
        return true;
    }
    if (prop.rfind("border-", 0) == 0 && prop.size() > 7)
    {
        std::string rest = prop.substr(7);
        int idx = -1;
        std::string tail;
        if (rest.rfind("top-", 0) == 0) { idx = 0; tail = rest.substr(4); }
        else if (rest.rfind("right-", 0) == 0) { idx = 1; tail = rest.substr(6); }
        else if (rest.rfind("bottom-", 0) == 0) { idx = 2; tail = rest.substr(7); }
        else if (rest.rfind("left-", 0) == 0) { idx = 3; tail = rest.substr(5); }
        else { tail = rest; }
        if (tail == "width" || tail == "color" || tail == "radius")
        {
            if (tail == "width")
            {
                Length l;
                if (detail::parse_length_value(value, l))
                {
                    if (idx < 0)
                    {
                        for (int i = 0; i < 4; ++i) { st.border_width[i] = l; }
                    }
                    else { st.border_width[idx] = l; }
                }
            }
            else if (tail == "color")
            {
                Color c;
                if (detail::parse_color_value(value, current, c))
                {
                    st.border_color = c;
                }
            }
            else
            {
                std::string v = value;
                std::size_t slash = v.find('/');
                if (slash != std::string::npos)
                {
                    v = v.substr(0, slash);
                }
                Length tmp[4] = {Length::px(st.radius[0]), Length::px(st.radius[1]), Length::px(st.radius[2]), Length::px(st.radius[3])};
                detail::apply_box_sides(detail::split_ws(v), tmp);
                for (int i = 0; i < 4; ++i)
                {
                    if (!tmp[i].is_auto())
                    {
                        st.radius[i] = tmp[i].value;
                    }
                }
            }
            return true;
        }
        if (rest == "radius")
        {
            std::string v = value;
            std::size_t slash = v.find('/');
            if (slash != std::string::npos)
            {
                v = v.substr(0, slash);
            }
            Length tmp[4] = {};
            detail::apply_box_sides(detail::split_ws(v), tmp);
            for (int i = 0; i < 4; ++i)
            {
                if (!tmp[i].is_auto())
                {
                    st.radius[i] = tmp[i].value;
                }
            }
            return true;
        }
        return true;
    }
    if (prop == "border-radius")
    {
        std::string v = value;
        std::size_t slash = v.find('/');
        if (slash != std::string::npos)
        {
            v = v.substr(0, slash);
        }
        Length tmp[4] = {};
        detail::apply_box_sides(detail::split_ws(v), tmp);
        for (int i = 0; i < 4; ++i)
        {
            if (!tmp[i].is_auto())
            {
                st.radius[i] = tmp[i].value;
            }
        }
        return true;
    }
    if (prop == "overflow" || prop == "overflow-x" || prop == "overflow-y")
    {
        Overflow o = Overflow::k_visible;
        if (value == "hidden") { o = Overflow::k_hidden; }
        else if (value == "scroll") { o = Overflow::k_scroll; }
        else if (value == "auto") { o = Overflow::k_auto; }
        if (prop == "overflow" || prop == "overflow-x") { st.overflow_x = o; }
        if (prop == "overflow" || prop == "overflow-y") { st.overflow_y = o; }
        return true;
    }
    if (prop == "box-sizing")
    {
        st.box_border = (value != "content-box");
        return true;
    }
    return false;
}

} // namespace detail
} // namespace wv
