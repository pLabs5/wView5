/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "wview/dom.hpp"

namespace wv
{
namespace
{

bool valid_node(const Document& doc, int index)
{
    return index >= 0 && static_cast<std::size_t>(index) < doc.node_count();
}

std::string_view store_string(Document& doc, std::string_view text)
{
    doc.source.reserve(doc.source.size() + text.size() + 64);
    std::size_t off = doc.source.size();
    doc.source.append(text.data(), text.size());
    return std::string_view(doc.source.data() + off, text.size());
}

void unlink(Document& doc, int index)
{
    Node& n = const_cast<Node&>(doc.node(index));
    int parent = n.parent;
    if (parent == k_no_node)
    {
        return;
    }
    Node& p = const_cast<Node&>(doc.node(parent));
    if (p.first_child == index)
    {
        p.first_child = n.next_sibling;
    }
    else
    {
        for (int c = p.first_child; c != k_no_node;
             c = const_cast<Node&>(doc.node(c)).next_sibling)
        {
            if (doc.node(c).next_sibling == index)
            {
                const_cast<Node&>(doc.node(c)).next_sibling = n.next_sibling;
                break;
            }
        }
    }
    n.parent = k_no_node;
    n.next_sibling = k_no_node;
}

bool is_ancestor(const Document& doc, int node, int maybe_ancestor)
{
    for (int p = doc.node(node).parent; p != k_no_node; p = doc.node(p).parent)
    {
        if (p == maybe_ancestor)
        {
            return true;
        }
    }
    return false;
}

bool set_class_value(Document& doc, int elem, std::string_view classes)
{
    Node& n = const_cast<Node&>(doc.node(elem));
    for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
    {
        if (doc.attrs[i].name == "class")
        {
            doc.attrs[i].value = store_string(doc, classes);
            return true;
        }
    }
    if (n.attr_end != doc.attrs.size())
    {
        std::uint32_t begin = static_cast<std::uint32_t>(doc.attrs.size());
        for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
        {
            if (!doc.attrs[i].name.empty())
            {
                doc.attrs.push_back(doc.attrs[i]);
            }
        }
        n.attr_begin = begin;
        n.attr_end = begin;
    }
    std::string_view name = store_string(doc, "class");
    std::string_view value = store_string(doc, classes);
    doc.attrs.push_back(Attr{name, value});
    n.attr_end = static_cast<std::uint32_t>(doc.attrs.size());
    return true;
}

} // namespace

int Document::create_element(std::string_view tag)
{
    if (tag.empty())
    {
        return k_no_node;
    }
    Node n;
    n.kind = NodeKind::k_element;
    n.tag = store_string(*this, tag);
    int index = static_cast<int>(nodes.size());
    nodes.push_back(n);
    return index;
}

int Document::create_text_node(std::string_view text)
{
    Node n;
    n.kind = NodeKind::k_text;
    n.text = store_string(*this, text);
    int index = static_cast<int>(nodes.size());
    nodes.push_back(n);
    return index;
}

bool Document::append_child(int parent, int child)
{
    if (!valid_node(*this, parent) || !valid_node(*this, child) || parent == child)
    {
        return false;
    }
    if (nodes[static_cast<std::size_t>(parent)].kind != NodeKind::k_element)
    {
        return false;
    }
    if (is_ancestor(*this, parent, child))
    {
        return false;
    }
    unlink(*this, child);
    Node& p = const_cast<Node&>(node(parent));
    Node& c = const_cast<Node&>(node(child));
    c.parent = parent;
    c.next_sibling = k_no_node;
    if (p.first_child == k_no_node)
    {
        p.first_child = child;
    }
    else
    {
        int sib = p.first_child;
        while (node(sib).next_sibling != k_no_node)
        {
            sib = node(sib).next_sibling;
        }
        const_cast<Node&>(node(sib)).next_sibling = child;
    }
    return true;
}

bool Document::insert_before(int parent, int child, int before)
{
    if (!valid_node(*this, parent) || !valid_node(*this, child) || parent == child)
    {
        return false;
    }
    if (nodes[static_cast<std::size_t>(parent)].kind != NodeKind::k_element)
    {
        return false;
    }
    if (is_ancestor(*this, parent, child))
    {
        return false;
    }
    bool found = false;
    for (int c = nodes[static_cast<std::size_t>(parent)].first_child; c != k_no_node;
         c = nodes[static_cast<std::size_t>(c)].next_sibling)
    {
        if (c == before)
        {
            found = true;
        }
    }
    if (!found)
    {
        return false;
    }
    unlink(*this, child);
    Node& p = const_cast<Node&>(node(parent));
    Node& c = const_cast<Node&>(node(child));
    c.parent = parent;
    c.next_sibling = before;
    if (p.first_child == before)
    {
        p.first_child = child;
    }
    else
    {
        for (int s = p.first_child; s != k_no_node;
             s = const_cast<Node&>(node(s)).next_sibling)
        {
            if (node(s).next_sibling == before)
            {
                const_cast<Node&>(node(s)).next_sibling = child;
                break;
            }
        }
    }
    return true;
}

bool Document::remove_child(int parent, int child)
{
    if (!valid_node(*this, parent) || !valid_node(*this, child))
    {
        return false;
    }
    if (nodes[static_cast<std::size_t>(child)].parent != parent)
    {
        return false;
    }
    unlink(*this, child);
    return true;
}

bool Document::set_attribute(int elem, std::string_view name, std::string_view value)
{
    if (!valid_node(*this, elem) || name.empty())
    {
        return false;
    }
    if (nodes[static_cast<std::size_t>(elem)].kind != NodeKind::k_element)
    {
        return false;
    }
    Node& n = const_cast<Node&>(node(elem));
    for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
    {
        if (!attrs[i].name.empty() && attrs[i].name == name)
        {
            attrs[i].value = store_string(*this, value);
            return true;
        }
    }
    std::string_view stored_name = store_string(*this, name);
    std::string_view stored_value = store_string(*this, value);
    if (n.attr_end != attrs.size())
    {
        std::uint32_t begin = static_cast<std::uint32_t>(attrs.size());
        for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
        {
            if (!attrs[i].name.empty())
            {
                attrs.push_back(attrs[i]);
            }
        }
        n.attr_begin = begin;
        n.attr_end = begin;
    }
    attrs.push_back(Attr{stored_name, stored_value});
    n.attr_end = static_cast<std::uint32_t>(attrs.size());
    return true;
}

bool Document::remove_attribute(int elem, std::string_view name)
{
    if (!valid_node(*this, elem) || name.empty())
    {
        return false;
    }
    Node& n = const_cast<Node&>(node(elem));
    for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
    {
        if (!attrs[i].name.empty() && attrs[i].name == name)
        {
            attrs[i].name = std::string_view();
            attrs[i].value = std::string_view();
            return true;
        }
    }
    return false;
}

bool Document::set_text(int node, std::string_view text)
{
    if (!valid_node(*this, node))
    {
        return false;
    }
    Node& n = const_cast<Node&>(this->node(node));
    if (n.kind == NodeKind::k_text)
    {
        n.text = store_string(*this, text);
        return true;
    }
    int child = n.first_child;
    while (child != k_no_node)
    {
        int next = nodes[static_cast<std::size_t>(child)].next_sibling;
        unlink(*this, child);
        child = next;
    }
    int text_node = create_text_node(text);
    if (text_node < 0)
    {
        return false;
    }
    return append_child(node, text_node);
}

bool Document::add_class(int elem, std::string_view cls)
{
    if (!valid_node(*this, elem) || cls.empty())
    {
        return false;
    }
    if (has_class(elem, cls))
    {
        return true;
    }
    std::string current(attr(elem, "class"));
    if (!current.empty())
    {
        current.push_back(' ');
    }
    current.append(cls.data(), cls.size());
    return set_class_value(*this, elem, current);
}

bool Document::remove_class(int elem, std::string_view cls)
{
    if (!valid_node(*this, elem) || cls.empty())
    {
        return false;
    }
    std::string_view current = attr(elem, "class");
    std::string rebuilt;
    std::size_t i = 0;
    while (i < current.size())
    {
        while (i < current.size() && current[i] == ' ')
        {
            ++i;
        }
        std::size_t j = i;
        while (j < current.size() && current[j] != ' ')
        {
            ++j;
        }
        if (j > i && current.substr(i, j - i) != cls)
        {
            if (!rebuilt.empty())
            {
                rebuilt.push_back(' ');
            }
            rebuilt.append(current.data() + i, j - i);
        }
        i = j;
    }
    return set_class_value(*this, elem, rebuilt);
}

bool Document::toggle_class(int elem, std::string_view cls)
{
    if (!valid_node(*this, elem) || cls.empty())
    {
        return false;
    }
    if (has_class(elem, cls))
    {
        return !remove_class(elem, cls);
    }
    return add_class(elem, cls);
}

} // namespace wv
