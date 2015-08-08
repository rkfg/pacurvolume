/*
 * TUI.cpp
 *
 *  Created on: 26 Jul 2015
 *      Author: rkfg
 */

#include "TUI.h"

TUI::TUI(shared_ptr<PAWrapper> psink_wrapper) :
        sink_wrapper(psink_wrapper) {
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
    redraw_screen();
}

void TUI::redraw_screen() {
    int y, x;
    getmaxyx(stdscr, y, x);
    clear();
    resize_term(y, x);
    box(stdscr, 0, 0);
    mvprintw(0, (x - title.length()) / 2, title.c_str());
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
        selection.dec();
        update_focus();
        break;
    case KEY_DOWN:
        selection.inc();
        update_focus();
        break;
    case KEY_RIGHT:
    case KEY_LEFT: {
        PPanel selectedPanel = get_selected_panel();
        if (selectedPanel != nullptr) {
            selectedPanel->update_sink(
                    sink_wrapper->change_volume((*sinks)[selection.get_selected()]->idx, 1024,
                            c == KEY_RIGHT));
            draw_windows();
        }
        break;
    }
    case 'q':
        return false;
    }
    return true;
}

void TUI::draw_windows() {
    for (PPanel panel : panels) {
        panel->redraw();
    }
    refresh();
}

void TUI::update_focus() {
    int panels_number = panels.size();
    selection.set_pos_count(panels_number);
    int selected = selection.get_selected();
    for (int i = 0; i < panels_number; i++) {
        panels[i]->select(i == selected);
    }
}

void TUI::update_panels(bool full) {
    sink_wrapper->collect_sinks();
    int y = 2;
    sinks = sink_wrapper->list_sinks();
    panels.clear();
    for (PSink sink : *sinks) {
        PPanel panel = PPanel(new Panel(2, y, 50, 3, sink));
        panels.push_back(panel);
        y += 4;
    }
    if (full) {
        redraw_screen();
    }
    update_focus();
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

PPanel TUI::get_selected_panel() {
    int selected = selection.get_selected();
    if (selected < 0) {
        return nullptr;
    }
    return panels[selected];
}

TUI::~TUI() {
    endwin();
}
