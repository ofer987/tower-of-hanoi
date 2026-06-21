#include <stdbool.h>
#include <stdlib.h>

#include "./solution.h"

struct Tower {
  unsigned char max_height;
  unsigned char current_height;
  unsigned char array[255];
};

static struct Tower* towers[3];

unsigned char*
get_tower_stacks(struct Tower* tower) {
  return tower->array;
}

unsigned char
get_current_tower_height(struct Tower* tower) {
  return tower->current_height;
}

struct Tower**
create_towers(unsigned char max_height) {
  for (unsigned char index = 0; index < TOWER_COUNT; index += 1) {
    // TODO: reference static Towers
    towers[index] = malloc(sizeof(struct Tower));

    struct Tower* tower = towers[index];

    tower->max_height = max_height;
    tower->current_height = 0;
  }

  struct Tower* tower = towers[0];
  tower->current_height = max_height;

  // Tower height is from 1 to 255)
  for (unsigned char tower_index = 0; tower_index < max_height; tower_index += 1) {
    unsigned char height_of_index = max_height - tower_index;

    tower->array[tower_index] = height_of_index;
  }

  return towers;
}

void
move_stack(struct Tower* source, struct Tower* dest) {
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
}

void
build_tower(
  struct Tower* delivery_tower,
  struct Tower* first_tower,
  struct Tower* second_tower,
  unsigned char up_to_height) {
  for (unsigned char h = 0; h <= up_to_height; h += 1) {
    switch (h % 2) {
      case 0: move_stack(delivery_tower, first_tower); break;
      case 1: move_stack(delivery_tower, second_tower); break;
    }
  }

  return;
}

// Move tower from delivery_tower to dest_tower via
// the source_tower
void
move_height(
  struct Tower* delivery_tower,
  struct Tower* source_tower,
  struct Tower* dest_tower,
  unsigned char current_height,
  unsigned char up_to_height) {
  unsigned char current_value_of_delivery_tower = delivery_tower->array[0];
  unsigned char current_value_of_dest_tower = dest_tower->array[0];

  /* if (current_value_of_delivery_tower == current_value_of_dest_tower - 1) { */
  /*   move_stack(delivery_tower, dest_tower); */
  /* } */

  size_t source_or_dest_tower = ((current_value_of_delivery_tower % 2) == (current_value_of_dest_tower - 1) % 2);

  for (unsigned char h = 0; h <= current_height; h += 1) {
    // I think I should move here
    // What is the value (dest_tower - 1) % 2
    /* unsigned char current_value_of_dest_tower = dest_tower->array[dest_tower->current_height - 1]; */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-bool"
    switch (source_or_dest_tower) {
      case true: move_stack(delivery_tower, dest_tower); break;
      default:
      case false: move_stack(delivery_tower, source_tower); break;
    }

    source_or_dest_tower = (source_or_dest_tower + 1) % 2;
  }
#pragma clang diagnostic pop

  if (current_height + 1 <= up_to_height) {
    move_height(delivery_tower, source_tower, dest_tower, current_height + 1, up_to_height);
  }
}

void
solve_tower_of_hanoi(struct Tower** towers) {
  struct Tower* source_tower = towers[0];
  struct Tower* second_tower = towers[1];
  struct Tower* third_tower = towers[2];

  unsigned char max_height = source_tower->max_height;

  for (unsigned char h = 0; h < max_height; h += 1) {
    switch (h % 2) {
      case 0:
        move_stack(source_tower, second_tower);
        if (h > 0) {
          move_height(third_tower, source_tower, second_tower, 0, h - 1);
        }

        break;
      case 1:
        move_stack(source_tower, third_tower);
        if (h > 0) {
          move_height(second_tower, source_tower, third_tower, 0, h - 1);
        }

        break;
    }
  }
}
