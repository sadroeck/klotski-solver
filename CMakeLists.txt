cmake_minimum_required (VERSION 2.8.11)
project (KLOTSKI-SOLVER)

add_executable (solve      main.cpp MoveDiscovery.cpp MoveValidation.cpp block.cpp printer.cpp puzzle.cpp solver.cpp)
add_executable (unit-tests test.cpp MoveDiscovery.cpp MoveValidation.cpp block.cpp printer.cpp puzzle.cpp solver.cpp)

target_compile_options(solve PRIVATE
#	-std=c++11
	-pipe

#	-O0

	-g

	-Wall
	-Wextra
	-Wpedantic

	-Wno-unused-function
)

