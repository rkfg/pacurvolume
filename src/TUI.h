/*
 * TUI.h
 *
 *  Created on: 26 Jul 2015
 *      Author: rkfg
 */

#ifndef SRC_TUI_H_
#define SRC_TUI_H_

#include <algorithm>
#include <signal.h>
#include <ncurses.h>
#include <vector>
#include "Panel.h"
#include "PAWrapper.h"
#include "Selection.h"

using namespace std;

class TUI {
private:
    PSelection selection = PSelection(new Selection);
    const string title = "PulseAudio Volume Control";
    shared_ptr<PAWrapper> sink_wrapper;
    vector<PPanel> panels;
    shared_ptr<vector<PSink>> sinks;
    static void finish(int sig) {
        endwin();
        exit(0);
    }
    PPanel get_selected_panel();
    void update_panels();
    void redraw_root();

public:
    TUI(shared_ptr<PAWrapper> psink_wrapper);
    bool handle_keys();
    void draw_windows();
    void update_focus(bool redraw);
    void run();
    virtual ~TUI();
};

#endif /* SRC_TUI_H_ */
