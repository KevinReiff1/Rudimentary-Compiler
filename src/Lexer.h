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

    std::vector<Token> tokens;

    bool isEOF() const {
        return pos >= size;
    }

    void scanToken() {
    }

public:
    std::vector<Token> scan() {
        while (!isEOF()) {
            scanToken();
        }
        return tokens;
    }
};
