/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace wv
{

constexpr int k_event_none = -1;

struct Event
{
    std::string type;
    int target = k_event_none;
    int current_target = k_event_none;
    bool bubbles = true;
    bool cancelable = false;
    bool default_prevented = false;
    bool propagation_stopped = false;
    bool immediate_stopped = false;

    void prevent_default() noexcept
    {
        if (cancelable)
        {
            default_prevented = true;
        }
    }

    void stop_propagation() noexcept
    {
        propagation_stopped = true;
    }

    void stop_immediate() noexcept
    {
        immediate_stopped = true;
        propagation_stopped = true;
    }
};

using EventCallback = void (*)(Event& ev, void* user) noexcept;

struct EventListener
{
    int node = k_event_none;
    std::string type;
    EventCallback callback = nullptr;
    void* user = nullptr;
    bool capture = false;
    bool once = false;
};

} // namespace wv
