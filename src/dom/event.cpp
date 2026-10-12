/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "wview/dom.hpp"

namespace wv
{

bool Document::add_event_listener(int node, std::string_view type, EventCallback callback,
                                  void* user, bool capture, bool once)
{
    if (node < 0 || static_cast<std::size_t>(node) >= nodes.size() || type.empty() ||
        callback == nullptr)
    {
        return false;
    }
    EventListener listener;
    listener.node = node;
    listener.type = std::string(type);
    listener.callback = callback;
    listener.user = user;
    listener.capture = capture;
    listener.once = once;
    listeners.push_back(listener);
    return true;
}

bool Document::remove_event_listener(int node, std::string_view type,
                                     EventCallback callback, bool capture)
{
    for (auto it = listeners.begin(); it != listeners.end(); ++it)
    {
        if (it->node == node && it->type == type && it->callback == callback &&
            it->capture == capture)
        {
            listeners.erase(it);
            return true;
        }
    }
    return false;
}

bool Document::dispatch_event(int target, Event& ev)
{
    if (target < 0 || static_cast<std::size_t>(target) >= nodes.size() || ev.type.empty())
    {
        return false;
    }
    ev.target = target;
    std::vector<int> path;
    for (int p = target; p != k_no_node; p = nodes[static_cast<std::size_t>(p)].parent)
    {
        path.push_back(p);
    }
    auto run_listeners = [&](int node, bool capture_phase) {
        for (std::size_t i = 0; i < listeners.size();)
        {
            EventListener& listener = listeners[i];
            if (listener.node != node || listener.type != ev.type ||
                listener.capture != capture_phase)
            {
                ++i;
                continue;
            }
            EventCallback callback = listener.callback;
            void* user = listener.user;
            bool once = listener.once;
            if (once)
            {
                listeners.erase(listeners.begin() + static_cast<std::ptrdiff_t>(i));
            }
            else
            {
                ++i;
            }
            ev.current_target = node;
            callback(ev, user);
            if (ev.immediate_stopped)
            {
                return false;
            }
        }
        return true;
    };
    for (int i = static_cast<int>(path.size()) - 1; i >= 1; --i)
    {
        if (ev.propagation_stopped)
        {
            break;
        }
        if (!run_listeners(path[static_cast<std::size_t>(i)], true))
        {
            break;
        }
    }
    if (!ev.propagation_stopped)
    {
        for (std::size_t i = 0; i < listeners.size();)
        {
            EventListener& listener = listeners[i];
            if (listener.node != target || listener.type != ev.type)
            {
                ++i;
                continue;
            }
            EventCallback callback = listener.callback;
            void* user = listener.user;
            if (listener.once)
            {
                listeners.erase(listeners.begin() + static_cast<std::ptrdiff_t>(i));
            }
            else
            {
                ++i;
            }
            ev.current_target = target;
            callback(ev, user);
            if (ev.immediate_stopped)
            {
                ev.propagation_stopped = true;
                break;
            }
        }
    }
    if (ev.bubbles && !ev.propagation_stopped)
    {
        for (std::size_t i = 1; i < path.size(); ++i)
        {
            if (ev.propagation_stopped)
            {
                break;
            }
            if (!run_listeners(path[i], false))
            {
                break;
            }
        }
    }
    ev.current_target = k_no_node;
    return !(ev.cancelable && ev.default_prevented);
}

Event Document::click(int node)
{
    Event ev;
    ev.type = "click";
    ev.bubbles = true;
    ev.cancelable = true;
    dispatch_event(node, ev);
    return ev;
}

} // namespace wv
