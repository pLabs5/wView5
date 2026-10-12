/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "dom/internal.hpp"

#include "wview/dom.hpp"

namespace wv
{
namespace detail
{

bool is_void_tag(std::string_view tag)
{
    constexpr std::string_view voids[] = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr",
    };
    for (const auto& v : voids)
    {
        if (v == tag)
        {
            return true;
        }
    }
    return false;
}

bool is_raw_text_tag(std::string_view tag)
{
    return tag == "script" || tag == "style" || tag == "textarea" || tag == "title" ||
           tag == "iframe" || tag == "noembed" || tag == "noframes" || tag == "noscript" ||
           tag == "xmp";
}

bool is_special_tag(std::string_view tag)
{
    constexpr std::string_view specials[] = {
        "address", "applet",  "area",    "article", "aside",   "base",     "basefont",
        "bgsound", "blockquote", "body", "br",      "button",  "caption",  "center",
        "col",     "colgroup", "dd",     "details", "dialog",  "dir",      "div",
        "dl",      "fieldset", "figcaption", "figure", "footer", "form",   "frame",
        "frameset", "h1", "h2", "h3", "h4", "h5", "h6", "head", "header", "hgroup",
        "hr", "html", "iframe", "img", "input", "keygen", "li", "link", "listing",
        "main", "marquee", "menu", "meta", "nav", "noembed", "noframes", "noscript",
        "object", "ol", "p", "param", "plaintext", "pre", "script", "search", "section",
        "select", "source", "style", "summary", "table", "tbody", "td", "template",
        "textarea", "tfoot", "th", "thead", "title", "tr", "track", "ul", "wbr", "xmp",
    };
    for (const auto& s : specials)
    {
        if (s == tag)
        {
            return true;
        }
    }
    return false;
}

bool is_formatting_tag(std::string_view tag)
{
    return tag == "a" || tag == "b" || tag == "big" || tag == "code" || tag == "em" ||
           tag == "font" || tag == "i" || tag == "s" || tag == "small" || tag == "strike" ||
           tag == "strong" || tag == "tt" || tag == "u";
}

bool closes_paragraph(std::string_view tag)
{
    return tag == "address" || tag == "article" || tag == "aside" ||
           tag == "blockquote" || tag == "center" || tag == "details" ||
           tag == "dialog" || tag == "dir" || tag == "div" || tag == "dl" ||
           tag == "fieldset" || tag == "figcaption" || tag == "figure" ||
           tag == "footer" || tag == "form" || tag == "h1" || tag == "h2" ||
           tag == "h3" || tag == "h4" || tag == "h5" || tag == "h6" ||
           tag == "header" || tag == "hgroup" || tag == "hr" || tag == "main" ||
           tag == "menu" || tag == "nav" || tag == "ol" || tag == "p" ||
           tag == "pre" || tag == "search" || tag == "section" || tag == "table" ||
           tag == "ul";
}

bool is_table_part(std::string_view tag)
{
    return tag == "table" || tag == "caption" || tag == "colgroup" || tag == "thead" ||
           tag == "tbody" || tag == "tfoot" || tag == "tr" || tag == "td" || tag == "th" ||
           tag == "col";
}

bool is_table_section_or_row(std::string_view tag)
{
    return tag == "table" || tag == "thead" || tag == "tbody" || tag == "tfoot" ||
           tag == "tr";
}

void to_lower_inplace(std::string& s, std::size_t begin, std::size_t end)
{
    for (std::size_t i = begin; i < end && i < s.size(); ++i)
    {
        char c = s[i];
        if (c >= 'A' && c <= 'Z')
        {
            s[i] = static_cast<char>(c + ('a' - 'A'));
        }
    }
}

std::string_view decode_entity(std::string_view e)
{
    if (e == "amp")
    {
        return "&";
    }
    if (e == "lt")
    {
        return "<";
    }
    if (e == "gt")
    {
        return ">";
    }
    if (e == "quot")
    {
        return "\"";
    }
    if (e == "apos" || e == "#39")
    {
        return "'";
    }
    if (e == "nbsp")
    {
        return "\xc2\xa0";
    }
    if (e == "copy")
    {
        return "\xc2\xa9";
    }
    if (e == "reg")
    {
        return "\xc2\xae";
    }
    if (e == "trade")
    {
        return "\xe2\x84\xa2";
    }
    if (e == "euro")
    {
        return "\xe2\x82\xac";
    }
    if (e == "pound")
    {
        return "\xc2\xa3";
    }
    if (e == "yen")
    {
        return "\xc2\xa5";
    }
    if (e == "cent")
    {
        return "\xc2\xa2";
    }
    if (e == "hellip")
    {
        return "\xe2\x80\xa6";
    }
    if (e == "mdash")
    {
        return "\xe2\x80\x94";
    }
    if (e == "ndash")
    {
        return "\xe2\x80\x93";
    }
    if (e == "ldquo")
    {
        return "\xe2\x80\x9c";
    }
    if (e == "rdquo")
    {
        return "\xe2\x80\x9d";
    }
    if (e == "lsquo")
    {
        return "\xe2\x80\x98";
    }
    if (e == "rsquo")
    {
        return "\xe2\x80\x99";
    }
    if (e == "laquo")
    {
        return "\xc2\xab";
    }
    if (e == "raquo")
    {
        return "\xc2\xbb";
    }
    if (e == "lsaquo")
    {
        return "\xe2\x80\xb9";
    }
    if (e == "rsaquo")
    {
        return "\xe2\x80\xba";
    }
    if (e == "sbquo")
    {
        return "\xe2\x80\x9a";
    }
    if (e == "bdquo")
    {
        return "\xe2\x80\x9e";
    }
    if (e == "dagger")
    {
        return "\xe2\x80\xa0";
    }
    if (e == "Dagger")
    {
        return "\xe2\x80\xa1";
    }
    if (e == "bull")
    {
        return "\xe2\x80\xa2";
    }
    if (e == "middot")
    {
        return "\xc2\xb7";
    }
    if (e == "sect")
    {
        return "\xc2\xa7";
    }
    if (e == "para")
    {
        return "\xc2\xb6";
    }
    if (e == "prime")
    {
        return "\xe2\x80\xb2";
    }
    if (e == "Prime")
    {
        return "\xe2\x80\xb3";
    }
    if (e == "oline")
    {
        return "\xe2\x80\xbe";
    }
    if (e == "frasl")
    {
        return "\xe2\x81\x84";
    }
    if (e == "larr")
    {
        return "\xe2\x86\x90";
    }
    if (e == "rarr")
    {
        return "\xe2\x86\x92";
    }
    if (e == "uarr")
    {
        return "\xe2\x86\x91";
    }
    if (e == "darr")
    {
        return "\xe2\x86\x93";
    }
    if (e == "harr")
    {
        return "\xe2\x86\x94";
    }
    if (e == "crarr")
    {
        return "\xe2\x86\xb5";
    }
    if (e == "lArr")
    {
        return "\xe2\x87\x90";
    }
    if (e == "rArr")
    {
        return "\xe2\x87\x92";
    }
    return {};
}

bool decode_numeric_entity(std::string_view e, char* out, std::size_t& length)
{
    if (e.empty() || e[0] != '#')
    {
        return false;
    }
    e.remove_prefix(1);
    unsigned value = 0;
    if (!e.empty() && (e[0] == 'x' || e[0] == 'X'))
    {
        e.remove_prefix(1);
        if (e.empty() || e.size() > 6)
        {
            return false;
        }
        for (char c : e)
        {
            int d = -1;
            if (c >= '0' && c <= '9')
            {
                d = c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                d = c - 'a' + 10;
            }
            else if (c >= 'A' && c <= 'F')
            {
                d = c - 'A' + 10;
            }
            if (d < 0)
            {
                return false;
            }
            value = value * 16 + static_cast<unsigned>(d);
        }
    }
    else
    {
        if (e.empty() || e.size() > 7)
        {
            return false;
        }
        for (char c : e)
        {
            if (c < '0' || c > '9')
            {
                return false;
            }
            value = value * 10 + static_cast<unsigned>(c - '0');
        }
    }
    if (value == 0 || (value >= 0xd800 && value <= 0xdfff) || value > 0x10ffff)
    {
        value = 0xfffd;
    }
    if (value < 0x80)
    {
        out[0] = static_cast<char>(value);
        length = 1;
    }
    else if (value < 0x800)
    {
        out[0] = static_cast<char>(0xc0 | (value >> 6));
        out[1] = static_cast<char>(0x80 | (value & 0x3f));
        length = 2;
    }
    else if (value < 0x10000)
    {
        out[0] = static_cast<char>(0xe0 | (value >> 12));
        out[1] = static_cast<char>(0x80 | ((value >> 6) & 0x3f));
        out[2] = static_cast<char>(0x80 | (value & 0x3f));
        length = 3;
    }
    else
    {
        out[0] = static_cast<char>(0xf0 | (value >> 18));
        out[1] = static_cast<char>(0x80 | ((value >> 12) & 0x3f));
        out[2] = static_cast<char>(0x80 | ((value >> 6) & 0x3f));
        out[3] = static_cast<char>(0x80 | (value & 0x3f));
        length = 4;
    }
    return true;
}

constexpr int k_marker = -2;

struct Parser
{
    Document doc;
    std::vector<int> stack;
    std::vector<int> fmt;

