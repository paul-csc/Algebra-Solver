#pragma once

#include <string>
#include <variant>
#include <vector>

enum class TokenType {
    IDENTIFIER,
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

struct Token {
    Token(double num) : Type(TokenType::NUMBER), Value(num) {}
    Token(const std::string& s) : Type(TokenType::IDENTIFIER), Value(s) {}
    Token(TokenType type) : Type(type), Value(std::monostate{}) {}

    TokenType Type;
    std::variant<std::monostate, double, std::string> Value;
};

class Tokenizer {
  public:
    Tokenizer(std::string_view src);
    std::vector<Token> Tokenize();

  private:
    static bool IsAlpha(char c);
    static bool IsAlnum(char c);
    static bool IsDigit(char c);
    static bool IsSpace(char c);

    const std::string_view m_Src;
    const size_t m_Size;
    size_t m_Index;
};