/* File name: main.c
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Initialize ncurses, call debugger menu
 * Last Modified: 2019-07-27
 */

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
