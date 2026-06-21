#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./solution.h"
#include "./termbox2.h"

unsigned char HEIGHT;

/* struct Tower* towers[TOWER_COUNT]; */

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

struct DisplayArguments {
  struct Tower** towers;
  pthread_mutex_t* mutex;
};

void*
display(void* arg) {
  struct DisplayArguments* args = (struct DisplayArguments*)arg;

  // Display thread is ready
  pthread_mutex_unlock(args->mutex);

  for (size_t index = 0; index < TOWER_COUNT; index += 1) {
    struct Tower* tower = args->towers[index];

    unsigned char current_height = get_current_tower_height(tower);
    unsigned char* tower_stacks = get_tower_stacks(tower);
    for (unsigned char tower_stack_index = 0; tower_stack_index < current_height; tower_stack_index += 1) {
      unsigned char tower_stack = tower_stacks[tower_stack_index];

      tb_printf(index, tower_stack_index, TB_WHITE, TB_DEFAULT, "%hhu", tower_stack);
    }
  }
  tb_present();

  sleep(2);

  return NULL;
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

  printf("Total height is %hhu\n", height_of_tower);
  height_of_tower = 2;
  struct Tower** towers = create_towers(height_of_tower);

  tb_init();
  tb_present();

  pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;

  struct DisplayArguments args = {.towers = towers, .mutex = &display_mutex};

  pthread_t display_tid;
  pthread_create(&display_tid, NULL, display, &args);

  // Wait for Display thread to start up
  pthread_mutex_lock(&display_mutex);

  // Now solve the puzzle
  solve_tower_of_hanoi(towers);

  pthread_join(display_tid, NULL);
  tb_shutdown();
  return 0;
}
