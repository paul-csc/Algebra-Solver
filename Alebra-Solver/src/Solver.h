#pragma once

#include <string_view>
#include <vector>

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

struct Solutions {
    std::vector<double> Values;
    bool IsInfinite = false;
    bool IsNone = false;
};

Solutions Solve(std::string_view equation);
