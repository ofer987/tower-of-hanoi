#include <stdbool.h>
#include <stddef.h>

#include "unity/unity.h"

#include "../solution.h"

void
setUp(void) {}

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
assert_towers_are_valid(struct Tower** towers) {
  bool seen[256] = {false};

  assert_tower_is_valid(towers[0], seen);
  assert_tower_is_valid(towers[1], seen);
  assert_tower_is_valid(towers[2], seen);
}

static unsigned char
count_discs(struct Tower** towers) {
  return (unsigned char)(get_current_tower_height(towers[0]) + get_current_tower_height(towers[1])
                         + get_current_tower_height(towers[2]));
}

void
test_create_towers_starts_with_a_valid_stack_on_the_first_tower(void) {
  unsigned char heights[] = {1, 2, 3, 5, 8};

  for (size_t i = 0; i < sizeof(heights) / sizeof(heights[0]); i += 1) {
    unsigned char max_height = heights[i];
    struct Tower** towers = create_towers(max_height);

    assert_towers_are_valid(towers);

    TEST_ASSERT_EQUAL_UINT8(max_height, get_current_tower_height(towers[0]));
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(towers[1]));
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(towers[2]));
    TEST_ASSERT_EQUAL_UINT8(max_height, count_discs(towers));
  }
}

void
test_solve_tower_of_hanoi_keeps_a_valid_stack_at_every_height(void) {
  unsigned char heights[] = {1, 2, 3, 4, 5, 6};

  for (size_t i = 0; i < sizeof(heights) / sizeof(heights[0]); i += 1) {
    unsigned char max_height = heights[i];
    struct Tower** towers = create_towers(max_height);

    solve_tower_of_hanoi(towers);

    assert_towers_are_valid(towers);
    TEST_ASSERT_EQUAL_UINT8(max_height, count_discs(towers));

    // The algorithm is expected to deliver every disc to the third tower.
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(towers[0]));
    TEST_ASSERT_EQUAL_UINT8(0, get_current_tower_height(towers[1]));
    TEST_ASSERT_EQUAL_UINT8(max_height, get_current_tower_height(towers[2]));
  }
}

int
main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_create_towers_starts_with_a_valid_stack_on_the_first_tower);
  RUN_TEST(test_solve_tower_of_hanoi_keeps_a_valid_stack_at_every_height);

  return UNITY_END();
}
