/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "wview/css.hpp"
#include "wview/dom.hpp"

namespace wv
{

struct TextMetrics
{
    virtual ~TextMetrics() = default;
    virtual float measure(std::string_view text, const ComputedStyle& style) = 0;
    virtual float line_height(const ComputedStyle& style) = 0;
};

struct ImageMetrics
{
    virtual ~ImageMetrics() = default;
    virtual bool size_of(std::string_view src, float& w, float& h) = 0;
};

struct Run
{
    int node = k_no_node;
    std::string text;
    float x = 0.0f;
    float width = 0.0f;
    ComputedStyle style;
};

struct Line
{
    float y = 0.0f;
    float height = 0.0f;
    std::vector<Run> runs;
};

struct Box
{
    int node = k_no_node;
    ComputedStyle style;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    float scroll_x = 0.0f;
    float scroll_y = 0.0f;
    float scroll_w = 0.0f;
    float scroll_h = 0.0f;
    int parent = -1;
    int first_child = -1;
    int next_sibling = -1;
    std::vector<Line> lines;
    bool intrinsic_image = false;
    float intrinsic_w = 0.0f;
    float intrinsic_h = 0.0f;
};

struct LayoutResult
{
    std::vector<Box> boxes;
    std::vector<ComputedStyle> styles;
    std::vector<std::vector<std::pair<std::string, std::string>>> vars;
    int root_box = -1;
};

LayoutResult layout_document(
    const Document& doc,
    const Stylesheet& sheet,
    TextMetrics& text_metrics,
    ImageMetrics* image_metrics,
    const ResolveContext& ctx,
    float viewport_w,
    float viewport_h);

int find_box_for_node(const LayoutResult& layout, int node);
bool set_scroll(LayoutResult& layout, int box_idx, float x, float y);
bool ensure_visible(LayoutResult& layout, int box_idx);

}
