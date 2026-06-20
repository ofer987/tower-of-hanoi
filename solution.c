#include <stdlib.h>

#include "./solution.h"

struct Tower {
  unsigned char max_height;
  unsigned char current_height;
  unsigned char array[255];
};

unsigned char*
get_tower_stacks(struct Tower* tower) {
  return tower->array;
}

unsigned char
get_current_tower_height(struct Tower* tower) {
  return tower->current_height;
}

void
create_towers(struct Tower** towers, unsigned char max_height) {
  for (unsigned char index = 0; index < TOWER_COUNT; index += 1) {
    struct Tower* tower = towers[index];
    tower = malloc(sizeof(Tower));

    tower->max_height = max_height;
    tower->current_height = max_height;

    // Tower height is from 0 to 255)
    for (unsigned char tower_index = 0; tower_index <= max_height; tower_index += 1) {
      unsigned char height_of_index = max_height - tower_index;

      tower->array[tower_index] = height_of_index;
    }
  }

  return;
}

void
move_stack(struct Tower* source, struct Tower* dest) {
  if (source->current_height == 0) {
    exit(EXIT_FAILURE);
  }

  if (source->current_height > source->max_height || dest->current_height >= dest->max_height) {
    exit(EXIT_FAILURE);
  }

  unsigned char stack_height = source->array[source->current_height];
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
  unsigned char up_to_height) {
  for (unsigned char h = 0; h <= up_to_height; h += 1) {
    switch (h % 2) {
      case 0: build_tower(delivery_tower, source_tower, dest_tower, h); break;
      case 1: build_tower(delivery_tower, dest_tower, source_tower, h); break;
    }
    move_stack(dest_tower, source_tower);
  }
}

void
solve_tower_of_hanoi(struct Tower* towers) {
  struct Tower first_tower = towers[0];
  struct Tower second_tower = towers[1];
  struct Tower third_tower = towers[2];

  unsigned char max_height = first_tower.max_height;

  for (unsigned char h = 0; h < max_height; h += 1) {
    switch (h % 2) {
      case 0:
        move_stack(&first_tower, &second_tower);
        move_height(&third_tower, &first_tower, &second_tower, h - 1);

        break;
      case 1:
        move_stack(&first_tower, &third_tower);
        move_height(&second_tower, &first_tower, &third_tower, h - 1);

        break;
    }
  }
}
