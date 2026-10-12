/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "layout/internal.hpp"

namespace wv
{
using namespace detail;

namespace detail
{

float resolve_len(Length l, float percent_base, float font_size, float vw, float vh)
{
    switch (l.unit)
    {
    case Unit::k_px:
        return l.value;
    case Unit::k_percent:
        return l.value / 100.0f * percent_base;
    case Unit::k_em:
        return l.value * font_size;
    case Unit::k_rem:
        return l.value * 16.0f;
    case Unit::k_vw:
        return l.value / 100.0f * vw;
    case Unit::k_vh:
        return l.value / 100.0f * vh;
    default:
        return 0.0f;
    }
}

bool is_inline_display(Display d)
{
    return d == Display::k_inline || d == Display::k_inline_block;
}

std::string transform_text(const std::string& in, int mode)
{
    if (mode == 0)
    {
        return in;
    }
    std::string out = in;
    if (mode == 1)
    {
        for (char& c : out)
        {
            if (c >= 'a' && c <= 'z')
            {
                c = static_cast<char>(c - 32);
            }
        }
        return out;
    }
    if (mode == 2)
    {
        for (char& c : out)
        {
            if (c >= 'A' && c <= 'Z')
            {
                c = static_cast<char>(c + 32);
            }
        }
        return out;
    }
    bool cap = true;
    for (char& c : out)
    {
        bool is_letter = ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
        if (is_letter && cap)
        {
            if (c >= 'a' && c <= 'z')
            {
                c = static_cast<char>(c - 32);
            }
            cap = false;
        }
        else if (is_letter)
        {
            if (c >= 'A' && c <= 'Z')
            {
                c = static_cast<char>(c + 32);
            }
        }
        else
        {
            cap = (c == ' ' || c == '\t' || c == '\n');
        }
    }
    return out;
}

} // namespace detail

float Engine::pad_top(const ComputedStyle& st, float cb_w) const
{
    return resolve_len(st.padding[0], cb_w, st.font_size, viewport_w, viewport_h) +
        resolve_len(st.border_width[0], cb_w, st.font_size, viewport_w, viewport_h);
}

float Engine::pad_bottom(const ComputedStyle& st, float cb_w) const
{
    return resolve_len(st.padding[2], cb_w, st.font_size, viewport_w, viewport_h) +
        resolve_len(st.border_width[2], cb_w, st.font_size, viewport_w, viewport_h);
}

float Engine::pad_left(const ComputedStyle& st, float cb_w) const
{
    return resolve_len(st.padding[3], cb_w, st.font_size, viewport_w, viewport_h) +
        resolve_len(st.border_width[3], cb_w, st.font_size, viewport_w, viewport_h);
}

float Engine::pad_x(const ComputedStyle& st, float cb_w) const
{
    return pad_left(st, cb_w) + resolve_len(st.padding[1], cb_w, st.font_size, viewport_w, viewport_h) +
        resolve_len(st.border_width[1], cb_w, st.font_size, viewport_w, viewport_h);
}

float Engine::margin_x(const ComputedStyle& st, float cb_w) const
{
    return resolve_len(st.margin[1], cb_w, st.font_size, viewport_w, viewport_h) +
        resolve_len(st.margin[3], cb_w, st.font_size, viewport_w, viewport_h);
}

float Engine::content_width(const ComputedStyle& st, float cb_w) const
{
    if (!st.width.is_auto())
    {
        float w = resolve_len(st.width, cb_w, st.font_size, viewport_w, viewport_h);
        if (st.box_border)
        {
            w -= pad_x(st, cb_w);
            if (w < 0.0f)
            {
                w = 0.0f;
            }
        }
        return w;
    }
    return cb_w - margin_x(st, cb_w) - pad_x(st, cb_w);
}

float Engine::box_width(const ComputedStyle& st, float cb_w) const
{
    if (!st.width.is_auto())
    {
        float w = resolve_len(st.width, cb_w, st.font_size, viewport_w, viewport_h);
        return st.box_border ? w : w + pad_x(st, cb_w);
    }
    return cb_w - margin_x(st, cb_w);
}

float Engine::line_height_of(const ComputedStyle& st)
{
    if (!st.line_height_normal && st.line_height > 0.0f)
    {
        return st.line_height;
    }
    float lh = text_metrics->line_height(st);
    return (lh > 0.0f) ? lh : st.font_size * 1.2f;
}

float Engine::text_width(const std::string& text, const ComputedStyle& st)
{
    if (text.empty())
    {
        return 0.0f;
    }
    float w = text_metrics->measure(text, st);
    if (st.letter_spacing != 0.0f && text.size() > 1)
    {
        w += st.letter_spacing * static_cast<float>(text.size() - 1);
    }
    return w;
}

} // namespace wv
