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

class Panel;

typedef shared_ptr<Panel> PPanel;

class Panel {
private:
    WINDOW* window;
    PSink psink;
    bool selected = false;
public:
    Panel(int x, int y, int w, int h, PSink psink);
    void redraw();
    void select(bool selected);
    void update_sink(PSink psink);
    virtual ~Panel();
};

#endif /* SRC_PANEL_H_ */
