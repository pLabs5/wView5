/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "wview/event.hpp"

namespace wv
{

constexpr int k_no_node = -1;

enum class NodeKind : std::uint8_t
{
    k_element,
    k_text,
};

struct Attr
{
    std::string_view name;
    std::string_view value;
};

struct Node
{
    NodeKind kind = NodeKind::k_element;
    int parent = k_no_node;
    int first_child = k_no_node;
    int next_sibling = k_no_node;
    std::string_view tag;
    std::string_view text;
    std::uint32_t attr_begin = 0;
    std::uint32_t attr_end = 0;
};

class Document
{
public:
    Document() = default;
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;
    Document(Document&&) = default;
    Document& operator=(Document&&) = default;

    int root() const
    {
        return root_;
    }

    const Node& node(int index) const
    {
        return nodes[static_cast<std::size_t>(index)];
    }

    std::size_t node_count() const
    {
        return nodes.size();
    }

    std::string_view attr(int index, std::string_view name) const;
    bool has_attr(int index, std::string_view name) const;
    bool has_class(int index, std::string_view cls) const;
    std::string_view text_of(int index) const;

    int first_element_by_tag(int from, std::string_view tag) const;
    int first_element_by_class(int from, std::string_view cls) const;
    int first_element_child(int index) const;
    int next_element_sibling(int index) const;
    int previous_element_sibling(int index) const;
    int child_element_count(int index) const;
    int query_selector(int root, std::string_view selectors) const;
    void query_selector_all(int root, std::string_view selectors,
                            std::vector<int>& out) const;

    int create_element(std::string_view tag);
    int create_text_node(std::string_view text);
    bool append_child(int parent, int child);
    bool insert_before(int parent, int child, int before);
    bool remove_child(int parent, int child);
    bool set_attribute(int elem, std::string_view name, std::string_view value);
    bool remove_attribute(int elem, std::string_view name);
    bool set_text(int node, std::string_view text);
    bool add_class(int elem, std::string_view cls);
    bool remove_class(int elem, std::string_view cls);
    bool toggle_class(int elem, std::string_view cls);

    bool focus(int node);
    void blur();
    int active_element() const
    {
        return focused_node;
    }
    bool add_event_listener(int node, std::string_view type, EventCallback callback,
                            void* user, bool capture, bool once);
    bool remove_event_listener(int node, std::string_view type, EventCallback callback,
                               bool capture);
    bool dispatch_event(int target, Event& ev);
    Event click(int node);
    void dump(int index, std::string& out, int depth = 0) const;

    std::string source;
    std::vector<Node> nodes;
    std::vector<Attr> attrs;
    std::vector<EventListener> listeners;
    int focused_node = k_no_node;
    int root_ = k_no_node;
};

Document parse_html(std::string_view src);

bool focus_next(Document& doc, bool reverse);

}