    int top_tag_is(std::string_view tag) const
    {
        if (stack.empty())
        {
            return false;
        }
        return doc.nodes[static_cast<std::size_t>(stack.back())].tag == tag;
    }

    bool in_scope(std::string_view tag, bool button_scope, bool list_scope) const
    {
        for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
        {
            std::string_view current =
                doc.nodes[static_cast<std::size_t>(stack[static_cast<std::size_t>(depth)])].tag;
            if (current == tag)
            {
                return true;
            }
            if (current == "html" || current == "table" || current == "template" ||
                current == "applet" || current == "marquee" || current == "object")
            {
                return false;
            }
            if (current == "caption" || current == "td" || current == "th")
            {
                return false;
            }
            if (button_scope && current == "button")
            {
                return false;
            }
            if (list_scope && (current == "ol" || current == "ul"))
            {
                return false;
            }
        }
        return false;
    }

    bool in_table_scope(std::string_view tag) const
    {
        for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
        {
            std::string_view current =
                doc.nodes[static_cast<std::size_t>(stack[static_cast<std::size_t>(depth)])].tag;
            if (current == tag)
            {
                return true;
            }
            if (current == "html" || current == "table" || current == "template")
            {
                return false;
            }
        }
        return false;
    }

    bool in_select() const
    {
        for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
        {
            std::string_view current =
                doc.nodes[static_cast<std::size_t>(stack[static_cast<std::size_t>(depth)])].tag;
            if (current == "select")
            {
                return true;
            }
            if (current == "table" || current == "template")
            {
                return false;
            }
        }
        return false;
    }

    int push_node(NodeKind kind)
    {
        Node n;
        n.kind = kind;
        int index = static_cast<int>(doc.nodes.size());
        doc.nodes.push_back(n);
        if (!stack.empty())
        {
            attach(index, stack.back(), k_no_node);
        }
        else if (doc.root_ == k_no_node && kind == NodeKind::k_element)
        {
            doc.root_ = index;
        }
        return index;
    }

