/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "css/internal.hpp"
#include "wview/css.hpp"
#include "wview/dom.hpp"
#include "wview/layout.hpp"

namespace wv
{

struct Segment
{
    int node = k_no_node;
    std::string text;
    ComputedStyle style;
    bool replaced = false;
    float replaced_w = 0.0f;
    float replaced_h = 0.0f;
    bool break_before = false;
};

struct Engine
{
    const Document* doc = nullptr;
    const Stylesheet* sheet = nullptr;
    TextMetrics* text_metrics = nullptr;
    ImageMetrics* image_metrics = nullptr;
    ResolveContext ctx;
    float viewport_w = 1280.0f;
    float viewport_h = 720.0f;
    LayoutResult* result = nullptr;
    std::vector<int> node_box;

    float pad_top(const ComputedStyle& st, float cb_w) const;

    float pad_bottom(const ComputedStyle& st, float cb_w) const;

    float pad_left(const ComputedStyle& st, float cb_w) const;

    float pad_x(const ComputedStyle& st, float cb_w) const;

    float margin_x(const ComputedStyle& st, float cb_w) const;

    float content_width(const ComputedStyle& st, float cb_w) const;

    float box_width(const ComputedStyle& st, float cb_w) const;

    float line_height_of(const ComputedStyle& st);

    float text_width(const std::string& text, const ComputedStyle& st);

    void collect_inline(int node, const ComputedStyle& st, std::vector<Segment>& out);

    std::vector<std::string> split_words(const std::string& text, int white_space)
    {
        std::vector<std::string> words;
        if (white_space == 2 || white_space == 3)
        {
            words.push_back(text);
            return words;
        }
        std::string cur;
        for (char c : text)
        {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            {
                if (white_space == 4 && c == '\n')
                {
                    if (!cur.empty())
                    {
                        words.push_back(cur);
                        cur.clear();
                    }
                    words.push_back("\n");
                }
                else if (!cur.empty() || !words.empty())
                {
                    if (!cur.empty())
                    {
                        words.push_back(cur);
                        cur.clear();
                    }
                    if (words.empty() || words.back() != " ")
                    {
                        words.push_back(" ");
                    }
                }
            }
            else
            {
                cur.push_back(c);
            }
        }
        if (!cur.empty())
        {
            words.push_back(cur);
        }
        return words;
    }

    void layout_lines(Box& box, std::vector<Segment> segs, float avail_w, float ox, float start_y);

    int box_for_node(int node) const;

    int alloc_box(int node, int parent);

    void layout_box(int box_idx, float cb_w, int abs_cb);
    void layout_table(int box_idx, float cb_w, int abs_cb);

    void pseudo_segments(int box_idx, std::vector<Segment>& before, std::vector<Segment>& after);

    void layout_block(int box_idx, float cb_w, int abs_cb);

    void layout_flex(int box_idx, float cb_w, int abs_cb);

    void e_layout_box_measure(int box_idx, float cb_w, int abs_cb);

    void layout_grid(int box_idx, float cb_w, int abs_cb);

    void apply_position(int box_idx, float cb_w, int abs_cb);

    void layout_inline_block(int box_idx, float cb_w, int abs_cb);

    void layout_replaced(int box_idx, float cb_w, int abs_cb);

    bool has_pseudo_rule(int node, const std::string& pe, const ElementState& state);

    int build_boxes(int node, int parent_box, const ElementState& state);

    void finalize(int box_idx, float ox, float oy);
};

namespace detail
{
constexpr float k_inf = 1e9f;
float resolve_len(Length l, float percent_base, float font_size, float vw, float vh);
bool is_inline_display(Display d);
std::string transform_text(const std::string& in, int mode);
} // namespace detail

void measure_scroll_extents(LayoutResult& layout, int box_idx);
} // namespace wv
