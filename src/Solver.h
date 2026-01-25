#pragma once

#include <string_view>
#include <vector>

struct Solutions {
    std::vector<double> Values;
    bool IsInfinite = false;
    bool IsNone = false;
};

Solutions Solve(std::string_view equation);
