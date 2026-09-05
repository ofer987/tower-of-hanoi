/* Expose the BSD/POSIX extensions termbox2.h needs (cfmakeraw, SIGWINCH):
   _DEFAULT_SOURCE on glibc, _DARWIN_C_SOURCE on macOS. _POSIX_C_SOURCE keeps
   the rest of the POSIX surface available under -std=c2x. */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#define _DARWIN_C_SOURCE

#include <locale.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./hanoi.h"
#include "./render.h"

#define TB_IMPL
#include "./termbox2.h"

#define AREA_LEFT         0
#define AREA_RIGHT        126
#define AREA_TOP          0
#define AREA_BOTTOM       20

#define FLOOR_ROW         9  /* the bottom disc of every tower sits on this row */
#define TOWER_STRIDE      40 /* horizontal distance between one tower and the next */
#define TOWER_CENTRE      10 /* centre column of a tower, relative to its block */
#define LABEL_COLUMN      24 /* "(n)" size label, relative to the tower's block */

#define STATUS_ROW        (AREA_BOTTOM - 1)
#define HINT_ROW          AREA_BOTTOM

#define COUNT_BUFFER_SIZE 4 /* up to three digits plus a terminator */

enum screen_slot { SLOT_BLANK = 0, SLOT_TOWER_1, SLOT_TOWER_2, SLOT_TOWER_3, SLOT_STATUS, SLOT_COUNT };

static const uintattr_t foreground[SLOT_COUNT] = {
  [SLOT_BLANK] = TB_DEFAULT,
  [SLOT_TOWER_1] = TB_BLUE,
  [SLOT_TOWER_2] = TB_GREEN,
  [SLOT_TOWER_3] = TB_RED,
  [SLOT_STATUS] = TB_WHITE,
};

static const uintattr_t background[SLOT_COUNT] = {
  [SLOT_BLANK] = TB_DEFAULT,
  [SLOT_TOWER_1] = TB_DEFAULT,
  [SLOT_TOWER_2] = TB_DEFAULT,
  [SLOT_TOWER_3] = TB_DEFAULT,
  [SLOT_STATUS] = TB_DEFAULT,
};

static bool terminal_is_up = false;

void
render_init(void) {
  setlocale(LC_ALL, "");

  if (tb_init() != 0) {
    fprintf(stderr, "failed to initialise the terminal\n");
    exit(EXIT_FAILURE);
  }

  terminal_is_up = true;
}

void
render_shutdown(void) {
  if (!terminal_is_up) {
    return;
  }

  tb_shutdown();
  terminal_is_up = false;
}

static void
render_blank_area(void) {
  for (int x = AREA_LEFT; x < AREA_RIGHT; x += 1) {
    for (int y = AREA_TOP; y < AREA_BOTTOM; y += 1) {
      tb_printf(x, y, foreground[SLOT_BLANK], background[SLOT_BLANK], " ");
    }
  }
}

static void
render_towers(const struct Puzzle* puzzle) {
  for (size_t tower = 0; tower < TOWER_COUNT; tower += 1) {
    enum screen_slot slot = SLOT_TOWER_1 + tower;
    uintattr_t fg = foreground[slot];
    uintattr_t bg = background[slot];

    size_t block = tower * TOWER_STRIDE;
    unsigned char height = puzzle->towers[tower].height;

    for (unsigned char level = 0; level < height; level += 1) {
      unsigned char disc = puzzle->towers[tower].discs[level];
      int row = FLOOR_ROW - level;

      char digit[2];
      snprintf(digit, sizeof(digit), "%hhu", disc);

      /* The disc body is (2 * disc - 1) cells of its own digit, centred on
         the tower. */
      size_t left = block + TOWER_CENTRE - disc;
      size_t span = ((size_t)disc - 1) * 2 + 1;
      for (size_t x = left; x < left + span; x += 1) {
        tb_printf(x, row, fg, bg, digit);
      }

      char label[4];
      snprintf(label, sizeof(label), "(%hhu)", disc);
      tb_printf(block + LABEL_COLUMN, row, fg, bg, label);
    }
  }
}

/* Right-align `value` (0..999) into a fixed-width field so the " / " separator
   never moves. */
static void
render_count_field(size_t value, char* buf, size_t buf_size) {
  if (value < 10) {
    snprintf(&buf[2], buf_size - 2, "%zu", value);
    buf[0] = ' ';
    buf[1] = ' ';
  } else if (value < 100) {
    snprintf(&buf[1], buf_size - 1, "%zu", value);
    buf[0] = ' ';
  } else {
    snprintf(buf, buf_size, "%zu", value);
  }
}

static void
render_progress(const struct Puzzle* puzzle, uintattr_t fg, uintattr_t bg) {
  char played[COUNT_BUFFER_SIZE];
  char total[COUNT_BUFFER_SIZE];
  render_count_field(puzzle->moves, played, COUNT_BUFFER_SIZE);
  render_count_field(puzzle->total_moves, total, COUNT_BUFFER_SIZE);

  tb_printf(0, STATUS_ROW, fg, bg, "Moves: ");
  tb_printf(8, STATUS_ROW, fg, bg, played);
  tb_printf(11, STATUS_ROW, fg, bg, " / ");
  tb_printf(14, STATUS_ROW, fg, bg, total);
}

static void
render_status(const struct Puzzle* puzzle) {
  uintattr_t fg = foreground[SLOT_STATUS];
  uintattr_t bg = background[SLOT_STATUS];

  if (puzzle->moves == 0) {
    tb_printf(0, STATUS_ROW, fg, bg, "Ready to solve");
  } else {
    render_progress(puzzle, fg, bg);
  }

  if (puzzle->solved) {
    tb_printf(18, STATUS_ROW, fg, bg, "Solved!");
  }

  tb_printf(0, HINT_ROW, fg, bg, "h/l or arrows: step back/forward    q/Esc: quit");
}

void
render_puzzle(const struct Puzzle* puzzle) {
  render_blank_area();
  render_towers(puzzle);
  render_status(puzzle);
  tb_present();
}

enum render_input
render_wait_for_input(int timeout_ms) {
  struct tb_event ev;

  if (tb_peek_event(&ev, timeout_ms) != TB_OK) {
    return RENDER_NONE;
  }

  if (ev.type != TB_EVENT_KEY) {
    return RENDER_NONE;
  }

  if (ev.key == TB_KEY_CTRL_C || ev.key == TB_KEY_ESC || ev.ch == 'q' || ev.ch == 'Q') {
    return RENDER_QUIT;
  }

  if (
    ev.key == TB_KEY_ARROW_LEFT || ev.key == TB_KEY_BACKSPACE || ev.key == TB_KEY_BACKSPACE2 || ev.ch == 'h'
    || ev.ch == 'H') {
    return RENDER_BACK;
  }

  return RENDER_FORWARD;
}
