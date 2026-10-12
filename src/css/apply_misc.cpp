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

bool apply_misc(ComputedStyle& st, const std::string& prop,
                 const std::string& value, const ComputedStyle* parent,
                 float, float)
{
    Color current = parent ? parent->color : Color{0, 0, 0, 255};
    if (prop == "z-index")
    {
        if (value == "auto")
        {
            st.has_z_index = false;
            return true;
        }
        const char* b = value.c_str();
        st.z_index = static_cast<int>(detail::parse_float(b, b + value.size(), nullptr));
        st.has_z_index = true;
        return true;
    }
    if (prop == "box-shadow")
    {
        if (value == "none")
        {
            st.shadow.has = false;
            return true;
        }
        std::vector<std::string> bits = detail::split_ws(value);
        Shadow sh;
        int nums = 0;
        for (const std::string& bit : bits)
        {
            Length l;
            if (detail::parse_length_value(bit, l) && l.unit == Unit::k_px)
            {
                if (nums == 0) { sh.dx = l.value; }
                else if (nums == 1) { sh.dy = l.value; }
                else if (nums == 2) { sh.blur = l.value; }
                ++nums;
            }
            else
            {
                Color c;
                if (detail::parse_color_value(bit, current, c))
                {
                    sh.color = c;
                }
            }
        }
        if (nums >= 2)
        {
            sh.has = true;
            st.shadow = sh;
        }
        return true;
    }
    if (prop == "transform")
    {
        if (value == "none")
        {
            st.has_translate = false;
            return true;
        }
        std::size_t lp = value.find('(');
        std::size_t rp = value.rfind(')');
        if (lp == std::string::npos || rp == std::string::npos)
        {
            return true;
        }
        std::string name = detail::trim_copy(value.substr(0, lp));
        std::vector<std::string> args = detail::split_top(value.substr(lp + 1, rp - lp - 1), ',');
        Length a;
        Length b2;
        if (!args.empty())
        {
            detail::parse_length_value(detail::trim_copy(args[0]), a);
        }
        if (args.size() > 1)
        {
            detail::parse_length_value(detail::trim_copy(args[1]), b2);
        }
        if (name == "translate" || name == "translateX" || name == "translateY")
        {
            st.has_translate = true;
            st.translate_x = (a.unit == Unit::k_px) ? a.value : 0.0f;
            st.translate_y = (name == "translate" && b2.unit == Unit::k_px) ? b2.value : 0.0f;
            if (name == "translateY")
            {
                st.translate_y = st.translate_x;
                st.translate_x = 0.0f;
            }
        }
        return true;
    }
    if (prop == "content")
    {
        std::string v = detail::trim_copy(value);
        if (v == "none" || v == "normal")
        {
            st.has_content = false;
            return true;
        }
        if (v.size() >= 2 && ((v.front() == '"' && v.back() == '"') || (v.front() == '\'' && v.back() == '\'')))
        {
            st.content = v.substr(1, v.size() - 2);
            st.has_content = true;
        }
        return true;
    }
    if (prop == "pointer-events")
    {
        st.pointer_events_none = (value == "none");
        return true;
    }
    return false;
}

} // namespace detail
} // namespace wv
