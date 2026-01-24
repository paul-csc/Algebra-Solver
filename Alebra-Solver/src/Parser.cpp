#include "Parser.h"

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(2 * 1024 * 1024), m_Variable(" ") {}

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
    } else if (Match(TokenType::IDENTIFIER)) {
        std::string var = std::get<std::string>(Consume().Value);
        if (m_Variable == " ") {
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

UnaryExpression* Parser::ParseUnaryExpression() {
    if (Match(TokenType::PLUS)) {
        Consume();
        return ParseUnaryExpression(); // unary + does nothing
    } else if (Match(TokenType::MINUS)) {
        Consume();
        return m_Allocator.alloc<UnaryExpression>(UnaryOp::Neg, ParseUnaryExpression());
    }

    return m_Allocator.alloc<UnaryExpression>(UnaryOp::None, ParsePrimary());
}

PowerExpression* Parser::ParsePowerExpression() {
    UnaryExpression* base = ParseUnaryExpression();

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
        TokenType::STAR, TokenType::FSLASH, TokenType::NUMBER, TokenType::IDENTIFIER, TokenType::LPAREN)) {
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
