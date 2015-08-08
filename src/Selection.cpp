/*
 * Selection.cpp
 *
 *  Created on: 8 Aug 2015
 *      Author: rkfg
 */

#include "Selection.h"

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

Selection::Selection(int pos_count) :
        pos_count(pos_count) {
}

void Selection::set_pos_count(int pos_count){
    this->pos_count = pos_count;
    if (selected >= pos_count) {
        selected = pos_count - 1;
    }
    if (selected < 0 && pos_count > 0) {
        selected = 0;
    }
}

int Selection::get_selected() {
    return selected;
}

Selection::~Selection() {
}

