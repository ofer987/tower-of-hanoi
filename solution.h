#pragma once

#include <stddef.h>
typedef struct Tower Tower;

#define TOWER_COUNT 3

struct Tower*
get_tower_by_index(struct Tower*, size_t);

unsigned char*
get_tower_stacks(struct Tower*);

unsigned char
get_current_tower_height(struct Tower*);

struct Tower*
init_towers(unsigned char);

void
solve_tower_of_hanoi(struct Tower*);

#ifdef TOWER_TEST_HOOKS
// Only declared when TOWER_TEST_HOOKS is defined at compile time (see the
// `test` target in the Makefile). Test code implements this function to
// observe the state of all three towers after every individual move made
// while solving.
void
tower_test_on_move(struct Tower*);
#endif
