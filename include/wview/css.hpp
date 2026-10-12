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

namespace wv
{

enum class Display : std::uint8_t
{
    k_inline,
    k_inline_block,
    k_block,
    k_flex,
    k_inline_flex,
    k_grid,
    k_table,
    k_table_row_group,
    k_table_row,
    k_table_cell,
    k_table_caption,
    k_none,
};

enum class Position : std::uint8_t
{
    k_static,
    k_relative,
    k_absolute,
    k_fixed,
};

enum class Unit : std::uint8_t
{
    k_px,
    k_percent,
    k_em,
    k_rem,
    k_vw,
    k_vh,
    k_fr,
    k_auto,
};

struct Length
{
    float value = 0.0f;
    Unit unit = Unit::k_px;

    static Length px(float v)
    {
        return Length{v, Unit::k_px};
    }

    static Length auto_()
    {
        return Length{0.0f, Unit::k_auto};
    }

    bool is_auto() const
    {
        return unit == Unit::k_auto;
    }
};

struct Color
{
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

struct GradientStop
{
    Color color;
    float offset = 0.0f;
    bool has_offset = false;
};

struct Background
{
    bool has_color = false;
    Color color;
    bool has_gradient = false;
    float angle_deg = 180.0f;
    std::vector<GradientStop> stops;
};

enum class Overflow : std::uint8_t
{
    k_visible,
    k_hidden,
    k_scroll,
    k_auto,
};

enum class FlexDirection : std::uint8_t
{
    k_row,
    k_column,
};

enum class Align : std::uint8_t
{
    k_auto,
    k_flex_start,
    k_flex_end,
    k_center,
    k_stretch,
    k_baseline,
    k_space_between,
    k_space_around,
    k_space_evenly,
};

struct Track
{
    float fr = 1.0f;
    Length fixed;
    bool is_fr = true;
    Length min;
    bool has_min = false;
};

struct Shadow
{
    float dx = 0.0f;
    float dy = 0.0f;
    float blur = 0.0f;
    Color color;
    bool has = false;
};

struct ComputedStyle
{
    Display display = Display::k_inline;
    Position position = Position::k_static;
    Length top = Length::auto_();
    Length right = Length::auto_();
    Length bottom = Length::auto_();
    Length left = Length::auto_();
    Length width = Length::auto_();
    Length height = Length::auto_();
    Length min_width = Length::auto_();
    Length min_height = Length::auto_();
    Length max_width = Length::auto_();
    Length max_height = Length::auto_();
    Length margin[4] = {};
    Length padding[4] = {};
    Length border_width[4] = {};
    Color border_color = {0, 0, 0, 255};
    float radius[4] = {};
    bool box_border = true;
    Overflow overflow_x = Overflow::k_visible;
    Overflow overflow_y = Overflow::k_visible;
    FlexDirection flex_direction = FlexDirection::k_row;
    bool flex_wrap = false;
    Align justify_content = Align::k_flex_start;
    Align align_items = Align::k_stretch;
    Align align_content = Align::k_flex_start;
    Align align_self = Align::k_auto;
    float flex_grow = 0.0f;
    float flex_shrink = 1.0f;
    Length flex_basis = Length::auto_();
    bool flex_shrink_none = false;
    float gap_row = 0.0f;
    float gap_col = 0.0f;
    std::vector<Track> grid_columns;
    Background background;
    Color color = {0, 0, 0, 255};
    float opacity = 1.0f;
    std::string font_family;
    float font_size = 16.0f;
    int font_weight = 400;
    bool font_italic = false;
    float line_height = 0.0f;
    bool line_height_normal = true;
    float letter_spacing = 0.0f;
    int text_align = 0;
    int text_transform = 0;
    bool underline = false;
    int white_space = 0;
    bool ellipsis = false;
    int z_index = 0;
    bool has_z_index = false;
    Shadow shadow;
    float translate_x = 0.0f;
    float translate_y = 0.0f;
    bool has_translate = false;
    std::string content;
    bool has_content = false;
    bool pointer_events_none = false;
};

struct ElementState
{
    bool hover = false;
    bool focus = false;
    bool active = false;
};

struct Selector;

struct Compound
{
    std::string tag;
    std::string id;
    std::vector<std::string> classes;
    std::string attr_name;
    std::string attr_value;
    int attr_op = 0;
    bool attr_ignore_case = false;
    bool has_attr = false;
    std::string pseudo;
    std::string pseudo_element;
    std::string pseudo_arg;
    std::vector<Selector> pseudo_selectors;
    char has_combinator = ' ';
    int nth_a = 0;
    int nth_b = 0;
    bool has_nth = false;
};

struct Selector
{
    std::vector<Compound> parts;
    std::vector<char> combinators;
};

struct Declaration
{
    std::string property;
    std::string value;
    bool important = false;
};

struct Rule
{
    std::vector<Selector> selectors;
    std::vector<Declaration> declarations;
    float media_max_width = -1.0f;
    float media_min_width = -1.0f;
    int layer = -1;
    std::uint32_t order = 0;
};

struct Stylesheet
{
    std::string source;
    std::vector<Rule> rules;
    std::vector<std::string> layer_order;
};

Stylesheet parse_css(std::string_view src);

bool match_selector(
    const class Document& doc,
    int node,
    const Selector& sel,
    const ElementState& state,
    const std::string& pseudo);

void apply_declaration(
    ComputedStyle& st,
    const std::string& prop,
    const std::string& value,
    const ComputedStyle* parent,
    float viewport_w,
    float viewport_h);

std::string substitute_vars(
    const std::string& value,
    const std::vector<std::pair<std::string, std::string>>& vars);

bool rule_applies(const struct Rule& rule, float viewport_w);

struct ResolveContext
{
    float viewport_width = 1280.0f;
};

ComputedStyle resolve_style(
    const class Document& doc,
    int node,
    const ComputedStyle* parent,
    const std::vector<std::pair<std::string, std::string>>* parent_vars,
    const Stylesheet& sheet,
    const ElementState& state,
    const ResolveContext& ctx,
    std::vector<std::pair<std::string, std::string>>& out_vars);

ComputedStyle default_style_for_tag(std::string_view tag);

}