    void attach(int index, int parent, int before)
    {
        doc.nodes[static_cast<std::size_t>(index)].parent = parent;
        Node& p = doc.nodes[static_cast<std::size_t>(parent)];
        if (before == k_no_node)
        {
            if (p.first_child == k_no_node)
            {
                p.first_child = index;
            }
            else
            {
                int sib = p.first_child;
                while (doc.nodes[static_cast<std::size_t>(sib)].next_sibling != k_no_node)
                {
                    sib = doc.nodes[static_cast<std::size_t>(sib)].next_sibling;
                }
                doc.nodes[static_cast<std::size_t>(sib)].next_sibling = index;
            }
            return;
        }
        int prev = k_no_node;
        for (int c = p.first_child; c != k_no_node;
             c = doc.nodes[static_cast<std::size_t>(c)].next_sibling)
        {
            if (c == before)
            {
                break;
            }
            prev = c;
        }
        doc.nodes[static_cast<std::size_t>(index)].next_sibling = before;
        if (prev == k_no_node)
        {
            p.first_child = index;
        }
        else
        {
            doc.nodes[static_cast<std::size_t>(prev)].next_sibling = index;
        }
    }

    int foster_table() const
    {
        if (stack.empty() || !is_table_section_or_row(
                doc.nodes[static_cast<std::size_t>(stack.back())].tag))
        {
            return k_no_node;
        }
        for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
        {
            int node = stack[static_cast<std::size_t>(depth)];
            std::string_view tag = doc.nodes[static_cast<std::size_t>(node)].tag;
            if (tag == "table")
            {
                return node;
            }
            if (!is_table_section_or_row(tag) && tag != "template")
            {
                return k_no_node;
            }
        }
        return k_no_node;
    }

    int open_element(std::string_view tag)
    {
        int elem = push_node(NodeKind::k_element);
        doc.nodes[static_cast<std::size_t>(elem)].tag = tag;
        return elem;
    }

    void set_attrs(int elem, const std::vector<Attr>& attrs)
    {
        std::uint32_t abegin = static_cast<std::uint32_t>(doc.attrs.size());
        for (const auto& attr : attrs)
        {
            if (!attr.name.empty())
            {
                doc.attrs.push_back(attr);
            }
        }
        doc.nodes[static_cast<std::size_t>(elem)].attr_begin = abegin;
        doc.nodes[static_cast<std::size_t>(elem)].attr_end =
            static_cast<std::uint32_t>(doc.attrs.size());
    }

    void merge_attrs(int elem, const std::vector<Attr>& attrs)
    {
        for (const auto& attr : attrs)
        {
            if (attr.name.empty())
            {
                continue;
            }
            bool present = false;
            const Node& n = doc.nodes[static_cast<std::size_t>(elem)];
            for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
            {
                if (doc.attrs[i].name == attr.name)
                {
                    present = true;
                }
            }
            if (!present)
            {
                doc.attrs.push_back(attr);
                doc.nodes[static_cast<std::size_t>(elem)].attr_end =
                    static_cast<std::uint32_t>(doc.attrs.size());
            }
        }
    }

    void pop_to_tag(std::string_view tag)
    {
        for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
        {
            if (doc.nodes[static_cast<std::size_t>(stack[static_cast<std::size_t>(depth)])].tag == tag)
            {
                stack.resize(static_cast<std::size_t>(depth));
                return;
            }
        }
    }

    void pop_to_table_context()
    {
        while (!stack.empty())
        {
            std::string_view tag =
                doc.nodes[static_cast<std::size_t>(stack.back())].tag;
            if (tag == "table" || tag == "template" || tag == "html")
            {
                return;
            }
            stack.pop_back();
        }
    }

    void clear_to_marker()
    {
        while (!fmt.empty() && fmt.back() != k_marker)
        {
            fmt.pop_back();
        }
        if (!fmt.empty())
        {
            fmt.pop_back();
        }
    }

    void insert_formatting(std::string_view tag, const std::vector<Attr>& attrs)
    {
        int same = 0;
        for (int depth = static_cast<int>(fmt.size()) - 1; depth >= 0; --depth)
        {
            int entry = fmt[static_cast<std::size_t>(depth)];
            if (entry == k_marker)
            {
                break;
            }
            if (doc.nodes[static_cast<std::size_t>(entry)].tag == tag)
            {
                ++same;
            }
        }
        if (same >= 3)
        {
            for (std::size_t i = 0; i < fmt.size(); ++i)
            {
                int entry = fmt[i];
                if (entry != k_marker &&
                    doc.nodes[static_cast<std::size_t>(entry)].tag == tag)
                {
                    fmt.erase(fmt.begin() + static_cast<std::ptrdiff_t>(i));
                    break;
                }
            }
        }
        int elem = open_element(tag);
        set_attrs(elem, attrs);
        stack.push_back(elem);
        fmt.push_back(elem);
    }

    int copy_node(int source)
    {
        const Node& from = doc.nodes[static_cast<std::size_t>(source)];
        int elem = push_node(NodeKind::k_element);
        doc.nodes[static_cast<std::size_t>(elem)].tag = from.tag;
        std::uint32_t abegin = static_cast<std::uint32_t>(doc.attrs.size());
        for (std::uint32_t i = from.attr_begin; i < from.attr_end; ++i)
        {
            doc.attrs.push_back(doc.attrs[i]);
        }
        doc.nodes[static_cast<std::size_t>(elem)].attr_begin = abegin;
        doc.nodes[static_cast<std::size_t>(elem)].attr_end =
            static_cast<std::uint32_t>(doc.attrs.size());
        int parent = from.parent;
        doc.nodes[static_cast<std::size_t>(elem)].parent = k_no_node;
        doc.nodes[static_cast<std::size_t>(elem)].next_sibling = k_no_node;
        if (parent != k_no_node)
        {
            attach(elem, parent, k_no_node);
        }
        return elem;
    }

