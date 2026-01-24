#pragma once

#include <string_view>

struct LinearForm {
    double Coefficient = 0.0;
    double Constant = 0.0;
};

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

double Solve(std::string_view equation);