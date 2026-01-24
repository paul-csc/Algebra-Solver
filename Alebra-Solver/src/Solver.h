#pragma once

#include <string_view>

struct LinearForm {
    double Coefficient = 0.0;
    double Constant = 0.0;
};

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

double Solve(std::string_view equation);