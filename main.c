#include <stdio.h>
#include <ncurses.h>
#include "debugger.h"

int main(int argc, char *argv[]){
  // initialise ncurses
  initscr();
  scrollok(stdscr,TRUE);
  debuggerMenu(); // open UI for debugger
  endwin();
  return 0;
}
