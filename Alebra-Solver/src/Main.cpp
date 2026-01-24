#include "Parser.h"
#include "Solver.h"
#include <iostream>

int main() {
    const std::string_view equation = "y = --1";

    double result = Solve(equation);
    std::cout << "Result: " << result << '\n';

    std::cin.get();
    return 0;
}