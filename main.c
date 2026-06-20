#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./solution.h"

unsigned char HEIGHT;
struct Tower* towers[TOWER_COUNT];

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
  create_towers(towers, height_of_tower);

  solve_tower_of_hanoi(*towers);

  return 0;
}
