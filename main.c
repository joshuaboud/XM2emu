#include <stdio.h>
#include <ncurses.h>
#include "debugger.h"



int main(int argc, char *argv[]){
  // initialise ncurses
  initscr();
  scrollok(stdscr,TRUE);
  debuggerMenu();
  endwin();
  return 0;
}
