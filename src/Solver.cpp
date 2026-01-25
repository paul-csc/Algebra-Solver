#include "Solver.h"
#include "Parser.h"
#include <cmath>
#include <numbers>

constexpr double EPS = 1e-12;

struct Polynomial { // Ax^2 + Bx + C
    double A = 0.0;
    double B = 0.0;
    double C = 0.0;
};

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

Polynomial operator+(const Polynomial& left, const Polynomial& right) {
    return { left.A + right.A, left.B + right.B, left.C + right.C };
}
Polynomial operator-(const Polynomial& left, const Polynomial& right) {
    return { left.A - right.A, left.B - right.B, left.C - right.C };
}
Polynomial operator*(const Polynomial& left, const Polynomial& right) {
    if ((left.A != 0.0 && right.A != 0.0) || (left.A != 0.0 && right.B != 0.0) ||
        (left.B != 0.0 && right.A != 0.0)) {
        Error("Multiplication results in degree > 2");
    }

    Polynomial result;
    result.A = left.A * right.C + left.B * right.B + left.C * right.A;
    result.B = left.B * right.C + left.C * right.B;
    result.C = left.C * right.C;
    return result;
}
Polynomial operator/(const Polynomial& left, const Polynomial& right) {
    if (right.A != 0 || right.B != 0) {
        Error("Division by variable expression");
    } else if (right.A == 0 && right.B == 0 && right.C == 0) {
        Error("Division by zero");
    }
    return { left.A / right.C, left.B / right.C, left.C / right.C };
}

static constexpr double DegToRadians(double deg) {
    return deg * std::numbers::pi / 180.0;
}
static constexpr double RadiansToDeg(double rad) {
    return rad * 180.0 / std::numbers::pi;
}

static Polynomial AnalyzeAdditive(AdditiveExpression* expr);

static Polynomial AnalyzePrimary(Primary* prim) {
    Polynomial result;
    std::visit(Overloaded{ [&](double value) { result = { 0.0, 0.0, value }; },
                   [&](std::string) { result = { 0.0, 1.0, 0.0 }; },
                   [&](AdditiveExpression* expr) { result = AnalyzeAdditive(expr); },
                   [&](FunctionCall* fn) {
                       result = AnalyzeAdditive(fn->Argument);

                       if (result.A != 0 || result.B != 0) {
                           Error("Variable expression in function");
                       }

                       double& n = result.C;
                       switch (fn->Fn) {
                           case FunctionType::Sin: n = std::sin(DegToRadians(n)); break;
                           case FunctionType::Cos: n = std::cos(DegToRadians(n)); break;
                           case FunctionType::Tan:
                               if (std::abs(std::cos(DegToRadians(n))) < EPS) {
                                   Error("Tan undefined (cos(x) = 0)");
                               }
                               n = std::tan(DegToRadians(n));
                               break;
                           case FunctionType::Asin:
                               if (n < -1.0 || n > 1.0) {
                                   Error("Asin domain is [-1, 1]");
                               }
                               n = RadiansToDeg(std::asin(n));
                               break;
                           case FunctionType::Acos:
                               if (n < -1.0 || n > 1.0) {
                                   Error("Acos domain is [-1, 1]");
                               }
                               n = RadiansToDeg(std::acos(n));
                               break;
                           case FunctionType::Atan: n = RadiansToDeg(std::atan(n)); break;
                           case FunctionType::Log:
                               if (n <= 0) {
                                   Error("Logarithm of non-positive number");
                               }
                               n = std::log10(n);
                               break;
                           case FunctionType::Ln:
                               if (n <= 0) {
                                   Error("Logarithm of non-positive number");
                               }
                               n = std::log(n);
                               break;
                           case FunctionType::Sqrt:
                               if (n < 0) {
                                   Error("Square root of negative number");
                               }
                               n = std::sqrt(n);
                               break;
                           case FunctionType::Floor: n = std::floor(n); break;
                           case FunctionType::Ceil: n = std::ceil(n); break;
                           case FunctionType::Abs: n = std::abs(n); break;
                           default: Error("Unknown function");
                       }
                   } },
        prim->Value);
    return result;
}

