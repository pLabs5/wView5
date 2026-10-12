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

bool apply_text(ComputedStyle& st, const std::string& prop,
                 const std::string& value, const ComputedStyle* parent,
                 float viewport_w, float viewport_h)
{
    if (prop == "font-family")
    {
        std::vector<std::string> parts = detail::split_top(value, ',');
        if (!parts.empty())
        {
            std::string f = detail::trim_copy(parts[0]);
            if (f.size() >= 2 && ((f.front() == '"' && f.back() == '"') || (f.front() == '\'' && f.back() == '\'')))
            {
                f = f.substr(1, f.size() - 2);
            }
            st.font_family = f;
        }
        return true;
    }
    if (prop == "font-size")
    {
        Length l;
        if (detail::parse_length_value(value, l))
        {
            float base = parent ? parent->font_size : 16.0f;
            if (l.unit == Unit::k_px)
            {
                st.font_size = l.value;
            }
            else if (l.unit == Unit::k_percent || l.unit == Unit::k_em)
            {
                st.font_size = (l.unit == Unit::k_em) ? base * l.value : base * l.value / 100.0f;
            }
            else if (l.unit == Unit::k_rem)
            {
                st.font_size = 16.0f * l.value;
            }
            else if (l.unit == Unit::k_vw)
            {
                st.font_size = l.value / 100.0f * viewport_w;
            }
            else if (l.unit == Unit::k_vh)
            {
                st.font_size = l.value / 100.0f * viewport_h;
            }
        }
        return true;
    }
    if (prop == "font-weight")
    {
        if (value == "bold") { st.font_weight = 700; }
        else if (value == "normal") { st.font_weight = 400; }
        else
        {
            const char* b = value.c_str();
            float v = detail::parse_float(b, b + value.size(), nullptr);
            if (v >= 100.0f)
            {
                st.font_weight = static_cast<int>(v);
            }
        }
        return true;
    }
    if (prop == "font-style")
    {
        st.font_italic = (value == "italic" || value == "oblique");
        return true;
    }
    if (prop == "line-height")
    {
        if (value == "normal")
        {
            st.line_height_normal = true;
            return true;
        }
        Length l;
        if (detail::parse_length_value(value, l))
        {
            st.line_height_normal = false;
            if (l.unit == Unit::k_px)
            {
                st.line_height = l.value;
            }
            else
            {
                st.line_height = l.value * st.font_size;
            }
        }
        else
        {
            const char* b = value.c_str();
            const char* e = b + value.size();
            const char* pe = b;
            float v = detail::parse_float(b, e, &pe);
            if (pe > b && pe == e)
            {
                st.line_height_normal = false;
                st.line_height = v * st.font_size;
            }
        }
        return true;
    }
    if (prop == "letter-spacing")
    {
        Length l;
        if (detail::parse_length_value(value, l) && l.unit == Unit::k_px)
        {
            st.letter_spacing = l.value;
        }
        return true;
    }
    if (prop == "text-align")
    {
        if (value == "center") { st.text_align = 1; }
        else if (value == "right") { st.text_align = 2; }
        else if (value == "justify") { st.text_align = 3; }
        else { st.text_align = 0; }
        return true;
    }
    if (prop == "text-transform")
    {
        if (value == "uppercase") { st.text_transform = 1; }
        else if (value == "lowercase") { st.text_transform = 2; }
        else if (value == "capitalize") { st.text_transform = 3; }
        else { st.text_transform = 0; }
        return true;
    }
    if (prop == "text-decoration")
    {
        st.underline = (value.find("underline") != std::string::npos);
        return true;
    }
    if (prop == "white-space")
    {
        if (value == "nowrap") { st.white_space = 1; }
        else if (value == "pre") { st.white_space = 2; }
        else if (value == "pre-wrap") { st.white_space = 3; }
        else if (value == "pre-line") { st.white_space = 4; }
        else { st.white_space = 0; }
        return true;
    }
    if (prop == "text-overflow")
    {
        st.ellipsis = (value == "ellipsis");
        return true;
    }
    return false;
}

} // namespace detail
} // namespace wv
