# This is a solver/visualizer, not an interactive game

The program exists to show *how* the Tower of Hanoi puzzle is solved: a fixed
algorithm generates the optimal solution and the program plays it back move by
move in the terminal. A human never makes a move. Interactive play — letting a
person attempt the puzzle, with the algorithm reduced to a hint or auto-solve
button — is explicitly out of scope.

This is recorded because the early code contradicts it: UI strings like "Game
started" and framing like "game mechanics" imply a game that does not exist and
was never intended. Anyone extending the project should assume there is no
player and no move-input layer unless this decision is superseded.