static Polynomial AnalyzeUnary(UnaryExpression* expr) {
    Polynomial result;

    std::visit(Overloaded{ [&](Primary* prim) { result = AnalyzePrimary(prim); },
                   [&](UnaryExpression* inner) { result = AnalyzeUnary(inner); } },
        expr->Expr);

    if (expr->Op == UnaryOp::Neg) {
        result.A *= -1;
        result.B *= -1;
        result.C *= -1;
    }

    return result;
}

static Polynomial AnalyzePower(PowerExpression* expr) {
    double exponentValue = 1.0;

    if (expr->Exponent) {
        Polynomial exp = AnalyzePower(expr->Exponent);

        if (exp.A != 0 || exp.B != 0) {
            Error("Exponent contains variable");
        }

        exponentValue = exp.C;
    }

    Polynomial base = AnalyzeUnary(expr->Base);

    if (std::abs(exponentValue) < EPS) { // x^0
        return { 0.0, 0.0, 1.0 };
    } else if (std::abs(exponentValue - 1.0) < EPS) { // x^1
        return base;
    } else if (std::abs(exponentValue - 2.0) < EPS) { // (linear)^2
        if (base.A != 0.0) {
            Error("Exponent too high");
        }
        return { base.B * base.B, 2 * base.B * base.C, base.C * base.C };
    } else if (base.B != 0.0 || base.A != 0.0) {
        Error("Exponent too high");
    }

    return { 0.0, 0.0, std::pow(base.C, exponentValue) }; // constant^exponent
}

static Polynomial AnalyzeMultiplicative(MultiplicativeExpression* expr) {
    Polynomial result = AnalyzePower(expr->Left);

    for (const auto& [op, rhs] : expr->Right) {
        Polynomial rhsResult = AnalyzePower(rhs);

        if (op == BinaryOp::Mul) {
            result = result * rhsResult;
        } else if (op == BinaryOp::Div) {
            result = result / rhsResult;
        }
    }

    return result;
}

static Polynomial AnalyzeAdditive(AdditiveExpression* expr) {
    Polynomial result = AnalyzeMultiplicative(expr->Left);

    for (const auto& [op, rhs] : expr->Right) {
        Polynomial rhsResult = AnalyzeMultiplicative(rhs);

        if (op == BinaryOp::Add) {
            result = result + rhsResult;
        } else if (op == BinaryOp::Sub) {
            result = result - rhsResult;
        }
    }

    return result;
}

Solutions Solve(std::string_view equation) {
    Tokenizer tokenizer(equation);
    const auto tokens = tokenizer.Tokenize();

    // for (size_t i = 0; i < tokens.size(); i++) {
    //     const auto& token = tokens[i];
    //    std::cout << i + 1 << ": " << int(token.Type) << " ";
    //    std::visit(
    //        [](auto&& arg) {
    //            using T = std::decay_t<decltype(arg)>;
    //            if constexpr (!std::is_same_v<T, std::monostate>) {
    //                std::cout << arg;
    //            }
    //        },
    //        token.Value);
    //    std::cout << "\n";
    // }

    Parser parser(tokens);
    const Equation* eq = parser.ParseEquation();

    Polynomial left = AnalyzeAdditive(eq->Lhs);
    Polynomial right = AnalyzeAdditive(eq->Rhs);

    double a = left.A - right.A;
    double b = left.B - right.B;
    double c = left.C - right.C;
    double delta = b * b - 4 * a * c;

    Solutions solutions;

    if (std::abs(a) < EPS) { // linear
        if (std::abs(b) < EPS) {
            if (std::abs(c) < EPS) {
                solutions.IsInfinite = true;
            } else {
                solutions.IsNone = true;
            }
        } else {
            solutions.Values.emplace_back(-c / b);
        }
        return solutions;
    } else if (delta < 0) { // no solution
        solutions.IsNone = true;
    } else if (delta < EPS) { // 1 solution
        solutions.Values.emplace_back(-b / (2 * a));
    } else { // 2 solutions
        double sqrtDelta = std::sqrt(delta);
        solutions.Values.emplace_back((-b + sqrtDelta) / (2 * a));
        solutions.Values.emplace_back((-b - sqrtDelta) / (2 * a));
    }

    return solutions;
}
