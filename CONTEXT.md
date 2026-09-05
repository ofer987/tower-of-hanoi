# Tower of Hanoi

This project animates the Tower of Hanoi puzzle being solved. A predetermined
algorithm produces the moves; the program steps through them one at a time -
forwards or backwards - and draws each resulting state in the terminal. It is a
visualizer, not a puzzle a person plays.

## Language

**Disc**:
A single ring that rests on a tower. Discs move one at a time, and a disc may
never rest on a smaller one.
_Avoid_: ring, piece, plate

**Disc size**:
The integer `1..N` that identifies a disc. A larger number is a wider disc and
always sits lower on its tower.
_Avoid_: width, disc height, stack height

**Tower**:
One of the three uprights that discs are stacked on. The puzzle starts with
every disc on the first tower and finishes with every disc on the third.
_Avoid_: peg, rod, pole, spike

**Tower height**:
The number of discs currently on a tower.
_Avoid_: tower size, stack height, depth

**Move**:
The transfer of one disc from the top of one tower to the top of another. The
solution is `2^N - 1` moves long.
_Avoid_: turn

**Solution**:
The full ordered list of moves that takes the puzzle from its start state to
its finished state, produced by the solving algorithm rather than by a player.
_Avoid_: game, playthrough, run

**Played moves**:
How many of the solution's moves are currently applied to the towers. It fixes
exactly which state the towers are in: 0 is the start, `2^N - 1` is solved.
_Avoid_: cursor, position, index, current step

**Step forward / step back**:
Playing the next move of the solution, or undoing the last one. Stepping changes
only the played-move count; the solution itself never changes.
_Avoid_: undo / redo, scrub, seek

**Puzzle**:
The three towers, the solution in full, and the played-move count. Everything on
screen is a function of those.
_Avoid_: game, board, state, session
