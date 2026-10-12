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

float parse_float(const char* p, const char* end, const char** out_end)
{
    bool neg = false;
    if (p < end && (*p == '-' || *p == '+'))
    {
        neg = (*p == '-');
        ++p;
    }
    float v = 0.0f;
    while (p < end && *p >= '0' && *p <= '9')
    {
        v = v * 10.0f + static_cast<float>(*p - '0');
        ++p;
    }
    if (p < end && *p == '.')
    {
        ++p;
        float m = 0.1f;
        while (p < end && *p >= '0' && *p <= '9')
        {
            v += static_cast<float>(*p - '0') * m;
            m *= 0.1f;
            ++p;
        }
    }
    if (out_end)
    {
        *out_end = p;
    }
    return neg ? -v : v;
}

std::string trim_copy(std::string_view s)
{
    std::size_t a = 0;
    while (a < s.size() && (s[a] == ' ' || s[a] == '\t' || s[a] == '\n' || s[a] == '\r'))
    {
        ++a;
    }
    std::size_t b = s.size();
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\n' || s[b - 1] == '\r'))
    {
        --b;
    }
    return std::string(s.substr(a, b - a));
}

std::vector<std::string> split_top(std::string_view s, char sep)
{
    std::vector<std::string> out;
    int depth = 0;
    bool sq = false;
    bool dq = false;
    std::size_t start = 0;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        if (c == '\'' && !dq)
        {
            sq = !sq;
        }
        else if (c == '"' && !sq)
        {
            dq = !dq;
        }
        else if (!sq && !dq)
        {
            if (c == '(')
            {
                ++depth;
            }
            else if (c == ')' && depth > 0)
            {
                --depth;
            }
            else if (c == sep && depth == 0)
            {
                out.push_back(trim_copy(s.substr(start, i - start)));
                start = i + 1;
            }
        }
    }
    out.push_back(trim_copy(s.substr(start)));
    return out;
}

std::vector<std::string> split_ws(std::string_view s)
{
    std::vector<std::string> out;
    std::size_t i = 0;
    while (i < s.size())
    {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
        {
            ++i;
        }
        std::size_t j = i;
        int depth = 0;
        while (j < s.size() && (depth > 0 || (s[j] != ' ' && s[j] != '\t' && s[j] != '\n' && s[j] != '\r')))
        {
            if (s[j] == '(')
            {
                ++depth;
            }
            else if (s[j] == ')')
            {
                --depth;
            }
            ++j;
        }
        if (j > i)
        {
            out.push_back(std::string(s.substr(i, j - i)));
        }
        i = j;
    }
    return out;
}

bool parse_length_value(std::string_view s, Length& out)
{
    std::string t = trim_copy(s);
    if (t == "auto")
    {
        out = Length::auto_();
        return true;
    }
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
    std::string_view u(pe, static_cast<std::size_t>(e - pe));
    Unit unit = Unit::k_px;
    if (u.empty() || u == "px")
    {
        unit = Unit::k_px;
    }
    else if (u == "%")
    {
        unit = Unit::k_percent;
    }
    else if (u == "em")
    {
        unit = Unit::k_em;
    }
    else if (u == "rem")
    {
        unit = Unit::k_rem;
    }
    else if (u == "vw")
    {
        unit = Unit::k_vw;
    }
    else if (u == "vh")
    {
        unit = Unit::k_vh;
    }
    else if (u == "fr")
    {
        unit = Unit::k_fr;
    }
    else
    {
        return false;
    }
    out.value = v;
    out.unit = unit;
    return true;
}

} // namespace detail
} // namespace wv
