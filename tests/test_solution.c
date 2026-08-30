#include <stdbool.h>
#include <stddef.h>

#include "unity/unity.h"

#include "../solution.h"

static size_t moves_observed;

// Do nothing
void
render_screen(struct Tower*) {}

void
setUp(void) {
  moves_observed = 0;
}

void
tearDown(void) {}

// Confirms that no disc value appears on more than one tower, and marks
// it as seen. Fails the current test if the disc was already seen.
static void
assert_disc_not_seen_elsewhere(unsigned char value, bool seen[256]) {
  char message[64];
  snprintf(message, sizeof(message), "disc %u appears on more than one tower", (unsigned int)value);

  TEST_ASSERT_FALSE_MESSAGE(seen[value], message);
  seen[value] = true;
}

// Confirms a single tower's discs strictly decrease from bottom (index 0)
// to top, and records each disc value into `seen` for the cross-tower
// uniqueness check.
static void
assert_tower_is_valid(struct Tower* tower, bool seen[256]) {
  unsigned char height = get_current_tower_height(tower);
  unsigned char* stack = get_tower_stacks(tower);

  for (unsigned char index = 0; index < height; index += 1) {
    assert_disc_not_seen_elsewhere(stack[index], seen);

    if (index > 0) {
      TEST_ASSERT_GREATER_THAN_MESSAGE(
        stack[index],
        stack[index - 1],
        "each disc must be smaller than the disc beneath it");
    }
  }
}

// Confirms the invariant across all three towers: every disc value is
// unique across the towers, and each tower's discs strictly decrease
// from bottom to top.
static void
assert_towers_are_valid(struct Tower* towers) {
  bool seen[256] = {false};

  assert_tower_is_valid(get_tower_by_index(towers, 0), seen);
  assert_tower_is_valid(get_tower_by_index(towers, 1), seen);
  assert_tower_is_valid(get_tower_by_index(towers, 2), seen);
}

static unsigned char
count_discs(struct Tower* towers) {
  struct Tower* first_tower = get_tower_by_index(towers, 0);
  struct Tower* second_tower = get_tower_by_index(towers, 1);
  struct Tower* third_tower = get_tower_by_index(towers, 2);

  return (unsigned char)(get_current_tower_height(first_tower) + get_current_tower_height(second_tower)
                         + get_current_tower_height(third_tower));
}

// Confirms the end state of solve_tower_of_hanoi(): exactly one tower
// holds every disc, stacked with no number skipped (e.g. 5,4,3,2,1
// passes; 5,3,2,1 - missing 4 - fails), and the other two towers are
// empty.
static void
assert_no_disc_number_is_skipped(struct Tower* towers, unsigned char max_height) {
  unsigned char empty_towers = 0;
  struct Tower* full_tower = NULL;

  for (unsigned char index = 0; index < TOWER_COUNT; index += 1) {
    struct Tower* tower = get_tower_by_index(towers, index);

    if (get_current_tower_height(tower) == 0) {
      empty_towers += 1;
    } else {
      full_tower = tower;
    }
  }

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, empty_towers, "exactly two towers must be empty");
  TEST_ASSERT_NOT_NULL_MESSAGE(full_tower, "one tower must hold every disc");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
    max_height,
    get_current_tower_height(full_tower),
    "the non-empty tower must hold every disc");

  unsigned char* stack = get_tower_stacks(full_tower);

  for (unsigned char index = 1; index < max_height; index += 1) {
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, stack[index - 1] - stack[index], "consecutive discs must not skip a number");
  }
}

// Implements the hook declared in solution.h (compiled in because
// TEST_CFLAGS defines TOWER_TEST_HOOKS). Called by move_stack() after
// every single move made during solve_tower_of_hanoi().
void
tower_test_on_move(struct Tower* towers) {
  moves_observed += 1;
  assert_towers_are_valid(towers);
}

