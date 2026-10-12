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

bool apply_paint(ComputedStyle& st, const std::string& prop,
                 const std::string& value, const ComputedStyle* parent,
                 float, float)
{
    Color current = parent ? parent->color : Color{0, 0, 0, 255};
    if (prop == "background" || prop == "background-color")
    {
        if (prop == "background-color")
        {
            Color c;
            if (detail::parse_color_value(value, current, c))
            {
                st.background.has_color = true;
                st.background.color = c;
            }
            return true;
        }
        std::vector<std::string> layers = detail::split_top(value, ',');
        for (const std::string& layer : layers)
        {
            Background tmp = st.background;
            if (detail::parse_gradient_layer(layer, current, tmp))
            {
                st.background = tmp;
                continue;
            }
            Color c;
            if (detail::parse_color_value(layer, current, c))
            {
                st.background.has_color = true;
                st.background.color = c;
            }
        }
        return true;
    }
    if (prop == "color")
    {
        Color c;
        if (detail::parse_color_value(value, current, c))
        {
            st.color = c;
        }
        return true;
    }
    if (prop == "opacity")
    {
        const char* b = value.c_str();
        float v = detail::parse_float(b, b + value.size(), nullptr);
        if (v < 0.0f) { v = 0.0f; }
        if (v > 1.0f) { v = 1.0f; }
        st.opacity = v;
        return true;
    }
    return false;
}

} // namespace detail
} // namespace wv
