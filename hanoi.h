#pragma once

#include <stdbool.h>
#include <stddef.h>

#define TOWER_COUNT 3
#define MAX_DISCS   9

/* MAX_DISCS is capped for visualization, not by the algorithm: more discs than
   this cannot be drawn in the terminal or watched move by move. The whole
   solution is held in memory as an array of (2^MAX_DISCS - 1) moves, so a
   careless increase would blow the buffer up - hence the ceiling here. See
   docs/adr/0003-solver-is-pure-and-emits-a-move-list.md. */
_Static_assert(MAX_DISCS <= 16, "the full move list is held in memory; 2^16 moves is already unwatchable");

/* Longest solution we ever hold: 2^MAX_DISCS - 1 moves. */
#define MAX_MOVES ((1UL << MAX_DISCS) - 1UL)

struct Tower {
  unsigned char discs[MAX_DISCS]; /* index 0 is the bottom disc */
  unsigned char height;           /* number of discs on this tower */
};

struct Puzzle {
  struct Tower towers[TOWER_COUNT];
  size_t moves;       /* moves played so far */
  size_t total_moves; /* 2^discs - 1 */
  bool solved;
};

struct Move {
  unsigned char from; /* source tower index, 0..TOWER_COUNT-1 */
  unsigned char to;   /* destination tower index */
};

/* Set up the starting position: every disc on tower 0, largest at the bottom. */
void
puzzle_init(struct Puzzle* puzzle, unsigned char discs);

/* Apply one move. The move must be legal - a non-empty source tower, and the
   moved disc smaller than whatever it lands on; an illegal move aborts. */
void
puzzle_apply_move(struct Puzzle* puzzle, struct Move move);

/* Write the full optimal solution for `discs` discs (tower 0 -> tower 2) into
   `out`, which must hold at least 2^discs - 1 moves. Returns the number of
   moves written. Pure: reads only `discs`, writes only `out`. */
size_t
solve(unsigned char discs, struct Move* out);
