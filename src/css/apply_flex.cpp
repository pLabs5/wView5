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

bool apply_flex(ComputedStyle& st, const std::string& prop,
                 const std::string& value, const ComputedStyle*,
                 float viewport_w, float)
{
    if (prop == "flex-direction")
    {
        st.flex_direction = (value == "column" || value == "column-reverse") ? FlexDirection::k_column : FlexDirection::k_row;
        return true;
    }
    if (prop == "flex-wrap")
    {
        st.flex_wrap = (value == "wrap" || value == "wrap-reverse");
        return true;
    }
    if (prop == "flex")
    {
        if (value == "none")
        {
            st.flex_grow = 0.0f;
            st.flex_shrink = 0.0f;
            st.flex_basis = Length::auto_();
            return true;
        }
        std::vector<std::string> bits = detail::split_ws(value);
        const char* b0 = bits.empty() ? "" : bits[0].c_str();
        const char* e0 = b0 + (bits.empty() ? 0 : bits[0].size());
        const char* pe = b0;
        float g = detail::parse_float(b0, e0, &pe);
        if (pe > b0 && pe == e0)
        {
            st.flex_grow = g;
            st.flex_shrink = 1.0f;
            st.flex_basis = Length::px(0.0f);
        }
        if (bits.size() > 1)
        {
            const char* b1 = bits[1].c_str();
            const char* e1 = b1 + bits[1].size();
            float s2 = detail::parse_float(b1, e1, &pe);
            if (pe > b1 && pe == e1)
            {
                st.flex_shrink = s2;
            }
            else
            {
                Length bl;
                if (detail::parse_length_value(bits[1], bl))
                {
                    st.flex_basis = bl;
                }
            }
        }
        if (bits.size() > 2)
        {
            Length bl;
            if (detail::parse_length_value(bits[2], bl))
            {
                st.flex_basis = bl;
            }
        }
        return true;
    }
    if (prop == "flex-grow" || prop == "flex-shrink")
    {
        const char* b = value.c_str();
        float v = detail::parse_float(b, b + value.size(), nullptr);
        if (prop == "flex-grow") { st.flex_grow = v; }
        else { st.flex_shrink = v; st.flex_shrink_none = (v == 0.0f); }
        return true;
    }
    if (prop == "flex-basis")
    {
        Length l;
        if (detail::parse_length_value(value, l))
        {
            st.flex_basis = l;
        }
        return true;
    }
    if (prop == "gap" || prop == "row-gap" || prop == "column-gap")
    {
        std::vector<std::string> bits = detail::split_ws(value);
        Length a;
        Length b2;
        if (!bits.empty())
        {
            detail::parse_length_value(bits[0], a);
        }
        if (bits.size() > 1)
        {
            detail::parse_length_value(bits[1], b2);
        }
        else
        {
            b2 = a;
        }
        float fa = detail::resolve_px(a, st.font_size, 16.0f, viewport_w, viewport_w, 0.0f);
        float fb = detail::resolve_px(b2, st.font_size, 16.0f, viewport_w, viewport_w, 0.0f);
        if (prop == "gap")
        {
            st.gap_row = fa;
            st.gap_col = fb;
        }
        else if (prop == "row-gap")
        {
            st.gap_row = fa;
        }
        else
        {
            st.gap_col = fb;
        }
        return true;
    }
    if (prop == "grid-template-columns")
    {
        st.grid_columns.clear();
        std::string v = value;
        std::size_t rp = v.find("repeat(");
        while (rp != std::string::npos)
        {
            std::size_t lp2 = rp + 7;
            std::size_t comma = v.find(',', lp2);
            std::size_t close2 = v.find(')', comma);
            if (comma == std::string::npos || close2 == std::string::npos)
            {
                break;
            }
            std::string ns = detail::trim_copy(v.substr(lp2, comma - lp2));
            const char* nb = ns.c_str();
            int n = static_cast<int>(detail::parse_float(nb, nb + ns.size(), nullptr));
            std::string inner = detail::trim_copy(v.substr(comma + 1, close2 - comma - 1));
            std::string rep;
            for (int i = 0; i < n && i < 64; ++i)
            {
                rep += inner + " ";
            }
            v = v.substr(0, rp) + rep + v.substr(close2 + 1);
            rp = v.find("repeat(");
        }
        for (const std::string& tok : detail::split_ws(v))
        {
            Track t;
            if (tok.rfind("minmax(", 0) == 0 && tok.back() == ')')
            {
                std::string inner = tok.substr(7, tok.size() - 8);
                std::vector<std::string> mm = detail::split_top(inner, ',');
                if (mm.size() == 2)
                {
                    Length mn;
                    Length mx;
                    if (detail::parse_length_value(detail::trim_copy(mm[0]), mn))
                    {
                        t.min = mn;
                        t.has_min = true;
                    }
                    if (detail::parse_length_value(detail::trim_copy(mm[1]), mx))
                    {
                        if (mx.unit == Unit::k_fr)
                        {
                            t.is_fr = true;
                            t.fr = mx.value;
                        }
                        else
                        {
                            t.is_fr = false;
                            t.fixed = mx;
                        }
                    }
                    st.grid_columns.push_back(t);
                    continue;
                }
            }
            Length l;
            if (detail::parse_length_value(tok, l))
            {
                if (l.unit == Unit::k_fr)
                {
                    t.is_fr = true;
                    t.fr = l.value;
                }
                else
                {
                    t.is_fr = false;
                    t.fixed = l;
                }
                st.grid_columns.push_back(t);
            }
        }
        return true;
    }
    if (prop == "justify-content" || prop == "align-items" || prop == "align-content" || prop == "align-self" || prop == "place-items")
    {
        Align a = Align::k_flex_start;
        std::string v = detail::trim_copy(detail::split_ws(value).empty() ? value : detail::split_ws(value)[0]);
        if (v == "flex-end" || v == "end") { a = Align::k_flex_end; }
        else if (v == "center") { a = Align::k_center; }
        else if (v == "stretch") { a = Align::k_stretch; }
        else if (v == "baseline") { a = Align::k_baseline; }
        else if (v == "space-between") { a = Align::k_space_between; }
        else if (v == "space-around") { a = Align::k_space_around; }
        else if (v == "space-evenly") { a = Align::k_space_evenly; }
        if (prop == "justify-content") { st.justify_content = a; }
        else if (prop == "align-content") { st.align_content = a; }
        else if (prop == "align-self") { st.align_self = a; }
        else { st.align_items = a; }
        return true;
    }
    return false;
}

} // namespace detail
} // namespace wv
