#pragma once
#include <string>
#include <vector>

enum class TokenType {
    EOF_TOKEN,
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
    static constexpr char SPACE{' '};
    static constexpr char TAB{'\t'};
    static constexpr char NEWLINE{'\n'};
    static constexpr char CARRIAGE_RETURN{'\r'};

    std::vector<Token> tokens;

    bool isEOF() const {
        return pos >= size;
    }

    char advance() {
        return source[pos++];
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

            default: ;
        }
    }

public:
    explicit Lexer(const std::string& src) : source{src}, size(source.size()) {
    }

    std::vector<Token> scan() {
        while (!isEOF()) {
            scanToken();
        }
        return tokens;
    }
};
