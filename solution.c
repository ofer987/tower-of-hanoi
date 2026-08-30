#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./common.h"
#include "./solution.h"

enum DEST_OR_TEMP_TOWER { DEST_TOWER = 0, TEMP_TOWER };

enum INITIAL_OR_SOLVED_STATE { INITIAL_STATE = 0, SOLVED_STATE };

struct Tower {
  unsigned char index;
  unsigned char max_height;
  unsigned char current_height;
  unsigned char array[256];

  size_t steps;
  size_t total_steps;
  bool is_solved;
};

static struct Tower towers[3];

size_t
get_total_steps(struct Tower* towers) {
  struct Tower* first_tower = &towers[0];

  return first_tower->total_steps;
}

bool
get_is_solved(struct Tower* towers) {
  struct Tower* first_tower = &towers[0];

  return first_tower->is_solved;
}

void
set_is_solved(struct Tower* towers) {
  struct Tower* first_tower = &towers[0];

  first_tower->is_solved = true;

  return;
}

size_t
get_steps(struct Tower* towers) {
  struct Tower* first_tower = &towers[0];

  return first_tower->steps;
}

struct Tower*
get_tower_by_index(struct Tower* towers, size_t index) {
  if (index >= 3) {
    exit(EXIT_FAILURE);
  }

  return &towers[index];
}

unsigned char*
get_tower_stacks(struct Tower* tower) {
  return tower->array;
}

unsigned char
get_current_tower_height(struct Tower* tower) {
  return tower->current_height;
}

struct Tower*
init_towers(unsigned char max_height) {
  for (unsigned char index = 0; index < TOWER_COUNT; index += 1) {
    struct Tower* tower = &towers[index];

    tower->index = index + 1;
    tower->max_height = max_height;
    tower->current_height = 0;

    for (unsigned char array_index = 0; array_index <= 254; array_index += 1) {
      tower->array[array_index] = 0;
    }

    tower->array[255] = 0;
    tower->total_steps = (size_t)(pow(2, max_height) - 1);
  }

  struct Tower* first_tower = &towers[0];
  first_tower->current_height = max_height;

  // Tower height is from 1 to 255)
  for (unsigned char tower_index = 0; tower_index < max_height; tower_index += 1) {
    unsigned char height_of_index = max_height - tower_index;

    first_tower->array[tower_index] = height_of_index;
  }

  return towers;
}

void
move_stack(struct Tower* source, struct Tower* dest, struct Tower* towers, render_screen_function render) {
  if (source->current_height == 0) {
    return;
  }

  if (source->current_height > source->max_height || dest->current_height >= dest->max_height) {
    exit(EXIT_FAILURE);
  }

  unsigned char stack_height = source->array[source->current_height - 1];
  source->array[source->current_height - 1] = 0;
  source->current_height -= 1;

  dest->array[dest->current_height] += stack_height;
  dest->current_height += 1;

  towers[0].steps += 1;
  render(towers);
#ifdef TOWER_TEST_HOOKS
  tower_test_on_move(towers);
#endif
}

// Move tower from delivery_tower to dest_tower via
// the source_tower
void
move_height(
  struct Tower* delivery_tower,
  struct Tower* dest_tower,
  struct Tower* temp_tower,
  unsigned char current_height,
  unsigned char up_to_height,
  enum DEST_OR_TEMP_TOWER temp_or_dest_tower,
  struct Tower* towers,
  render_screen_function render) {

  if (current_height != 0 && current_height > up_to_height) {
    move_height(
      delivery_tower,
      temp_tower,
      dest_tower,
      current_height - 1,
      up_to_height,
      temp_or_dest_tower,
      towers,
      render);
  }

  switch (temp_or_dest_tower) {
    case DEST_TOWER: move_stack(delivery_tower, dest_tower, towers, render); break;
    default:
    case TEMP_TOWER: move_stack(delivery_tower, temp_tower, towers, render); break;
  }

  if (current_height != 0 && current_height > up_to_height) {
    switch (temp_or_dest_tower) {
      case DEST_TOWER:
        temp_or_dest_tower = (temp_or_dest_tower + 1) % 2;
        move_height(
          temp_tower,
          delivery_tower,
          dest_tower,
          current_height - 1,
          up_to_height,
          temp_or_dest_tower,
          towers,
          render);

        break;
      case TEMP_TOWER:
      default:
        move_height(
          dest_tower,
          delivery_tower,
          temp_tower,
          current_height - 1,
          up_to_height,
          temp_or_dest_tower,
          towers,
          render);

        break;
    }
  }
}

void
solve_tower_of_hanoi(struct Tower* towers, render_screen_function render) {
  struct Tower* source_tower = &towers[0];
  struct Tower* second_tower = &towers[1];
  struct Tower* third_tower = &towers[2];

  unsigned char max_height = source_tower->max_height;

  render(towers);

  move_height(source_tower, third_tower, second_tower, max_height, 1, DEST_TOWER, towers, render);

  set_is_solved(towers);
  render(towers);
}
