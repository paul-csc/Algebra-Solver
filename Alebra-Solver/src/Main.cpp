#include "Parser.h"
#include "Solver.h"
#include <iostream>

int main() {
    const std::string_view equation = "(10^0)x = 0-200";

    double result = Solve(equation);
    std::cout << "Result: " << result << '\n';

    std::cin.get();
    return 0;
}