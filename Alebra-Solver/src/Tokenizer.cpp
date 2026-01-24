#include "Tokenizer.h"
#include "Utils.h"
#include <charconv>
#include <unordered_map>

static const std::unordered_map<std::string_view, TokenType> functions{};

Tokenizer::Tokenizer(std::string_view src) : m_Src(src), m_Size(src.size()), m_Index(0) {}

std::vector<Token> Tokenizer::Tokenize() {
    m_Index = 0;
    std::vector<Token> tokens;
    tokens.reserve(5);

    while (m_Index < m_Size) {
        const char c = m_Src[m_Index];

        if (IsSpace(c)) {
            m_Index++;
            continue;
        } else if (IsAlpha(c)) {
            const size_t start = m_Index;
            while (m_Index < m_Size && IsAlnum(m_Src[m_Index])) {
                m_Index++;
            }
            std::string lexeme(m_Src.data() + start, m_Index - start);

            auto it = functions.find(lexeme);
            if (it != functions.end()) {
                tokens.emplace_back(it->second);
            } else {
                tokens.emplace_back(lexeme);
            }
            continue;
        } else if (IsDigit(c) || c == '.') {
            const size_t start = m_Index;
            bool hasDot = false;

            while (m_Index < m_Size && (IsDigit(m_Src[m_Index]) || m_Src[m_Index] == '.')) {
                if (m_Src[m_Index] == '.') {
                    if (hasDot) {
                        Error("Mutiple dots in a number");
                    }
                    hasDot = true;
                }
                m_Index++;
            }

            double value = 0.0;
            auto result = std::from_chars(m_Src.data() + start, m_Src.data() + m_Index, value);
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
            default: Error(std::string("Invalid symbol '") + c + "'");
        }

        m_Index++;
    }

    tokens.emplace_back(TokenType::END_OF_FILE);

    return tokens;
}

bool Tokenizer::IsAlpha(char c) {
    return std::isalpha(static_cast<unsigned char>(c));
}

bool Tokenizer::IsAlnum(char c) {
    return std::isalnum(static_cast<unsigned char>(c));
}

bool Tokenizer::IsDigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Tokenizer::IsSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}
