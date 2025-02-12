#pragma once
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum class TokenType {
    EOP,
    OPEN_BLOCK,
    CLOSE_BLOCK,
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


enum class Keyword {
    EOP,
    OPEN_BLOCK,
    CLOSE_BLOCK,
    PRINT,
    ASSIGNMENT,
    WHILE,
    IF,
    QUOTE,
    LEFT_PAREN,
    RIGHT_PAREN,
    INT,
    STRING,
    BOOLEAN,
    EQUAL,
    NOT_EQUAL,
    TRUE,
    FALSE,
    PLUS
};

static std::array<std::string, static_cast<size_t>(Keyword::PLUS) + 1> token_names = {
    "$",
    "{",
    "}",
    "print",
    "=",
    "while",
    "if",
    "\"",
    "(",
    ")",
    "int",
    "string",
    "boolean",
    "==",
    "!=",
    "true",
    "false",
    "+"
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

    bool match(char expected) {
        if (isEOF() || source[pos] != expected) return false;
        ++pos;
        return true;
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

    void scan_comment() {
        while (peek() != '*' || !match('*')) {
            if (peek() == '\n') line++;
            advance();
        }

        if (isEOF()) {
            throw std::runtime_error("Unterminated comment");
        }
    }

    void scan_keyword() {
        size_t start = pos;

        while (isalpha(peek())) {
            advance();
        }

        const auto keyword = source.substr(start, pos - start);
        tokens.push_back({TokenType::UNKNOWN, keyword});
    }

    void scanToken() {
        auto c = advance();
        while (isspace(c))
            c = advance();

        switch (c) {
            case '{':
                tokens.push_back({TokenType::OPEN_BLOCK, "{"});

                std::cout << "DEBUG Lexer - OPEN_BLOCK [ { ] found at (" << line << ':' << pos << ")\n";
                break;
            case '}':
                tokens.push_back({TokenType::CLOSE_BLOCK, "}"});

                std::cout << "DEBUG Lexer - CLOSE_BLOCK [ } ] found at (" << line << ':' << pos << ")\n";
                break;
            case '(':

                tokens.push_back({TokenType::LEFT_PAREN, "("});
                std::cout << "DEBUG Lexer - LEFT_PAREN [ ( ] found at (" << line << ':' << pos << ")\n";
                break;
            case ')':
                tokens.push_back({TokenType::RIGHT_PAREN, ")"});
                std::cout << "DEBUG Lexer - RIGHT_PAREN [ ) ] found at (" << line << ':' << pos << ")\n";
                break;
            case '/':
                if (match('*'))
                    scan_comment();
                break;
            case '"':
                scan_string();
                break;

            case '$':
                tokens.push_back({TokenType::EOP, "$"});
                std::cout << "DEBUG Lexer - EOP [ $ ] found at (" << line << ':' << pos << ")\n";
                break;

            default:
                if (isalpha(c)) {
                    scan_keyword();
                }
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

    int getErrorCount() const {
        return 0;
    }
};