    void detach(int index)
    {
        int parent = doc.nodes[static_cast<std::size_t>(index)].parent;
        if (parent == k_no_node)
        {
            return;
        }
        Node& p = doc.nodes[static_cast<std::size_t>(parent)];
        if (p.first_child == index)
        {
            p.first_child = doc.nodes[static_cast<std::size_t>(index)].next_sibling;
        }
        else
        {
            for (int c = p.first_child; c != k_no_node;
                 c = doc.nodes[static_cast<std::size_t>(c)].next_sibling)
            {
                if (doc.nodes[static_cast<std::size_t>(c)].next_sibling == index)
                {
                    doc.nodes[static_cast<std::size_t>(c)].next_sibling =
                        doc.nodes[static_cast<std::size_t>(index)].next_sibling;
                    break;
                }
            }
        }
        doc.nodes[static_cast<std::size_t>(index)].parent = k_no_node;
        doc.nodes[static_cast<std::size_t>(index)].next_sibling = k_no_node;
    }

    void close_formatting(std::string_view tag)
    {
        if (!stack.empty() &&
            doc.nodes[static_cast<std::size_t>(stack.back())].tag == tag)
        {
            bool in_list = false;
            for (int entry : fmt)
            {
                if (entry != k_marker &&
                    doc.nodes[static_cast<std::size_t>(entry)].tag == tag)
                {
                    in_list = true;
                }
            }
            if (!in_list)
            {
                stack.pop_back();
                return;
            }
        }
        for (int outer = 0; outer < 8; ++outer)
        {
            int fe = k_no_node;
            for (int depth = static_cast<int>(fmt.size()) - 1; depth >= 0; --depth)
            {
                int entry = fmt[static_cast<std::size_t>(depth)];
                if (entry != k_marker &&
                    doc.nodes[static_cast<std::size_t>(entry)].tag == tag)
                {
                    fe = entry;
                    break;
                }
            }
            if (fe == k_no_node)
            {
                close_any(tag);
                return;
            }
            bool in_stack = false;
            for (int node : stack)
            {
                if (node == fe)
                {
                    in_stack = true;
                }
            }
            if (!in_stack)
            {
                for (auto it = fmt.begin(); it != fmt.end(); ++it)
                {
                    if (*it == fe)
                    {
                        fmt.erase(it);
                        break;
                    }
                }
                return;
            }
            if (!in_scope(tag, false, false))
            {
                return;
            }
            int furthest = k_no_node;
            bool past_fe = false;
            for (int node : stack)
            {
                if (node == fe)
                {
                    past_fe = true;
                    continue;
                }
                if (past_fe && is_special_tag(
                        doc.nodes[static_cast<std::size_t>(node)].tag))
                {
                    furthest = node;
                    break;
                }
            }
            if (furthest != k_no_node)
            {
                bool inside = false;
                int walk = furthest;
                for (int guard = 0; guard < 1000 && walk != k_no_node; ++guard)
                {
                    if (walk == fe)
                    {
                        inside = true;
                        break;
                    }
                    walk = doc.nodes[static_cast<std::size_t>(walk)].parent;
                }
                if (inside)
                {
                    while (!stack.empty())
                    {
                        int node = stack.back();
                        stack.pop_back();
                        if (node == fe)
                        {
                            break;
                        }
                    }
                    for (auto it = fmt.begin(); it != fmt.end(); ++it)
                    {
                        if (*it == fe)
                        {
                            fmt.erase(it);
                            break;
                        }
                    }
                    return;
                }
            }
            if (furthest == k_no_node)
            {
                while (!stack.empty())
                {
                    int node = stack.back();
                    stack.pop_back();
                    if (node == fe)
                    {
                        break;
                    }
                }
                for (auto it = fmt.begin(); it != fmt.end(); ++it)
                {
                    if (*it == fe)
                    {
                        fmt.erase(it);
                        break;
                    }
                }
                return;
            }
            int ancestor = k_no_node;
            for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
            {
                if (stack[static_cast<std::size_t>(depth)] == fe && depth > 0)
                {
                    ancestor = stack[static_cast<std::size_t>(depth) - 1];
                    break;
                }
            }
            std::size_t bookmark = 0;
            for (std::size_t i = 0; i < fmt.size(); ++i)
            {
                if (fmt[i] == fe)
                {
                    bookmark = i;
                }
            }
            int node = fe;
            int last = fe;
            for (int inner = 0; inner < 100; ++inner)
            {
                int above = k_no_node;
                for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
                {
                    if (stack[static_cast<std::size_t>(depth)] == node &&
                        depth + 1 < static_cast<int>(stack.size()))
                    {
                        above = stack[static_cast<std::size_t>(depth) + 1];
                        break;
                    }
                }
                node = above;
                if (node == k_no_node || node == furthest)
                {
                    break;
                }
                bool in_list = false;
                for (int entry : fmt)
                {
                    if (entry == node)
                    {
                        in_list = true;
                    }
                }
                if (inner > 3 && in_list)
                {
                    for (auto it = fmt.begin(); it != fmt.end(); ++it)
                    {
                        if (*it == node)
                        {
                            fmt.erase(it);
                            break;
                        }
                    }
                    in_list = false;
                }
                if (!in_list)
                {
                    for (auto it = stack.begin(); it != stack.end(); ++it)
                    {
                        if (*it == node)
                        {
                            stack.erase(it);
                            break;
                        }
                    }
                    continue;
                }
                int replacement = copy_node(node);
                for (std::size_t i = 0; i < fmt.size(); ++i)
                {
                    if (fmt[i] == node)
                    {
                        fmt[i] = replacement;
                    }
                }
                for (std::size_t i = 0; i < stack.size(); ++i)
                {
                    if (stack[i] == node)
                    {
                        stack[i] = replacement;
                    }
                }
                node = replacement;
                last = node;
            }
            detach(last);
            attach(last, furthest, k_no_node);
            int fresh = copy_node(fe);
            if (ancestor != k_no_node)
            {
                detach(fresh);
                attach(fresh, ancestor, k_no_node);
            }
            for (auto it = fmt.begin(); it != fmt.end(); ++it)
            {
                if (*it == fe)
                {
                    fmt.erase(it);
                    break;
                }
            }
            fmt.insert(fmt.begin() + static_cast<std::ptrdiff_t>(bookmark), fresh);
            for (auto it = stack.begin(); it != stack.end(); ++it)
            {
                if (*it == fe)
                {
                    stack.erase(it);
                    break;
                }
            }
            int at = 0;
            for (std::size_t i = 0; i < stack.size(); ++i)
            {
                if (stack[i] == furthest)
                {
                    at = static_cast<int>(i);
                    break;
                }
            }
            stack.insert(stack.begin() + at, fresh);
            for (auto it = stack.begin(); it != stack.end(); ++it)
            {
                if (*it == fe)
                {
                    stack.erase(it);
                    break;
                }
            }
            return;
        }
    }

