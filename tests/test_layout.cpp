/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/layout.hpp"

#include <cassert>
#include <cstdio>
#include <string>

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

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--conform")
    {
        wv::Document doc = wv::parse_html(
            "<table><caption>Cap</caption><thead><tr><th>H1</th><th>H2</th></tr></thead>"
            "<tbody><tr><td>a</td><td>b</td></tr>"
            "<tr><td colspan=\"2\">wide</td></tr></tbody></table>");
        wv::Stylesheet sheet = wv::parse_css("td{font-size:10px}th{font-size:10px}");
        TestText text;
        wv::ResolveContext ctx;
        ctx.viewport_width = 640.0f;
        wv::LayoutResult out = wv::layout_document(doc, sheet, text, nullptr, ctx, 640.0f, 480.0f);
        int table = doc.first_element_by_tag(0, "table");
        assert(table >= 0);
        int table_box = -1;
        for (std::size_t i = 0; i < out.boxes.size(); ++i)
        {
            if (out.boxes[i].node == table)
            {
                table_box = static_cast<int>(i);
            }
        }
        assert(table_box >= 0);
        const wv::Box& tb = out.boxes[static_cast<std::size_t>(table_box)];
        assert(tb.w == 640.0f && tb.h > 0.0f);
        float row1_y = -1.0f;
        int row1_cells = 0;
        float wide_w = 0.0f;
        float cell_w = 0.0f;
        for (std::size_t i = 0; i < out.boxes.size(); ++i)
        {
            const wv::Box& b = out.boxes[i];
            if (b.node >= 0 && doc.node(b.node).kind == wv::NodeKind::k_element &&
                doc.node(b.node).tag == "td")
            {
                if (doc.attr(b.node, "colspan") == "2")
                {
                    wide_w = b.w;
                }
                else
                {
                    if (row1_y < 0.0f)
                    {
                        row1_y = b.y;
                    }
                    if (b.y == row1_y)
                    {
                        ++row1_cells;
                        cell_w = b.w;
                    }
                }
            }
        }
        assert(row1_cells == 2);
        assert(wide_w > cell_w);
        assert(wide_w <= 640.0f + 1.0f);
        std::printf("table ALL OK\n");
        return 0;
    }
    const char* path = (argc > 1) ? argv[1] : "/home/fox/Downloads/yFree5.html";
    FILE* f = std::fopen(path, "rb");
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
    std::printf("boxes=%zu root=%d\n", out.boxes.size(), out.root_box);
    std::printf("boxes=%zu root=%d\n", out.boxes.size(), out.root_box);
    assert(out.root_box >= 0);

    const wv::Box& root = out.boxes[static_cast<std::size_t>(out.root_box)];
    std::printf("root %.0fx%.0f\n", root.w, root.h);
    assert(root.w == 1280.0f && root.h > 700.0f);

    int grid_box = -1;
    for (std::size_t i = 0; i < out.boxes.size(); ++i)
    {
        const wv::Box& b = out.boxes[i];
        if (b.node >= 0 && doc.node(b.node).kind == wv::NodeKind::k_element &&
            doc.has_class(b.node, "grid") && b.style.display == wv::Display::k_grid)
        {
            grid_box = static_cast<int>(i);
            break;
        }
    }
    assert(grid_box >= 0);
    const wv::Box& grid = out.boxes[static_cast<std::size_t>(grid_box)];
    float first_y = -1.0f;
    int first_row = 0;
    for (int c = grid.first_child; c >= 0; c = out.boxes[static_cast<std::size_t>(c)].next_sibling)
    {
        const wv::Box& ch = out.boxes[static_cast<std::size_t>(c)];
        if (first_y < 0.0f)
        {
            first_y = ch.y;
        }
        if (ch.y == first_y)
        {
            ++first_row;
        }
    }
    std::printf("gridcols=%zu firstrow=%d gridw=%.0f\n", grid.style.grid_columns.size(), first_row, grid.w);
    assert(grid.style.grid_columns.size() == 4 && first_row == 4);

    int header = doc.first_element_by_tag(0, "header");
    int header_box = -1;
    for (std::size_t i = 0; i < out.boxes.size(); ++i)
    {
        if (out.boxes[i].node == header)
        {
            header_box = static_cast<int>(i);
        }
    }
    assert(header_box >= 0);
    std::printf("header h=%.0f lines=%zu\n", out.boxes[static_cast<std::size_t>(header_box)].h, out.boxes[static_cast<std::size_t>(header_box)].lines.size());

    int lines_total = 0;
    for (const auto& b : out.boxes)
    {
        lines_total += static_cast<int>(b.lines.size());
    }
    std::printf("lines=%d OK\n", lines_total);
    assert(lines_total > 100);
    return 0;
}
