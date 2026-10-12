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

bool focusable(const Document& doc, int node)
{
    if (node < 0 || static_cast<std::size_t>(node) >= doc.node_count())
    {
        return false;
    }
    const Node& n = doc.node(node);
    if (n.kind != NodeKind::k_element)
    {
        return false;
    }
    if (doc.has_attr(node, "disabled"))
    {
        return false;
    }
    if (doc.has_attr(node, "tabindex"))
    {
        std::string_view value = doc.attr(node, "tabindex");
        if (value == "-1")
        {
            return false;
        }
        return true;
    }
    if (n.tag == "button" || n.tag == "input" || n.tag == "select" ||
        n.tag == "textarea" || n.tag == "summary")
    {
        if (n.tag == "input" && doc.attr(node, "type") == "hidden")
        {
            return false;
        }
        return true;
    }
    if (n.tag == "a")
    {
        return !doc.attr(node, "href").empty();
    }
    return false;
}

int tabindex_of(const Document& doc, int node)
{
    std::string_view value = doc.attr(node, "tabindex");
    if (value.empty())
    {
        return 0;
    }
    bool neg = false;
    std::size_t i = 0;
    if (value[0] == '-' || value[0] == '+')
    {
        neg = (value[0] == '-');
        i = 1;
    }
    int v = 0;
    for (; i < value.size(); ++i)
    {
        if (value[i] < '0' || value[i] > '9')
        {
            return 0;
        }
        v = v * 10 + (value[i] - '0');
    }
    return neg ? -v : v;
}

} // namespace

bool Document::focus(int node)
{
    if (!focusable(*this, node))
    {
        return false;
    }
    if (focused_node == node)
    {
        return true;
    }
    if (focused_node != k_no_node)
    {
        Event blur_ev;
        blur_ev.type = "blur";
        blur_ev.bubbles = false;
        dispatch_event(focused_node, blur_ev);
        Event focusout_ev;
        focusout_ev.type = "focusout";
        focusout_ev.bubbles = true;
        dispatch_event(focused_node, focusout_ev);
    }
    focused_node = node;
    Event focus_ev;
    focus_ev.type = "focus";
    focus_ev.bubbles = false;
    dispatch_event(node, focus_ev);
    Event focusin_ev;
    focusin_ev.type = "focusin";
    focusin_ev.bubbles = true;
    dispatch_event(node, focusin_ev);
    return true;
}

void Document::blur()
{
    if (focused_node == k_no_node)
    {
        return;
    }
    int node = focused_node;
    focused_node = k_no_node;
    Event blur_ev;
    blur_ev.type = "blur";
    blur_ev.bubbles = false;
    dispatch_event(node, blur_ev);
    Event focusout_ev;
    focusout_ev.type = "focusout";
    focusout_ev.bubbles = true;
    dispatch_event(node, focusout_ev);
}

bool focus_next(Document& doc, bool reverse)
{
    if (doc.root() == k_no_node)
    {
        return false;
    }
    std::vector<int> order;
    std::vector<int> work;
    work.push_back(doc.root());
    while (!work.empty())
    {
        int cur = work.back();
        work.pop_back();
        if (focusable(doc, cur))
        {
            order.push_back(cur);
        }
        std::vector<int> kids;
        for (int c = doc.node(cur).first_child; c != k_no_node;
             c = doc.node(c).next_sibling)
        {
            kids.push_back(c);
        }
        for (int i = static_cast<int>(kids.size()) - 1; i >= 0; --i)
        {
            work.push_back(kids[static_cast<std::size_t>(i)]);
        }
    }
    if (order.empty())
    {
        return false;
    }
    std::vector<int> positive;
    std::vector<int> rest;
    for (int node : order)
    {
        if (tabindex_of(doc, node) > 0)
        {
            positive.push_back(node);
        }
        else
        {
            rest.push_back(node);
        }
    }
    for (std::size_t i = 0; i < positive.size(); ++i)
    {
        for (std::size_t j = i + 1; j < positive.size(); ++j)
        {
            if (tabindex_of(doc, positive[j]) < tabindex_of(doc, positive[i]))
            {
                int tmp = positive[i];
                positive[i] = positive[j];
                positive[j] = tmp;
            }
        }
    }
    std::vector<int> sequence = positive;
    sequence.insert(sequence.end(), rest.begin(), rest.end());
    int current = doc.active_element();
    std::size_t at = sequence.size();
    for (std::size_t i = 0; i < sequence.size(); ++i)
    {
        if (sequence[i] == current)
        {
            at = i;
        }
    }
    std::size_t next = 0;
    if (reverse)
    {
        next = (at == sequence.size() || at == 0) ? sequence.size() - 1 : at - 1;
    }
    else
    {
        next = (at == sequence.size()) ? 0 : (at + 1) % sequence.size();
    }
    return doc.focus(sequence[next]);
}

} // namespace wv