    void close_any(std::string_view tag)
    {
        for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
        {
            std::string_view current =
                doc.nodes[static_cast<std::size_t>(stack[static_cast<std::size_t>(depth)])].tag;
            if (current == tag)
            {
                while (static_cast<int>(stack.size()) > depth)
                {
                    stack.pop_back();
                }
                return;
            }
            if (is_special_tag(current))
            {
                return;
            }
        }
    }

    void close_in_scope(std::string_view tag, bool button_scope, bool list_scope)
    {
        if (in_scope(tag, button_scope, list_scope))
        {
            pop_to_tag(tag);
        }
    }

    bool preserve_ws() const
    {
        if (stack.empty())
        {
            return false;
        }
        std::string_view tag =
            doc.nodes[static_cast<std::size_t>(stack.back())].tag;
        return tag == "pre" || tag == "listing" || tag == "textarea" ||
               tag == "script" || tag == "style";
    }

    bool raw_content() const
    {
        if (stack.empty())
        {
            return false;
        }
        std::string_view tag =
            doc.nodes[static_cast<std::size_t>(stack.back())].tag;
        return tag == "script" || tag == "style";
    }

    void push_text(std::string_view raw, bool is_raw, bool preserve)
    {
        if (!preserve)
        {
            std::size_t start = 0;
            while (start < raw.size() && is_space(raw[start]))
            {
                ++start;
            }
            std::size_t end = raw.size();
            while (end > start && is_space(raw[end - 1]))
            {
                --end;
            }
            if (start >= end)
            {
                return;
            }
            raw = raw.substr(start, end - start);
        }
        else if (raw.empty())
        {
            return;
        }
        if (is_raw)
        {
            std::string decoded;
            decoded.reserve(raw.size());
            decoded.append(raw.data(), raw.size());
            if (decoded.empty())
            {
                return;
            }
            std::size_t off = doc.source.size();
            doc.source.append(decoded);
            int index = push_node(NodeKind::k_text);
            doc.nodes[static_cast<std::size_t>(index)].text =
                std::string_view(doc.source.data() + off, decoded.size());
            int foster = foster_table();
            if (foster != k_no_node)
            {
                int parent = doc.nodes[static_cast<std::size_t>(foster)].parent;
                detach(index);
                attach(index, parent, foster);
            }
            return;
        }
        std::string decoded;
        decoded.reserve(raw.size());
        for (std::size_t i = 0; i < raw.size();)
        {
            if (raw[i] == '&')
            {
                std::size_t semi = raw.find(';', i + 1);
                if (semi != std::string_view::npos && semi - i < 14)
                {
                    std::string_view ent = raw.substr(i + 1, semi - i - 1);
                    if (!ent.empty() && ent[0] == '#')
                    {
                        char encoded[4];
                        std::size_t length = 0;
                        if (decode_numeric_entity(ent, encoded, length))
                        {
                            decoded.append(encoded, length);
                            i = semi + 1;
                            continue;
                        }
                    }
                    else
                    {
                        std::string_view rep = decode_entity(ent);
                        if (!rep.empty())
                        {
                            decoded.append(rep);
                            i = semi + 1;
                            continue;
                        }
                    }
                }
            }
            decoded.push_back(raw[i]);
            ++i;
        }
        if (decoded.empty())
        {
            return;
        }
        std::size_t off = doc.source.size();
        doc.source.append(decoded);
        int index = push_node(NodeKind::k_text);
        doc.nodes[static_cast<std::size_t>(index)].text =
            std::string_view(doc.source.data() + off, decoded.size());
        int foster = foster_table();
        if (foster != k_no_node)
        {
            int parent = doc.nodes[static_cast<std::size_t>(foster)].parent;
            detach(index);
            attach(index, parent, foster);
        }
    }

    void ensure_html()
    {
        if (stack.empty())
        {
            int elem = push_node(NodeKind::k_element);
            doc.nodes[static_cast<std::size_t>(elem)].tag = "html";
            stack.push_back(elem);
        }
    }

    void ensure_body()
    {
        ensure_html();
        while (!stack.empty() && top_tag_is("head"))
        {
            stack.pop_back();
        }
        if (!top_tag_is("body"))
        {
            int elem = open_element("body");
            stack.push_back(elem);
        }
    }

    int find_child(std::string_view tag) const
    {
        if (stack.empty())
        {
            return k_no_node;
        }
        int parent = stack.back();
        for (int c = doc.nodes[static_cast<std::size_t>(parent)].first_child; c != k_no_node;
             c = doc.nodes[static_cast<std::size_t>(c)].next_sibling)
        {
            if (doc.nodes[static_cast<std::size_t>(c)].kind == NodeKind::k_element &&
                doc.nodes[static_cast<std::size_t>(c)].tag == tag)
            {
                return c;
            }
        }
        return k_no_node;
    }

    void close_paragraph()
    {
        if (in_scope("p", true, false))
        {
            pop_to_tag("p");
        }
    }

