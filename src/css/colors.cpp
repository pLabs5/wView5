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

int hex_val(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

bool parse_hex_color(std::string_view s, Color& out)
{
    if (s.empty() || s[0] != '#')
    {
        return false;
    }
    s.remove_prefix(1);
    auto dig = [&](std::size_t i) -> int
    {
        return (i < s.size()) ? hex_val(s[i]) : -1;
    };
    if (s.size() == 3 || s.size() == 4)
    {
        int r = dig(0);
        int g = dig(1);
        int b = dig(2);
        int a = (s.size() == 4) ? dig(3) : 15;
        if (r < 0 || g < 0 || b < 0 || a < 0)
        {
            return false;
        }
        out = Color{
            static_cast<std::uint8_t>(r * 17),
            static_cast<std::uint8_t>(g * 17),
            static_cast<std::uint8_t>(b * 17),
            static_cast<std::uint8_t>(a * 17)};
        return true;
    }
    if (s.size() == 6 || s.size() == 8)
    {
        int v[8];
        for (std::size_t i = 0; i < s.size(); ++i)
        {
            v[i] = dig(i);
            if (v[i] < 0)
            {
                return false;
            }
        }
        out = Color{
            static_cast<std::uint8_t>(v[0] * 16 + v[1]),
            static_cast<std::uint8_t>(v[2] * 16 + v[3]),
            static_cast<std::uint8_t>(v[4] * 16 + v[5]),
            static_cast<std::uint8_t>((s.size() == 8) ? (v[6] * 16 + v[7]) : 255)};
        return true;
    }
    return false;
}

float clamp01(float v)
{
    if (v < 0.0f)
    {
        return 0.0f;
    }
    if (v > 1.0f)
    {
        return 1.0f;
    }
    return v;
}

bool parse_component(std::string_view s, float maxv, std::uint8_t& out)
{
    std::string t = trim_copy(s);
    if (t.empty())
    {
        return false;
    }
    const char* b = t.c_str();
    const char* e = b + t.size();
    const char* pe = b;
    float v = parse_float(b, e, &pe);
    if (pe == b)
    {
        return false;
    }
    if (pe < e && *pe == '%')
    {
        v = v / 100.0f * maxv;
    }
    else if (maxv <= 1.0f)
    {
        v = v * 255.0f;
    }
    if (v < 0.0f)
    {
        v = 0.0f;
    }
    if (v > 255.0f)
    {
        v = 255.0f;
    }
    out = static_cast<std::uint8_t>(v + 0.5f);
    return true;
}

bool parse_rgb_func(std::string_view s, Color& out)
{
    std::size_t lp = s.find('(');
    std::size_t rp = s.rfind(')');
    if (lp == std::string_view::npos || rp == std::string_view::npos || rp <= lp)
    {
        return false;
    }
    std::string_view name = s.substr(0, lp);
    bool alpha = (name == "rgba" || name == "hsla");
    bool hsl = (name == "hsl" || name == "hsla");
    if (name != "rgb" && name != "rgba" && !hsl)
    {
        return false;
    }
    std::string_view inner = s.substr(lp + 1, rp - lp - 1);
    std::string_view a_part;
    std::size_t slash = inner.find('/');
    if (slash != std::string_view::npos)
    {
        a_part = inner.substr(slash + 1);
        inner = inner.substr(0, slash);
        alpha = true;
    }
    std::vector<std::string> parts;
    if (inner.find(',') != std::string_view::npos)
    {
        parts = split_top(inner, ',');
    }
    else
    {
        parts = split_ws(inner);
    }
    if (parts.size() < 3)
    {
        return false;
    }
    if (hsl)
    {
        const char* b = parts[0].c_str();
        const char* e = b + parts[0].size();
        float h = parse_float(b, e, nullptr);
        while (h < 0.0f)
        {
            h += 360.0f;
        }
        while (h >= 360.0f)
        {
            h -= 360.0f;
        }
        auto pct = [](const std::string& p) -> float
        {
            const char* pb = p.c_str();
            const char* pe = pb + p.size();
            float v = parse_float(pb, pe, nullptr);
            return clamp01(v / 100.0f);
        };
        float sat = pct(parts[1]);
        float li = pct(parts[2]);
        float c = (1.0f - ((li * 2.0f > 1.0f) ? (2.0f - li * 2.0f) : (li * 2.0f))) * sat;
        float x = c * (1.0f - ((h / 60.0f) - static_cast<float>(static_cast<int>(h / 60.0f)) * 2.0f - 1.0f > 0 ? ((h / 60.0f) - static_cast<float>(static_cast<int>(h / 60.0f)) * 2.0f - 1.0f) : -((h / 60.0f) - static_cast<float>(static_cast<int>(h / 60.0f)) * 2.0f - 1.0f)));
        float r1 = 0.0f;
        float g1 = 0.0f;
        float b1 = 0.0f;
        int seg = static_cast<int>(h / 60.0f);
        if (seg == 0) { r1 = c; g1 = x; }
        else if (seg == 1) { r1 = x; g1 = c; }
        else if (seg == 2) { g1 = c; b1 = x; }
        else if (seg == 3) { g1 = x; b1 = c; }
        else if (seg == 4) { r1 = x; b1 = c; }
        else { r1 = c; b1 = x; }
        float m = li - c / 2.0f;
        out.r = static_cast<std::uint8_t>((r1 + m) * 255.0f + 0.5f);
        out.g = static_cast<std::uint8_t>((g1 + m) * 255.0f + 0.5f);
        out.b = static_cast<std::uint8_t>((b1 + m) * 255.0f + 0.5f);
        out.a = 255;
    }
    else
    {
        if (!parse_component(parts[0], 255.0f, out.r))
        {
            return false;
        }
        if (!parse_component(parts[1], 255.0f, out.g))
        {
            return false;
        }
        if (!parse_component(parts[2], 255.0f, out.b))
        {
            return false;
        }
        out.a = 255;
    }
    if (alpha)
    {
        std::string astr;
        if (!a_part.empty())
        {
            astr = trim_copy(a_part);
        }
        else if (parts.size() >= 4)
        {
            astr = parts[3];
        }
        if (!astr.empty() && !parse_component(astr, 1.0f, out.a))
        {
            return false;
        }
    }
    return true;
}

bool parse_named_color(std::string_view s, Color& out)
{
    struct Entry
    {
        const char* name;
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
    };
    static const Entry table[] = {
        {"black", 0, 0, 0}, {"white", 255, 255, 255}, {"red", 255, 0, 0},
        {"green", 0, 128, 0}, {"lime", 0, 255, 0}, {"blue", 0, 0, 255},
        {"yellow", 255, 255, 0}, {"gray", 128, 128, 128}, {"grey", 128, 128, 128},
        {"silver", 192, 192, 192}, {"maroon", 128, 0, 0}, {"olive", 128, 128, 0},
        {"navy", 0, 0, 128}, {"purple", 128, 0, 128}, {"fuchsia", 255, 0, 255},
        {"aqua", 0, 255, 255}, {"teal", 0, 128, 128}, {"orange", 255, 165, 0},
    };
    for (const auto& e : table)
    {
        if (s == e.name)
        {
            out = Color{e.r, e.g, e.b, 255};
            return true;
        }
    }
    if (s == "transparent")
    {
        out = Color{0, 0, 0, 0};
        return true;
    }
    return false;
}

bool parse_color_value(std::string_view s, const Color& current, Color& out)
{
    std::string t = trim_copy(s);
    if (t.empty())
    {
        return false;
    }
    if (t == "currentcolor")
    {
        out = current;
        return true;
    }
    if (parse_hex_color(t, out))
    {
        return true;
    }
    if (parse_rgb_func(t, out))
    {
        return true;
    }
    return parse_named_color(t, out);
}

} // namespace detail
} // namespace wv
