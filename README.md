# klotski-solver
a klotski-solver written in C++  
No external libraries used. Not even for UTests.  
text input.  
text, svg, animated html output.  

# Build instructions
## Linux
rm -frv "/tmp/build";  
mkdir -pv "/tmp/build" && cmake -B "/tmp/build" -S . && make -j --directory="/tmp/build" && echo done

## Windows
mkdir build && cd build && cmake ..  
Build with Visual studio 2017

# Run
## Tests
### Linux
"/tmp/build/unit-tests" && echo done

### Windows
run `unit-tests.exe` in debug build

## Solve
### Linux
"/tmp/build/solve" < "in_smallPuzzle.txt" 1> out_smallPuzzle.html  
"/tmp/build/solve" < "in_smallPuzzle.txt" 2> /dev/null  
"/tmp/build/solve" < "in_largePuzzle.txt"  
time "/tmp/build/solve" < "in_hardPuzzle.txt" 1> out_hardPuzzle.html 2> out_hardPuzzle.txt  

### Windows
`solve.exe` < "in_smallPuzzle.txt" 1> out_smallPuzzle.html

# Notes
`solve` is using :
- standard input as puzzle to solve
- standard output as html solution
- error output as text solution and puzzle bug
