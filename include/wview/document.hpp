/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "wview/css.hpp"
#include "wview/dom.hpp"

namespace wv
{

struct ResourceLoader
{
    virtual ~ResourceLoader() = default;
    virtual bool load(std::string_view url, std::string& bytes, std::string& mime) = 0;
};

struct Script
{
    std::string text;
    std::string url;
    bool external = false;
};

struct DocumentBundle
{
    Document dom;
    Stylesheet sheet;
    std::vector<Script> scripts;
    std::string base_url;
    std::string title;
};

std::string resolve_url(std::string_view base, std::string_view ref);

DocumentBundle load_document(const std::string& html, std::string_view base_url,
                             ResourceLoader& loader);

} // namespace wv
