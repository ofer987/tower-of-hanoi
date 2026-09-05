/* _POSIX_C_SOURCE exposes struct sigaction under -std=c2x, which otherwise
   restricts headers to ISO C. */
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./hanoi.h"
#include "./render.h"

#define POLL_TIMEOUT_MS 100

enum arg_error {
  ARG_NOT_A_NUMBER = -1,
  ARG_TRAILING_CHARS = -2,
  ARG_NOT_POSITIVE = -3,
  ARG_TOO_LARGE = -4,
};

/* Set by the signal handler, polled by the driver loop. The handler does
   nothing else: tb_shutdown() is not async-signal-safe, so terminal restore
   happens on the loop's normal exit path. See
   docs/adr/0002-terminating-signals-set-a-flag-checked-by-the-loop.md. */
static volatile sig_atomic_t terminate_requested = 0;

static void
handle_terminating_signal(int sig) {
  (void)sig;
  terminate_requested = 1;
}

static void
install_signal_handlers(void) {
  struct sigaction sa = {0};
  sa.sa_handler = handle_terminating_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  /* SIGKILL and SIGSTOP cannot be caught. */
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
}

static long
read_argument(const char* string) {
  char* end_str;
  long value = strtol(string, &end_str, 10);

  if (end_str == string) {
    return ARG_NOT_A_NUMBER;
  }
  if (*end_str != '\0') {
    return ARG_TRAILING_CHARS;
  }
  if (value <= 0) {
    return ARG_NOT_POSITIVE;
  }
  if (value > MAX_DISCS) {
    return ARG_TOO_LARGE;
  }

  return value;
}

int
main(int argc, char* argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "usage: %s <disc count 1..%d>\n", argv[0], MAX_DISCS);
    return EXIT_FAILURE;
  }

  long argument = read_argument(argv[1]);
  if (argument < 0) {
    switch (argument) {
      case ARG_NOT_A_NUMBER: fprintf(stderr, "not a number: '%s'\n", argv[1]); break;
      case ARG_TRAILING_CHARS: fprintf(stderr, "trailing characters after the number: '%s'\n", argv[1]); break;
      case ARG_NOT_POSITIVE: fprintf(stderr, "the disc count must be positive: '%s'\n", argv[1]); break;
      case ARG_TOO_LARGE: fprintf(stderr, "the disc count must be at most %d: '%s'\n", MAX_DISCS, argv[1]); break;
    }
    return EXIT_FAILURE;
  }

  unsigned char discs = (unsigned char)argument;

  struct Move moves[MAX_MOVES];
  size_t move_count = solve(discs, moves);

  struct Puzzle puzzle;
  puzzle_init(&puzzle, discs);

  install_signal_handlers();
  render_init();
  render_puzzle(&puzzle);

  size_t next_move = 0;
  while (!terminate_requested) {
    enum render_input input = render_wait_for_input(POLL_TIMEOUT_MS);

    if (input == RENDER_QUIT) {
      break;
    }

    if (input == RENDER_ADVANCE) {
      if (next_move >= move_count) {
        break; /* already solved - any key quits */
      }

      puzzle_apply_move(&puzzle, moves[next_move]);
      next_move += 1;
    }

    /* Covers RENDER_ADVANCE and RENDER_NONE (a resize or an interrupted poll);
       tb_present() diffs against the front buffer, so an unchanged redraw is
       free. */
    render_puzzle(&puzzle);
  }

  render_shutdown();
  return 0;
}
