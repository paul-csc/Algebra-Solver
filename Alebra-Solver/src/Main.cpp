#include "Solver.h"
#include <iostream>

int main() {
    const std::string_view equation = "x = asin(1)";
    const auto solutions = Solve(equation);

    if (solutions.IsNone) {
        std::cout << "No solution\n";
    } else if (solutions.IsInfinite) {
        std::cout << "Infinite solutions\n";
    } else {
        for (double solution : solutions.Values) {
            std::cout << solution << "\n";
        }
    }
    
    std::cin.get();
    return 0;
}
