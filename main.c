/* Expose POSIX APIs (struct sigaction) under -std=c2x, which otherwise
   restricts headers to ISO C. _DEFAULT_SOURCE (glibc) and _DARWIN_C_SOURCE
   (macOS) keep the BSD extensions termbox2.h needs: cfmakeraw, SIGWINCH. */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#define _DARWIN_C_SOURCE

#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./common.h"
#include "./solution.h"

#define TB_IMPL
#include "./termbox2.h"

#define SCREEN_SIZE             2520

#define SIDE_VERTICAL_START     0
#define SIDE_VERTICAL_END       20

#define SIDE_HORIZONTAL_START   0
#define SIDE_HORIZONTAL_END     126
#define MAX_TOWER_HEIGHT        10
#define MAX_TOWER_INFO_WIDTH    40
#define MAX_ALLOWED_WIDTH       9
#define MAX_TOWER_DISPLAY_WIDTH ((MAX_ALLOWED_WIDTH + 1) * 2)

#define LINE_BUFFER_SIZE        4

enum screen_state { AVAILABLE = 1, FIRST_TOWER, SECOND_TOWER, THIRD_TOWER, STEP_INFO, TOTAL };

unsigned char HEIGHT;

struct coordinates {
  enum screen_state state;
  size_t x;
  size_t y;

  struct coordinates* next;
};

struct coordinates coordinates;

void
set_height(unsigned char height) {
  HEIGHT = height;
}

long
read_argument(const char* string) {
  char* end_str;

  long converted_input = strtol(string, &end_str, 10);

  if (end_str == string) {
    return -1;
  }

  if (*end_str != '\0') {
    return -2;
  }

  if (converted_input <= 0) {
    return -3;
  }

  if (converted_input > MAX_ALLOWED_WIDTH) {
    return -4;
  }

  return (unsigned char)converted_input;
}

size_t screen_colors[TOTAL] = {
  [AVAILABLE] = TB_DEFAULT,
  [FIRST_TOWER] = TB_BLUE,
  [SECOND_TOWER] = TB_GREEN,
  [THIRD_TOWER] = TB_RED,
  [STEP_INFO] = TB_WHITE};

size_t screen_background_colors[TOTAL] = {
  [AVAILABLE] = TB_DEFAULT,
  [FIRST_TOWER] = TB_DEFAULT,
  [SECOND_TOWER] = TB_DEFAULT,
  [THIRD_TOWER] = TB_DEFAULT,
  [STEP_INFO] = TB_DEFAULT};

size_t
coordinates_to_index(size_t x, size_t y) {
  return (SIDE_HORIZONTAL_END * y) + x;
}

struct coordinates
index_to_coorindates(size_t index) {
  size_t y = index / SIDE_HORIZONTAL_END;
  size_t x = index - (SIDE_HORIZONTAL_END * y);

  struct coordinates foobar = {.x = x, .y = y};

  return foobar;
}

void
render_blank_screen() {
  for (size_t x = SIDE_HORIZONTAL_START; x < SIDE_HORIZONTAL_END; x += 1) {
    for (size_t y = SIDE_VERTICAL_START; y < SIDE_VERTICAL_END; y += 1) {
      int32_t bg_color = screen_background_colors[AVAILABLE];
      int32_t color = screen_colors[AVAILABLE];
      char* ch = " ";

      tb_printf(x, y, color, bg_color, ch);
    }
  }
}

void
render_towers(struct Tower* towers) {
  for (size_t i = 0; i < 3; i += 1) {
    struct Tower* tower = get_tower_by_index(towers, i);
    unsigned char* tower_stack = get_tower_stacks(tower);
    unsigned char max_width = tower_stack[0];

    char ch[2];
    size_t y_coordinate = MAX_TOWER_HEIGHT;
    for (unsigned char y = 0; y < max_width; y += 1) {
      enum screen_state tower_index = FIRST_TOWER + i;
      y_coordinate -= 1;
      unsigned char current_width = tower_stack[y];

      snprintf(ch, sizeof(ch), "%hhu", current_width);

      int32_t bg_color = screen_background_colors[tower_index];
      int32_t color = screen_colors[tower_index];

      // render_width
      size_t start_tower_width = i * MAX_TOWER_INFO_WIDTH;
      size_t start_width = start_tower_width + (MAX_TOWER_DISPLAY_WIDTH / 2) - current_width;
      size_t width = ((current_width - 1) * 2) + 1;

      for (size_t width_x = start_width; width_x < start_width + width; width_x += 1) {
        tb_printf(width_x, y_coordinate, color, bg_color, ch);
      }

      if (current_width == 0) {
        continue;
      }

      char ch[4];
      snprintf(ch, sizeof(ch), "%hhu", current_width);

      ch[1] = ch[0];
      ch[0] = '(';
      ch[2] = ')';
      ch[3] = '\0';

      size_t x = start_tower_width + (MAX_TOWER_DISPLAY_WIDTH) + 4;

      tb_printf(x, y_coordinate, color, bg_color, ch);
    }
  }
}