    void handle_close(std::string_view tag)
    {
        if (tag == "br")
        {
            int elem = open_element("br");
            (void)elem;
            return;
        }
        if (tag == "p")
        {
            if (!in_scope("p", true, false))
            {
                int elem = open_element("p");
                stack.push_back(elem);
            }
            pop_to_tag("p");
            return;
        }
        if (tag == "li" || tag == "dt" || tag == "dd")
        {
            close_in_scope(tag, false, tag == "li");
            return;
        }
        if (tag == "option" || tag == "optgroup")
        {
            if (top_tag_is(tag))
            {
                stack.pop_back();
            }
            else
            {
                close_in_scope(tag, false, false);
            }
            return;
        }
        if (tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" || tag == "h5" ||
            tag == "h6")
        {
            close_in_scope(tag, false, false);
            return;
        }
        if (tag == "body" || tag == "html")
        {
            if (in_scope("body", false, false))
            {
                while (!stack.empty() && !top_tag_is("body"))
                {
                    stack.pop_back();
                }
            }
            return;
        }
        if (tag == "form")
        {
            close_in_scope("form", false, false);
            return;
        }
        if (tag == "button")
        {
            close_in_scope("button", false, false);
            return;
        }
        if (tag == "marquee" || tag == "applet" || tag == "object")
        {
            if (in_scope(tag, false, false))
            {
                pop_to_tag(tag);
                clear_to_marker();
            }
            return;
        }
        if (tag == "select")
        {
            if (in_scope("select", false, false))
            {
                pop_to_tag("select");
            }
            return;
        }
        if (tag == "table" || tag == "caption" || tag == "colgroup" || tag == "thead" ||
            tag == "tbody" || tag == "tfoot" || tag == "tr" || tag == "td" ||
            tag == "th")
        {
            if (in_table_scope(tag))
            {
                pop_to_tag(tag);
            }
            return;
        }
        if (tag == "template")
        {
            close_in_scope("template", false, false);
            return;
        }
        if (is_formatting_tag(tag) || tag == "nobr")
        {
            close_formatting(tag);
            return;
        }
        if (is_special_tag(tag))
        {
            close_in_scope(tag, false, false);
            return;
        }
        close_any(tag);
    }

    bool foster_exempt(std::string_view tag, const std::vector<Attr>& attrs) const
    {
        if (is_table_part(tag) || tag == "form")
        {
            return true;
        }
        if (tag == "script" || tag == "style" || tag == "template")
        {
            return true;
        }
        if (tag == "input")
        {
            for (const auto& attr : attrs)
            {
                if (attr.name == "type" && attr.value == "hidden")
                {
                    return true;
                }
            }
        }
        return false;
    }

