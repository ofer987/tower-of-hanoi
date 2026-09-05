# The solver is a pure function that emits the whole move list

`solve(discs, out)` is a pure function of the disc count. It runs once at
startup, writes all `2^discs - 1` moves into caller-owned storage, and returns
the count. It mutates no shared state and knows nothing about the terminal.
`main` then owns a driver loop that walks the array: apply one move to the
`Puzzle`, render, wait for a keypress. Solving and animating are two separate
passes.

The alternative, and what the first version did, was a solver whose recursion
*is* the animation loop: it mutated the towers in place and fired a render
callback after each move, so pacing and input handling lived inside the
recursion and every recursive call carried a function pointer.

Pulling rendering out makes the solver trivially testable — compare its output
against a known sequence, with no compile-time hooks — and puts pacing and input
in `main`, where input handling belongs. The cost is holding up to 511 moves
(disc count is capped at 9) in an array, which is nothing. Precomputing the full
list is also what makes stepping backward cheap: to go back a move, reset the
puzzle and replay a shorter prefix. No dedicated undo, no re-solving.

Holding the whole list is sound only because `MAX_DISCS` is small — and it is
small for reasons independent of the solver (see ADR 0001: you cannot render or
watch thousands of moves). A `_Static_assert` on `MAX_DISCS` keeps a careless
increase from turning into a runtime memory blowout. If this program ever grew a
non-visual mode that generates moves for large `N`, this decision would have to
be revisited in favour of a streaming solver whose memory is `O(N)`.
