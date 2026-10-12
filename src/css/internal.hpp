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

#include "wview/css.hpp"
#include "wview/dom.hpp"

namespace wv
{
namespace detail
{
float parse_float(const char* p, const char* end, const char** out_end);
std::string trim_copy(std::string_view s);
std::vector<std::string> split_top(std::string_view s, char sep);
std::vector<std::string> split_ws(std::string_view s);
bool parse_length_value(std::string_view s, Length& out);
bool parse_color_value(std::string_view s, const Color& current, Color& out);
bool parse_gradient_layer(std::string_view s, const Color& current, Background& bg);
void skip_ws_comments(const char*& p, const char* end);
std::string parse_ident(const char*& p, const char* end);
bool parse_compound(const char*& p, const char* end, Compound& out);
bool parse_selector(const char*& p, const char* end, Selector& out);
void parse_selector_list(std::string_view text, std::vector<Selector>& out, char& leading);
bool parse_nth(std::string_view arg, int& a, int& b);
void parse_declarations(const char*& p, const char* end, std::vector<Declaration>& out);
void parse_rules(const char*& p, const char* end, Stylesheet& sheet, float media_max,
                 float media_min, int layer, std::uint32_t& order);
int layer_index(Stylesheet& sheet, std::string_view name);
bool match_compound(const Document& doc, int node, const Compound& c,
                    const ElementState& state, const std::string& pseudo);
std::uint32_t specificity(const Selector& sel);
std::string substitute_vars(
    const std::string& value,
    const std::vector<std::pair<std::string, std::string>>& vars);
float resolve_px(Length l, float font_size, float root_size, float viewport_w,
                 float viewport_h, float percent_base);
void apply_box_sides(const std::vector<std::string>& parts, Length out[4]);
bool apply_box(ComputedStyle& st, const std::string& prop, const std::string& value,
               const ComputedStyle* parent, float viewport_w, float viewport_h);
bool apply_flex(ComputedStyle& st, const std::string& prop, const std::string& value,
                const ComputedStyle* parent, float viewport_w, float viewport_h);
bool apply_paint(ComputedStyle& st, const std::string& prop, const std::string& value,
                 const ComputedStyle* parent, float viewport_w, float viewport_h);
bool apply_text(ComputedStyle& st, const std::string& prop, const std::string& value,
                const ComputedStyle* parent, float viewport_w, float viewport_h);
bool apply_misc(ComputedStyle& st, const std::string& prop, const std::string& value,
                const ComputedStyle* parent, float viewport_w, float viewport_h);
} // namespace detail
} // namespace wv
