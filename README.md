# klotski-solver
a klotski-solver written in C++
No external libraries used. Not even for UTests.

# Build instructions
## Linux
mkdir build && cd build && cmake .. && make -j<N>

## Windows
mkdir build && cd build && cmake .. 
Build with Visual studio 2017

# Run
## Tests
run `run-test.exe` in debug build

## Solve
run `solve.exe`
Puzzles are hardcoded in `main.cpp`

# Notes
Solvers can be created using:   
`auto solver = makeSolver( Puzzle{ ... });`
See `puzzle.h` for how to construct Puzzles.

Printing & debug information on the end-result can be obtained by solves' template parameter:   
`solver.solve<true>()`
