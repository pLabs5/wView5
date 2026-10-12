/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <string_view>

namespace wv
{
namespace detail
{
bool is_space(char c);
bool is_void_tag(std::string_view tag);
bool is_raw_text_tag(std::string_view tag);
void to_lower_inplace(std::string& s, std::size_t begin, std::size_t end);
std::string_view decode_entity(std::string_view e);
} // namespace detail
} // namespace wv