    void open_in_table(std::string_view tag, const std::vector<Attr>& attrs, bool)
    {
        if (tag == "caption" || tag == "colgroup" || tag == "thead" || tag == "tbody" ||
            tag == "tfoot")
        {
            if (!in_table_scope("table"))
            {
                return;
            }
            pop_to_table_context();
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (tag == "col")
        {
            if (top_tag_is("colgroup"))
            {
                int elem = open_element(tag);
                set_attrs(elem, attrs);
            }
            return;
        }
        if (tag == "tr")
        {
            if (!in_table_scope("table"))
            {
                return;
            }
            if (top_tag_is("table"))
            {
                int section = open_element("tbody");
                stack.push_back(section);
            }
            else
            {
                std::string_view top =
                    doc.nodes[static_cast<std::size_t>(stack.back())].tag;
                if (top != "tbody" && top != "thead" && top != "tfoot" &&
                    top != "template" && top != "html")
                {
                    pop_to_table_context();
                    if (!top_tag_is("table"))
                    {
                        return;
                    }
                    int section = open_element("tbody");
                    stack.push_back(section);
                }
            }
            if (in_table_scope("tr"))
            {
                pop_to_tag("tr");
            }
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (tag == "td" || tag == "th")
        {
            if (!in_table_scope("table"))
            {
                return;
            }
            if (top_tag_is("td") || top_tag_is("th"))
            {
                stack.pop_back();
            }
            std::string_view top =
                doc.nodes[static_cast<std::size_t>(stack.back())].tag;
            if (top != "tr")
            {
                open_in_table("tr", {}, false);
                if (!top_tag_is("tr"))
                {
                    return;
                }
            }
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (tag == "form")
        {
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            return;
        }
    }

    bool open_in_select(std::string_view tag, const std::vector<Attr>& attrs, bool self_close)
    {
        if (tag == "option" || tag == "optgroup")
        {
            if (top_tag_is("option"))
            {
                stack.pop_back();
            }
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            if (!self_close)
            {
                stack.push_back(elem);
            }
            return true;
        }
        if (tag == "select")
        {
            if (in_scope("select", false, false))
            {
                pop_to_tag("select");
            }
            return false;
        }
        if (tag == "hr" || tag == "script" || tag == "template")
        {
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            if (tag == "template" || (!self_close && !is_void_tag(tag)))
            {
                stack.push_back(elem);
            }
            return true;
        }
        return false;
    }

    void open_element_push(std::string_view tag, const std::vector<Attr>& attrs,
                           bool self_close)
    {
        int elem = open_element(tag);
        set_attrs(elem, attrs);
        if (!self_close && !is_void_tag(tag))
        {
            stack.push_back(elem);
        }
    }

    void dispatch_open(std::string_view tag, std::vector<Attr>& attrs, bool self_close)
    {
        if (tag == "html")
        {
            if (stack.empty())
            {
                int elem = push_node(NodeKind::k_element);
                doc.nodes[static_cast<std::size_t>(elem)].tag = tag;
                set_attrs(elem, attrs);
                stack.push_back(elem);
            }
            else
            {
                merge_attrs(stack.front(), attrs);
            }
            return;
        }
        ensure_html();
        if (tag == "head")
        {
            if (find_child("head") != k_no_node)
            {
                return;
            }
            while (!stack.empty() && !top_tag_is("html"))
            {
                stack.pop_back();
            }
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (tag == "body")
        {
            while (!stack.empty() && !top_tag_is("html"))
            {
                stack.pop_back();
            }
            int existing = find_child("body");
            if (existing != k_no_node)
            {
                merge_attrs(existing, attrs);
                return;
            }
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (tag == "plaintext")
        {
            ensure_body();
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (top_tag_is("html") || top_tag_is("head"))
        {
            if (tag != "base" && tag != "basefont" && tag != "bgsound" && tag != "link" &&
                tag != "meta" && tag != "noframes" && tag != "script" && tag != "style" &&
                tag != "template" && tag != "title" && tag != "head" && tag != "html")
            {
                while (!stack.empty() && top_tag_is("head"))
                {
                    stack.pop_back();
                }
                if (!top_tag_is("body"))
                {
                    int elem = open_element("body");
                    stack.push_back(elem);
                }
            }
        }
        if (tag == "table")
        {
            close_paragraph();
            int elem = open_element(tag);
            set_attrs(elem, attrs);
            stack.push_back(elem);
            return;
        }
        if (foster_table() != k_no_node && !foster_exempt(tag, attrs))
        {
            int table = foster_table();
            int parent = doc.nodes[static_cast<std::size_t>(table)].parent;
            int elem = push_node(NodeKind::k_element);
            doc.nodes[static_cast<std::size_t>(elem)].tag = tag;
            doc.nodes[static_cast<std::size_t>(elem)].parent = k_no_node;
            doc.nodes[static_cast<std::size_t>(elem)].next_sibling = k_no_node;
            attach(elem, parent, table);
            set_attrs(elem, attrs);
            if (!self_close && !is_void_tag(tag) && !is_raw_text_tag(tag))
            {
                stack.push_back(elem);
            }
            return;
        }
        if (is_table_part(tag) || tag == "form")
        {
            open_in_table(tag, attrs, self_close);
            return;
        }
        if (in_select())
        {
            if (open_in_select(tag, attrs, self_close))
            {
                return;
            }
            if (tag != "input" && tag != "keygen" && tag != "textarea")
            {
                return;
            }
        }
        if (tag == "select")
        {
            if (in_scope("select", false, false))
            {
                pop_to_tag("select");
            }
            open_element_push(tag, attrs, self_close);
            return;
        }
        if (closes_paragraph(tag))
        {
            close_paragraph();
        }
        if (tag == "li")
        {
            if (top_tag_is("li"))
            {
                stack.pop_back();
            }
        }
        else if (tag == "dt" || tag == "dd")
        {
            if (top_tag_is("dt") || top_tag_is("dd"))
            {
                stack.pop_back();
            }
        }
        else if (tag == "option" || tag == "optgroup")
        {
            if (top_tag_is("option"))
            {
                stack.pop_back();
            }
        }
        else if (tag == "button")
        {
            if (in_scope("button", false, false))
            {
                close_in_scope("button", false, false);
            }
        }
        else if (tag == "form")
        {
            if (in_scope("form", false, false))
            {
                return;
            }
        }
        else if (tag == "a")
        {
            bool present = false;
            for (int entry : fmt)
            {
                if (entry != k_marker &&
                    doc.nodes[static_cast<std::size_t>(entry)].tag == "a")
                {
                    present = true;
                }
            }
            if (present)
            {
                close_formatting("a");
            }
        }
        else if (tag == "nobr")
        {
            bool present = false;
            for (int entry : fmt)
            {
                if (entry != k_marker &&
                    doc.nodes[static_cast<std::size_t>(entry)].tag == "nobr")
                {
                    present = true;
                }
            }
            if (present)
            {
                close_formatting("nobr");
            }
        }
        else if (tag == "marquee" || tag == "applet" || tag == "object")
        {
            fmt.push_back(k_marker);
        }
        if (is_formatting_tag(tag) || tag == "nobr")
        {
            insert_formatting(tag, attrs);
            return;
        }
        open_element_push(tag, attrs, self_close);
    }

    void parse_tag(std::size_t& i)
    {
        const std::string& s = doc.source;
        std::size_t n = s.size();
        if (i + 3 < n && s[i + 1] == '!' && s[i + 2] == '-' && s[i + 3] == '-')
        {
            std::size_t end = s.find("-->", i + 4);
            i = (end == std::string::npos) ? n : end + 3;
            return;
        }
        if (i + 8 < n && s[i + 1] == '!' && s[i + 2] == '[' && s.compare(i + 3, 6, "CDATA[") == 0)
        {
            std::size_t end = s.find("]]>", i + 9);
            i = (end == std::string::npos) ? n : end + 3;
            return;
        }
        if (i + 1 < n && (s[i + 1] == '!' || s[i + 1] == '?'))
        {
            std::size_t end = s.find('>', i + 2);
            i = (end == std::string::npos) ? n : end + 1;
            return;
        }
        bool closing = (i + 1 < n && s[i + 1] == '/');
        std::size_t p = i + (closing ? 2 : 1);
        while (p < n && is_space(s[p]))
        {
            ++p;
        }
        std::size_t name_begin = p;
        while (p < n && !is_space(s[p]) && s[p] != '>' && s[p] != '/')
        {
            ++p;
        }
        to_lower_inplace(doc.source, name_begin, p);
        std::string_view raw_tag(doc.source.data() + name_begin, p - name_begin);
        std::string_view tag = (raw_tag == "image") ? std::string_view("img") : raw_tag;
        if (tag.empty())
        {
            std::size_t end = s.find('>', p);
            i = (end == std::string::npos) ? n : end + 1;
            return;
        }
        if (closing)
        {
            std::size_t end = s.find('>', p);
            i = (end == std::string::npos) ? n : end + 1;
            handle_close(tag);
            return;
        }
        std::vector<Attr> attrs;
        bool self_close = false;
        while (p < n && s[p] != '>')
        {
            while (p < n && is_space(s[p]))
            {
                ++p;
            }
            if (p >= n || s[p] == '>')
            {
                break;
            }
            if (s[p] == '/' && p + 1 < n && s[p + 1] == '>')
            {
                self_close = true;
                break;
            }
            std::size_t an_begin = p;
            while (p < n && !is_space(s[p]) && s[p] != '=' && s[p] != '>' && s[p] != '/')
            {
                ++p;
            }
            to_lower_inplace(doc.source, an_begin, p);
            std::string_view aname(doc.source.data() + an_begin, p - an_begin);
            while (p < n && is_space(s[p]))
            {
                ++p;
            }
            std::string_view avalue;
            if (p < n && s[p] == '=')
            {
                ++p;
                while (p < n && is_space(s[p]))
                {
                    ++p;
                }
                if (p < n && (s[p] == '"' || s[p] == '\''))
                {
                    char q = s[p];
                    ++p;
                    std::size_t vbegin = p;
                    while (p < n && s[p] != q)
                    {
                        ++p;
                    }
                    avalue = std::string_view(doc.source.data() + vbegin, p - vbegin);
                    if (p < n)
                    {
                        ++p;
                    }
                }
                else
                {
                    std::size_t vbegin = p;
                    while (p < n && !is_space(s[p]) && s[p] != '>')
                    {
                        ++p;
                    }
                    avalue = std::string_view(doc.source.data() + vbegin, p - vbegin);
                }
            }
            if (!aname.empty())
            {
                attrs.push_back(Attr{aname, avalue});
            }
        }
        std::size_t end = s.find('>', p);
        i = (end == std::string::npos) ? n : end + 1;
        if (tag == "plaintext")
        {
            dispatch_open(tag, attrs, self_close);
            push_text(std::string_view(doc.source.data() + i, n - i), true, true);
            i = n;
            return;
        }
        if (is_raw_text_tag(tag))
        {
            std::size_t cpos = n;
            for (std::size_t at = i; at + tag.size() + 2 <= n; ++at)
            {
                if (doc.source[at] != '<' || doc.source[at + 1] != '/')
                {
                    continue;
                }
                bool match = true;
                for (std::size_t k = 0; k < tag.size(); ++k)
                {
                    char a = doc.source[at + 2 + k];
                    if (a >= 'A' && a <= 'Z')
                    {
                        a = static_cast<char>(a + ('a' - 'A'));
                    }
                    if (a != tag[k])
                    {
                        match = false;
                        break;
                    }
                }
                if (!match)
                {
                    continue;
                }
                std::size_t after = at + tag.size() + 2;
                if (after < n && (s[after] == '>' || s[after] == '/' || is_space(s[after])))
                {
                    cpos = at;
                    break;
                }
            }
            dispatch_open(tag, attrs, self_close);
            bool preserve = true;
            bool raw = (tag == "script" || tag == "style");
            std::size_t text_begin = i;
            if (tag == "textarea" && text_begin < n && doc.source[text_begin] == '\n')
            {
                ++text_begin;
            }
            push_text(std::string_view(doc.source.data() + text_begin, cpos - text_begin),
                      raw, preserve);
            if (cpos < n)
            {
                std::size_t cend = doc.source.find('>', cpos);
                i = (cend == std::string::npos) ? n : cend + 1;
                for (int depth = static_cast<int>(stack.size()) - 1; depth >= 0; --depth)
                {
                    if (doc.nodes[static_cast<std::size_t>(stack[static_cast<std::size_t>(depth)])].tag == tag)
                    {
                        stack.resize(static_cast<std::size_t>(depth));
                        break;
                    }
                }
            }
            else
            {
                i = n;
            }
            return;
        }
        if (tag == "pre" || tag == "listing")
        {
            dispatch_open(tag, attrs, self_close);
            if (i < n && doc.source[i] == '\n')
            {
                ++i;
            }
            return;
        }
        dispatch_open(tag, attrs, self_close);
    }

    bool has_text(std::string_view raw) const
    {
        for (char c : raw)
        {
            if (!is_space(c))
            {
                return true;
            }
        }
        return false;
    }

    void run()
    {
        const std::string& s = doc.source;
        std::size_t n = s.size();
        std::size_t i = 0;
        std::size_t text_begin = 0;
        bool in_text = true;
        while (i < n)
        {
            if (s[i] == '<')
            {
                if (in_text && i > text_begin)
                {
                    std::string_view raw(s.data() + text_begin, i - text_begin);
                    bool preserve = preserve_ws();
                    bool content_raw = raw_content();
                    if (!preserve && !content_raw)
                    {
                        if (has_text(raw))
                        {
                            if (stack.empty())
                            {
                                ensure_html();
                                ensure_body();
                            }
                            else if (!stack.empty() && top_tag_is("head"))
                            {
                                stack.pop_back();
                                ensure_body();
                            }
                            push_text(raw, false, false);
                        }
                    }
                    else
                    {
                        push_text(raw, content_raw, preserve);
                    }
                }
                in_text = false;
                parse_tag(i);
                text_begin = i;
                in_text = true;
            }
            else
            {
                ++i;
            }
        }
        if (in_text && n > text_begin)
        {
            std::string_view raw(s.data() + text_begin, n - text_begin);
            if (has_text(raw) || preserve_ws() || raw_content())
            {
                push_text(raw, raw_content(), preserve_ws());
            }
        }
    }
};

} // namespace detail

Document parse_html(std::string_view src)
{
    detail::Parser p;
    p.doc.source.assign(src.data(), src.size());
    p.doc.source.reserve(src.size() * 2 + 64);
    p.run();
    return std::move(p.doc);
}

} // namespace wv
