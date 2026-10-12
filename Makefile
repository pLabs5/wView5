CXX ?= c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -Iinclude -Isrc
BUILD := build

all: $(BUILD)/test_dom $(BUILD)/test_css $(BUILD)/test_layout $(BUILD)/test_paint $(BUILD)/test_document $(BUILD)/test_api

$(BUILD)/test_dom: src/dom/parse.cpp src/dom/tree.cpp tests/test_dom.cpp include/wview/dom.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) src/dom/parse.cpp src/dom/tree.cpp tests/test_dom.cpp -o $@

$(BUILD)/test_css: src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp tests/test_css.cpp include/wview/dom.hpp include/wview/css.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp tests/test_css.cpp -o $@

$(BUILD)/test_layout: src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/layout/util.cpp src/layout/inline.cpp src/layout/tree.cpp src/layout/block.cpp src/layout/flex.cpp src/layout/grid.cpp src/layout/table.cpp src/layout/scroll.cpp tests/test_layout.cpp include/wview/dom.hpp include/wview/css.hpp include/wview/layout.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/layout/util.cpp src/layout/inline.cpp src/layout/tree.cpp src/layout/block.cpp src/layout/flex.cpp src/layout/grid.cpp src/layout/table.cpp src/layout/scroll.cpp tests/test_layout.cpp -o $@

$(BUILD)/test_paint: src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/layout/util.cpp src/layout/inline.cpp src/layout/tree.cpp src/layout/block.cpp src/layout/flex.cpp src/layout/grid.cpp src/layout/table.cpp src/layout/scroll.cpp src/paint.cpp tests/test_paint.cpp include/wview/dom.hpp include/wview/css.hpp include/wview/layout.hpp include/wview/paint.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/layout/util.cpp src/layout/inline.cpp src/layout/tree.cpp src/layout/block.cpp src/layout/flex.cpp src/layout/grid.cpp src/layout/table.cpp src/layout/scroll.cpp src/paint.cpp tests/test_paint.cpp -o $@

test: $(BUILD)/test_dom $(BUILD)/test_css $(BUILD)/test_layout $(BUILD)/test_paint $(BUILD)/test_document $(BUILD)/test_api
	./$(BUILD)/test_dom
	./$(BUILD)/test_dom --conform
	./$(BUILD)/test_css
	./$(BUILD)/test_css --conform
	./$(BUILD)/test_layout
	./$(BUILD)/test_layout --conform
	./$(BUILD)/test_paint
	./$(BUILD)/test_document
	./$(BUILD)/test_api

$(BUILD)/test_document: src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/doc/document.cpp tests/test_document.cpp include/wview/dom.hpp include/wview/css.hpp include/wview/document.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) src/dom/parse.cpp src/dom/tree.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/doc/document.cpp tests/test_document.cpp -o $@

$(BUILD)/test_api: src/dom/parse.cpp src/dom/tree.cpp src/dom/query.cpp src/dom/edit.cpp src/dom/event.cpp src/dom/focus.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/layout/util.cpp src/layout/inline.cpp src/layout/tree.cpp src/layout/block.cpp src/layout/flex.cpp src/layout/grid.cpp src/layout/table.cpp src/layout/scroll.cpp tests/test_api.cpp include/wview/dom.hpp include/wview/css.hpp include/wview/layout.hpp include/wview/event.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) src/dom/parse.cpp src/dom/tree.cpp src/dom/query.cpp src/dom/edit.cpp src/dom/event.cpp src/dom/focus.cpp src/css/lengths.cpp src/css/colors.cpp src/css/gradients.cpp src/css/parse.cpp src/css/match.cpp src/css/cascade.cpp src/css/apply_box.cpp src/css/apply_flex.cpp src/css/apply_paint.cpp src/css/apply_text.cpp src/css/apply_misc.cpp src/layout/util.cpp src/layout/inline.cpp src/layout/tree.cpp src/layout/block.cpp src/layout/flex.cpp src/layout/grid.cpp src/layout/table.cpp src/layout/scroll.cpp tests/test_api.cpp -o $@

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
