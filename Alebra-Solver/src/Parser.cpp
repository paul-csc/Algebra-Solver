#include "Parser.h"
#include <cctype>
#include <charconv>

std::vector<Token> Tokenize(std::string_view src) {
    const size_t size = src.size();
    size_t i = 0;

    std::vector<Token> tokens;
    tokens.reserve(3);

    while (i < size) {
        char c = src[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            tokens.emplace_back(c);
            i++;
            continue;
        } else if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            const size_t start = i;
            bool hasDot = false;

            while (i < size && (std::isdigit(static_cast<unsigned char>(src[i])) || src[i] == '.')) {
                if (src[i] == '.') {
                    if (hasDot) {
                        Error("Mutiple dots in a number");
                    }
                    hasDot = true;
                }
                i++;
            }

            double value = 0.0;
            auto result = std::from_chars(src.data() + start, src.data() + i, value);
            if (result.ec != std::errc{}) {
                Error("Invalid number format");
            }

            tokens.emplace_back(value);
            continue;
        }

        switch (c) {
            case '+': tokens.emplace_back(TokenType::PLUS); break;
            case '-': tokens.emplace_back(TokenType::MINUS); break;
            case '*': tokens.emplace_back(TokenType::STAR); break;
            case '/': tokens.emplace_back(TokenType::FSLASH); break;
            case '^': tokens.emplace_back(TokenType::CARET); break;
            case '(': tokens.emplace_back(TokenType::LPAREN); break;
            case ')': tokens.emplace_back(TokenType::RPAREN); break;
            case '=': tokens.emplace_back(TokenType::EQUAL); break;
            default: Error("Invalid symbol");
        }

        i++;
    }

    tokens.emplace_back(TokenType::END_OF_FILE);
    return tokens;
}

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(2 * 1024 * 1024), m_Variable(' ') {}

Equation* Parser::ParseEquation() {
    m_Index = 0;
    AdditiveExpression* lhs = ParseAdditiveExpression();
    Expect(TokenType::EQUAL);
    AdditiveExpression* rhs = ParseAdditiveExpression();
    return m_Allocator.alloc<Equation>(lhs, rhs);
}

Primary* Parser::ParsePrimary() {
    if (Match(TokenType::END_OF_FILE)) {
        Error("Expected primary");
    } else if (Match(TokenType::NUMBER)) {
        return m_Allocator.alloc<Primary>(std::get<double>(Consume().Value));
    } else if (Match(TokenType::VARIABLE)) {
        char var = std::get<char>(Consume().Value);
        if (m_Variable == ' ') {
            m_Variable = var;
            return m_Allocator.alloc<Primary>(var);
        } else if (m_Variable == var) {
            return m_Allocator.alloc<Primary>(var);
        } else {
            Error("More than 1 variable");
        }
    } else if (Match(TokenType::LPAREN)) {
        Consume();
        AdditiveExpression* expr = ParseAdditiveExpression();
        Expect(TokenType::RPAREN);
        return m_Allocator.alloc<Primary>(expr);
    }

    Error("Unexpected token in primary");
    return nullptr; // never reached
}

PowerExpression* Parser::ParsePowerExpression() {
    Primary* base = ParsePrimary();

    if (Match(TokenType::CARET)) {
        Consume();
        PowerExpression* exponent = ParsePowerExpression();
        return m_Allocator.alloc<PowerExpression>(base, exponent);
    }

    return m_Allocator.alloc<PowerExpression>(base);
}

MultiplicativeExpression* Parser::ParseMultiplicativeExpression() {
    PowerExpression* left = ParsePowerExpression();
    MultiplicativeExpression* expr = m_Allocator.alloc<MultiplicativeExpression>(left);

    while (Match(
        TokenType::STAR, TokenType::FSLASH, TokenType::NUMBER, TokenType::VARIABLE, TokenType::LPAREN)) {
        BinaryOp op;
        if (Match(TokenType::STAR, TokenType::FSLASH)) {
            op = static_cast<BinaryOp>(Consume().Type);
        } else {
            op = BinaryOp::Mul;
        }
        PowerExpression* right = ParsePowerExpression();
        expr->Right.emplace_back(op, right);
    }

    return expr;
}

AdditiveExpression* Parser::ParseAdditiveExpression() {
    MultiplicativeExpression* left = ParseMultiplicativeExpression();
    AdditiveExpression* expr = m_Allocator.alloc<AdditiveExpression>(left);

    while (Match(TokenType::PLUS, TokenType::MINUS)) {
        BinaryOp op = static_cast<BinaryOp>(Consume().Type);
        MultiplicativeExpression* right = ParseMultiplicativeExpression();
        expr->Right.emplace_back(op, right);
    }

    return expr;
}
