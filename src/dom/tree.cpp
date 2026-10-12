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

bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

} // namespace detail

std::string_view Document::attr(int index, std::string_view name) const
{
    const Node& n = nodes[static_cast<std::size_t>(index)];
    for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
    {
        if (attrs[i].name == name)
        {
            return attrs[i].value;
        }
    }
    return {};
}

bool Document::has_attr(int index, std::string_view name) const
{
    const Node& n = nodes[static_cast<std::size_t>(index)];
    for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
    {
        if (attrs[i].name == name)
        {
            return true;
        }
    }
    return false;
}

bool Document::has_class(int index, std::string_view cls) const
{
    std::string_view classes = attr(index, "class");
    std::size_t i = 0;
    while (i < classes.size())
    {
        while (i < classes.size() && detail::is_space(classes[i]))
        {
            ++i;
        }
        std::size_t j = i;
        while (j < classes.size() && !detail::is_space(classes[j]))
        {
            ++j;
        }
        if (j > i && classes.substr(i, j - i) == cls)
        {
            return true;
        }
        i = j;
    }
    return false;
}

std::string_view Document::text_of(int index) const
{
    const Node& n = nodes[static_cast<std::size_t>(index)];
    if (n.kind == NodeKind::k_text)
    {
        return n.text;
    }
    return {};
}

int Document::first_element_by_tag(int from, std::string_view tag) const
{
    for (std::size_t i = static_cast<std::size_t>(from); i < nodes.size(); ++i)
    {
        const Node& n = nodes[i];
        if (n.kind == NodeKind::k_element && n.tag == tag &&
            (n.parent != k_no_node || static_cast<int>(i) == root_))
        {
            return static_cast<int>(i);
        }
    }
    return k_no_node;
}

int Document::first_element_by_class(int from, std::string_view cls) const
{
    for (std::size_t i = static_cast<std::size_t>(from); i < nodes.size(); ++i)
    {
        const Node& n = nodes[i];
        if (n.kind == NodeKind::k_element &&
            (n.parent != k_no_node || static_cast<int>(i) == root_) &&
            has_class(static_cast<int>(i), cls))
        {
            return static_cast<int>(i);
        }
    }
    return k_no_node;
}

void Document::dump(int index, std::string& out, int depth) const
{
    const Node& n = nodes[static_cast<std::size_t>(index)];
    for (int d = 0; d < depth; ++d)
    {
        out += "  ";
    }
    if (n.kind == NodeKind::k_text)
    {
        out += "#text ";
        out.append(n.text.substr(0, 40));
        out += "\n";
    }
    else
    {
        out += "<";
        out.append(n.tag);
        std::string_view id = attr(index, "id");
        std::string_view cl = attr(index, "class");
        if (!id.empty())
        {
            out += " #";
            out.append(id.substr(0, 24));
        }
        if (!cl.empty())
        {
            out += " .";
            out.append(cl.substr(0, 48));
        }
        out += ">\n";
    }
    for (int c = n.first_child; c != k_no_node; c = nodes[static_cast<std::size_t>(c)].next_sibling)
    {
        dump(c, out, depth + 1);
    }
}

} // namespace wv
