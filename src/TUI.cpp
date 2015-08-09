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
}

void TUI::redraw_root() {
    int y, x;
    getmaxyx(stdscr, y, x);
    clear();
    resize_term(y, x);
    box(stdscr, 0, 0);
    mvprintw(0, (x - title.length()) / 2, title.c_str());
    selection->set_height(y);
    refresh();
}

bool TUI::handle_keys() {
    int c = getch();
    unsigned int sinksCount = sink_wrapper->get_sinks_count();
    switch (c) {
    case KEY_RESIZE:
        redraw_root();
        draw_windows();
        break;
    case KEY_UP:
        selection->dec();
        update_focus();
        break;
    case KEY_DOWN:
        selection->inc();
        update_focus();
        break;
    case KEY_RIGHT:
    case KEY_LEFT: {
        PPanel selectedPanel = get_selected_panel();
        if (selectedPanel != nullptr) {
            selectedPanel->update_sink(
                    sink_wrapper->change_volume(
                            (*sinks)[selection->get_selected()]->idx, 1024,
                            c == KEY_RIGHT));
            //draw_windows();
        }
        break;
    }
    case 'q':
        return false;
    }
    return true;
}

void TUI::draw_windows() {
    refresh();
    selection->move_to_view();
    for (int i = selection->getVisibleStart(); i < selection->getVisibleEnd();
            i++) {
        panels[i]->redraw();
    }
}

void TUI::update_focus() {
    int panels_number = panels.size();
    int selected = selection->get_selected();
    for (int i = 0; i < panels_number; i++) {
        panels[i]->select(i == selected);
    }
    draw_windows();
}

void TUI::update_panels(bool full) {
    sink_wrapper->collect_sinks();
    int pos = 0;
    sinks = sink_wrapper->list_sinks();
    selection->set_pos_count(sinks->size());
    panels.clear();
    for (PSink sink : *sinks) {
        PPanel panel = PPanel(new Panel(pos++, 50, 3, sink, selection));
        panels.push_back(panel);
    }
    if (full) {
        redraw_root();
    }
    update_focus();
}

void TUI::run() {
    update_panels(true);
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
    int selected = selection->get_selected();
    if (selected < 0) {
        return nullptr;
    }
    return panels[selected];
}

TUI::~TUI() {
    endwin();
}
