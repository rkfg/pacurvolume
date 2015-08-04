/*
 * Panel.cpp
 *
 *  Created on: 3 Aug 2015
 *      Author: rkfg
 */

#include "Panel.h"

Panel::Panel(int x, int y, int w, int h, Sink sink) {
    window = subwin(stdscr, h, w, y, x);
    this->sink = sink;
    redraw();
}

void Panel::redraw() {
    box(window, 0, 0);
    mvwprintw(window, 0, 1, sink.name.c_str());
    if (selected) {
        wattron(window, COLOR_PAIR(1));
    }
    long vol = sink.volume * (getmaxx(window) - 1) / PA_VOLUME_UI_MAX;
    long volperc = sink.volume * 100 / PA_VOLUME_NORM;
    for (int i = 1; i < vol; i++) {
        mvwaddch(window, 1, i, BLOCK);
    }
    for (int i = max(1L, vol); i < getmaxx(window) - 1; i++) {
        mvwaddch(window, 1, i, ' ');
    }
    wattron(window, A_BOLD);
    mvwprintw(window, 2, 1, "Volume: %d%%", volperc);
    wattroff(window, A_BOLD);
    wattroff(window, COLOR_PAIR(1));
    wrefresh(window);
}

void Panel::select(bool selected) {
    this->selected = selected;
    redraw();
}

void Panel::update_sink(Sink sink) {
    this->sink = sink;
    redraw();
}

Panel::~Panel() {
}

