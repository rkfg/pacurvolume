/*
 * Selection.cpp
 *
 *  Created on: 8 Aug 2015
 *      Author: rkfg
 */

#include "Selection.h"
#include "ncurses.h"

void Selection::dec() {
    if (--selected < 0) {
        selected = 0;
    }
}

void Selection::inc() {
    if (++selected >= pos_count) {
        selected = pos_count - 1;
    }
}

Selection::Selection(int pos_count, int panel_height) :
        pos_count(pos_count), panel_height(panel_height) {
}

void Selection::set_pos_count(int pos_count) {
    this->pos_count = pos_count;
    if (selected >= pos_count) {
        selected = pos_count - 1;
    }
    if (selected < 0 && pos_count > 0) {
        selected = 0;
    }
    debug();
}

int Selection::get_selected() {
    return selected;
}

void Selection::move_to_view() {
    int shift = 0;
    if (selected >= visible_end && selected < pos_count) {
        shift = visible_end - selected + 1;
    }
    if (selected < visible_start && selected >= 0) {
        shift = selected - visible_start; // negative
    }
    visible_start += shift;
    visible_end += shift;
    debug();
}

Selection::~Selection() {
}

void Selection::debug() {
    mvwprintw(stdscr, 1, 1, "Start: %d End: %d", visible_start, visible_end);
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
    debug();
}

bool Selection::is_visible(int idx) {
    return idx >= visible_start && idx < visible_end;
}
