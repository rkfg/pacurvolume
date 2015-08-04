/*
 * Panel.h
 *
 *  Created on: 3 Aug 2015
 *      Author: rkfg
 */

#ifndef SRC_PANEL_H_
#define SRC_PANEL_H_

#define BLOCK ' ' | A_REVERSE

#include <string>
#include <ncurses.h>
#include <pulse/volume.h>
#include "PAWrapper.h"

using namespace std;

class Panel {
private:
    WINDOW* window;
    Sink sink;
    bool selected = false;
public:
    Panel(int x, int y, int w, int h, Sink sink);
    void redraw();
    void select(bool selected);
    void update_sink(Sink sink);
    virtual ~Panel();
};

#endif /* SRC_PANEL_H_ */
