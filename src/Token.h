#pragma once
#include <array>
#include <string>

enum class TokenType {
    EOP,
    OPEN_BLOCK,
    CLOSE_BLOCK,
    PRINT,
    ASSIGN_OP,
    WHILE,
    IF,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    I_TYPE,
    ID,
    NUMBER,
    QUOTE,
    CHAR,
    BOOL_VAL,
    EQUALITY_OP,
    INEQUALITY_OP,
    INT_OP,
    UNKNOWN
};

static std::array<std::string, static_cast<size_t>(TokenType::UNKNOWN) + 1> token_type_names = {
    "EOP",
    "OPEN_BLOCK",
    "CLOSE_BLOCK",
    "PRINT",
    "ASSIGN_OP",
    "WHILE",
    "IF",
    "OPEN_PARENTHESIS",
    "CLOSE_PARENTHESIS",
    "I_TYPE",
    "ID",
    "NUMBER",
    "QUOTE",
    "CHAR",
    "BOOL_VAL",
    "EQUALITY_OP",
    "INEQUALITY_OP",
    "INT_OP",
    "UNKNOWN"
};

struct Token {
    TokenType type;
    std::string value;
    size_t line{0};
};