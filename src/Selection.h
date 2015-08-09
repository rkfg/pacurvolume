/*
 * Selection.h
 *
 *  Created on: 8 Aug 2015
 *      Author: rkfg
 */

#ifndef SRC_SELECTION_H_
#define SRC_SELECTION_H_

#include <algorithm>
#include <memory>

using namespace std;

class Selection;

typedef shared_ptr<Selection> PSelection;

class Selection {
private:
    int visible_start = 0;
    int visible_end = 0;
    int selected = 0;
    int pos_count = 0;
    int panel_height = 0;
public:
    Selection(int pos_count = 0, int panel_height = 4);
    void dec();
    void inc();
    void set_pos_count(int pos_count);
    int get_selected();
    void set_height(int h);
    bool is_visible(int idx);
    void move_to_view();
    void debug();
    virtual ~Selection();

    int getVisibleEnd() const {
        return visible_end;
    }

    int getVisibleStart() const {
        return visible_start;
    }
};

#endif /* SRC_SELECTION_H_ */
