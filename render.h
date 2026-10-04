#pragma once

#include "./hanoi.h"

enum render_input {
  RENDER_NONE = 0, /* timeout, resize, or interrupted - caller re-checks state */
  RENDER_FORWARD,  /* the viewer wants the next move */
  RENDER_BACK,     /* the viewer wants to undo the last move */
  RENDER_QUIT,     /* the viewer wants to stop (q / Esc / Ctrl+C) */
};

/* Put the terminal into raw/alt-screen mode and draw nothing yet. Exits the
   process if termbox cannot start. */
void
render_init(void);

/* Draw the current puzzle state and present it. */
void
render_puzzle(const struct Puzzle* puzzle);

/* Block for up to `timeout_ms` waiting for a keypress, then classify it. */
enum render_input
render_wait_for_input(int timeout_ms);

/* Restore the terminal. Safe to call more than once. */
void
render_shutdown(void);