void
handle_terminating_signal(int sig);

void
render_step_count_into_buffer(size_t value, char* buf, size_t buf_size) {
  snprintf(buf, buf_size, "%zu", value);

  if (value < 10) {
    buf[2] = buf[0];
    buf[0] = ' ';
    buf[1] = ' ';
    buf[3] = '\0';
  } else if (value < 100) {
    buf[2] = buf[1];
    buf[1] = buf[0];
    buf[0] = ' ';
    buf[3] = '\0';
  }

  return;
}

void
render_steps(size_t current_step, size_t total_steps, uintattr_t color, uintattr_t bg_color) {
  char total_steps_buf[LINE_BUFFER_SIZE];
  render_step_count_into_buffer(total_steps, total_steps_buf, LINE_BUFFER_SIZE);

  char steps_buf[LINE_BUFFER_SIZE];
  render_step_count_into_buffer(current_step, steps_buf, LINE_BUFFER_SIZE);

  tb_printf(0, SIDE_VERTICAL_END - 1, color, bg_color, "Steps: ");
  tb_printf(8, SIDE_VERTICAL_END - 1, color, bg_color, steps_buf);
  tb_printf(12, SIDE_VERTICAL_END - 1, color, bg_color, " / ");
  tb_printf(15, SIDE_VERTICAL_END - 1, color, bg_color, total_steps_buf);
}

void
render_info(struct Tower* towers) {
  size_t steps = get_steps(towers);

  int32_t color = screen_colors[STEP_INFO];
  int32_t bg_color = screen_background_colors[STEP_INFO];

  if (get_is_solved(towers)) {
    tb_printf(0, SIDE_VERTICAL_END - 1, color, bg_color, "Solved!");
    tb_printf(0, SIDE_VERTICAL_END, color, bg_color, "Press any key to quit");

    return;
  }

  if (steps == 0) {
    tb_printf(0, SIDE_VERTICAL_END - 1, color, bg_color, "Game started");

    tb_printf(0, SIDE_VERTICAL_END, color, bg_color, "Press CTRL+C to quit");
    return;
  }

  size_t total_steps = get_total_steps(towers);
  render_steps(steps, total_steps, color, bg_color);

  tb_printf(0, SIDE_VERTICAL_END, color, bg_color, "Press CTRL+C to quit");
}

void
render_screen(struct Tower* towers) {
  render_blank_screen();
  render_towers(towers);
  render_info(towers);

  tb_present();

  /* Wait for a keypress before showing the next move. Ctrl+C must be caught
     here: tb_init() puts the terminal in raw mode (cfmakeraw clears ISIG), so
     the tty never raises SIGINT and the installed signal handler never runs. */
  struct tb_event ev;
  while (tb_poll_event(&ev) == TB_OK) {
    if (ev.type != TB_EVENT_KEY) {
      continue;
    }

    if (ev.key == TB_KEY_CTRL_C) {
      handle_terminating_signal(SIGINT);
    }

    break;
  }
}

void
init_screen() {
  setlocale(LC_ALL, "");

  /* initialize the termbox library */
  if (tb_init() != 0) {
    exit(EXIT_FAILURE);
  }

  render_blank_screen();

  tb_present();
}

void
handle_terminating_signal(int sig) {
  /* Restore the terminal: leave the alt screen, disable raw mode. */
  tb_shutdown();

  /* Re-raise with the default action so the exit status reflects the signal. */
  signal(sig, SIG_DFL);
  raise(sig);

  _exit(EXIT_FAILURE);
}

void
install_signal_handlers() {
  struct sigaction sa = {0};

  sa.sa_handler = handle_terminating_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  /* SIGKILL and SIGSTOP cannot be caught, blocked, or ignored. */
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
}

int
main(int argc, char* argv[]) {
  printf("Hello, World!\n");

  if (argc <= 1) {
    printf("You should have provided at least one argument! %d\n", argc);

    exit(EXIT_FAILURE);
  }

  long argument = read_argument(argv[1]);
  if (argument < 0) {
    switch (argument) {
      case -1: {
        printf("Failed to parse '%s'\n", argv[1]);

        exit(EXIT_FAILURE);
      }
      case -2: {
        printf("Failed to find null pointer character\n");

        exit(EXIT_FAILURE);
      }
      case -3: {
        printf("Zero and negative numbers are not accepted '%s'\n", argv[1]);

        exit(EXIT_FAILURE);
      }
      case -4: {
        printf("Number must be smaller than %d '%s'\n", MAX_ALLOWED_WIDTH, argv[1]);

        exit(EXIT_FAILURE);
      }
    }
  }

  unsigned char height_of_tower = argument;
  struct Tower* towers = init_towers(height_of_tower);

  init_screen();
  install_signal_handlers();

  // Now solve the puzzle
  solve_tower_of_hanoi(towers, &render_screen);

  // shutdown termbox2
  tb_shutdown();

  return 0;
}
