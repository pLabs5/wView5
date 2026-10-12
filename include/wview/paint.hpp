/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string_view>

#include "wview/css.hpp"
#include "wview/layout.hpp"

namespace wv
{

struct Painter
{
    virtual ~Painter() = default;
    virtual void fill_rect(float x, float y, float w, float h, Color color) = 0;
    virtual void fill_rrect(float x, float y, float w, float h, const float radius[4], Color color) = 0;
    virtual void fill_gradient(float x, float y, float w, float h, const float radius[4], const Background& bg) = 0;
    virtual void draw_text(float x, float y, std::string_view text, const ComputedStyle& style, Color color) = 0;
    virtual void draw_image(float x, float y, float w, float h, int node, std::string_view src) = 0;
    virtual void draw_shadow(float x, float y, float w, float h, const float radius[4], const Shadow& shadow) = 0;
    virtual void push_clip(float x, float y, float w, float h) = 0;
    virtual void pop_clip() = 0;
};

void paint_tree(const Document& doc, const LayoutResult& layout, int box_idx, Painter& painter);

}
