#pragma once

#include "Utils.h"
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class TokenType {
    VARIABLE,
    NUMBER,
    PLUS,
    MINUS,
    STAR,
    CARET,
    FSLASH,
    LPAREN,
    RPAREN,
    EQUAL,
    END_OF_FILE
};
enum class BinaryOp {
    Add = TokenType::PLUS,
    Sub = TokenType::MINUS,
    Mul = TokenType::STAR,
    Div = TokenType::FSLASH,
    Pow = TokenType::CARET,
};
enum class UnaryOp {
    Add = TokenType::PLUS,
    Sub = TokenType::MINUS,
};

struct AdditiveExpression;

struct Primary {
    explicit Primary(double d) : Value(d) {}
    explicit Primary(char c) : Value(c) {}
    explicit Primary(AdditiveExpression* e) : Value(e) {}
    std::variant<double, char, AdditiveExpression*> Value;
};
struct UnaryExpression {
    
};
struct PowerExpression {
    explicit PowerExpression(Primary* p, PowerExpression* e = nullptr) : Base(p), Exponent(e) {}
    Primary* Base;
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
struct Equation {
    Equation(AdditiveExpression* l, AdditiveExpression* r) : Lhs(l), Rhs(r) {}
    AdditiveExpression* Lhs;
    AdditiveExpression* Rhs;
};

struct Token {
    Token(double num) : Type(TokenType::NUMBER), Value(num) {}
    Token(char var) : Type(TokenType::VARIABLE), Value(var) {}
    Token(TokenType type) : Type(type), Value(std::monostate{}) {}

    TokenType Type;
    std::variant<std::monostate, double, char> Value;
};

std::vector<Token> Tokenize(std::string_view src);

class Parser {
  public:
    Parser(const std::vector<Token>& tokens);
    Equation* ParseEquation();

  private:
    Primary* ParsePrimary();
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

    char m_Variable;
};
