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
    ASSIGN_OP,
    WHILE,
    IF,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    I_TYPE,
    ID,
    NUMBER,
    STRING,
    BOOL_VAL,
    EQUALITY_OP,
    INEQUALITY_OP,
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
    size_t column{0};
    size_t error_count{0};
    static constexpr char SPACE{' '};
    static constexpr char TAB{'\t'};
    static constexpr char NEWLINE{'\n'};
    static constexpr char CARRIAGE_RETURN{'\r'};

    std::vector<Token> tokens;

    void new_line() {
        ++line;
        column = 0;
    }

    bool isEOF() const {
        return pos >= size;
    }

    char advance() {
        const auto value = source[pos++];
        if (value == NEWLINE) {
            new_line();
        }

        if (value == SPACE || !isspace(value))
            ++column;

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
        ++column;
        return true;
    }

    void scan_string() {
        char ch{};
        while (ch = peek(), ch != '"' && ch != '\0') {
            advance();
        }

        if (isEOF()) {
            throw std::runtime_error("Unterminated string");
        }

        advance(); // The closing "

        tokens.emplace_back(TokenType::STRING, source.substr(pos - 1, source.size() - pos));
    }

    void scan_comment() {
        while (peek() != '*' || !match('*')) {
            advance();
        }

        if (isEOF()) {
            throw std::runtime_error("Unterminated comment");
        }
    }

    void scan_keyword() {
        // Since the position advanced one char to the right
        const auto start = pos - 1;
        const auto col = column - 1;

        // Skip all the alpha characters
        while (isalpha(peek())) {
            advance();
        }

        const auto keyword = source.substr(start, pos - start);
        if (keyword == "print") {
            tokens.emplace_back(TokenType::PRINT, keyword);
        } else if (keyword == "if") {
            tokens.emplace_back(TokenType::IF, keyword);
        } else if (keyword == "while") {
            tokens.emplace_back(TokenType::WHILE, keyword);
        } else if (keyword == "int" || keyword == "string" || keyword == "boolean") {
            tokens.emplace_back(TokenType::I_TYPE, keyword);
            std::cout << "DEBUG Lexer - I_TYPE [ " << keyword << " ] found at (" << line << ':' << col << ")\n";
        } else if (keyword == "true" || keyword == "false") {
        } else {
            tokens.emplace_back(TokenType::ID, keyword);
            std::cout << "DEBUG Lexer - ID [ " << keyword << " ] found at (" << line << ':' << col << ")\n";
        }
    }

    void report_error(const std::string &message) {
        ++error_count;
        std::cout << "ERROR Lexer - Error:" << line << ':' << column << " " << message << '\n';
    }

    void debug_lexer(const std::string &token_name, const std::string &value) {
        std::cout << "DEBUG Lexer - " << token_name << " [ " << value << " ] found at (" << line << ':' << column <<
                ")\n";
    }

    void scanToken() {
        auto c = advance();
        while (isspace(c))
            c = advance();

        switch (c) {
            case '{':
                tokens.emplace_back(TokenType::OPEN_BLOCK, "{");
                debug_lexer("OPEN_BLOCK", "{");

                break;
            case '}':
                tokens.emplace_back(TokenType::CLOSE_BLOCK, "}");
                debug_lexer("CLOSE_BLOCK", "}");

                break;
            case '(':
                tokens.emplace_back(TokenType::OPEN_PARENTHESIS, "(");
                debug_lexer("OPEN_PARENTHESIS", "(");

                break;
            case ')':
                tokens.emplace_back(TokenType::CLOSE_PARENTHESIS, ")");
                debug_lexer("CLOSE_PARENTHESIS", ")");

                break;
            case '/':
                if (match('*'))
                    scan_comment();
                break;
            case '"':
                scan_string();
                break;

            case '=':
                if (match('=')) {
                    tokens.emplace_back(TokenType::EQUALITY_OP, "==");
                    debug_lexer("EQUALITY_OP", "==");
                } else {
                    tokens.emplace_back(TokenType::ASSIGN_OP, "=");
                    debug_lexer("ASSIGN_OP", "=");
                }
                break;
            case '!':
                if (match('=')) {
                    tokens.emplace_back(TokenType::INEQUALITY_OP, "!=");
                    debug_lexer("INEQUALITY_OP", "!=");
                } else
                    report_error("Expected '='");

                break;

            case '$':
                tokens.emplace_back(TokenType::EOP, "$");
                debug_lexer("EOP", "$");
                break;

            default:
                if (isalpha(c)) {
                    scan_keyword();
                } else {
                    report_error("Unrecognized Token: " + c);
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

    size_t getErrorCount() const {
        return error_count;
    }
};
