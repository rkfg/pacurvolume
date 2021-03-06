/*
 * Panel.cpp
 *
 *  Created on: 3 Aug 2015
 *      Author: rkfg
 */

#include "Panel.h"

Panel::Panel(int pos, int w, int h, PSink psink, shared_ptr<Selection> sel) {
    this->pos = pos;
    this->height = h;
    this->width = w;
    window = newwin(h, w, pos_to_y(pos), 1);
    this->psink = psink;
    this->sel = sel;
    redraw();
}

void Panel::redraw() {
    int target_y = pos_to_y(pos - sel->getVisibleStart());
    if (target_y != getpary(window)) {
        mvwin(window, target_y, 1);
    }
    if (getmaxx(window) != width) {
        wresize(window, height, width);
    }
    box(window, 0, 0);
    if (selected) {
        wattron(window, COLOR_PAIR(1));
    }
    pa_volume_t avg_vol = pa_cvolume_avg(&psink->volume);
    long vol = avg_vol * (getmaxx(window) - 1) / PA_VOLUME_UI_MAX;
    long volperc = avg_vol * 100 / PA_VOLUME_NORM;
    for (int i = 1; i < vol; i++) {
        mvwaddch(window, 1, i, BLOCK);
    }
    for (int i = max(1L, vol); i < getmaxx(window) - 1; i++) {
        mvwaddch(window, 1, i, ' ');
    }
    wattron(window, A_BOLD);
    string muted = psink->muted ? "[M] " : "";
    mvwprintw(window, 2, 1, "%sVolume: %d%%", muted.c_str(), volperc);
    wattroff(window, A_BOLD);
    wattroff(window, COLOR_PAIR(1));
    string name = psink->name;
    if (psink->sound) {
        name += " [+]";
    }
    mvwprintw(window, 0, 1, name.substr(0, width - 2).c_str());
    wnoutrefresh(window);
}

void Panel::select(bool selected) {
    this->selected = selected;
}

void Panel::update_sink(PSink psink) {
    this->psink = psink;
    redraw();
}

int Panel::pos_to_y(int pos) {
    return pos * (height + 1) + 2;
}

Panel::~Panel() {
    delwin(window);
}

