#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "./hanoi.h"

/* Clear every tower and put all `puzzle->discs` discs back on tower 0, largest
   (value `discs`) on the bottom, smallest (value 1) on top. */
static void
reset_towers(struct Puzzle* puzzle) {
  for (unsigned char tower = 0; tower < TOWER_COUNT; tower += 1) {
    puzzle->towers[tower].height = 0;

    for (unsigned char level = 0; level < MAX_DISCS; level += 1) {
      puzzle->towers[tower].discs[level] = 0;
    }
  }

  for (unsigned char level = 0; level < puzzle->discs; level += 1) {
    puzzle->towers[0].discs[level] = puzzle->discs - level;
  }
  puzzle->towers[0].height = puzzle->discs;
}

void
puzzle_apply_move(struct Puzzle* puzzle, struct Move move) {
  assert(move.from < TOWER_COUNT && move.to < TOWER_COUNT);

  struct Tower* source = &puzzle->towers[move.from];
  struct Tower* dest = &puzzle->towers[move.to];

  /* assert(move.from == move.to && "Cannot change the same tower"); */
  assert(source->height > 0 && "move from an empty tower");

  unsigned char disc = source->discs[source->height - 1];

  /* printf("\nMoving from %hhu to %hhu\n", move.from, move.to); */
  /* printf("\n\n\n\n\tMove from %hhu to %hhu\n", move.from, move.to); */
  assert(dest->height == 0 && "destination is empty");
  assert((dest->height == 0 || dest->discs[dest->height - 1] > disc) && "disc landed on a smaller disc");

  source->discs[source->height - 1] = 0;
  source->height -= 1;

  dest->discs[dest->height] = disc;
  dest->height += 1;
}

/* Move the puzzle to exactly `target` moves played, by resetting to the start
   and replaying the first `target` moves. Stepping backward has no dedicated
   undo: the full move list is already in hand, so replaying a shorter prefix
   is simpler and, at 511 moves max, free. See
   docs/adr/0003-solver-is-pure-and-emits-a-move-list.md. */
static void
replay_to(struct Puzzle* puzzle, size_t target) {
  reset_towers(puzzle);

  for (size_t i = 0; i < target; i += 1) {
    puzzle_apply_move(puzzle, puzzle->solution[i]);
  }

  puzzle->played_moves = target;
  puzzle->solved = (target == puzzle->total_moves);
}

void
puzzle_init(struct Puzzle* puzzle, unsigned char discs) {
  assert(discs >= 1 && discs <= MAX_DISCS);

  puzzle->discs = discs;
  puzzle->total_moves = solve(discs, puzzle->solution);

  replay_to(puzzle, 0);
}

void
puzzle_forward(struct Puzzle* puzzle) {
  if (puzzle->played_moves >= puzzle->total_moves) {
    return;
  }

  puzzle_apply_move(puzzle, puzzle->solution[puzzle->played_moves]);
  puzzle->played_moves += 1;
  puzzle->solved = (puzzle->played_moves == puzzle->total_moves);
}

void
puzzle_back(struct Puzzle* puzzle) {
  if (puzzle->played_moves == 0) {
    return;
  }

  replay_to(puzzle, puzzle->played_moves - 1);
}

/* Classic recursive Tower of Hanoi. To move `n` discs from `from` to `to` using
   `via`: move the top n-1 to `via`, move disc n to `to`, move the n-1 back onto
   `to`. Each base step appends one move. `count` is the number of moves already
   written; the new count is returned. */
static size_t
solve_recursive(
  unsigned char total_discs,
  unsigned char remaining_discs,
  unsigned char tower_01,
  unsigned char tower_02,
  unsigned char tower_03,
  struct Move* out) {

  size_t total_moves = (1UL << total_discs) - 1UL;
  unsigned char from_towers[2] = {tower_01, tower_02};
  unsigned char to_towers[2] = {tower_02, tower_03};

  unsigned char from_tower = from_towers[0];
  unsigned char other_from_tower = from_towers[1];

  unsigned char to_tower = to_towers[0];
  unsigned char other_to_tower = to_towers[1];

  // TODO: use value of `total_discs % 2 == 1`
  if (total_discs % 2 == 1) {
    to_tower = to_towers[1];
    other_to_tower = to_towers[0];
  }

  /* printf("Total moves %zu", total_moves); */
  for (size_t count = 0; count < total_moves; count += 1) {
    /* if (remaining_discs == 0) { */
    /*   tower_02 = tower_01; */
    /* } */
    /* printf("\nMove %zu is from %hhu to %hhu\n", count, from_tower, to_tower); */
    out[count].from = from_tower;
    out[count].to = to_tower;

    if (count == total_moves - 1) {
      break;
    }

    if ((count % 2) == 0) {
      unsigned char temp_to_tower = to_tower;

      // Change to_tower
      to_tower = other_to_tower;
      other_to_tower = temp_to_tower;
      /* printf("Count is %zu, Moving To %hhu to %hhu", count, other_to_tower, to_tower); */
    } else {
      unsigned char temp_from_tower = from_tower;

      // Change from_tower
      from_tower = other_from_tower;
      other_from_tower = temp_from_tower;
      /* printf("Count is %zu, Moving From %hhu to %hhu", count, other_from_tower, from_tower); */
    }
  }

  return total_moves;
}

size_t
solve(unsigned char discs, struct Move* out) {
  assert(discs >= 1 && discs <= MAX_DISCS);

  return solve_recursive(discs, discs, 0, 1, 2, out);
}
