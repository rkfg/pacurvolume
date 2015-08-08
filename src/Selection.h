/*
 * Selection.h
 *
 *  Created on: 8 Aug 2015
 *      Author: rkfg
 */

#ifndef SRC_SELECTION_H_
#define SRC_SELECTION_H_

class Selection {
private:
    int visible_start = 0;
    int visible_end = 0;
    int selected = 0;
    int pos_count = 0;
public:
    Selection(int pos_count = 0);
    void dec();
    void inc();
    void set_pos_count(int pos_count);
    int get_selected();
    virtual ~Selection();
};

#endif /* SRC_SELECTION_H_ */
