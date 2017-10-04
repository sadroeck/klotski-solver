cmake_minimum_required (VERSION 2.8.11)
project (KLOTSKI-SOLVER)

add_executable (run-tests test.cpp solver.cpp printer.cpp block.cpp MoveDiscovery.cpp MoveValidation.cpp puzzle.cpp)
add_executable (solve main.cpp solver.cpp block.cpp MoveDiscovery.cpp MoveValidation.cpp puzzle.cpp)