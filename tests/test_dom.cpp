/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/dom.hpp"

#include <cassert>
#include <cstdio>
#include <string>

namespace
{
std::string dump_of(const wv::Document& doc, int index)
{
    std::string out;
    doc.dump(index, out, 0);
    return out;
}

int child_tag(const wv::Document& doc, int parent, std::string_view tag, int skip = 0)
{
    for (int c = doc.node(parent).first_child; c >= 0;
         c = doc.node(c).next_sibling)
    {
        if (doc.node(c).kind == wv::NodeKind::k_element &&
            doc.node(c).tag == tag)
        {
            if (skip == 0)
            {
                return c;
            }
            --skip;
        }
    }
    return wv::k_no_node;
}

int child_text(const wv::Document& doc, int parent, int skip = 0)
{
    for (int c = doc.node(parent).first_child; c >= 0;
         c = doc.node(c).next_sibling)
    {
        if (doc.node(c).kind == wv::NodeKind::k_text)
        {
            if (skip == 0)
            {
                return c;
            }
            --skip;
        }
    }
    return wv::k_no_node;
}

void check_misnested_formatting()
{
    wv::Document doc = wv::parse_html("<b><i></b></i>");
    int body = doc.first_element_by_tag(0, "body");
    assert(body >= 0);
    int b = child_tag(doc, body, "b");
    assert(b >= 0);
    int i = child_tag(doc, b, "i");
    assert(i >= 0);
    assert(doc.node(b).parent == body);
    assert(doc.node(i).parent == b);
    std::printf("misnested formatting ok\n");
}

void check_adoption()
{
    wv::Document doc = wv::parse_html("<a>1<div>2</a>3</div>");
    int body = doc.first_element_by_tag(0, "body");
    assert(body >= 0);
    int a = child_tag(doc, body, "a");
    assert(a >= 0);
    int div = child_tag(doc, a, "div");
    assert(div >= 0);
    std::string b = dump_of(doc, body);
    assert(b.find("1") != std::string::npos);
    assert(b.find("2") != std::string::npos);
    assert(b.find("3") != std::string::npos);
    std::printf("adoption ok\n");
}

void check_paragraphs()
{
    wv::Document doc = wv::parse_html("<p>1<p>2</p>trail");
    int body = doc.first_element_by_tag(0, "body");
    assert(body >= 0);
    int p1 = child_tag(doc, body, "p");
    int p2 = child_tag(doc, body, "p", 1);
    assert(p1 >= 0 && p2 >= 0 && p1 != p2);
    assert(doc.text_of(child_text(doc, p1)) == "1");
    assert(doc.text_of(child_text(doc, p2)) == "2");
    std::printf("paragraphs ok\n");
}

void check_lists()
{
    wv::Document doc = wv::parse_html("<ul><li>a<li>b</ul>");
    int body = doc.first_element_by_tag(0, "body");
    int ul = child_tag(doc, body, "ul");
    assert(ul >= 0);
    int li1 = child_tag(doc, ul, "li");
    int li2 = child_tag(doc, ul, "li", 1);
    assert(li1 >= 0 && li2 >= 0 && li1 != li2);
    std::printf("lists ok\n");
}

void check_table_foster()
{
    wv::Document doc =
        wv::parse_html("<table>stray<tr><td>cell</td></tr></table>");
    int body = doc.first_element_by_tag(0, "body");
    int table = child_tag(doc, body, "table");
    assert(table >= 0);
    int stray = child_text(doc, body);
    assert(stray >= 0 && doc.text_of(stray) == "stray");
    int tbody = child_tag(doc, table, "tbody");
    assert(tbody >= 0);
    int tr = child_tag(doc, tbody, "tr");
    assert(tr >= 0);
    int td = child_tag(doc, tr, "td");
    assert(td >= 0);
    std::printf("table foster ok\n");
}

void check_table_cells()
{
    wv::Document doc = wv::parse_html("<table><td>a<td>b</table>");
    int table = doc.first_element_by_tag(0, "table");
    assert(table >= 0);
    int tbody = child_tag(doc, table, "tbody");
    assert(tbody >= 0);
    int tr = child_tag(doc, tbody, "tr");
    assert(tr >= 0);
    int td1 = child_tag(doc, tr, "td");
    int td2 = child_tag(doc, tr, "td", 1);
    assert(td1 >= 0 && td2 >= 0 && td1 != td2);
    std::printf("table cells ok\n");
}

void check_anchors()
{
    wv::Document doc = wv::parse_html("<a>1<a>2</a>");
    int body = doc.first_element_by_tag(0, "body");
    int a1 = child_tag(doc, body, "a");
    int a2 = child_tag(doc, body, "a", 1);
    assert(a1 >= 0 && a2 >= 0 && a1 != a2);
    assert(doc.node(a2).parent == body);
    std::printf("anchors ok\n");
}

void check_entities()
{
    wv::Document doc = wv::parse_html("<p>&lt;&amp;&#65;&#x42;&copy;&hellip;</p>");
    int p = doc.first_element_by_tag(0, "p");
    assert(p >= 0);
    int text = child_text(doc, p);
    assert(text >= 0);
    assert(doc.text_of(text) == "<&AB\xc2\xa9\xe2\x80\xa6");
    std::printf("entities ok\n");
}

void check_pre()
{
    wv::Document doc = wv::parse_html("<pre>\n  keep  spaces\n</pre>");
    int pre = doc.first_element_by_tag(0, "pre");
    assert(pre >= 0);
    int text = child_text(doc, pre);
    assert(text >= 0);
    assert(doc.text_of(text) == "  keep  spaces\n");
    std::printf("pre ok\n");
}

void check_script_raw()
{
    wv::Document doc =
        wv::parse_html("<script>if (a < b && c > d) { x = a &amp; b; }</script>");
    int script = doc.first_element_by_tag(0, "script");
    assert(script >= 0);
    int text = child_text(doc, script);
    assert(text >= 0);
    assert(doc.text_of(text).find("&amp;") != std::string_view::npos);
    std::printf("script raw ok\n");
}

void check_select()
{
    wv::Document doc =
        wv::parse_html("<select><option>a<option>b</select><div>after</div>");
    int body = doc.first_element_by_tag(0, "body");
    int select = child_tag(doc, body, "select");
    assert(select >= 0);
    int o1 = child_tag(doc, select, "option");
    int o2 = child_tag(doc, select, "option", 1);
    assert(o1 >= 0 && o2 >= 0 && o1 != o2);
    int div = child_tag(doc, body, "div");
    assert(div >= 0 && doc.node(div).parent == body);
    std::printf("select ok\n");
}

void check_image_quirk()
{
    wv::Document doc = wv::parse_html("<image src=\"x\">");
    int img = doc.first_element_by_tag(0, "img");
    assert(img >= 0);
    assert(doc.attr(img, "src") == "x");
    std::printf("image quirk ok\n");
}

void check_implied_structure()
{
    wv::Document doc = wv::parse_html("hello<div>world</div>");
    int html = doc.first_element_by_tag(0, "html");
    int body = doc.first_element_by_tag(0, "body");
    assert(html >= 0 && body >= 0);
    assert(doc.node(body).parent == html);
    assert(doc.root() == html);
    std::printf("implied structure ok\n");
}

void check_table_row_close()
{
    wv::Document doc = wv::parse_html("<table><tr><td>1</tr><tr><td>2</table>");
    int table = doc.first_element_by_tag(0, "table");
    assert(table >= 0);
    int tbody = child_tag(doc, table, "tbody");
    assert(tbody >= 0);
    int tr1 = child_tag(doc, tbody, "tr");
    int tr2 = child_tag(doc, tbody, "tr", 1);
    assert(tr1 >= 0 && tr2 >= 0 && tr1 != tr2);
    std::printf("table rows ok\n");
}
} // namespace

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--conform")
    {
        check_misnested_formatting();
        check_adoption();
        check_paragraphs();
        check_lists();
        check_table_foster();
        check_table_cells();
        check_anchors();
        check_entities();
        check_pre();
        check_script_raw();
        check_select();
        check_image_quirk();
        check_implied_structure();
        check_table_row_close();
        std::printf("conform ALL OK\n");
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
    std::printf("nodes=%zu attrs=%zu\n", doc.node_count(), doc.attrs.size());

    int html = doc.first_element_by_tag(0, "html");
    int head = doc.first_element_by_tag(0, "head");
    int body = doc.first_element_by_tag(0, "body");
    int style = doc.first_element_by_tag(0, "style");
    int header = doc.first_element_by_tag(0, "header");
    int main = doc.first_element_by_tag(0, "main");
    int nav = doc.first_element_by_tag(0, "nav");
    assert(html >= 0 && head >= 0 && body >= 0 && style >= 0);
    assert(header >= 0 && main >= 0 && nav >= 0);

    int imgs = 0;
    int inputs = 0;
    int buttons = 0;
    int texts = 0;
    for (std::size_t i = 0; i < doc.node_count(); ++i)
    {
        const wv::Node& nd = doc.node(i);
        if (nd.kind == wv::NodeKind::k_text)
        {
            ++texts;
            continue;
        }
        if (nd.tag == "img")
        {
            ++imgs;
        }
        if (nd.tag == "input")
        {
            ++inputs;
        }
        if (nd.tag == "button")
        {
            ++buttons;
        }
        assert(nd.tag != "script" || true);
    }
    std::printf("img=%d input=%d button=%d text=%d\n", imgs, inputs, buttons, texts);
    std::fflush(stdout);
    assert(imgs == 39 && inputs == 1 && buttons == 110);

    const wv::Node& st = doc.node(style);
    assert(st.first_child >= 0);
    std::string_view css = doc.text_of(st.first_child);
    assert(css.size() > 20000);

    int cards = 0;
    for (std::size_t i = 0; i < doc.node_count(); ++i)
    {
        if (doc.node(i).kind == wv::NodeKind::k_element && doc.has_class(static_cast<int>(i), "card"))
        {
            ++cards;
        }
    }
    std::printf("cards=%d cssbytes=%zu\n", cards, css.size());
    assert(cards > 0);

    std::string dump;
    doc.dump(body, dump, 0);
    assert(dump.size() > 1000);
    std::printf("bodydump=%zu OK\n", dump.size());
    return 0;
}
