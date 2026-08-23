#include <locale.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./common.h"
#include "./solution.h"

#define TB_IMPL
#include "./termbox2.h"

// TODO: change to real screen size
// TODO: calculate it first
#define SCREEN_SIZE            900

#define BORDER_HORIZONTAL_SIZE 30
#define BORDER_VERTICAL_SIZE   20

#define SIDE_VERTICAL_START    0
#define SIDE_VERTICAL_END      30

#define SIDE_HORIZONTAL_START  0
#define SIDE_HORIZONTAL_END    10

enum screen_state {
  AVAILABLE = 1,
  USED_BY_SNAKE_TAIL,
  USED_BY_SNAKE_HEAD,
  USED_BY_FOOD,
  USED_BY_TOP_BORDER,
  USED_BY_TOP_RIGHT_BORDER,
  USED_BY_RIGHT_BORDER,
  USED_BY_BOTTOM_RIGHT_BORDER,
  USED_BY_BOTTOM_BORDER,
  USED_BY_BOTTOM_LEFT_BORDER,
  USED_BY_LEFT_BORDER,
  USED_BY_TOP_LEFT_BORDER,
  USED_BY_MESSAGE,
  TOTAL
};

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

  if (converted_input > 255) {
    return -4;
  }

  return (unsigned char)converted_input;
}

char* screen_chars[TOTAL] = {
  [AVAILABLE] = " ",
  [USED_BY_SNAKE_TAIL] = "#",
  [USED_BY_SNAKE_HEAD] = "@",
  [USED_BY_FOOD] = "$",
  /* [USED_BY_TOP_BORDER] = BORDER_ROW, */
  /* [USED_BY_TOP_RIGHT_BORDER] = BORDER_TOP_RIGHT, */
  /* [USED_BY_RIGHT_BORDER] = BORDER_COLUMN, */
  /* [USED_BY_BOTTOM_RIGHT_BORDER] = BORDER_BOTTOM_RIGHT, */
  /* [USED_BY_BOTTOM_BORDER] = BORDER_ROW, */
  /* [USED_BY_BOTTOM_LEFT_BORDER] = BORDER_BOTTOM_LEFT, */
  /* [USED_BY_LEFT_BORDER] = BORDER_COLUMN, */
  /* [USED_BY_TOP_LEFT_BORDER] = BORDER_TOP_LEFT, */
  /* [USED_BY_MESSAGE] = " ", */
};

size_t screen_colors[TOTAL] = {
  [AVAILABLE] = TB_WHITE,
  [USED_BY_SNAKE_TAIL] = TB_BLUE,
  [USED_BY_SNAKE_HEAD] = TB_MAGENTA,
  [USED_BY_FOOD] = TB_GREEN,
  [USED_BY_TOP_BORDER] = TB_WHITE,
  [USED_BY_TOP_RIGHT_BORDER] = TB_WHITE,
  [USED_BY_RIGHT_BORDER] = TB_WHITE,
  [USED_BY_BOTTOM_RIGHT_BORDER] = TB_WHITE,
  [USED_BY_BOTTOM_BORDER] = TB_WHITE,
  [USED_BY_BOTTOM_LEFT_BORDER] = TB_WHITE,
  [USED_BY_LEFT_BORDER] = TB_WHITE,
  [USED_BY_TOP_LEFT_BORDER] = TB_WHITE,
  [USED_BY_MESSAGE] = TB_WHITE,
};

size_t screen_background_colors[TOTAL] = {
  [AVAILABLE] = TB_DEFAULT,
  [USED_BY_SNAKE_TAIL] = TB_BLACK,
  [USED_BY_SNAKE_HEAD] = TB_BLACK,
  [USED_BY_FOOD] = TB_BLACK,
  [USED_BY_TOP_BORDER] = TB_BLUE,
  [USED_BY_TOP_RIGHT_BORDER] = TB_BLUE,
  [USED_BY_RIGHT_BORDER] = TB_BLUE,
  [USED_BY_BOTTOM_RIGHT_BORDER] = TB_BLUE,
  [USED_BY_BOTTOM_BORDER] = TB_BLUE,
  [USED_BY_BOTTOM_LEFT_BORDER] = TB_BLUE,
  [USED_BY_LEFT_BORDER] = TB_BLUE,
  [USED_BY_TOP_LEFT_BORDER] = TB_BLUE,
  [USED_BY_MESSAGE] = TB_BLACK,
};

