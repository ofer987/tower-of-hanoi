#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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

void
test_puzzle_init_loads_a_valid_starting_position(void) {
  for (unsigned char discs = 1; discs <= MAX_DISCS; discs += 1) {
    struct Puzzle puzzle;
    puzzle_init(&puzzle, discs);

    assert_puzzle_is_valid(&puzzle);
    TEST_ASSERT_EQUAL_UINT8(discs, count_discs(&puzzle));
    TEST_ASSERT_EQUAL_UINT8(discs, puzzle.towers[0].height);
    TEST_ASSERT_EQUAL_UINT8(0, puzzle.towers[1].height);
    TEST_ASSERT_EQUAL_UINT8(0, puzzle.towers[2].height);

    TEST_ASSERT_EQUAL_size_t(0, puzzle.played_moves);
    TEST_ASSERT_EQUAL_UINT32((1UL << discs) - 1UL, puzzle.total_moves);
    TEST_ASSERT_FALSE(puzzle.solved);
  }
}

void
test_playing_forward_is_legal_at_every_step(void) {
  for (unsigned char discs = 1; discs <= MAX_DISCS; discs += 1) {
    struct Puzzle puzzle;
    puzzle_init(&puzzle, discs);

    for (size_t step = 0; step < puzzle.total_moves; step += 1) {
      TEST_ASSERT_EQUAL_size_t(step, puzzle.played_moves);
      TEST_ASSERT_FALSE(puzzle.solved);

      puzzle_forward(&puzzle);

      TEST_ASSERT_EQUAL_size_t(step + 1, puzzle.played_moves);
      assert_puzzle_is_valid(&puzzle);
      TEST_ASSERT_EQUAL_UINT8(discs, count_discs(&puzzle));
    }

    // Every disc has arrived on the last tower, stacked with no number skipped.
    TEST_ASSERT_TRUE(puzzle.solved);
    TEST_ASSERT_EQUAL_UINT8(0, puzzle.towers[0].height);
    TEST_ASSERT_EQUAL_UINT8(0, puzzle.towers[1].height);
    TEST_ASSERT_EQUAL_UINT8(discs, puzzle.towers[2].height);
    for (unsigned char level = 0; level < discs; level += 1) {
      TEST_ASSERT_EQUAL_UINT8(discs - level, puzzle.towers[2].discs[level]);
    }
  }
}

void
test_forward_past_the_end_is_a_noop(void) {
  struct Puzzle puzzle;
  puzzle_init(&puzzle, 4);

  while (puzzle.played_moves < puzzle.total_moves) {
    puzzle_forward(&puzzle);
  }

  struct Tower snapshot[TOWER_COUNT];
  memcpy(snapshot, puzzle.towers, sizeof(snapshot));

  puzzle_forward(&puzzle);
  puzzle_forward(&puzzle);

  TEST_ASSERT_EQUAL_size_t(puzzle.total_moves, puzzle.played_moves);
  TEST_ASSERT_TRUE(puzzle.solved);
  TEST_ASSERT_EQUAL_MEMORY(snapshot, puzzle.towers, sizeof(snapshot));
}

void
test_stepping_back_rewinds_through_the_same_states(void) {
  for (unsigned char discs = 1; discs <= 7; discs += 1) {
    struct Puzzle forward;
    puzzle_init(&forward, discs);

    // Record the tower state after each move played forward.
    struct Tower history[MAX_MOVES + 1][TOWER_COUNT];
    memcpy(history[0], forward.towers, sizeof(history[0]));
    for (size_t step = 1; step <= forward.total_moves; step += 1) {
      puzzle_forward(&forward);
      memcpy(history[step], forward.towers, sizeof(history[step]));
    }

    // Now walk all the way back, and confirm each state matches.
    struct Puzzle back;
    puzzle_init(&back, discs);
    for (size_t step = 0; step < forward.total_moves; step += 1) {
      puzzle_forward(&back);
    }

    for (size_t step = forward.total_moves; step > 0; step -= 1) {
      TEST_ASSERT_EQUAL_size_t(step, back.played_moves);
      TEST_ASSERT_EQUAL_MEMORY(history[step], back.towers, sizeof(history[step]));

      puzzle_back(&back);

      TEST_ASSERT_EQUAL_size_t(step - 1, back.played_moves);
      TEST_ASSERT_FALSE(back.solved);
      assert_puzzle_is_valid(&back);
      TEST_ASSERT_EQUAL_MEMORY(history[step - 1], back.towers, sizeof(history[step - 1]));
    }
  }
}

void
test_back_from_the_start_is_a_noop(void) {
  struct Puzzle puzzle;
  puzzle_init(&puzzle, 5);

  struct Tower snapshot[TOWER_COUNT];
  memcpy(snapshot, puzzle.towers, sizeof(snapshot));

  puzzle_back(&puzzle);
  puzzle_back(&puzzle);

  TEST_ASSERT_EQUAL_size_t(0, puzzle.played_moves);
  TEST_ASSERT_FALSE(puzzle.solved);
  TEST_ASSERT_EQUAL_MEMORY(snapshot, puzzle.towers, sizeof(snapshot));
}

int
main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_solve_returns_the_optimal_move_count);
  RUN_TEST(test_solve_matches_the_known_sequences_for_small_inputs);
  RUN_TEST(test_puzzle_init_loads_a_valid_starting_position);
  RUN_TEST(test_playing_forward_is_legal_at_every_step);
  RUN_TEST(test_forward_past_the_end_is_a_noop);
  RUN_TEST(test_stepping_back_rewinds_through_the_same_states);
  RUN_TEST(test_back_from_the_start_is_a_noop);

  return UNITY_END();
}
