//============================================================================
// Name        : cpptest.cpp
// Author      : rkfg
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include "TUI.h"
#include "PAWrapper.h"

using namespace std;

int main(void) {
    shared_ptr<PAWrapper> wrapper = shared_ptr<PAWrapper>(
            new PAWrapper("ncurses volume app"));
    TUI tui(wrapper);
    tui.run();
    return EXIT_SUCCESS;
}