void
test_init_towers_starts_with_a_valid_stack_on_the_first_tower(void) {
  unsigned char heights[] = {1, 2, 3, 5, 8};

  for (size_t i = 0; i < sizeof(heights) / sizeof(heights[0]); i += 1) {
    unsigned char max_height = heights[i];
    struct Tower* towers = init_towers(max_height);
    struct Tower* first_tower = get_tower_by_index(towers, 0);
    struct Tower* second_tower = get_tower_by_index(towers, 1);
    struct Tower* third_tower = get_tower_by_index(towers, 2);

    assert_towers_are_valid(towers);

    TEST_ASSERT_EQUAL_UINT8(max_height, get_current_tower_height(first_tower));
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(second_tower));
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(third_tower));
    TEST_ASSERT_EQUAL_UINT8(max_height, count_discs(towers));
  }
}

void
test_solve_tower_of_hanoi_keeps_a_valid_stack_at_every_height(void) {
  unsigned char heights[] = {1, 2, 3, 4, 5, 6};

  for (size_t i = 0; i < sizeof(heights) / sizeof(heights[0]); i += 1) {
    unsigned char max_height = heights[i];
    struct Tower* towers = init_towers(max_height);

    solve_tower_of_hanoi(towers, &render_screen);
    struct Tower* first_tower = get_tower_by_index(towers, 0);
    struct Tower* second_tower = get_tower_by_index(towers, 1);
    struct Tower* third_tower = get_tower_by_index(towers, 2);

    assert_towers_are_valid(towers);
    TEST_ASSERT_EQUAL_UINT8(max_height, count_discs(towers));

    // The algorithm is expected to deliver every disc to the third tower.
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(first_tower));
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(second_tower));
    TEST_ASSERT_EQUAL_UINT8(max_height, get_current_tower_height(third_tower));
  }
}

void
test_solve_tower_of_hanoi_keeps_a_valid_stack_after_every_single_move(void) {
  struct Test {
    unsigned char height;
    size_t expected_moves;
  };

  struct Test tests[] = {
    {.height = 1, .expected_moves = 1},
    {.height = 2, .expected_moves = 3},
    {.height = 3, .expected_moves = 7},
    {.height = 4, .expected_moves = 15},
    {.height = 5, .expected_moves = 31},
    {.height = 6, .expected_moves = 63}};

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i += 1) {
    moves_observed = 0;
    unsigned char max_height = tests[i].height;
    struct Tower* towers = init_towers(max_height);

    solve_tower_of_hanoi(towers, &render_screen);
    size_t expected_moves = tests[i].expected_moves;

    // Every move triggered tower_test_on_move(), which asserted the
    // invariant right then and there - a failure there aborts this test
    // via Unity's longjmp before we ever reach this line. This just
    // confirms the hook actually ran instead of silently never firing.
    TEST_ASSERT_GREATER_THAN(0, moves_observed);
    TEST_ASSERT_EQUAL_UINT8(expected_moves, moves_observed);
  }
}

void
test_solve_tower_of_hanoi_never_skips_a_disc_number(void) {
  unsigned char heights[] = {1, 2, 3, 4, 5, 6};

  for (size_t i = 0; i < sizeof(heights) / sizeof(heights[0]); i += 1) {
    unsigned char max_height = heights[i];
    struct Tower* towers = init_towers(max_height);

    solve_tower_of_hanoi(towers, &render_screen);

    assert_no_disc_number_is_skipped(towers, max_height);
  }
}

int
main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_init_towers_starts_with_a_valid_stack_on_the_first_tower);
  RUN_TEST(test_solve_tower_of_hanoi_keeps_a_valid_stack_at_every_height);
  RUN_TEST(test_solve_tower_of_hanoi_keeps_a_valid_stack_after_every_single_move);
  RUN_TEST(test_solve_tower_of_hanoi_never_skips_a_disc_number);

  return UNITY_END();
}
