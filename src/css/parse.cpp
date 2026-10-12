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

void parse_selector_list(std::string_view text, std::vector<Selector>& out, char& leading);

bool parse_nth(std::string_view arg, int& a, int& b)
{
    std::string t;
    for (char c : arg)
    {
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
        {
            t.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c);
        }
    }
    if (t == "odd")
    {
        a = 2;
        b = 1;
        return true;
    }
    if (t == "even")
    {
        a = 2;
        b = 0;
        return true;
    }
    std::size_t npos = t.find('n');
    auto to_int = [](std::string_view s, bool& ok) -> int {
        if (s.empty())
        {
            ok = false;
            return 0;
        }
        bool neg = false;
        std::size_t i = 0;
        if (s[0] == '+' || s[0] == '-')
        {
            neg = (s[0] == '-');
            i = 1;
        }
        if (i >= s.size())
        {
            ok = false;
            return 0;
        }
        int v = 0;
        for (; i < s.size(); ++i)
        {
            if (s[i] < '0' || s[i] > '9')
            {
                ok = false;
                return 0;
            }
            v = v * 10 + (s[i] - '0');
        }
        ok = true;
        return neg ? -v : v;
    };
    if (npos == std::string::npos)
    {
        bool ok = false;
        int v = to_int(t, ok);
        if (!ok)
        {
            return false;
        }
        a = 0;
        b = v;
        return true;
    }
    std::string_view left(t.data(), npos);
    std::string_view right(t.data() + npos + 1, t.size() - npos - 1);
    if (left.empty() || left == "+")
    {
        a = 1;
    }
    else if (left == "-")
    {
        a = -1;
    }
    else
    {
        bool ok = false;
        a = to_int(left, ok);
        if (!ok)
        {
            return false;
        }
    }
    if (right.empty())
    {
        b = 0;
        return true;
    }
    bool ok = false;
    b = to_int(right, ok);
    return ok;
}

void skip_ws_comments(const char*& p, const char* end)
{
    while (p < end)
    {
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f')
        {
            ++p;
        }
        else if (p + 1 < end && p[0] == '/' && p[1] == '*')
        {
            p += 2;
            while (p + 1 < end && !(p[0] == '*' && p[1] == '/'))
            {
                ++p;
            }
            if (p + 1 < end)
            {
                p += 2;
            }
        }
        else
        {
            break;
        }
    }
}

