/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "css/internal.hpp"

#include "wview/css.hpp"
#include "wview/dom.hpp"

#include <algorithm>

namespace wv
{
namespace detail
{

std::string substitute_vars(const std::string& value, const std::vector<std::pair<std::string, std::string>>& vars)
{
    std::string out = value;
    for (int iter = 0; iter < 8; ++iter)
    {
        std::size_t pos = out.find("var(");
        if (pos == std::string::npos)
        {
            break;
        }
        std::size_t start = pos + 4;
        int depth = 1;
        std::size_t i = start;
        while (i < out.size() && depth > 0)
        {
            if (out[i] == '(')
            {
                ++depth;
            }
            else if (out[i] == ')')
            {
                --depth;
            }
            ++i;
        }
        if (depth != 0)
        {
            break;
        }
        std::string inner = out.substr(start, i - start - 1);
        std::vector<std::string> bits = split_top(inner, ',');
        std::string name = trim_copy(bits.empty() ? std::string() : bits[0]);
        std::string rep;
        bool found = false;
        for (const auto& v : vars)
        {
            if (v.first == name)
            {
                rep = v.second;
                found = true;
            }
        }
        if (!found && bits.size() > 1)
        {
            rep = trim_copy(bits[1]);
            for (std::size_t k = 2; k < bits.size(); ++k)
            {
                rep += "," + bits[k];
            }
            found = true;
        }
        if (!found)
        {
            rep = "";
        }
        out = out.substr(0, pos) + rep + out.substr(i);
    }
    return out;
}

float resolve_px(Length l, float font_size, float root_size, float viewport_w, float viewport_h, float percent_base)
{
    switch (l.unit)
    {
    case Unit::k_px:
        return l.value;
    case Unit::k_percent:
        return l.value / 100.0f * percent_base;
    case Unit::k_em:
        return l.value * font_size;
    case Unit::k_rem:
        return l.value * root_size;
    case Unit::k_vw:
        return l.value / 100.0f * viewport_w;
    case Unit::k_vh:
        return l.value / 100.0f * viewport_h;
    default:
        return 0.0f;
    }
}

void apply_box_sides(const std::vector<std::string>& parts, Length out[4])
{
    Length v[4];
    int n = 0;
    for (const auto& p : parts)
    {
        if (n >= 4)
        {
            break;
        }
        Length l;
        if (parse_length_value(p, l))
        {
            v[n++] = l;
        }
    }
    if (n == 0)
    {
        return;
    }
    if (n == 1)
    {
        out[0] = out[1] = out[2] = out[3] = v[0];
    }
    else if (n == 2)
    {
        out[0] = out[2] = v[0];
        out[1] = out[3] = v[1];
    }
    else if (n == 3)
    {
        out[0] = v[0];
        out[1] = out[3] = v[1];
        out[2] = v[2];
    }
    else
    {
        out[0] = v[0];
        out[1] = v[1];
        out[2] = v[2];
        out[3] = v[3];
    }
}

} // namespace detail

namespace
{

struct Matched
{
    std::uint32_t spec = 0;
    bool important = false;
    int layer_rank = 0;
    std::uint32_t order = 0;
    const Declaration* decl = nullptr;
};

} // namespace

void apply_declaration(
    ComputedStyle& st,
    const std::string& prop,
    const std::string& value,
    const ComputedStyle* parent,
    float viewport_w,
    float viewport_h)
{
    if (detail::apply_box(st, prop, value, parent, viewport_w, viewport_h))
        return;
    if (detail::apply_flex(st, prop, value, parent, viewport_w, viewport_h))
        return;
    if (detail::apply_paint(st, prop, value, parent, viewport_w, viewport_h))
        return;
    if (detail::apply_text(st, prop, value, parent, viewport_w, viewport_h))
        return;
    if (detail::apply_misc(st, prop, value, parent, viewport_w, viewport_h))
        return;
}

ComputedStyle default_style_for_tag(std::string_view tag)
{
    ComputedStyle st;
    st.display = Display::k_inline;
    if (tag == "head" || tag == "style" || tag == "script" || tag == "title" || tag == "meta" || tag == "link")
    {
        st.display = Display::k_none;
        return st;
    }
    if (tag == "div" || tag == "header" || tag == "nav" || tag == "main" || tag == "section" ||
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" || tag == "p" || tag == "body" ||
        tag == "html" || tag == "ul" || tag == "li" || tag == "head")
    {
        st.display = Display::k_block;
    }
    if (tag == "table")
    {
        st.display = Display::k_table;
    }
    if (tag == "thead" || tag == "tbody" || tag == "tfoot")
    {
        st.display = Display::k_table_row_group;
    }
    if (tag == "tr")
    {
        st.display = Display::k_table_row;
    }
    if (tag == "td" || tag == "th")
    {
        st.display = Display::k_table_cell;
    }
    if (tag == "caption")
    {
        st.display = Display::k_table_caption;
    }
    if (tag == "col" || tag == "colgroup")
    {
        st.display = Display::k_none;
    }
    if (tag == "button" || tag == "input")
    {
        st.display = Display::k_inline_block;
    }
    if (tag == "img")
    {
        st.display = Display::k_inline_block;
    }
    return st;
}

ComputedStyle resolve_style(
    const Document& doc,
    int node,
    const ComputedStyle* parent,
    const std::vector<std::pair<std::string, std::string>>* parent_vars,
    const Stylesheet& sheet,
    const ElementState& state,
    const ResolveContext& ctx,
    std::vector<std::pair<std::string, std::string>>& out_vars)
{
    ComputedStyle st = default_style_for_tag(doc.node(node).tag);
    if (parent)
    {
        st.color = parent->color;
        st.font_family = parent->font_family;
        st.font_size = parent->font_size;
        st.font_weight = parent->font_weight;
        st.font_italic = parent->font_italic;
        st.line_height = parent->line_height;
        st.line_height_normal = parent->line_height_normal;
        st.letter_spacing = parent->letter_spacing;
        st.text_align = parent->text_align;
        st.text_transform = parent->text_transform;
        st.white_space = parent->white_space;
        st.underline = parent->underline;
    }
    else
    {
        st.font_family = "sans-serif";
    }

    std::vector<Matched> matched;
    for (const auto& rule : sheet.rules)
    {
        if (rule.media_max_width >= 0.0f && ctx.viewport_width > rule.media_max_width)
        {
            continue;
        }
        if (rule.media_min_width >= 0.0f && ctx.viewport_width < rule.media_min_width)
        {
            continue;
        }
        for (const auto& sel : rule.selectors)
        {
            std::string pseudo;
            for (const auto& part : sel.parts)
            {
                if (!part.pseudo_element.empty())
                {
                    pseudo = part.pseudo_element;
                }
            }
            if (!pseudo.empty())
            {
                continue;
            }
            if (match_selector(doc, node, sel, state, std::string()))
            {
                for (const auto& decl : rule.declarations)
                {
                    int layers = static_cast<int>(sheet.layer_order.size());
                    int rank = 0;
                    if (decl.important)
                    {
                        rank = (rule.layer < 0) ? 0 : (layers - rule.layer);
                    }
                    else
                    {
                        rank = (rule.layer < 0) ? 2000000000 : rule.layer;
                    }
                    matched.push_back(Matched{detail::specificity(sel), decl.important, rank, rule.order, &decl});
                }
                break;
            }
        }
    }

    std::vector<std::pair<std::string, std::string>> vars;
    if (parent_vars)
    {
        vars = *parent_vars;
    }
    for (int pass = 0; pass < 3; ++pass)
    {
        for (const auto& m : matched)
        {
            if (m.decl->property.rfind("--", 0) != 0)
            {
                continue;
            }
            std::string sub = detail::substitute_vars(m.decl->value, vars);
            bool found = false;
            for (auto& v : vars)
            {
                if (v.first == m.decl->property)
                {
                    v.second = sub;
                    found = true;
                }
            }
            if (!found)
            {
                vars.push_back({m.decl->property, sub});
            }
        }
    }

    std::stable_sort(
        matched.begin(),
        matched.end(),
        [](const Matched& a, const Matched& b)
        {
            if (a.important != b.important)
            {
                return b.important;
            }
            if (a.layer_rank != b.layer_rank)
            {
                return a.layer_rank < b.layer_rank;
            }
            if (a.spec != b.spec)
            {
                return a.spec < b.spec;
            }
            return a.order < b.order;
        });

    std::string inline_style = std::string(doc.attr(node, "style"));
    std::vector<Declaration> inline_decls;
    if (!inline_style.empty())
    {
        const char* p = inline_style.c_str();
        detail::parse_declarations(p, p + inline_style.size(), inline_decls);
    }

    static const char* fonts[] = {"font-size", "font-family", "font-weight", "font-style"};
    for (const char* fname : fonts)
    {
        for (const auto& m : matched)
        {
            if (!m.important && m.decl->property == fname)
            {
                apply_declaration(st, fname, detail::substitute_vars(m.decl->value, vars), parent, ctx.viewport_width, ctx.viewport_width * 9.0f / 16.0f);
            }
        }
    }
    for (const auto& m : matched)
    {
        if (m.important || m.decl->property.rfind("--", 0) == 0)
        {
            continue;
        }
        if (m.decl->property == "font-size" || m.decl->property == "font-family" || m.decl->property == "font-weight" || m.decl->property == "font-style")
        {
            continue;
        }
        apply_declaration(st, m.decl->property, detail::substitute_vars(m.decl->value, vars), parent, ctx.viewport_width, ctx.viewport_width * 9.0f / 16.0f);
    }
    for (const auto& d : inline_decls)
    {
        if (d.property.rfind("--", 0) == 0)
        {
            std::string sub = detail::substitute_vars(d.value, vars);
            bool found = false;
            for (auto& v : vars)
            {
                if (v.first == d.property)
                {
                    v.second = sub;
                    found = true;
                }
            }
            if (!found)
            {
                vars.push_back({d.property, sub});
            }
        }
        else
        {
            apply_declaration(st, d.property, detail::substitute_vars(d.value, vars), parent, ctx.viewport_width, ctx.viewport_width * 9.0f / 16.0f);
        }
    }
    for (const auto& m : matched)
    {
        if (!m.important || m.decl->property.rfind("--", 0) == 0)
        {
            continue;
        }
        apply_declaration(st, m.decl->property, detail::substitute_vars(m.decl->value, vars), parent, ctx.viewport_width, ctx.viewport_width * 9.0f / 16.0f);
    }

    out_vars = vars;
    return st;
}

Stylesheet parse_css(std::string_view src)
{
    Stylesheet sheet;
    sheet.source.assign(src.data(), src.size());
    const char* p = sheet.source.c_str();
    const char* end = p + sheet.source.size();
    std::uint32_t order = 0;
    detail::parse_rules(p, end, sheet, -1.0f, -1.0f, -1, order);
    return sheet;
}

} // namespace wv
