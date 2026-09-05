#include <stdbool.h>
#include <stddef.h>

#include "unity/unity.h"

#include "../hanoi.h"

void
setUp(void) {}

void
tearDown(void) {}

// Confirms the puzzle invariant across all three towers: every disc value is
// unique across the towers, each tower's discs strictly decrease from bottom
// to top, and every slot above a tower's stack is clear.
static void
assert_puzzle_is_valid(const struct Puzzle* puzzle) {
  bool seen[MAX_DISCS + 1] = {false};

  for (size_t tower = 0; tower < TOWER_COUNT; tower += 1) {
    unsigned char height = puzzle->towers[tower].height;
    const unsigned char* discs = puzzle->towers[tower].discs;

    for (unsigned char level = 0; level < height; level += 1) {
      unsigned char disc = discs[level];

      TEST_ASSERT_TRUE_MESSAGE(disc >= 1 && disc <= MAX_DISCS, "disc value out of range");
      TEST_ASSERT_FALSE_MESSAGE(seen[disc], "disc value appears more than once");
      seen[disc] = true;

      if (level > 0) {
        TEST_ASSERT_GREATER_THAN_MESSAGE(discs[level], discs[level - 1], "a disc is resting on a smaller disc");
      }
    }

    for (unsigned char level = height; level < MAX_DISCS; level += 1) {
      TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, discs[level], "a slot above the stack is not clear");
    }
  }
}

static unsigned char
count_discs(const struct Puzzle* puzzle) {
  unsigned char total = 0;

  for (size_t tower = 0; tower < TOWER_COUNT; tower += 1) {
    total = (unsigned char)(total + puzzle->towers[tower].height);
  }

  return total;
}

void
test_solve_returns_the_optimal_move_count(void) {
  for (unsigned char discs = 1; discs <= MAX_DISCS; discs += 1) {
    struct Move moves[MAX_MOVES];
    size_t count = solve(discs, moves);

    TEST_ASSERT_EQUAL_UINT32((1UL << discs) - 1UL, count);
  }
}

void
test_replaying_the_solution_is_legal_at_every_step(void) {
  for (unsigned char discs = 1; discs <= MAX_DISCS; discs += 1) {
    struct Move moves[MAX_MOVES];
    size_t count = solve(discs, moves);

    struct Puzzle puzzle;
    puzzle_init(&puzzle, discs);

    assert_puzzle_is_valid(&puzzle);
    TEST_ASSERT_EQUAL_UINT8(discs, count_discs(&puzzle));

    for (size_t i = 0; i < count; i += 1) {
      puzzle_apply_move(&puzzle, moves[i]);

      assert_puzzle_is_valid(&puzzle);
      TEST_ASSERT_EQUAL_UINT8(discs, count_discs(&puzzle));
    }
  }
}

void
test_solution_ends_with_every_disc_on_the_last_tower(void) {
  for (unsigned char discs = 1; discs <= MAX_DISCS; discs += 1) {
    struct Move moves[MAX_MOVES];
    size_t count = solve(discs, moves);

    struct Puzzle puzzle;
    puzzle_init(&puzzle, discs);
    for (size_t i = 0; i < count; i += 1) {
      puzzle_apply_move(&puzzle, moves[i]);
    }

    TEST_ASSERT_EQUAL_UINT8(0, puzzle.towers[0].height);
    TEST_ASSERT_EQUAL_UINT8(0, puzzle.towers[1].height);
    TEST_ASSERT_EQUAL_UINT8(discs, puzzle.towers[2].height);

    // Stacked with no disc number skipped: discs, discs-1, ..., 1.
    for (unsigned char level = 0; level < discs; level += 1) {
      TEST_ASSERT_EQUAL_UINT8(discs - level, puzzle.towers[2].discs[level]);
    }

    TEST_ASSERT_TRUE(puzzle.solved);
    TEST_ASSERT_EQUAL_UINT32(count, puzzle.moves);
    TEST_ASSERT_EQUAL_UINT32(count, puzzle.total_moves);
  }
}

void
test_solve_matches_the_known_sequences_for_small_inputs(void) {
  struct Move moves[MAX_MOVES];

  const struct Move expected_one[] = {{0, 2}};
  const struct Move expected_two[] = {{0, 1}, {0, 2}, {1, 2}};
  const struct Move expected_three[] = {{0, 2}, {0, 1}, {2, 1}, {0, 2}, {1, 0}, {1, 2}, {0, 2}};

  struct Case {
    unsigned char discs;
    const struct Move* expected;
    size_t length;
  };

  struct Case cases[] = {
    {.discs = 1, .expected = expected_one, .length = 1},
    {.discs = 2, .expected = expected_two, .length = 3},
    {.discs = 3, .expected = expected_three, .length = 7},
  };

  for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c += 1) {
    size_t count = solve(cases[c].discs, moves);

    TEST_ASSERT_EQUAL_UINT32(cases[c].length, count);

    for (size_t i = 0; i < cases[c].length; i += 1) {
      TEST_ASSERT_EQUAL_UINT8(cases[c].expected[i].from, moves[i].from);
      TEST_ASSERT_EQUAL_UINT8(cases[c].expected[i].to, moves[i].to);
    }
  }
}

int
main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_solve_returns_the_optimal_move_count);
  RUN_TEST(test_replaying_the_solution_is_legal_at_every_step);
  RUN_TEST(test_solution_ends_with_every_disc_on_the_last_tower);
  RUN_TEST(test_solve_matches_the_known_sequences_for_small_inputs);

  return UNITY_END();
}
