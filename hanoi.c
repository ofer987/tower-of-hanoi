#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "./hanoi.h"

void
puzzle_init(struct Puzzle* puzzle, unsigned char discs) {
  assert(discs >= 1 && discs <= MAX_DISCS);

  for (unsigned char tower = 0; tower < TOWER_COUNT; tower += 1) {
    puzzle->towers[tower].height = 0;

    for (unsigned char level = 0; level < MAX_DISCS; level += 1) {
      puzzle->towers[tower].discs[level] = 0;
    }
  }

  /* Largest disc (value `discs`) on the bottom, smallest (value 1) on top. */
  for (unsigned char level = 0; level < discs; level += 1) {
    puzzle->towers[0].discs[level] = discs - level;
  }
  puzzle->towers[0].height = discs;

  puzzle->moves = 0;
  puzzle->total_moves = (size_t)((1UL << discs) - 1UL);
  puzzle->solved = false;
}

void
puzzle_apply_move(struct Puzzle* puzzle, struct Move move) {
  assert(move.from < TOWER_COUNT && move.to < TOWER_COUNT);

  struct Tower* source = &puzzle->towers[move.from];
  struct Tower* dest = &puzzle->towers[move.to];

  assert(source->height > 0 && "move from an empty tower");

  unsigned char disc = source->discs[source->height - 1];

  assert((dest->height == 0 || dest->discs[dest->height - 1] > disc) && "disc landed on a smaller disc");

  source->discs[source->height - 1] = 0;
  source->height -= 1;

  dest->discs[dest->height] = disc;
  dest->height += 1;

  puzzle->moves += 1;
  if (puzzle->moves == puzzle->total_moves) {
    puzzle->solved = true;
  }
}

/* Classic recursive Tower of Hanoi. To move `n` discs from `from` to `to` using
   `via`: move the top n-1 to `via`, move disc n to `to`, move the n-1 back onto
   `to`. Each base step appends one move. `count` is the number of moves already
   written; the new count is returned. */
static size_t
solve_recursive(
  unsigned char n,
  unsigned char from,
  unsigned char to,
  unsigned char via,
  struct Move* out,
  size_t count) {

  if (n == 0) {
    return count;
  }

  count = solve_recursive(n - 1, from, via, to, out, count);

  out[count].from = from;
  out[count].to = to;
  count += 1;

  count = solve_recursive(n - 1, via, to, from, out, count);

  return count;
}

size_t
solve(unsigned char discs, struct Move* out) {
  assert(discs >= 1 && discs <= MAX_DISCS);

  return solve_recursive(discs, 0, 2, 1, out, 0);
}
