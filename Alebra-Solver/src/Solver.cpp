#include "Solver.h"
#include "Parser.h"
#include <iostream>

static constexpr double EPS = 1e-12;

static LinearForm AnalyzeAdditive(AdditiveExpression* expr);

static LinearForm AnalyzePrimary(Primary* prim) {
    LinearForm result;
    std::visit(
        Overloaded{ [&](double value) { result = { 0.0, value }; }, [&](char c) { result = { 1.0, 0.0 }; },
            [&](AdditiveExpression* expr) { result = AnalyzeAdditive(expr); } },
        prim->Value);
    return result;
}

static LinearForm AnalyzeUnary(UnaryExpression* expr) {
    LinearForm result;

    std::visit(Overloaded{ [&](Primary* prim) { result = AnalyzePrimary(prim); },
                   [&](UnaryExpression* inner) { result = AnalyzeUnary(inner); } },
        expr->Expr);

    if (expr->Op == UnaryOp::Neg) {
        result.Coefficient *= -1;
        result.Constant *= -1;
    }

    return result;
}

static LinearForm AnalyzePower(PowerExpression* expr) {
    double exponentValue = 1.0;

    if (expr->Exponent) {
        LinearForm exp = AnalyzePower(expr->Exponent);

        if (exp.Coefficient != 0) {
            Error("Exponent contains variable");
        }

        exponentValue = exp.Constant;
    }

    LinearForm base = AnalyzeUnary(expr->Base);

    if (!expr->Exponent) {
        return base;
    } else if (std::abs(exponentValue - 1.0) < EPS) {
        return base;
    } else if (base.Coefficient == 0.0) {
        return { 0.0, std::pow(base.Constant, exponentValue) };
    }

    Error("Non-linear power expression");
}

static LinearForm AnalyzeMultiplicative(MultiplicativeExpression* expr) {
    LinearForm result = AnalyzePower(expr->Left);

    for (const auto& [op, rhs] : expr->Right) {
        LinearForm rhsResult = AnalyzePower(rhs);

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
                Error("Division by variable expression");
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

static LinearForm AnalyzeAdditive(AdditiveExpression* expr) {
    LinearForm result = AnalyzeMultiplicative(expr->Left);

    for (const auto& [op, rhs] : expr->Right) {
        LinearForm rhsResult = AnalyzeMultiplicative(rhs);

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
                if constexpr (!std::is_same_v<T, std::monostate>) {
                    std::cout << arg;
                }
            },
            token.Value);
        std::cout << "\n";
    }

    Parser parser(tokens);
    const Equation* eq = parser.ParseEquation();

    LinearForm left = AnalyzeAdditive(eq->Lhs);
    LinearForm right = AnalyzeAdditive(eq->Rhs);

    double A = left.Coefficient - right.Coefficient;
    double B = right.Constant - left.Constant;

    if (std::abs(A) < EPS) {
        if (std::abs(B) < EPS) {
            Error("Infinite solutions");
        } else {
            Error("No solution");
        }
    }

    return B / A;
}
