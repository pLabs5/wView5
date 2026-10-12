/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "css/internal.hpp"

#include "wview/css.hpp"
#include "wview/dom.hpp"

namespace wv
{
namespace detail
{

bool match_compound(const Document& doc, int node, const Compound& c, const ElementState& state, const std::string& pseudo)
{
    const Node& n = doc.node(node);
    if (n.kind != NodeKind::k_element)
    {
        return false;
    }
    if (!c.pseudo_element.empty())
    {
        if (c.pseudo_element != pseudo)
        {
            return false;
        }
    }
    else if (!pseudo.empty())
    {
        return false;
    }
    if (!c.tag.empty() && c.tag != n.tag)
    {
        return false;
    }
    if (!c.id.empty() && doc.attr(node, "id") != c.id)
    {
        return false;
    }
    for (const auto& cls : c.classes)
    {
        if (!doc.has_class(node, cls))
        {
            return false;
        }
    }
    if (c.has_attr)
    {
        bool present = false;
        std::string_view actual;
        for (std::uint32_t i = n.attr_begin; i < n.attr_end; ++i)
        {
            if (doc.attrs[i].name == c.attr_name)
            {
                present = true;
                actual = doc.attrs[i].value;
                break;
            }
        }
        if (!present)
        {
            return false;
        }
        if (c.attr_op != 0)
        {
            std::string lhs(actual);
            std::string rhs(c.attr_value);
            if (c.attr_ignore_case)
            {
                for (char& ch : lhs)
                {
                    if (ch >= 'A' && ch <= 'Z')
                    {
                        ch = static_cast<char>(ch + 32);
                    }
                }
                for (char& ch : rhs)
                {
                    if (ch >= 'A' && ch <= 'Z')
                    {
                        ch = static_cast<char>(ch + 32);
                    }
                }
            }
            if (c.attr_op == 1)
            {
                if (lhs != rhs)
                {
                    return false;
                }
            }
            else if (c.attr_op == 2)
            {
                bool hit = false;
                std::size_t i = 0;
                while (i < lhs.size())
                {
                    while (i < lhs.size() && lhs[i] == ' ')
                    {
                        ++i;
                    }
                    std::size_t j = i;
                    while (j < lhs.size() && lhs[j] != ' ')
                    {
                        ++j;
                    }
                    if (j > i && lhs.compare(i, j - i, rhs) == 0)
                    {
                        hit = true;
                    }
                    i = j;
                }
                if (!hit)
                {
                    return false;
                }
            }
            else if (c.attr_op == 3)
            {
                if (lhs != rhs && (lhs.size() <= rhs.size() || lhs.compare(0, rhs.size(), rhs) != 0 ||
                                   lhs[rhs.size()] != '-'))
                {
                    return false;
                }
            }
            else if (c.attr_op == 4)
            {
                if (lhs.size() < rhs.size() || lhs.compare(0, rhs.size(), rhs) != 0)
                {
                    return false;
                }
            }
            else if (c.attr_op == 5)
            {
                if (lhs.size() < rhs.size() ||
                    lhs.compare(lhs.size() - rhs.size(), rhs.size(), rhs) != 0)
                {
                    return false;
                }
            }
            else if (c.attr_op == 6)
            {
                if (lhs.find(rhs) == std::string::npos)
                {
                    return false;
                }
            }
        }
    }
    const std::string& ps = c.pseudo;
    if (!ps.empty())
    {
        if (ps == "root")
        {
            return n.tag == "html";
        }
        if (ps == "hover")
        {
            return state.hover;
        }
        if (ps == "focus" || ps == "focus-visible" || ps == "focus-within")
        {
            return state.focus;
        }
        if (ps == "active")
        {
            return state.active;
        }
        if (ps == "link")
        {
            return n.tag == "a" && !doc.attr(node, "href").empty();
        }
        if (ps == "visited" || ps == "target")
        {
            return false;
        }
        if (ps == "empty")
        {
            return n.first_child == k_no_node;
        }
        if (ps == "first-child" || ps == "only-child")
        {
            int parent = n.parent;
            if (parent < 0)
            {
                return false;
            }
            if (doc.node(parent).first_child != node)
            {
                return false;
            }
            if (ps == "first-child")
            {
                return true;
            }
        }
        if (ps == "last-child" || ps == "only-child")
        {
            int parent = n.parent;
            if (parent < 0)
            {
                return false;
            }
            int c0 = doc.node(parent).first_child;
            int last = c0;
            while (c0 >= 0)
            {
                last = c0;
                c0 = doc.node(c0).next_sibling;
            }
            return last == node;
        }
        if (ps == "first-of-type" || ps == "only-of-type")
        {
            int parent = n.parent;
            if (parent < 0)
            {
                return false;
            }
            for (int s = doc.node(parent).first_child; s >= 0;
                 s = doc.node(s).next_sibling)
            {
                const Node& sib = doc.node(s);
                if (sib.kind == NodeKind::k_element && sib.tag == n.tag)
                {
                    if (s != node)
                    {
                        return false;
                    }
                    break;
                }
            }
            if (ps == "first-of-type")
            {
                return true;
            }
        }
        if (ps == "last-of-type" || ps == "only-of-type")
        {
            int parent = n.parent;
            if (parent < 0)
            {
                return false;
            }
            int last = k_no_node;
            for (int s = doc.node(parent).first_child; s >= 0;
                 s = doc.node(s).next_sibling)
            {
                const Node& sib = doc.node(s);
                if (sib.kind == NodeKind::k_element && sib.tag == n.tag)
                {
                    last = s;
                }
            }
            return last == node;
        }
        if ((ps == "nth-child" || ps == "nth-last-child" || ps == "nth-of-type" ||
             ps == "nth-last-of-type") &&
            c.has_nth)
        {
            int parent = n.parent;
            if (parent < 0)
            {
                return false;
            }
            bool of_type = (ps == "nth-of-type" || ps == "nth-last-of-type");
            bool from_end = (ps == "nth-last-child" || ps == "nth-last-of-type");
            int position = 0;
            int self = 0;
            for (int s = doc.node(parent).first_child; s >= 0;
                 s = doc.node(s).next_sibling)
            {
                const Node& sib = doc.node(s);
                if (sib.kind != NodeKind::k_element)
                {
                    continue;
                }
                if (of_type && sib.tag != n.tag)
                {
                    continue;
                }
                ++position;
                if (s == node)
                {
                    self = position;
                }
            }
            if (self <= 0)
            {
                return false;
            }
            int index = from_end ? (position - self + 1) : self;
            if (c.nth_a == 0)
            {
                return index == c.nth_b;
            }
            int diff = index - c.nth_b;
            if ((c.nth_a > 0 && diff < 0) || (c.nth_a < 0 && diff > 0))
            {
                return false;
            }
            return diff % c.nth_a == 0;
        }
        if (ps == "not" || ps == "is" || ps == "where")
        {
            for (const auto& inner : c.pseudo_selectors)
            {
                if (match_selector(doc, node, inner, state, pseudo))
                {
                    return ps != "not";
                }
            }
            return ps == "not";
        }
        if (ps == "has")
        {
            for (const auto& inner : c.pseudo_selectors)
            {
                if (c.has_combinator == '>')
                {
                    for (int s = n.first_child; s >= 0;
                         s = doc.node(s).next_sibling)
                    {
                        if (match_selector(doc, s, inner, state, pseudo))
                        {
                            return true;
                        }
                    }
                }
                else if (c.has_combinator == '+')
                {
                    int next = n.next_sibling;
                    if (next >= 0 && match_selector(doc, next, inner, state, pseudo))
                    {
                        return true;
                    }
                }
                else if (c.has_combinator == '~')
                {
                    for (int s = n.next_sibling; s >= 0;
                         s = doc.node(s).next_sibling)
                    {
                        if (match_selector(doc, s, inner, state, pseudo))
                        {
                            return true;
                        }
                    }
                }
                else
                {
                    for (int s = n.first_child; s >= 0;
                         s = doc.node(s).next_sibling)
                    {
                        std::vector<int> work;
                        work.push_back(s);
                        while (!work.empty())
                        {
                            int cur = work.back();
                            work.pop_back();
                            if (match_selector(doc, cur, inner, state, pseudo))
                            {
                                return true;
                            }
                            for (int k = doc.node(cur).first_child; k >= 0;
                                 k = doc.node(k).next_sibling)
                            {
                                work.push_back(k);
                            }
                        }
                    }
                }
            }
            return false;
        }
        if (ps == "checked")
        {
            return doc.has_attr(node, "checked");
        }
        if (ps == "disabled")
        {
            return doc.has_attr(node, "disabled");
        }
        if (ps == "enabled")
        {
            return !doc.has_attr(node, "disabled");
        }
        if (ps == "required")
        {
            return doc.has_attr(node, "required");
        }
        if (ps == "optional")
        {
            return !doc.has_attr(node, "required");
        }
        if (ps == "read-only")
        {
            return doc.has_attr(node, "readonly") || doc.has_attr(node, "disabled");
        }
        if (ps == "read-write")
        {
            return !doc.has_attr(node, "readonly") && !doc.has_attr(node, "disabled");
        }
        if (ps == "placeholder-shown")
        {
            return doc.has_attr(node, "placeholder");
        }
        return false;
    }
    return true;
}

std::uint32_t compound_specificity(const Compound& part)
{
    std::uint32_t a = 0;
    std::uint32_t b = 0;
    std::uint32_t c = 0;
    if (!part.id.empty())
    {
        ++a;
    }
    b += static_cast<std::uint32_t>(part.classes.size());
    if (part.has_attr)
    {
        ++b;
    }
    if (!part.pseudo.empty() && part.pseudo != "where")
    {
        if (part.pseudo == "is" || part.pseudo == "not" || part.pseudo == "has")
        {
            std::uint32_t inner = 0;
            for (const auto& sel : part.pseudo_selectors)
            {
                std::uint32_t spec = 0;
                for (const auto& inner_part : sel.parts)
                {
                    spec += compound_specificity(inner_part);
                }
                if (spec > inner)
                {
                    inner = spec;
                }
            }
            a += inner / 10000;
            b += (inner / 100) % 100;
            c += inner % 100;
        }
        else
        {
            ++b;
        }
    }
    if (!part.tag.empty())
    {
        ++c;
    }
    if (!part.pseudo_element.empty())
    {
        ++c;
    }
    return a * 10000 + b * 100 + c;
}

std::uint32_t specificity(const Selector& sel)
{
    std::uint32_t total = 0;
    for (const auto& part : sel.parts)
    {
        total += compound_specificity(part);
    }
    return total;
}

} // namespace detail

bool match_selector(const Document& doc, int node, const Selector& sel, const ElementState& state, const std::string& pseudo)
{
    int part = static_cast<int>(sel.parts.size()) - 1;
    int cur = node;
    if (!detail::match_compound(doc, cur, sel.parts[static_cast<std::size_t>(part)], state, pseudo))
    {
        return false;
    }
    while (part > 0)
    {
        char comb = sel.combinators[static_cast<std::size_t>(part - 1)];
        --part;
        const Compound& c = sel.parts[static_cast<std::size_t>(part)];
        if (comb == '>')
        {
            cur = doc.node(cur).parent;
            if (cur < 0 || !detail::match_compound(doc, cur, c, state, pseudo))
            {
                return false;
            }
        }
        else if (comb == '+')
        {
            int parent = doc.node(cur).parent;
            if (parent < 0)
            {
                return false;
            }
            int prev = -1;
            for (int s = doc.node(parent).first_child; s >= 0 && s != cur; s = doc.node(s).next_sibling)
            {
                prev = s;
            }
            cur = prev;
            if (cur < 0 || !detail::match_compound(doc, cur, c, state, pseudo))
            {
                return false;
            }
        }
        else if (comb == '~')
        {
            int parent = doc.node(cur).parent;
            if (parent < 0)
            {
                return false;
            }
            bool found = false;
            for (int s = doc.node(parent).first_child; s >= 0 && s != cur; s = doc.node(s).next_sibling)
            {
                if (detail::match_compound(doc, s, c, state, pseudo))
                {
                    cur = s;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return false;
            }
        }
        else
        {
            cur = doc.node(cur).parent;
            bool found = false;
            while (cur >= 0)
            {
                if (detail::match_compound(doc, cur, c, state, pseudo))
                {
                    found = true;
                    break;
                }
                cur = doc.node(cur).parent;
            }
            if (!found)
            {
                return false;
            }
        }
    }
    return true;
}

} // namespace wv
