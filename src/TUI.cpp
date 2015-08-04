/*
 * TUI.cpp
 *
 *  Created on: 26 Jul 2015
 *      Author: rkfg
 */

#include "TUI.h"

TUI::TUI(PAWrapper* sink_wrapper) {
    this->sink_wrapper = sink_wrapper;
    signal(SIGINT, finish);
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    keypad(stdscr, true);
    nonl();
    cbreak();
    noecho();
    timeout(100);
    curs_set(0);
    box(stdscr, 0, 0);
}

void TUI::redraw_screen() {
    int y, x;
    getmaxyx(stdscr, y, x);
    clear();
    resize_term(y, x);
    box(stdscr, 0, 0);
}

bool TUI::handle_keys() {
    int c = getch();
    unsigned int sinksCount = sink_wrapper->get_sinks_count();
    switch (c) {
    case KEY_RESIZE:
        redraw_screen();
        draw_windows();
        break;
    case KEY_UP:
        if (--selected < 0) {
            selected = 0;
        }
        update_focus();
        break;
    case KEY_DOWN:
        if (++selected >= sinksCount) {
            selected = sinksCount - 1;
        }
        update_focus();
        break;
    case KEY_RIGHT:
    case KEY_LEFT:
        get_selected_panel().update_sink(
                sink_wrapper->change_volume(sinks[selected].idx, 1024,
                        c == KEY_RIGHT));
        draw_windows();
        break;
    case 'q':
        return false;
    }
    return true;
}

void TUI::draw_windows() {
    for (Panel panel : panels) {
        panel.redraw();
    }
    refresh();
}

void TUI::create_panel(int x, int y, int w, int h, string name, long value) {
}

void TUI::update_focus() {
    for (int i = 0; i < panels.size(); i++) {
        panels[i].select(i == selected);
    }
}

void TUI::update_panels(bool full) {
    sink_wrapper->collect_sinks();
    int y = 2;
    sinks = sink_wrapper->list_sinks();
    panels.clear();
    for (Sink sink : sinks) {
        Panel panel = Panel(2, y, 50, 3, sink);
        panels.push_back(panel);
        y += 4;
    }
    update_focus();
    if (full) {
        redraw_screen();
    }
    draw_windows();
}

void TUI::run() {
    update_panels();
    for (;;) {
        if (!handle_keys()) {
            break;
        }
        if (sink_wrapper->get_external_change()) {
            update_panels(true);
        }
    }
}

Panel& TUI::get_selected_panel() {
    return panels[selected];
}

TUI::~TUI() {
    endwin();
}
