#pragma once
#include <array>
#include <iostream>
#include <optional>
#include "Token.h"
#include <string>
#include <vector>
#include "Log.h"

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

    /**
     * @brief Advances the current position in the source and retrieves the next character.
     *
     * This function increments the position within the source and retrieves the corresponding character.
     * It updates internal state, including handling line breaks and column count, based on the character encountered.
     *
     * @return The next character in the source from the current position.
     */
    char advance() {
        const auto value = source[pos++];
        if (value == NEWLINE) {
            new_line();
        }

        if (value == SPACE || !isspace(value))
            ++column;

        return value;
    }

    /**
     * @brief Retrieves the previous character in the source relative to the current position.
     *
     * This function returns the character immediately preceding the current position in the source.
     * If the current position is at the beginning of the source, it returns a special `EOFILE` value
     * to signify the lack of a previous character.
     *
     * @return The previous character in the source if the current position is greater than zero,
     *         otherwise `EOFILE`.
     */
    char prev() const {
        if (pos == 0)
            return EOFILE;
        return source[pos - 1];
    }

    /**
     * @brief Retrieves the current character in the source without advancing the position.
     *
     * This function returns the character at the current position in the source. If the end of the
     * file has been reached, it returns a special `EOFILE` value to indicate this state.
     *
     * @return The current character in the source if not at the end of the file,
     *         otherwise `EOFILE`.
     */
    char peek() const {
        if (isEOF())
            return EOFILE;
        return source[pos];
    }

    char next() const {
        if (pos + 1 >= size)
            return EOFILE;
        return source[pos + 1];
    }

    /**
     * @brief Matches the current character in the source with the expected character.
     *
     * This function checks if the current character in the source matches the specified
     * `expected` character. If the characters match and the end of the file has not been
     * reached, the position and column counters are advanced, and the match is considered successful.
     *
     * @param expected The character to match against the current character in the source.
     * @return `true` if the current character matches the expected one and is within bounds,
     *         `false` otherwise.
     */
    bool match(char expected) {
        if (isEOF()) return false;

        if (source[pos] == '/') {
            if (next() == '*') {
                advance();
                advance();
                while (peek() != '*' && peek() != EOFILE) {
                    advance();
                }

                if (peek() == '*' && next() == '/') {
                    advance();
                    advance();
                }

                if (peek() == EOFILE) {
                    log(LogLevel::WARNING,
                        "WARNING:" + std::to_string(line) + ':' + std::to_string(column) + " Unterminated comment");
                    return false;
                }
            }
        }

        if (isEOF() || source[pos] != expected) return false;
        ++pos;
        ++column;
        return true;
    }

    /**
     * @brief Scans and processes string literals in the source code.
     *
     * This function handles string literals that are enclosed by double quotes (`"`). It processes
     * characters within the string while ensuring that all characters conform to specific rules:
     * - Only lowercase alphabetic characters or spaces are permitted within the string.
     * - Characters that violate these constraints are logged as errors with their respective
     *   position in the source code.
     *
     * During parsing:
     * - The function advances the lexer position while collecting valid characters into
     *   individual tokens of type `TokenType::CHAR`.
     * - Upon encountering the closing double quote, a `TokenType::QUOTE` token is added to
     *   signify the end of the string literal.
     *
     * If the end of the file is reached or the string is not properly terminated with a closing
     * double quote, the function logs an error for an unterminated string.
     */
    void scan_string() {
        char ch{};
        ++column;

        while (ch = peek(), ch != '"' && ch != EOFILE && (isalpha(ch) || isspace(ch))) {
            if (!islower(ch) && ch != SPACE)
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

            --column;
            return;
        }

        advance(); // The closing "
        addToken(TokenType::QUOTE, "\"");
    }


    /**
     * @brief Scans and processes multi-line comments in the source code.
     *
     * This function handles multi-line comments enclosed */
    void scan_comment() {
        while (peek() != '*' && peek() != EOFILE || (next() != '/' && next() != EOFILE)) {
            advance();
        }

        if ((peek() == '*' && next() != '/') || isEOF()) {
            log(LogLevel::WARNING,
                "WARNING:" + std::to_string(line) + ':' + std::to_string(column) + " Unterminated comment");
        }

        advance(); // The closing */
    }


    /**
     * @brief Scans the source code for keywords and identifiers starting from the current position.
     *
     * This function identifies keywords from a predefined set, including "print", "if", "while",
     * and types such as "int", "string", and "boolean". It also identifies boolean values like
     * "true" and "false". If the scanned token does not match any keyword, it is classified
     * as an identifier (ID).
     *
     * During the scanning process, the function:
     * - Starts scanning from one character before the current position.
     * - Advances over alphabetical characters to determine the entire keyword or identifier.
     * - Extracts the substring that represents the potential keyword or identifier.
     * - Matches this substring against known keywords and assigns the appropriate token type.
     * - Adds the token to the lexer with additional context such as line and column if necessary.
     *
     * If the substring does not match any predefined keywords, it is treated as an identifier.
     */
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
            addTokenWithCustomMessage(TokenType::I_TYPE, keyword,
                                      "I_TYPE [ " + keyword + " ] found at (" + std::to_string(line) + ':' +
                                      std::to_string(col) + ")", line);
        } else if (keyword == "true" || keyword == "false") {
            addTokenWithCustomMessage(TokenType::BOOL_VAL, keyword,
                                      "BOOL_VAL [ " + keyword + " ] found at (" + std::to_string(line) + ':' +
                                      std::to_string(col) + ")", line);
        } else {
            addTokenWithCustomMessage(TokenType::ID, keyword,
                                      "ID [ " + keyword + " ] found at (" + std::to_string(line) + ':' +
                                      std::to_string(col) + ")", line);
        }
    }

    /**
     * @brief Scans and processes a numeric token from the source input.
     *
     * This function identifies a numeric sequence in the input source, converts it into an integer value, and
     * adds a token of type NUMBER with the corresponding value and a custom message. The scanning process
     * halts when a non-digit character or end of file is encountered.
     */
    void scan_number() {
        const auto col = column;
        char ch = prev();
        int value = 0;

        const auto first_pos{pos};

        do {
            value *= 10;
            value += ch - '0';

            ch = advance();
        } while (ch != EOFILE && isdigit(ch));

        if (pos > first_pos)
            --pos;

        if (ch == '\n')
            --line;
        addTokenWithCustomMessage(TokenType::NUMBER, std::to_string(value),
                                  "NUMBER [ " + std::to_string(value) + " ] found at (" + std::to_string(line) + ':' +
                                  std::to_string(col) + ")", line);
    }

    void log(LogLevel level, const std::string &message) {
        Logger::log(level, "Lexer", message);
        if (level == LogLevel::ERROR)
            ++error_count;
    }

    void addToken(TokenType type) {
        addToken(type, token_names[static_cast<size_t>(type)]);
    }

    void addToken(TokenType type, const std::string &value) {
        addTokenWithCustomMessage(type, value,
                                  token_type_names[static_cast<size_t>(type)] + " [ " + value + " ] found at (" +
                                  std::to_string(line) + ':' + std::to_string(column) + ")", line);
    }

    void addTokenWithCustomMessage(TokenType type, const std::string &value, const std::string &message, size_t line) {
        tokens.push_back({type, value, line});
        log(LogLevel::DEBUG, message);
    }

    /**
     * @brief Scans the source to identify and classify the next token.
     *
     * This function processes characters from the source, identifying tokens based on their type and context.
     * It skips over whitespace and delegates specific handling to other methods such as `scan_comment`,
     * `scan_string`, `scan_keyword`, and `scan_number`. The function emits identified tokens using
     * `addToken` or logs errors for unrecognized tokens.
     *
     * The function categorizes tokens such as brackets, operators, keywords, literals, or error conditions
     * based on the character encountered. Some tokens require additional handling for multi-character
     * sequences, such as comparison operators or comments.
     */
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
                    log(LogLevel::WARNING, "Expected '='");

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

    bool isEOF() const {
        return pos >= size;
    }

    /**
     * @brief Scans the source code, identifying tokens and logging the results.
     *
     * This function processes the source code in sequential programs, invoking token scanning mechanisms
     * and managing error reporting. It handles multiple programs, ensuring proper program termination
     * tokens are present at the end of each program. Scanning continues until the end of the source is reached.
     *
     * The function tracks errors encountered during lexing and logs messages accordingly. If no errors
     * are encountered during a program's scan, it logs a successful completion message. Additionally,
     * it detects and reports missing terminating tokens ('$') for the final program in the sequence.
     */
    std::optional<std::vector<Token>> scan() {
        error_count = 0;
        tokens.clear();

        do {
            scan_token();
        } while (!isEOF() && prev() != EOP);

        if (error_count > 0) {
            log(LogLevel::ERROR,
                "Lex failed with " + std::to_string(error_count) + " error(s)\n");
            return std::nullopt;
        }

        if (!tokens.empty() && tokens.back().type != TokenType::EOP) {
            log(LogLevel::ERROR,
                "Final program missing terminating '$'. Add '$' at the end of the program to mark its termination");
        }

        log(LogLevel::INFO, "Lex completed with 0 errors\n");
        return tokens;
    }
};
