#pragma once
#include <stdexcept>
#include <string>
#include <vector>

enum class TokenType {
    END_TOKEN,
    LEFT_BRACE,
    RIGHT_BRACE,
    PRINT,
    EQUALS,
    WHILE,
    IF,
    LEFT_PAREN,
    RIGHT_PAREN,
    TYPE,
    ID,
    NUMBER,
    STRING,
    BOOL_VAL,
    BOOL_OP,
    INT_OP,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
    std::string source;
    size_t pos{0};
    size_t size{0};
    size_t line{1};
    static constexpr char SPACE{' '};
    static constexpr char TAB{'\t'};
    static constexpr char NEWLINE{'\n'};
    static constexpr char CARRIAGE_RETURN{'\r'};

    std::vector<Token> tokens;

    bool isEOF() const {
        return pos >= size;
    }

    char advance() {
        const auto value = source[pos++];
        if (value == '\n')
            ++line;

        return value;
    }

    char peek() const {
        if (isEOF())
            return '\0';
        return source[pos];
    }

    void scan_string() {
        char ch{};
        while (ch = peek(), ch != '"' && ch != '\0') {
            if (peek() == '\n') line++;
            advance();
        }

        if (isEOF()) {
            throw std::runtime_error("Unterminated string");
        }

        advance(); // The closing "

        tokens.push_back({TokenType::STRING, source.substr(pos - 1, source.size() - pos)});
    }

    void scanToken() {
        auto c = advance();
        while (c == SPACE || c == NEWLINE || c == CARRIAGE_RETURN || c == TAB)
            c = advance();

        switch (c) {
            case '{':
                tokens.push_back({TokenType::LEFT_BRACE, "{"});
                break;
            case '}':
                tokens.push_back({TokenType::RIGHT_BRACE, "}"});
                break;
            case '(':
                tokens.push_back({TokenType::LEFT_PAREN, "("});
                break;
            case ')':
                tokens.push_back({TokenType::RIGHT_PAREN, ")"});
                break;
            case '/':
                break;
            case '"':
                scan_string();
                break;
                break;

            default: ;
        }
    }

public:
    explicit Lexer(const std::string &src) : source{src}, size(source.size()) {
    }

    std::vector<Token> scan() {
        while (!isEOF()) {
            scanToken();
        }
        return tokens;
    }
};
