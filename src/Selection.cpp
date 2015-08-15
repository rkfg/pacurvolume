/*
 * Selection.cpp
 *
 *  Created on: 8 Aug 2015
 *      Author: rkfg
 */

#include "Selection.h"
#include "ncurses.h"

void Selection::dec() {
    set_selected(selected - 1);
}

void Selection::inc() {
    set_selected(selected + 1);
}

Selection::Selection(int pos_count, int panel_height) :
        pos_count(pos_count), panel_height(panel_height) {
}

void Selection::set_pos_count(int pos_count) {
    this->pos_count = pos_count;
    if (selected >= pos_count) {
        set_selected(pos_count - 1);
    }
    if (selected < 0 && pos_count > 0) {
        set_selected(0);
    }
}

int Selection::get_selected() {
    return selected;
}

bool Selection::move_to_view() {
    int shift = 0;
    if (selected >= visible_end && selected < pos_count) {
        shift = visible_end - selected + 1;
    }
    if (selected < visible_start && selected >= 0) {
        shift = selected - visible_start; // negative
    }
    visible_start += shift;
    visible_end += shift;
    return shift != 0;
}

Selection::~Selection() {
}

void Selection::set_height(int h) {
    int available = (h - 2) / panel_height;
    int need = pos_count - visible_start;
    if (visible_start + available > need && visible_start > 0) {
        // scroll up, we have more space than we need to show all that's left
        visible_start -= (available - need);
        visible_end = pos_count;
    } else {
        visible_end = visible_start + min(need, available);
    }
}

bool Selection::is_visible(int idx) {
    return idx >= visible_start && idx < visible_end;
}

void Selection::set_selected(int new_selected) {
    prev_selected = selected;
    selected = new_selected;
    if (selected < 0) {
        selected = 0;
    }
    if (selected >= pos_count) {
        selected = pos_count - 1;
    }
}
