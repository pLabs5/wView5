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

bool parse_gradient_layer(std::string_view s, const Color& current, Background& bg)
{
    std::size_t lp = s.find('(');
    if (lp == std::string_view::npos)
    {
        return false;
    }
    std::string name = trim_copy(s.substr(0, lp));
    if (name != "linear-gradient" && name != "repeating-linear-gradient")
    {
        return false;
    }
    std::size_t rp = s.rfind(')');
    if (rp == std::string_view::npos)
    {
        return false;
    }
    std::vector<std::string> parts = split_top(s.substr(lp + 1, rp - lp - 1), ',');
    if (parts.empty())
    {
        return false;
    }
    float angle = 180.0f;
    std::size_t first = 0;
    std::string p0 = parts[0];
    if (p0 == "to bottom") { angle = 180.0f; first = 1; }
    else if (p0 == "to top") { angle = 0.0f; first = 1; }
    else if (p0 == "to right") { angle = 90.0f; first = 1; }
    else if (p0 == "to left") { angle = 270.0f; first = 1; }
    else if (p0.size() > 3 && p0.substr(p0.size() - 3) == "deg")
    {
        const char* b = p0.c_str();
        angle = parse_float(b, b + p0.size(), nullptr);
        first = 1;
    }
    bg.has_gradient = true;
    bg.angle_deg = angle;
    bg.stops.clear();
    for (std::size_t i = first; i < parts.size(); ++i)
    {
        std::vector<std::string> bits = split_ws(parts[i]);
        if (bits.empty())
        {
            continue;
        }
        GradientStop stop;
        std::string cstr = bits[0];
        for (std::size_t k = 1; k < bits.size(); ++k)
        {
            if (bits[k].find('%') == std::string::npos && bits[k].find("px") == std::string::npos)
            {
                cstr += " " + bits[k];
            }
        }
        if (!parse_color_value(cstr, current, stop.color))
        {
            continue;
        }
        for (std::size_t k = 1; k < bits.size(); ++k)
        {
            if (!bits[k].empty() && bits[k].back() == '%')
            {
                const char* b = bits[k].c_str();
                stop.offset = parse_float(b, b + bits[k].size() - 1, nullptr) / 100.0f;
                stop.has_offset = true;
            }
        }
        bg.stops.push_back(stop);
    }
    return !bg.stops.empty();
}

} // namespace detail
} // namespace wv
