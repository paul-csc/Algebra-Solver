#pragma once

#include "Tokenizer.h"
#include "Utils.h"
#include <string>
#include <variant>
#include <vector>

enum class BinaryOp {
    Add = TokenType::PLUS,
    Sub = TokenType::MINUS,
    Mul = TokenType::STAR,
    Div = TokenType::FSLASH,
    Pow = TokenType::CARET,
};
enum class UnaryOp { None = -1, Neg = TokenType::MINUS };
enum class FunctionType { Sin, Cos, Tan, Asin, Acos, Atan, Log, Ln, Sqrt, Floor, Ceil, Abs };

struct AdditiveExpression;
struct FunctionCall;

struct Primary {
    explicit Primary(double d) : Value(d) {}
    explicit Primary(const std::string& s) : Value(s) {}
    explicit Primary(AdditiveExpression* e) : Value(e) {}
    explicit Primary(FunctionCall* f) : Value(f) {}
    std::variant<double, std::string, AdditiveExpression*, FunctionCall*> Value;
};
struct UnaryExpression {
    UnaryExpression(UnaryOp o, Primary* p) : Op(o), Expr(p) {}
    UnaryExpression(UnaryOp o, UnaryExpression* u) : Op(o), Expr(u) {}
    UnaryOp Op;
    std::variant<Primary*, UnaryExpression*> Expr;
};
struct PowerExpression {
    explicit PowerExpression(UnaryExpression* u, PowerExpression* e = nullptr) : Base(u), Exponent(e) {}
    UnaryExpression* Base;
    PowerExpression* Exponent;
};
struct MultiplicativeExpression {
    explicit MultiplicativeExpression(PowerExpression* e) : Left(e) {}
    PowerExpression* Left;
    std::vector<std::pair<BinaryOp, PowerExpression*>> Right;
};
struct AdditiveExpression {
    explicit AdditiveExpression(MultiplicativeExpression* e) : Left(e) {}
    MultiplicativeExpression* Left;
    std::vector<std::pair<BinaryOp, MultiplicativeExpression*>> Right;
};
struct FunctionCall {
    FunctionCall(FunctionType t, AdditiveExpression* expr) : Fn(t), Argument(expr) {}
    FunctionType Fn;
    AdditiveExpression* Argument;
};
struct Equation {
    Equation(AdditiveExpression* l, AdditiveExpression* r) : Lhs(l), Rhs(r) {}
    AdditiveExpression* Lhs;
    AdditiveExpression* Rhs;
};

class Parser {
  public:
    Parser(const std::vector<Token>& tokens);
    Equation* ParseEquation();

  private:
    Primary* ParsePrimary();
    UnaryExpression* ParseUnaryExpression();
    PowerExpression* ParsePowerExpression();
    MultiplicativeExpression* ParseMultiplicativeExpression();
    AdditiveExpression* ParseAdditiveExpression();

    Token Consume() { return m_Tokens[m_Index++]; }

    template <typename... Args>
    bool Match(TokenType first, Args... rest) {
        if (((m_Tokens[m_Index].Type == first) || ... || (m_Tokens[m_Index].Type == rest))) {
            return true;
        }
        return false;
    }

    Token Expect(TokenType type) {
        if (m_Tokens[m_Index].Type != type) {
            Error("Expected " + std::to_string(int(type)));
        }
        return Consume();
    }

    const std::vector<Token>& m_Tokens;
    size_t m_Index;
    ArenaAllocator m_Allocator;

    std::string m_Variable;
};
