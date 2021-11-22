#include "printer.h"
#include "solver.h"
#include "MoveDiscovery.h"

#include <iostream>

int main(int argc, char* /*argv*/[]) {
	const bool benchmark = (1 < argc);
	const auto result = Solver(scanText(std::cin), MoveRunnerFirst()).solve(benchmark ? &std::cerr : nullptr);
	if (result.size() == 0) {
		std::cerr << "Could not find a solution." << std::endl;
		return 1;
	}

	printHtml(result, false, 100, 20, std::cout);
	if (!benchmark) { printText(result, std::cerr); }
	return 0;
}