std::string parse_ident(const char*& p, const char* end)
{
    std::string out;
    if (p < end && (*p == '-' || *p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')))
    {
        out.push_back(*p);
        ++p;
    }
    while (p < end && (*p == '-' || *p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')))
    {
        out.push_back(*p);
        ++p;
    }
    return out;
}

bool parse_compound(const char*& p, const char* end, Compound& out)
{
    out = Compound{};
    bool any = false;
    if (p < end && (*p == '*' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')))
    {
        if (*p == '*')
        {
            ++p;
        }
        else
        {
            std::string tag = parse_ident(p, end);
            for (char& c : tag)
            {
                if (c >= 'A' && c <= 'Z')
                {
                    c = static_cast<char>(c + 32);
                }
            }
            out.tag = tag;
        }
        any = true;
    }
    while (p < end)
    {
        if (*p == '#')
        {
            ++p;
            out.id = parse_ident(p, end);
            any = true;
        }
        else if (*p == '.')
        {
            ++p;
            std::string cls = parse_ident(p, end);
            if (!cls.empty())
            {
                out.classes.push_back(cls);
                any = true;
            }
        }
        else if (*p == '[')
        {
            ++p;
            skip_ws_comments(p, end);
            out.attr_name = parse_ident(p, end);
            skip_ws_comments(p, end);
            int op = 0;
            if (p + 1 < end && (*p == '~' || *p == '|' || *p == '^' || *p == '$' ||
                                *p == '*') &&
                p[1] == '=')
            {
                if (*p == '~')
                {
                    op = 2;
                }
                else if (*p == '|')
                {
                    op = 3;
                }
                else if (*p == '^')
                {
                    op = 4;
                }
                else if (*p == '$')
                {
                    op = 5;
                }
                else
                {
                    op = 6;
                }
                p += 2;
            }
            else if (p < end && *p == '=')
            {
                op = 1;
                ++p;
            }
            if (op != 0)
            {
                skip_ws_comments(p, end);
                if (p < end && (*p == '"' || *p == '\''))
                {
                    char q = *p;
                    ++p;
                    const char* vs = p;
                    while (p < end && *p != q)
                    {
                        ++p;
                    }
                    out.attr_value.assign(vs, static_cast<std::size_t>(p - vs));
                    if (p < end)
                    {
                        ++p;
                    }
                }
                else
                {
                    const char* vs = p;
                    while (p < end && *p != ']' && *p != ' ' && *p != '\t')
                    {
                        ++p;
                    }
                    out.attr_value.assign(vs, static_cast<std::size_t>(p - vs));
                }
                skip_ws_comments(p, end);
                if (p < end && (*p == 'i' || *p == 'I' || *p == 's' || *p == 'S'))
                {
                    out.attr_ignore_case = (*p == 'i' || *p == 'I');
                    ++p;
                    skip_ws_comments(p, end);
                }
            }
            if (p < end && *p == ']')
            {
                ++p;
            }
            out.attr_op = op;
            out.has_attr = !out.attr_name.empty();
            any = true;
        }
        else if (*p == ':')
        {
            ++p;
            if (p < end && *p == ':')
            {
                ++p;
                out.pseudo_element = parse_ident(p, end);
            }
            else
            {
                std::string name = parse_ident(p, end);
                std::string arg;
                if (p < end && *p == '(')
                {
                    ++p;
                    int depth = 1;
                    while (p < end && depth > 0)
                    {
                        if (*p == '(')
                        {
                            ++depth;
                        }
                        else if (*p == ')')
                        {
                            --depth;
                        }
                        if (depth > 0)
                        {
                            arg.push_back(*p);
                        }
                        ++p;
                    }
                }
                out.pseudo = name;
                out.pseudo_arg = arg;
                if (name == "not" || name == "is" || name == "where" || name == "has")
                {
                    parse_selector_list(arg, out.pseudo_selectors, out.has_combinator);
                }
                else if (name == "nth-child" || name == "nth-last-child" ||
                         name == "nth-of-type" || name == "nth-last-of-type" ||
                         name == "nth-col" || name == "nth-last-col")
                {
                    out.has_nth = parse_nth(arg, out.nth_a, out.nth_b);
                }
            }
            any = true;
        }
        else
        {
            break;
        }
    }
    return any;
}

bool parse_selector(const char*& p, const char* end, Selector& out)
{
    out = Selector{};
    skip_ws_comments(p, end);
    Compound c;
    if (!parse_compound(p, end, c))
    {
        return false;
    }
    out.parts.push_back(c);
    while (true)
    {
        skip_ws_comments(p, end);
        if (p >= end || *p == ',' || *p == '{')
        {
            break;
        }
        char comb = ' ';
        if (*p == '>' || *p == '+' || *p == '~')
        {
            comb = *p;
            ++p;
            skip_ws_comments(p, end);
        }
        if (!parse_compound(p, end, c))
        {
            return false;
        }
        out.combinators.push_back(comb);
        out.parts.push_back(c);
    }
    return !out.parts.empty();
}

void parse_selector_list(std::string_view text, std::vector<Selector>& out, char& leading)
{
    leading = ' ';
    std::string copy(text);
    const char* p = copy.c_str();
    const char* end = p + copy.size();
    skip_ws_comments(p, end);
    if (p < end && (*p == '>' || *p == '+' || *p == '~'))
    {
        leading = *p;
        ++p;
    }
    while (p < end)
    {
        Selector sel;
        if (parse_selector(p, end, sel))
        {
            out.push_back(sel);
        }
        skip_ws_comments(p, end);
        if (p < end && *p == ',')
        {
            ++p;
        }
        else
        {
            break;
        }
    }
}

void parse_declarations(const char*& p, const char* end, std::vector<Declaration>& out)
{
    while (p < end && *p != '}')
    {
        skip_ws_comments(p, end);
        if (p >= end || *p == '}')
        {
            break;
        }
        if (*p == ';')
        {
            ++p;
            continue;
        }
        std::string prop = parse_ident(p, end);
        if (prop.empty())
        {
            while (p < end && *p != ';' && *p != '}')
            {
                ++p;
            }
            if (p < end && *p == ';')
            {
                ++p;
            }
            continue;
        }
        skip_ws_comments(p, end);
        if (p >= end || *p != ':')
        {
            while (p < end && *p != ';' && *p != '}')
            {
                ++p;
            }
            if (p < end && *p == ';')
            {
                ++p;
            }
            continue;
        }
        ++p;
        const char* vs = p;
        int depth = 0;
        bool sq = false;
        bool dq = false;
        while (p < end)
        {
            char ch = *p;
            if (ch == '\'' && !dq)
            {
                sq = !sq;
            }
            else if (ch == '"' && !sq)
            {
                dq = !dq;
            }
            else if (!sq && !dq)
            {
                if (ch == '(')
                {
                    ++depth;
                }
                else if (ch == ')')
                {
                    --depth;
                }
                else if ((ch == ';' || ch == '}') && depth <= 0)
                {
                    break;
                }
            }
            ++p;
        }
        std::string val(vs, static_cast<std::size_t>(p - vs));
        if (p < end && *p == ';')
        {
            ++p;
        }
        std::string t = trim_copy(val);
        bool important = false;
        if (t.size() > 10 && t.substr(t.size() - 10) == "!important")
        {
            important = true;
            t = trim_copy(t.substr(0, t.size() - 10));
        }
        if (!t.empty() || prop.rfind("--", 0) == 0)
        {
            out.push_back(Declaration{prop, t, important});
        }
    }
}

int layer_index(Stylesheet& sheet, std::string_view name)
{
    for (std::size_t i = 0; i < sheet.layer_order.size(); ++i)
    {
        if (sheet.layer_order[i] == name)
        {
            return static_cast<int>(i);
        }
    }
    sheet.layer_order.push_back(std::string(name));
    return static_cast<int>(sheet.layer_order.size()) - 1;
}

void parse_rules(const char*& p, const char* end, Stylesheet& sheet, float media_max, float media_min, int layer, std::uint32_t& order)
{
    while (p < end)
    {
        skip_ws_comments(p, end);
        if (p >= end)
        {
            break;
        }
        if (*p == '}')
        {
            ++p;
            break;
        }
        if (*p == '@')
        {
            ++p;
            std::string at = parse_ident(p, end);
            if (at == "media")
            {
                const char* cs = p;
                while (p < end && *p != '{')
                {
                    ++p;
                }
                std::string cond(cs, static_cast<std::size_t>(p - cs));
                float mx = -1.0f;
                float mn = -1.0f;
                std::size_t pos = cond.find("max-width");
                if (pos != std::string::npos)
                {
                    std::size_t colon = cond.find(':', pos);
                    if (colon != std::string::npos)
                    {
                        const char* b = cond.c_str() + colon + 1;
                        mx = parse_float(b, b + cond.size() - colon - 1, nullptr);
                    }
                }
                pos = cond.find("min-width");
                if (pos != std::string::npos)
                {
                    std::size_t colon = cond.find(':', pos);
                    if (colon != std::string::npos)
                    {
                        const char* b = cond.c_str() + colon + 1;
                        mn = parse_float(b, b + cond.size() - colon - 1, nullptr);
                    }
                }
                if (p < end && *p == '{')
                {
                    ++p;
                }
                float eff_max = (mx >= 0.0f) ? mx : media_max;
                float eff_min = (mn >= 0.0f) ? mn : media_min;
                parse_rules(p, end, sheet, eff_max, eff_min, layer, order);
                continue;
            }
            if (at == "layer")
            {
                const char* cs = p;
                while (p < end && *p != '{' && *p != ';')
                {
                    ++p;
                }
                std::string prelude(cs, static_cast<std::size_t>(p - cs));
                if (p < end && *p == ';')
                {
                    ++p;
                    for (const std::string& name : split_top(prelude, ','))
                    {
                        std::string trimmed = trim_copy(name);
                        if (!trimmed.empty())
                        {
                            layer_index(sheet, trimmed);
                        }
                    }
                    continue;
                }
                int block_layer = layer;
                std::string name = trim_copy(prelude);
                if (!name.empty())
                {
                    block_layer = layer_index(sheet, name);
                }
                else
                {
                    std::string anon = "$" + std::to_string(sheet.layer_order.size());
                    block_layer = layer_index(sheet, anon);
                }
                if (p < end && *p == '{')
                {
                    ++p;
                }
                parse_rules(p, end, sheet, media_max, media_min, block_layer, order);
                continue;
            }
            int depth = 0;
            while (p < end)
            {
                if (*p == '{')
                {
                    ++depth;
                }
                else if (*p == '}')
                {
                    if (depth == 0)
                    {
                        ++p;
                        break;
                    }
                    --depth;
                }
                else if (*p == ';' && depth == 0)
                {
                    ++p;
                    break;
                }
                ++p;
            }
            continue;
        }
        const char* sels = p;
        while (p < end && *p != '{' && *p != '}')
        {
            ++p;
        }
        if (p >= end || *p != '{')
        {
            if (p < end)
            {
                ++p;
            }
            continue;
        }
        std::string sel_text(sels, static_cast<std::size_t>(p - sels));
        ++p;
        std::vector<Declaration> decls;
        parse_declarations(p, end, decls);
        if (p < end && *p == '}')
        {
            ++p;
        }
        const char* sp = sel_text.c_str();
        const char* se = sp + sel_text.size();
        std::vector<Selector> selectors;
        while (sp < se)
        {
            Selector sel;
            if (parse_selector(sp, se, sel))
            {
                selectors.push_back(sel);
            }
            skip_ws_comments(sp, se);
            if (sp < se && *sp == ',')
            {
                ++sp;
            }
            else
            {
                break;
            }
        }
        if (!selectors.empty() && !decls.empty())
        {
            Rule rule;
            rule.selectors = selectors;
            rule.declarations = decls;
            rule.media_max_width = media_max;
            rule.media_min_width = media_min;
            rule.layer = layer;
            rule.order = order++;
            sheet.rules.push_back(rule);
        }
    }
}

} // namespace detail
} // namespace wv
