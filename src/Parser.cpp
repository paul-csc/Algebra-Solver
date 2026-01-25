#include "Parser.h"
#include <unordered_map>

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(2 * 1024 * 1024), m_Variable(" ") {}

Equation* Parser::ParseEquation() {
    m_Index = 0;
    AdditiveExpression* lhs = ParseAdditiveExpression();
    Expect(TokenType::EQUAL);
    AdditiveExpression* rhs = ParseAdditiveExpression();
    return m_Allocator.alloc<Equation>(lhs, rhs);
}

static const std::unordered_map<std::string_view, FunctionType> FunctionTable{
    {   "sin",   FunctionType::Sin },
    {   "cos",   FunctionType::Cos },
    {   "tan",   FunctionType::Tan },
    {  "asin",  FunctionType::Asin },
    {  "acos",  FunctionType::Acos },
    {  "atan",  FunctionType::Atan },
    {   "log",   FunctionType::Log },
    {    "ln",    FunctionType::Ln },
    {  "sqrt",  FunctionType::Sqrt },
    { "floor", FunctionType::Floor },
    {  "ceil",  FunctionType::Ceil },
    {   "abs",   FunctionType::Abs },
};

Primary* Parser::ParsePrimary() {
    if (Match(TokenType::END_OF_FILE)) {
        Error("Expected primary");
    } else if (Match(TokenType::NUMBER)) {
        return m_Allocator.alloc<Primary>(std::get<double>(Consume().Value));
    } else if (Match(TokenType::IDENTIFIER)) {
        std::string ident = std::get<std::string>(Consume().Value);

        if (Match(TokenType::LPAREN)) {
            Consume();
            AdditiveExpression* expr = ParseAdditiveExpression();
            Expect(TokenType::RPAREN);

            auto it = FunctionTable.find(ident);
            if (it != FunctionTable.end()) {
                return m_Allocator.alloc<Primary>(m_Allocator.alloc<FunctionCall>(it->second, expr));
            } else {
                Error("No function named " + ident);
            }
        }

        if (m_Variable == " ") {
            m_Variable = ident;
            return m_Allocator.alloc<Primary>(ident);
        } else if (m_Variable == ident) {
            return m_Allocator.alloc<Primary>(ident);
        }
        Error("More than 1 variable");
    } else if (Match(TokenType::LPAREN)) {
        Consume();
        AdditiveExpression* expr = ParseAdditiveExpression();
        Expect(TokenType::RPAREN);
        return m_Allocator.alloc<Primary>(expr);
    }

    Error("Unexpected token in primary");
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
