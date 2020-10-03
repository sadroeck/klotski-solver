#include "printer.h"
#include "solver.h"
#include "MoveDiscovery.h"

#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) {
	const auto result = Solver(scanText(std::cin), MoveRunnerFirst()).solve();
	if (result.size() == 0) {
		std::cerr << "Could not find a solution." << std::endl;
		return 1;
	}

	printHtml(result, true, 100, std::cout);
	//printHtml(result, false, 20, std::cout);
	printText(result, std::cerr);
	return 0;
}

