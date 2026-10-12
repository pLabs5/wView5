/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/paint.hpp"

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

struct TestText : wv::TextMetrics
{
    float measure(std::string_view text, const wv::ComputedStyle& style) override
    {
        return static_cast<float>(text.size()) * style.font_size * 0.6f;
    }

    float line_height(const wv::ComputedStyle& style) override
    {
        return style.font_size * 1.2f;
    }
};

struct TestImage : wv::ImageMetrics
{
    bool size_of(std::string_view src, float& w, float& h) override
    {
        std::string s(src);
        if (s.find("avatar") != std::string::npos || s.find("channel") != std::string::npos)
        {
            w = 40.0f;
            h = 40.0f;
            return true;
        }
        w = 320.0f;
        h = 180.0f;
        return true;
    }
};

struct Ppm : wv::Painter
{
    float scale = 0.25f;
    int w = 0;
    int h = 0;
    std::vector<unsigned char> px;
    struct Clip
    {
        int x0;
        int y0;
        int x1;
        int y1;
    };
    std::vector<Clip> clips;

    void init(int vw, int vh)
    {
        w = static_cast<int>(vw * scale) + 1;
        h = static_cast<int>(vh * scale) + 1;
        px.assign(static_cast<std::size_t>(w * h * 3), 255);
        clips.push_back(Clip{0, 0, w, h});
    }

    int sx(float x) const
    {
        return static_cast<int>(x * scale);
    }

    void blend(int x, int y, wv::Color c)
    {
        const Clip& cl = clips.back();
        if (x < cl.x0 || y < cl.y0 || x >= cl.x1 || y >= cl.y1)
        {
            return;
        }
        std::size_t o = (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)) * 3;
        float a = c.a / 255.0f;
        px[o] = static_cast<unsigned char>(px[o] * (1.0f - a) + c.r * a);
        px[o + 1] = static_cast<unsigned char>(px[o + 1] * (1.0f - a) + c.g * a);
        px[o + 2] = static_cast<unsigned char>(px[o + 2] * (1.0f - a) + c.b * a);
    }

    void fill_rect(float x, float y, float w0, float h0, wv::Color c) override
    {
        for (int j = sy(y); j < sy(y + h0); ++j)
        {
            for (int i = sx(x); i < sx(x + w0); ++i)
            {
                blend(i, j, c);
            }
        }
    }

    int sy(float y) const
    {
        return static_cast<int>(y * scale);
    }

    void fill_rrect(float x, float y, float w0, float h0, const float radius[4], wv::Color c) override
    {
        float r = radius[0];
        if (radius[1] > r) { r = radius[1]; }
        if (radius[2] > r) { r = radius[2]; }
        if (radius[3] > r) { r = radius[3]; }
        for (int j = sy(y); j < sy(y + h0); ++j)
        {
            for (int i = sx(x); i < sx(x + w0); ++i)
            {
                float fx = (i + 0.5f) / scale;
                float fy = (j + 0.5f) / scale;
                float dx = 0.0f;
                float dy = 0.0f;
                if (fx < x + r && fy < y + r) { dx = x + r - fx; dy = y + r - fy; }
                else if (fx > x + w0 - r && fy < y + r) { dx = fx - (x + w0 - r); dy = y + r - fy; }
                else if (fx < x + r && fy > y + h0 - r) { dx = x + r - fx; dy = fy - (y + h0 - r); }
                else if (fx > x + w0 - r && fy > y + h0 - r) { dx = fx - (x + w0 - r); dy = fy - (y + h0 - r); }
                if (dx * dx + dy * dy <= r * r)
                {
                    blend(i, j, c);
                }
            }
        }
    }

    void fill_gradient(float x, float y, float w0, float h0, const float radius[4], const wv::Background& bg) override
    {
        if (!bg.has_gradient || bg.stops.empty())
        {
            if (bg.has_color)
            {
                fill_rrect(x, y, w0, h0, radius, bg.color);
            }
            return;
        }
        float rad = 3.14159265f * bg.angle_deg / 180.0f;
        float dx = 0.0f;
        float dy = 0.0f;
        dx = (rad > 1.5707f && rad < 4.7123f) ? -1.0f : 1.0f;
        dy = (rad > 3.14159f) ? -1.0f : 1.0f;
        if (bg.angle_deg == 90.0f) { dx = 1.0f; dy = 0.0f; }
        if (bg.angle_deg == 270.0f) { dx = -1.0f; dy = 0.0f; }
        if (bg.angle_deg == 0.0f) { dx = 0.0f; dy = -1.0f; }
        if (bg.angle_deg == 180.0f) { dx = 0.0f; dy = 1.0f; }
        for (int j = sy(y); j < sy(y + h0); ++j)
        {
            for (int i = sx(x); i < sx(x + w0); ++i)
            {
                float fx = (i + 0.5f) / scale;
                float fy = (j + 0.5f) / scale;
                float t = ((fx - x) * dx / (w0 > 0 ? w0 : 1) + (fy - y) * dy / (h0 > 0 ? h0 : 1)) / 2.0f + 0.5f;
                if (t < 0.0f) { t = 0.0f; }
                if (t > 1.0f) { t = 1.0f; }
                wv::Color c = bg.stops[0].color;
                for (std::size_t k = 1; k < bg.stops.size(); ++k)
                {
                    float o0 = bg.stops[k - 1].has_offset ? bg.stops[k - 1].offset : 0.0f;
                    float o1 = bg.stops[k].has_offset ? bg.stops[k].offset : 1.0f;
                    if (t >= o0 && t <= o1 && o1 > o0)
                    {
                        float f = (t - o0) / (o1 - o0);
                        const wv::Color& a = bg.stops[k - 1].color;
                        const wv::Color& b = bg.stops[k].color;
                        c.r = static_cast<unsigned char>(a.r + (b.r - a.r) * f);
                        c.g = static_cast<unsigned char>(a.g + (b.g - a.g) * f);
                        c.b = static_cast<unsigned char>(a.b + (b.b - a.b) * f);
                        c.a = static_cast<unsigned char>(a.a + (b.a - a.a) * f);
                        break;
                    }
                    c = bg.stops[k].color;
                }
                blend(i, j, c);
            }
        }
    }

    void draw_text(float x, float y, std::string_view text, const wv::ComputedStyle& style, wv::Color color) override
    {
        float lh = style.font_size * 1.2f;
        float tw = static_cast<float>(text.size()) * style.font_size * 0.6f;
        if (style.font_weight >= 700)
        {
            color.r = (color.r > 200) ? 255 : color.r;
        }
        fill_rect(x, y, tw, lh * 0.72f, color);
    }

    void draw_image(float x, float y, float w0, float h0, int node, std::string_view src) override
    {
        (void)node;
        unsigned char tint = static_cast<unsigned char>((src.size() * 37) % 120 + 80);
        wv::Color c{tint, static_cast<unsigned char>(200 - tint / 2), 180, 255};
        fill_rect(x, y, w0, h0, c);
        wv::Color edge{60, 60, 60, 255};
        fill_rect(x, y, w0, 1.5f, edge);
        fill_rect(x, y + h0 - 1.5f, w0, 1.5f, edge);
    }

    void draw_shadow(float x, float y, float w0, float h0, const float radius[4], const wv::Shadow& shadow) override
    {
        (void)radius;
        wv::Color c = shadow.color;
        c.a = static_cast<unsigned char>(c.a / 2);
        fill_rect(x + shadow.dx, y + shadow.dy, w0, h0, c);
    }

    void push_clip(float x, float y, float w0, float h0) override
    {
        const Clip& prev = clips.back();
        Clip c{sx(x), sy(y), sx(x + w0), sy(y + h0)};
        if (c.x0 < prev.x0) { c.x0 = prev.x0; }
        if (c.y0 < prev.y0) { c.y0 = prev.y0; }
        if (c.x1 > prev.x1) { c.x1 = prev.x1; }
        if (c.y1 > prev.y1) { c.y1 = prev.y1; }
        clips.push_back(c);
    }

    void pop_clip() override
    {
        if (clips.size() > 1)
        {
            clips.pop_back();
        }
    }
};

