/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "wview/dom.hpp"

#include "css/internal.hpp"
#include "wview/css.hpp"

namespace wv
{

int Document::first_element_child(int index) const
{
    if (index < 0 || static_cast<std::size_t>(index) >= nodes.size())
    {
        return k_no_node;
    }
    for (int c = nodes[static_cast<std::size_t>(index)].first_child; c != k_no_node;
         c = nodes[static_cast<std::size_t>(c)].next_sibling)
    {
        if (nodes[static_cast<std::size_t>(c)].kind == NodeKind::k_element)
        {
            return c;
        }
    }
    return k_no_node;
}

int Document::next_element_sibling(int index) const
{
    if (index < 0 || static_cast<std::size_t>(index) >= nodes.size())
    {
        return k_no_node;
    }
    for (int s = nodes[static_cast<std::size_t>(index)].next_sibling; s != k_no_node;
         s = nodes[static_cast<std::size_t>(s)].next_sibling)
    {
        if (nodes[static_cast<std::size_t>(s)].kind == NodeKind::k_element)
        {
            return s;
        }
    }
    return k_no_node;
}

int Document::previous_element_sibling(int index) const
{
    if (index < 0 || static_cast<std::size_t>(index) >= nodes.size())
    {
        return k_no_node;
    }
    int parent = nodes[static_cast<std::size_t>(index)].parent;
    if (parent < 0)
    {
        return k_no_node;
    }
    int prev = k_no_node;
    for (int s = nodes[static_cast<std::size_t>(parent)].first_child; s != k_no_node && s != index;
         s = nodes[static_cast<std::size_t>(s)].next_sibling)
    {
        if (nodes[static_cast<std::size_t>(s)].kind == NodeKind::k_element)
        {
            prev = s;
        }
    }
    return prev;
}

int Document::child_element_count(int index) const
{
    if (index < 0 || static_cast<std::size_t>(index) >= nodes.size())
    {
        return 0;
    }
    int count = 0;
    for (int c = nodes[static_cast<std::size_t>(index)].first_child; c != k_no_node;
         c = nodes[static_cast<std::size_t>(c)].next_sibling)
    {
        if (nodes[static_cast<std::size_t>(c)].kind == NodeKind::k_element)
        {
            ++count;
        }
    }
    return count;
}

int Document::query_selector(int root, std::string_view selectors) const
{
    std::vector<int> out;
    query_selector_all(root, selectors, out);
    return out.empty() ? k_no_node : out[0];
}

void Document::query_selector_all(int root, std::string_view selectors,
                                  std::vector<int>& out) const
{
    out.clear();
    if (root < 0 || static_cast<std::size_t>(root) >= nodes.size() || selectors.empty())
    {
        return;
    }
    std::vector<Selector> list;
    char leading = ' ';
    detail::parse_selector_list(selectors, list, leading);
    if (list.empty())
    {
        return;
    }
    ElementState state;
    std::vector<int> work;
    std::vector<int> seed;
    for (int c = nodes[static_cast<std::size_t>(root)].first_child; c != k_no_node;
         c = nodes[static_cast<std::size_t>(c)].next_sibling)
    {
        seed.push_back(c);
    }
    for (int i = static_cast<int>(seed.size()) - 1; i >= 0; --i)
    {
        work.push_back(seed[static_cast<std::size_t>(i)]);
    }
    while (!work.empty())
    {
        int cur = work.back();
        work.pop_back();
        const Node& n = nodes[static_cast<std::size_t>(cur)];
        if (n.kind == NodeKind::k_element)
        {
            for (const auto& sel : list)
            {
                if (match_selector(*this, cur, sel, state, std::string()))
                {
                    out.push_back(cur);
                    break;
                }
            }
        }
        std::vector<int> kids;
        for (int c = n.first_child; c != k_no_node;
             c = nodes[static_cast<std::size_t>(c)].next_sibling)
        {
            kids.push_back(c);
        }
        for (int i = static_cast<int>(kids.size()) - 1; i >= 0; --i)
        {
            work.push_back(kids[static_cast<std::size_t>(i)]);
        }
    }
}

} // namespace wv
