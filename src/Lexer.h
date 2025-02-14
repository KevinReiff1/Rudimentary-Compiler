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
    static constexpr char EOP{'$'};
    static constexpr char EOFILE{'\0'};

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

    char prev() const {
        if (pos == 0)
            return EOFILE;
        return source[pos - 1];
    }

    char peek() const {
        if (isEOF())
            return EOFILE;
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
        ++column;

        while (ch = peek(), ch != '"' && ch != EOFILE && (isalpha(ch) || isspace(ch))) {
            if (!islower(ch))
                log(LogLevel::ERROR,
                    "Error:" + std::to_string(line) + ':' + std::to_string(column) + " Unrecognized character: " +
                    std::string{ch});
            else
                addToken(TokenType::CHAR, std::string{ch});
            advance();
        }

        if (isEOF() || peek() != '"') {
            log(LogLevel::ERROR,
                "Error:" + std::to_string(line) + ':' + std::to_string(column) + " Unterminated string");
        }

        advance(); // The closing "
        addToken(TokenType::QUOTE, "\"");
    }

    void scan_comment() {
        while (peek() != '*' || !match('*')) {
            advance();
        }

        if (isEOF()) {
            log(LogLevel::ERROR,
                "Error:" + std::to_string(line) + ':' + std::to_string(column) + " Unterminated comment");
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
            addToken(TokenType::PRINT, keyword);
        } else if (keyword == "if") {
            addToken(TokenType::IF, keyword);
        } else if (keyword == "while") {
            addToken(TokenType::WHILE, keyword);
        } else if (keyword == "int" || keyword == "string" || keyword == "boolean") {
            addTokenWithCUstomMessage(TokenType::I_TYPE, keyword,
                                      "I_TYPE [ " + keyword + " ] found at (" + std::to_string(line) + ':' +
                                      std::to_string(col) + ")");
        } else if (keyword == "true" || keyword == "false") {
        } else {
            addTokenWithCUstomMessage(TokenType::ID, keyword,
                                      "ID [ " + keyword + " ] found at (" + std::to_string(line) + ':' +
                                      std::to_string(col) + ")");
        }
    }


    void scan_number() {
        const auto col = column;
        char ch = prev();
        int value = 0;

        while (ch != EOFILE && isdigit(ch)) {
            value *= 10;
            value += ch - '0';

            ch = peek();
        }

        addTokenWithCUstomMessage(TokenType::NUMBER, std::to_string(value),
                                  "NUMBER [ " + std::to_string(value) + " ] found at (" + std::to_string(line) + ':' +
                                  std::to_string(col) + ")");
    }

    enum class LogLevel {
        DEBUG,
        INFO,
        ERROR,
        WARNING
    };

    static inline std::array<std::string, static_cast<size_t>(LogLevel::WARNING) + 1> log_level_names = {
        "DEBUG", "INFO ", "ERROR", "WARNING"
    };

    void log(LogLevel level, const std::string &message) {
        if (level == LogLevel::ERROR)
            ++error_count;

        std::cout << log_level_names[static_cast<size_t>(level)] << " Lexer - " << message << '\n';
    }

    void addToken(TokenType type) {
        addToken(type, token_names[static_cast<size_t>(type)]);
    }

    void addToken(TokenType type, const std::string &value) {
        addTokenWithCUstomMessage(type, value,
                                  token_type_names[static_cast<size_t>(type)] + " [ " + value + " ] found at (" +
                                  std::to_string(line) + ':' + std::to_string(column) + ")");
    }

    void addTokenWithCUstomMessage(TokenType type, const std::string &value, const std::string &message) {
        tokens.emplace_back(type, value);
        log(LogLevel::DEBUG, message);
    }

    void scan_token() {
        auto c = advance();
        while (isspace(c))
            c = advance();

        switch (c) {
            case '{':
                addToken(TokenType::OPEN_BLOCK, "{");

                break;
            case '}':
                addToken(TokenType::CLOSE_BLOCK, "}");

                break;
            case '(':
                addToken(TokenType::OPEN_PARENTHESIS, "(");

                break;
            case ')':
                addToken(TokenType::CLOSE_PARENTHESIS, ")");

                break;
            case '/':
                if (match('*'))
                    scan_comment();
                break;
            case '+':
                addToken(TokenType::INT_OP, "+");
                break;
            case '"':
                addToken(TokenType::QUOTE, "\"");
                scan_string();
                break;

            case '=':
                if (match('=')) {
                    addToken(TokenType::EQUALITY_OP, "==");
                } else {
                    addToken(TokenType::ASSIGN_OP, "=");
                }
                break;
            case '!':
                if (match('=')) {
                    addToken(TokenType::INEQUALITY_OP, "!=");
                } else
                    log(LogLevel::ERROR, "Expected '='");

                break;

            case '$':
                addToken(TokenType::EOP, "$");

                break;

            default:
                if (isalpha(c)) {
                    scan_keyword();
                } else if (isdigit(c)) {
                    scan_number();
                } else {
                    log(LogLevel::ERROR,
                        "Error:" + std::to_string(line) + ':' + std::to_string(column) + " Unrecognized Token: " +
                        std::string{c});
                }
        }
    }

public:
    explicit Lexer(const std::string &src) : source{src}, size(source.size()) {
    }

    /*std::vector<Token> scan() {
        while (!isEOF()) {
            scan_token();
        }
        return tokens;
    }*/

    void scan() {
        size_t i{0};
        while (!isEOF()) {
            log(LogLevel::INFO, "Lexing program " + std::to_string(++i) + "..");

            do {
                scan_token();
            } while (!isEOF() && prev() != EOP);

            if (error_count > 0)
                log(LogLevel::ERROR,
                    "Lex failed with " + std::to_string(error_count) + " error(s)\n");
            else
                log(LogLevel::INFO, "Lex completed with 0 errors\n");

            error_count = 0;
        }

        if (!tokens.empty() && tokens.back().type != TokenType::EOP)
            log(LogLevel::ERROR,
                "Final program missing terminating '$'. Add '$' at the end of the program to mark its termination");
    }
};
