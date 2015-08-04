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
    PAWrapper wrapper("ncurses volume app");
    TUI tui(&wrapper);
    tui.run();
    return EXIT_SUCCESS;
}

