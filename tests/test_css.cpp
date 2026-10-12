/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/css.hpp"
#include "wview/dom.hpp"

#include <cassert>
#include <cstdio>
#include <string>

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--conform")
    {
        wv::Document doc = wv::parse_html(
            "<div id=\"a\" class=\"x y\" data-v=\"hello world\" lang=\"en-US\">"
            "<p class=\"x\">1</p><p>2</p><span>3</span><p></p>"
            "</div><input type=\"checkbox\" checked><input type=\"text\" required>");
        wv::ResolveContext ctx;
        ctx.viewport_width = 1280.0f;
        wv::ElementState no_state;
        auto match_first = [&](const char* sel_text) -> int {
            wv::Stylesheet sheet = wv::parse_css(std::string(sel_text) + "{color:red}");
            assert(!sheet.rules.empty() && !sheet.rules[0].selectors.empty());
            for (std::size_t i = 0; i < doc.node_count(); ++i)
            {
                if (doc.node(i).kind != wv::NodeKind::k_element)
                {
                    continue;
                }
                if (wv::match_selector(doc, static_cast<int>(i), sheet.rules[0].selectors[0],
                                       no_state, std::string()))
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        };
        auto style_of = [&](int node, const char* css_text) -> wv::ComputedStyle {
            wv::Stylesheet sheet = wv::parse_css(css_text);
            std::vector<std::pair<std::string, std::string>> vars;
            return wv::resolve_style(doc, node, nullptr, nullptr, sheet, no_state, ctx,
                                     vars);
        };
        int div = doc.first_element_by_tag(0, "div");
        assert(div >= 0);
        assert(match_first("[data-v]") == div);
        assert(match_first("[data-v=\"hello world\"]") == div);
        assert(match_first("[data-v~=world]") == div);
        assert(match_first("[data-v^=hello]") == div);
        assert(match_first("[data-v$=world]") == div);
        assert(match_first("[data-v*=\"o w\"]") == div);
        assert(match_first("[lang|=en]") == div);
        assert(match_first("[lang=en-us i]") == div);
        assert(match_first("[data-v~=xyz]") < 0);
        assert(match_first("div:not(.z)") == div);
        assert(match_first("div:not(.x)") < 0);
        assert(match_first("div:is(.x, .z)") == div);
        assert(match_first("div:where(.z)") < 0);
        assert(match_first("div:has(p)") == div);
        assert(match_first("div:has(> span)") == div);
        assert(match_first("div:has(table)") < 0);
        assert(match_first("div:empty") < 0);
        assert(match_first("p:nth-child(1)") >= 0);
        assert(match_first("p:nth-child(2n)") >= 0);
        assert(match_first("p:nth-child(odd)") >= 0);
        assert(match_first("p:nth-child(2)") >= 0);
        assert(match_first("span:only-of-type") >= 0);
        assert(match_first("p:first-of-type") >= 0);
        assert(match_first("input:checked") >= 0);
        assert(match_first("input:required") >= 0);
        assert(match_first("input:optional") >= 0);
        wv::ComputedStyle overridden =
            style_of(div, ".x{color:red}div.x{color:green}div:not(.z){color:blue}");
        assert(overridden.color.r == 0 && overridden.color.g == 0 &&
               overridden.color.b == 255);
        wv::ComputedStyle layered =
            style_of(div, "@layer a{div{color:red}}@layer b{div{color:green}}");
        assert(layered.color.g == 128);
        wv::ComputedStyle layered_beats =
            style_of(div, "@layer a{div{color:red}}div{color:green}");
        assert(layered_beats.color.g == 128);
        wv::ComputedStyle important_reversal = style_of(
            div, "@layer a{div{color:red !important}}@layer b{div{color:green !important}}");
        assert(important_reversal.color.r == 255);
        wv::ComputedStyle important_beats_unlayered =
            style_of(div, "@layer a{div{color:red !important}}div{color:green}");
        assert(important_beats_unlayered.color.r == 255);
        wv::ComputedStyle statement_order =
            style_of(div, "@layer b, a;@layer a{div{color:red}}@layer b{div{color:green}}");
        assert(statement_order.color.r == 255);
        std::printf("selectors ALL OK\n");
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
    assert(style >= 0);
    std::string_view css_text = doc.text_of(doc.node(style).first_child);
    wv::Stylesheet sheet = wv::parse_css(css_text);
    std::printf("rules=%zu\n", sheet.rules.size());
    assert(sheet.rules.size() > 150);

    int media_rules = 0;
    int var_decls = 0;
    for (const auto& r : sheet.rules)
    {
        if (r.media_max_width >= 0.0f)
        {
            ++media_rules;
        }
        for (const auto& d : r.declarations)
        {
            if (d.property.rfind("--", 0) == 0)
            {
                ++var_decls;
            }
        }
    }
    std::printf("media=%d vars=%d\n", media_rules, var_decls);
    assert(media_rules > 0 && var_decls > 0);

    wv::ResolveContext ctx;
    ctx.viewport_width = 1280.0f;
    int html = doc.first_element_by_tag(0, "html");
    assert(html >= 0);
    std::vector<std::pair<std::string, std::string>> html_vars;
    wv::ElementState no_state;
    wv::ComputedStyle hs = wv::resolve_style(doc, html, nullptr, nullptr, sheet, no_state, ctx, html_vars);
    int body = doc.first_element_by_tag(0, "body");
    assert(body >= 0);
    std::vector<std::pair<std::string, std::string>> body_vars;
    wv::ComputedStyle bs = wv::resolve_style(doc, body, &hs, &html_vars, sheet, no_state, ctx, body_vars);
    std::printf(
        "body bg=%02x%02x%02x display=%d fontsize=%.1f vars=%zu\n",
        bs.background.color.r,
        bs.background.color.g,
        bs.background.color.b,
        static_cast<int>(bs.display),
        bs.font_size,
        body_vars.size());
    assert(bs.background.has_color);
    assert(bs.background.color.r == 0x0b && bs.background.color.g == 0x0b && bs.background.color.b == 0x0e);
    assert(bs.display == wv::Display::k_block);

    int card = doc.first_element_by_class(0, "card");
    assert(card >= 0);
    int card_parent = doc.node(card).parent;
    std::vector<std::pair<std::string, std::string>> card_vars = body_vars;
    wv::ComputedStyle cs = wv::resolve_style(doc, card, &bs, &card_vars, sheet, no_state, ctx, card_vars);
    std::printf(
        "card display=%d gridcols=%zu gap=%.1f\n",
        static_cast<int>(cs.display),
        cs.grid_columns.size(),
        cs.gap_col);
    (void)card_parent;

    std::printf("OK\n");
    return 0;
}
