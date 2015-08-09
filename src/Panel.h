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
#include "Selection.h"

using namespace std;

class Panel;

typedef shared_ptr<Panel> PPanel;

class Panel {
private:
    WINDOW* window;
    PSink psink;
    bool selected = false;
    int pos, height, width;
    int pos_to_y(int pos);
    PSelection sel;
public:
    Panel(int pos, int w, int h, PSink psink, PSelection sel);
    void redraw();
    void select(bool selected);
    void update_sink(PSink psink);
    virtual ~Panel();
};

#endif /* SRC_PANEL_H_ */
