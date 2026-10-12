/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wview/css.hpp"
#include "wview/dom.hpp"
#include "wview/layout.hpp"

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
int order_log[16];
int order_count = 0;

void recorder(wv::Event& ev, void* user) noexcept
{
    (void)user;
    if (order_count < 16)
    {
        order_log[order_count++] = ev.current_target;
    }
}

void stopper(wv::Event& ev, void* user) noexcept
{
    (void)user;
    ev.stop_propagation();
}

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

void check_query()
{
    wv::Document doc = wv::parse_html(
        "<div id=\"a\" class=\"x\"><p class=\"x\">1</p><p>2</p><span>3</span><p></p>"
        "</div><div class=\"x\"><p>4</p></div>");
    int root = doc.root();
    assert(root >= 0);
    int first = doc.query_selector(root, "p.x");
    assert(first >= 0 && doc.text_of(doc.node(first).first_child) == "1");
    std::vector<int> all;
    doc.query_selector_all(root, "div.x p", all);
    assert(all.size() == 4);
    int by_id = doc.query_selector(root, "#a");
    assert(by_id >= 0 && doc.attr(by_id, "id") == "a");
    assert(doc.query_selector(root, "table") < 0);
    assert(doc.query_selector(root, "div > p.x") == first);
    std::printf("query ok\n");
}

void check_mutation()
{
    wv::Document doc = wv::parse_html("<div><p>old</p></div>");
    int div = doc.first_element_by_tag(0, "div");
    assert(div >= 0);
    int span = doc.create_element("span");
    assert(span >= 0);
    assert(doc.set_attribute(span, "class", "new"));
    assert(doc.append_child(div, span));
    assert(doc.has_class(span, "new"));
    assert(doc.query_selector(div, "span.new") == span);
    assert(doc.set_text(span, "hi"));
    assert(doc.text_of(doc.node(span).first_child) == "hi");
    int p = doc.first_element_by_tag(0, "p");
    assert(p >= 0);
    assert(doc.remove_child(div, p));
    assert(doc.first_element_by_tag(0, "p") < 0);
    assert(doc.add_class(span, "extra"));
    assert(doc.has_class(span, "extra"));
    assert(doc.toggle_class(span, "extra") == false);
    assert(!doc.has_class(span, "extra"));
    assert(doc.toggle_class(span, "extra") == true);
    assert(doc.remove_attribute(span, "class"));
    assert(!doc.has_class(span, "new"));
    int b = doc.create_element("b");
    assert(doc.insert_before(div, b, span));
    assert(doc.previous_element_sibling(span) == b);
    assert(!doc.append_child(span, div));
    std::printf("mutation ok\n");
}

void check_events()
{
    wv::Document doc = wv::parse_html("<div><p><b>x</b></p></div>");
    int div = doc.first_element_by_tag(0, "div");
    int p = doc.first_element_by_tag(0, "p");
    int b = doc.first_element_by_tag(0, "b");
    assert(div >= 0 && p >= 0 && b >= 0);
    assert(doc.add_event_listener(div, "test", recorder, nullptr, true, false));
    assert(doc.add_event_listener(p, "test", recorder, nullptr, false, false));
    assert(doc.add_event_listener(b, "test", recorder, nullptr, false, false));
    order_count = 0;
    wv::Event ev;
    ev.type = "test";
    ev.bubbles = true;
    assert(doc.dispatch_event(b, ev));
    assert(order_count == 3);
    assert(order_log[0] == div && order_log[1] == b && order_log[2] == p);
    order_count = 0;
    ev.bubbles = false;
    assert(doc.dispatch_event(b, ev));
    assert(order_count == 2);
    assert(doc.remove_event_listener(p, "test", recorder, false));
    order_count = 0;
    ev.bubbles = true;
    assert(doc.dispatch_event(b, ev));
    assert(order_count == 2);
    assert(doc.add_event_listener(p, "stop", stopper, nullptr, false, false));
    assert(doc.add_event_listener(div, "stop", recorder, nullptr, false, false));
    order_count = 0;
    wv::Event ev2;
    ev2.type = "stop";
    ev2.bubbles = true;
    assert(doc.dispatch_event(b, ev2));
    assert(order_count == 0);
    wv::Event clicked = doc.click(b);
    assert(clicked.target == b && clicked.type == "click");
    std::printf("events ok\n");
}

void check_focus()
{
    wv::Document doc = wv::parse_html(
        "<div><button id=\"a\">A</button><input type=\"text\"><a>No href</a>"
        "<a href=\"x\">Yes</a><input type=\"hidden\"></div>");
    int a = doc.first_element_by_tag(0, "button");
    assert(a >= 0);
    assert(doc.active_element() < 0);
    assert(doc.focus(a));
    assert(doc.active_element() == a);
    assert(wv::focus_next(doc, false));
    assert(doc.active_element() != a);
    assert(wv::focus_next(doc, true));
    assert(doc.active_element() == a);
    doc.blur();
    assert(doc.active_element() < 0);
    int hidden = doc.query_selector(doc.root(), "input[type=hidden]");
    assert(hidden >= 0 && !doc.focus(hidden));
    std::printf("focus ok\n");
}

void check_scroll()
{
    wv::Document doc = wv::parse_html(
        "<div style=\"height:20px;overflow-y:scroll\"><p>a</p><p>b</p><p>c</p></div>");
    wv::Stylesheet sheet = wv::parse_css("");
    TestText text;
    wv::ResolveContext ctx;
    ctx.viewport_width = 640.0f;
    wv::LayoutResult out = wv::layout_document(doc, sheet, text, nullptr, ctx, 640.0f, 480.0f);
    int div = doc.first_element_by_tag(0, "div");
    int box = wv::find_box_for_node(out, div);
    assert(box >= 0);
    assert(out.boxes[static_cast<std::size_t>(box)].scroll_h > 20.0f);
    assert(!wv::set_scroll(out, box, 0.0f, 0.0f));
    assert(wv::set_scroll(out, box, 0.0f, 100000.0f));
    assert(out.boxes[static_cast<std::size_t>(box)].scroll_y > 0.0f);
    assert(out.boxes[static_cast<std::size_t>(box)].scroll_y <=
           out.boxes[static_cast<std::size_t>(box)].scroll_h);
    std::printf("scroll ok\n");
}
} // namespace

int main()
{
    check_query();
    check_mutation();
    check_events();
    check_focus();
    check_scroll();
    std::printf("api ALL OK\n");
    return 0;
}