size_t
coordinates_to_index(size_t x, size_t y) {
  return BORDER_HORIZONTAL_SIZE * y + x;
}

struct coordinates
index_to_coorindates(size_t index) {
  size_t y = index / BORDER_HORIZONTAL_SIZE;
  size_t x = index - (BORDER_HORIZONTAL_SIZE * y);

  struct coordinates foobar = {.x = x, .y = y};

  return foobar;
}

void
render_blank_screen(enum screen_state* screen_states) {
  for (size_t x = SIDE_HORIZONTAL_START; x < SIDE_HORIZONTAL_END; x += 1) {
    for (size_t y = SIDE_VERTICAL_START; y < SIDE_VERTICAL_END; y += 1) {
      size_t i = coordinates_to_index(x, y);

      screen_states[i] = AVAILABLE;

      int32_t bg_color = screen_background_colors[AVAILABLE];
      int32_t color = screen_colors[AVAILABLE];
      char* ch = " ";

      tb_printf(x, y, color, bg_color, ch);
    }
  }
}

void
render_towers(enum screen_state* screen_state, struct Tower* towers) {
  for (size_t i = 0; i < 3; i += 1) {
    // Implement
    struct Tower* tower = get_tower_by_index(towers, i);
    unsigned char max_height = get_current_tower_height(tower);
    /* printf("Height is %hhu", max_height); */

    char ch[2];
    snprintf(ch, sizeof(ch), "%hhu", max_height);
    tb_printf(i * 3, 12, TB_BLACK, TB_WHITE, ch);
    for (unsigned char y = 0; y < max_height; y += 1) {
      struct coordinates coordinate;
      coordinate.x = i * 3;
      coordinate.y = y;
      coordinate.state = USED_BY_SNAKE_HEAD;

      size_t screen_index = coordinates_to_index(coordinate.x, coordinate.y);

      int32_t bg_color = TB_WHITE;
      int32_t color = TB_BLACK;
      char ch[2];
      snprintf(ch, sizeof(ch), "%hhu", y);

      screen_state[screen_index] = USED_BY_SNAKE_HEAD;

      tb_printf(i * 3, y, color, bg_color, ch);
    }
  }
}

void
render_screen(struct Tower* towers) {
  enum screen_state screen_states[SCREEN_SIZE];

  render_blank_screen(screen_states);
  render_towers(screen_states, towers);

  /* for (size_t i = 0; i < SCREEN_SIZE; i += 1) { */
  /*   struct coordinates coords = index_to_coorindates(i); */
  /*  */
  /*   size_t state = screen_states[i]; */
  /*   int32_t bg_color = screen_background_colors[state]; */
  /*   int32_t color = screen_colors[state]; */
  /*   char* ch = screen_chars[state]; */
  /*  */
  /*   tb_printf(coords.x, coords.y, color, bg_color, ch); */
  /* } */

  /* render_the_message_box_boarders(); */
  /* show_results(results); */

  tb_present();
}

void
init_screen() {
  setlocale(LC_ALL, "");

  /* initialize the termbox library */
  if (tb_init() != 0) {
    exit(EXIT_FAILURE);
  }

  enum screen_state screen_states[SCREEN_SIZE];
  render_blank_screen(screen_states);

  tb_present();
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
        printf("Number must be smaller than 256 '%s'\n", argv[1]);

        exit(EXIT_FAILURE);
      }
    }
  }

  unsigned char height_of_tower = argument;
  struct Tower* towers = init_towers(height_of_tower);

  init_screen();
  render_screen(towers);

  // Now solve the puzzle
  solve_tower_of_hanoi(towers, &render_screen);

  int quit_key;
  scanf("Press any key to quit %d\n", &quit_key);

  // shutdown termbox2
  tb_shutdown();

  return 0;
}
