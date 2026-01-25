#include "Solver.h"
#include <iomanip>
#include <iostream>
#include <sstream>

static std::string FormatDouble(double x, int precision = 6) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << x;

    std::string s = oss.str();
    s.erase(s.find_last_not_of('0') + 1);
    if (!s.empty() && s.back() == '.') {
        s.pop_back();
    }

    return s;
}

int main() {
    const std::string_view equation = "x = 3pi + e";

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input.empty()) {
            continue;
        }

        std::istringstream is(input);
        std::string cmd;
        is >> cmd;

        if (cmd == "quit") {
            break;
        }

        const auto solutions = Solve(is.str());
        if (solutions.IsNone) {
            std::cout << "No solution\n";
        } else if (solutions.IsInfinite) {
            std::cout << "Infinite solutions\n";
        } else {
            for (double solution : solutions.Values) {
                std::cout << FormatDouble(solution, 10) << "\n";
            }
        }
    }

    return 0;
}