int main(int argc, char** argv)
{
    const char* html_path = (argc > 1) ? argv[1] : "/home/fox/Downloads/yFree5.html";
    const char* out_path = (argc > 2) ? argv[2] : "/tmp/wview5.ppm";
    FILE* f = std::fopen(html_path, "rb");
    assert(f);
    std::string src;
    char buf[65536];
    std::size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0)
    {
        src.append(buf, n);
    }
    std::fclose(f);

    wv::Document doc = wv::parse_html(src);
    int style = doc.first_element_by_tag(0, "style");
    wv::Stylesheet sheet = wv::parse_css(doc.text_of(doc.node(style).first_child));

    TestText text;
    TestImage image;
    wv::ResolveContext ctx;
    ctx.viewport_width = 1280.0f;
    wv::LayoutResult out = wv::layout_document(doc, sheet, text, &image, ctx, 1280.0f, 720.0f);

    const wv::Box& root = out.boxes[static_cast<std::size_t>(out.root_box)];
    Ppm ppm;
    ppm.init(1280, static_cast<int>(root.h) + 1);
    wv::Color bg{11, 11, 14, 255};
    ppm.fill_rect(0, 0, 1280, root.h, bg);
    wv::paint_tree(doc, out, out.root_box, ppm);

    FILE* o = std::fopen(out_path, "wb");
    assert(o);
    std::fprintf(o, "P6\n%d %d\n255\n", ppm.w, ppm.h);
    std::fwrite(ppm.px.data(), 1, ppm.px.size(), o);
    std::fclose(o);
    std::printf("wrote %s %dx%d\n", out_path, ppm.w, ppm.h);
    return 0;
}
