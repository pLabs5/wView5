/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/document.hpp"

#include <cassert>
#include <cstdio>
#include <string>

namespace
{
struct MapLoader : wv::ResourceLoader
{
    bool load(std::string_view url, std::string& bytes, std::string& mime) override
    {
        if (url == "https://example.com/sub/style.css")
        {
            bytes = "div{color:green}div p{color:blue}";
            mime = "text/css";
            return true;
        }
        if (url == "https://example.com/sub/app.js")
        {
            bytes = "var b = 2;";
            mime = "text/javascript";
            return true;
        }
        if (url == "https://example.com/sub/print.css")
        {
            bytes = "body{color:black}";
            mime = "text/css";
            return true;
        }
        return false;
    }
};

void check_urls()
{
    assert(wv::resolve_url("https://example.com/a/b/c.html", "d.css") ==
           "https://example.com/a/b/d.css");
    assert(wv::resolve_url("https://example.com/a/b/c.html", "../d.css") ==
           "https://example.com/a/d.css");
    assert(wv::resolve_url("https://example.com/a/b/c.html", "/d.css") ==
           "https://example.com/d.css");
    assert(wv::resolve_url("https://example.com/a/b/c.html",
                           "https://other.com/d.css") == "https://other.com/d.css");
    assert(wv::resolve_url("https://example.com/a/b/c.html", "//other.com/d.css") ==
           "https://other.com/d.css");
    assert(wv::resolve_url("file:///a/b/c.html", "d.css") == "file:///a/b/d.css");
    assert(wv::resolve_url("file:///a/b/c.html", "../d.css") == "file:///a/d.css");
    std::printf("urls ok\n");
}

void check_bundle()
{
    FILE* f = std::fopen("/tmp/docfix/page.html", "rb");
    assert(f);
    std::string html;
    char buf[4096];
    std::size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0)
    {
        html.append(buf, n);
    }
    std::fclose(f);
    MapLoader loader;
    wv::DocumentBundle bundle =
        wv::load_document(html, "https://example.com/sub/page.html", loader);
    assert(bundle.title == "Fixture");
    assert(bundle.base_url == "https://example.com/sub/");
    assert(bundle.sheet.rules.size() == 3);
    assert(bundle.scripts.size() == 2);
    assert(bundle.scripts[0].text == "var a = 1;");
    assert(!bundle.scripts[0].external);
    assert(bundle.scripts[1].text == "var b = 2;");
    assert(bundle.scripts[1].external);
    assert(bundle.scripts[1].url == "https://example.com/sub/app.js");
    wv::ResolveContext ctx;
    ctx.viewport_width = 1280.0f;
    wv::ElementState no_state;
    int div = bundle.dom.first_element_by_tag(0, "div");
    assert(div >= 0);
    std::vector<std::pair<std::string, std::string>> vars;
    wv::ComputedStyle style =
        wv::resolve_style(bundle.dom, div, nullptr, nullptr, bundle.sheet, no_state,
                           ctx, vars);
    assert(style.color.g == 128);
    std::printf("bundle ok\n");
}
} // namespace

int main()
{
    check_urls();
    check_bundle();
    std::printf("document ALL OK\n");
    return 0;
}
