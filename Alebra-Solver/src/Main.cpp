#include "Parser.h"
#include "Utils.h"
#include <iostream>

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

LinearForm AnalyzeAdditive(AdditiveExpression* expr);

LinearForm AnalyzePrimary(Primary* prim) {
    LinearForm result;
    std::visit(overloaded{ [&](double value) { result = { 0.0, value }; },
                   [&](char c) { result = { 1.0, 0.0 }; },
                   [&](AdditiveExpression* expr) { result = AnalyzeAdditive(expr); } },
        prim->Value);
    return result;
}

LinearForm AnalyzeMultiplicative(MultiplicativeExpression* expr) {
    auto result = AnalyzePrimary(expr->Left);

    for (const auto& [op, rhs] : expr->Right) {
        auto rhsResult = AnalyzePrimary(rhs);

        if (op == BinaryOp::Mul) {
            if (result.Coefficient != 0 && rhsResult.Coefficient != 0) {
                Error("Non-linear multiplication");
            } else if (rhsResult.Coefficient == 0) {
                result.Coefficient *= rhsResult.Constant;
                result.Constant *= rhsResult.Constant;
            } else if (result.Coefficient == 0) {
                result.Coefficient = rhsResult.Coefficient * result.Constant;
                result.Constant *= rhsResult.Constant;
            }
        } else if (op == BinaryOp::Div) {
            if (rhsResult.Coefficient != 0) {
                Error("Division by expression containing x");
            } else if (rhsResult.Constant == 0) {
                Error("Division by zero");
            } else {
                result.Coefficient /= rhsResult.Constant;
                result.Constant /= rhsResult.Constant;
            }
        }
    }

    return result;
}

LinearForm AnalyzeAdditive(AdditiveExpression* expr) {
    auto result = AnalyzeMultiplicative(expr->Left);

    for (const auto& [op, rhs] : expr->Right) {
        auto rhsResult = AnalyzeMultiplicative(rhs);

        if (op == BinaryOp::Add) {
            result.Coefficient += rhsResult.Coefficient;
            result.Constant += rhsResult.Constant;
        } else if (op == BinaryOp::Sub) {
            result.Coefficient -= rhsResult.Coefficient;
            result.Constant -= rhsResult.Constant;
        }
    }

    return result;
}

double Solve(std::string_view equation) {
    const auto tokens = Tokenize(equation);
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& token = tokens[i];

        std::cout << i + 1 << ": " << int(token.Type) << " ";
        std::visit(
            [](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    std::cout << "none";
                } else {
                    std::cout << arg;
                }
            },
            token.Value);
        std::cout << "\n";
    }

    Parser parser(tokens);
    const auto eq = parser.ParseEquation();

    auto left = AnalyzeAdditive(eq->Lhs);
    auto right = AnalyzeAdditive(eq->Rhs);

    double A = left.Coefficient - right.Coefficient;
    double B = right.Constant - left.Constant;

    const double EPS = 1e-12;
    if (std::abs(A) < EPS) {
        if (std::abs(B) < EPS) {
            Error("Infinite solutions");
        } else {
            Error("No solution");
        }
    }

    return B / A;
}

int main() {
    const std::string_view equation = "3(2x + .5) = 7x";

    double result = Solve(equation);
    std::cout << "Result: " << result << '\n';

    std::cin.get();
    return 0;
}