#pragma once

#include <optional>
#include <vector>
#include "Lexer.h"
#include "Log.h"
#include "Token.h"


struct Node {
    Token token;
    std::string value;
    std::vector<Node> children;

    Node(Token token) : token{token} {
    }

    Node(const std::string &value) : value{value} {
    }
};

class CST {
    Node root;

    CST(const Token &token) : root{token} {
    }

    void add_child(const Node &child) {
        root.children.push_back(child);
    }

    void print_tree(int level = 0) {
        for (int i = 0; i < level; ++i) {
            std::cout << "  ";
        }
        std::cout << root.token.value << std::endl;
        for (const auto &child: root.children) {
            // child.print_tree(level + 1);
        }
    }

public:
    static CST create(const Token &token) {
        return {token};
    }

    void print() {
        print_tree();
    }
};

class Parser {
    std::vector<Token> tokens;
    size_t error_count{0};

    std::vector<Token>::iterator current_token;

    /**
     * Advances the current token iterator to the next token in the `tokens` vector.
     *
     * If the `current_token` iterator has not reached the end of the `tokens` vector,
     * this function increments the iterator to point to the next token. Otherwise,
     * the iterator remains unchanged.
     */
    void advance() {
        if (current_token != tokens.end())
            ++current_token;
    }

    /**
     * Parses the program by initializing the parsing process.
     *
     * This function begins at the top-level of the program structure, invoking
     * `parse_block` to handle the program's main block and ensures that the
     * program concludes with an End of Program (EOP) token.
     *
     * Logs the start and execution of the parsing process, and calls helper
     * functions to parse specific parts of the program structure.
     */
    void parse_program() {
        log(LogLevel::INFO, "parseProgram()");
        parse_block();
        match(TokenType::EOP);
    }

    /**
     * Parses a block of code enclosed by block delimiters.
     *
     * This function begins by matching the opening block token. It then
     * parses the list of statements contained within the block. Finally,
     * it matches the closing block token to ensure proper block termination.
     *
     * Logs the parsing process for debugging or informational purposes.
     *
     * Relies on helper functions `match` to check for specific tokens and
     * `parse_statement_list` to handle the statements within the block.
     */
    void parse_block() {
        log(LogLevel::INFO, "parseBlock()");

        match(TokenType::OPEN_BLOCK);
        parse_statement_list();
        match(TokenType::CLOSE_BLOCK);
    }

    /**
     * Parses a list of statements within a block or program structure.
     *
     * This function determines the type of each token in the statement list and
     * delegates further processing to the appropriate parsing functions based on
     * the token type. If the token indicates the start of a valid statement, the function
     * parses the statement and recursively processes the remaining statement list.
     *
     * Valid token types for statements include PRINT, ID, I_TYPE, WHILE, IF, and OPEN_BLOCK.
     * When encountering a CLOSE_BLOCK token, the parser stops processing the statement list.
     * If the token does not match any of the expected types, an error is logged and reported
     * using `report_token_mismatch`.
     *
     * Logs the parsing process for informational or debugging purposes.
     */
    void parse_statement_list() {
        log(LogLevel::INFO, "parseStatementList()");
        switch (current_token->type) {
            case TokenType::PRINT:
            case TokenType::ID:
            case TokenType::I_TYPE:
            case TokenType::WHILE:
            case TokenType::IF:
            case TokenType::OPEN_BLOCK:
                parse_statement();
                parse_statement_list();
            case TokenType::CLOSE_BLOCK:
                break;
            default:
                report_token_mismatch("statement list", *current_token);
        }
    }

    /**
     * Parses a single statement based on the current token type.
     *
     * This function examines the type of the current token and delegates the
     * parsing process to the appropriate helper function corresponding to the
     * token type. Valid token types include PRINT, ID, I_TYPE, WHILE, IF, and OPEN_BLOCK,
     * each of which triggers a corresponding parsing function.
     *
     * If the current token does not match any valid statement type, an error
     * is logged and reported using `report_token_mismatch`. Additionally, debug
     * information is logged to indicate that the statement parsing process has
     * begun.
     */
    void parse_statement() {
        log(LogLevel::INFO, "parseStatement()");
        switch (current_token->type) {
            case TokenType::PRINT:
                parse_print_statement();
                break;
            case TokenType::ID:
                parse_assignment_statement();
                break;
            case TokenType::I_TYPE:
                parse_var_declaration();
                break;
            case TokenType::WHILE:
                parse_while_statement();
                break;
            case TokenType::IF:
                parse_if_statement();
                break;
            case TokenType::OPEN_BLOCK:
                parse_block();
                break;
            default:
                report_token_mismatch("statement", *current_token);
        }
    }

    /**
     * Parses a print statement in the source code.
     */
    void parse_print_statement() {
        log(LogLevel::INFO, "parsePrintStatement()");
        match(TokenType::PRINT);
        match(TokenType::OPEN_PARENTHESIS);
        parse_expression();
        match(TokenType::CLOSE_PARENTHESIS);
    }

    /**
     * Parses an assignment statement in the input source code.
     *
     */
    void parse_assignment_statement() {
        log(LogLevel::INFO, "parseAssignmentStatement()");
        parse_id();
        match(TokenType::ASSIGN_OP);
        parse_expression();
    }

    /**
     * Parses a variable declaration in the input by processing the expected tokens.
     *
     */
    void parse_var_declaration() {
        log(LogLevel::INFO, "parseVarDeclaration()");
        match(TokenType::I_TYPE);
        parse_id();
    }

    /**
     * Parses a `while` statement in the source code.
     *
     */
    void parse_while_statement() {
        log(LogLevel::INFO, "parseWhileStatement()");
        match(TokenType::WHILE);
        parse_boolean_expression();
        parse_block();
    }

    /**
     * Parses an `if` statement in the source code.
     *
     */
    void parse_if_statement() {
        log(LogLevel::INFO, "parseIfStatement()");
        match(TokenType::IF);
        parse_boolean_expression();
        parse_block();
    }

    /**
     * Parses an expression based on the current token type and delegates further handling
     * to the appropriate parsing function.
     *
     */
    void parse_expression() {
        log(LogLevel::INFO, "parseExpression()");

        switch (current_token->type) {
            case TokenType::NUMBER:
                parse_int_expression();
                break;
            case TokenType::QUOTE:
                parse_string_expression();
                break;
            case TokenType::BOOL_VAL:
            case TokenType::OPEN_PARENTHESIS:
                parse_boolean_expression();
                break;
            case TokenType::ID:
                parse_id();
                break;
            default:
                report_token_mismatch("expression", *current_token);
        }
    }

    /**
     * Parses an integer expression from the input token stream.
     *
     * This method begins by matching a `NUMBER` token. If the next token is an
     * integer operator (`INT_OP`), it matches the operator token and then
     * calls `parse_expression()` to handle the subsequent part of the expression.
     *
     * The method logs its invocation using a message at the `INFO` log level.
     */
    void parse_int_expression() {
        log(LogLevel::INFO, "parseIntExpression()");
        match(TokenType::NUMBER);

        if (current_token->type == TokenType::INT_OP) {
            match(TokenType::INT_OP);
            parse_expression();
        }
    }

    /**
     * Parses a string expression enclosed in quotation marks.
     *
     */
    void parse_string_expression() {
        log(LogLevel::INFO, "parseStringExpression()");
        match(TokenType::QUOTE);
        if (current_token->type == TokenType::QUOTE) {
            match(TokenType::QUOTE);
        } else {
            parse_char_list();
            match(TokenType::QUOTE);
        }
    }

    /**
     * Parses a boolean expression based on the current token in the parsing process.
     *
     * This function handles boolean expressions by analyzing the type of the current token.
     * If the token is an opening block, it recursively parses a boolean operation enclosed
     * by the opening and closing blocks. If the token is a boolean value, it processes it
     * directly. For any unexpected token type, it reports a token mismatch error.
     */
    void parse_boolean_expression() {
        log(LogLevel::INFO, "parseBooleanExpression()");
        switch (current_token->type) {
            case TokenType::OPEN_PARENTHESIS:
                match(TokenType::OPEN_PARENTHESIS);
                parse_expression();
                parse_boolean_operation();
                parse_expression();
                match(TokenType::CLOSE_PARENTHESIS);
                break;
            case TokenType::BOOL_VAL:
                match(TokenType::BOOL_VAL);
                break;
            default:
                report_token_mismatch("boolean expression", *current_token);
        }
    }

    void parse_id() {
        log(LogLevel::INFO, "parseId()");
        match(TokenType::ID);
    }

    void parse_char_list() {
        log(LogLevel::INFO, "parseCharList()");
        match(TokenType::CHAR);
        while (current_token->type == TokenType::CHAR) {
            match(TokenType::CHAR);
        }
    }

    /**
     * Parses a boolean operation token in the current token stream.
     *
     * This function evaluates the type of the current token and processes
     * it accordingly. If the token corresponds to an equality or inequality
     * operation, it matches and consumes the token. If the token does not
     * match the expected boolean operation types, an error is reported.
     *
     * Logs the parsing process for debugging or tracing purposes.
     */
    void parse_boolean_operation() {
        log(LogLevel::INFO, "parseBooleanOperation()");
        switch (current_token->type) {
            case TokenType::EQUALITY_OP:
                match(TokenType::EQUALITY_OP);
                break;
            case TokenType::INEQUALITY_OP:
                match(TokenType::INEQUALITY_OP);
                break;
            default:
                report_token_mismatch("boolean operation", *current_token);
        }
    }

    /**
     * Matches the current token type with the expected token type.
     *
     * If the type of the current token matches the expected token type, this function
     * advances the token iterator to the next token in the sequence. If the token types
     * do not match, it reports a type mismatch error using `report_token_mismatch`.
     *
     * @param token The expected token type to match against the current token type.
     */
    void match(TokenType token) {
        if (current_token->type == token) {
            advance();
        } else {
            // Report error for type mismatch
            report_token_mismatch(token_type_names[static_cast<size_t>(token)], *current_token);
        }
    }

    void log(LogLevel level, const std::string &message) {
        Logger::log(level, "PARSER", message);

        if (level == LogLevel::ERROR)
            ++error_count;
    }

    /**
     * Reports a token type mismatch by logging an error message.
     *
     * This function is called when the current token type does not match
     * the expected token type during the parsing process. It logs an error
     * message indicating the expected token type, the actual token type,
     * the token's value, and the line number where the error occurred.
     *
     * @param expected The expected token type as a string.
     * @param actual The actual Token object containing the mismatched type,
     *               value, and line number.
     */
    void report_token_mismatch(const std::string &expected, Token actual) {
        log(LogLevel::ERROR,
            "ERROR: Expected [" + expected + "] got [" + token_type_names[
                static_cast<size_t>(actual.type)] + "] with value " + actual.value + " on line " + std::to_string(
                actual.line));
    }

public:
    explicit Parser(const std::vector<Token> &items) : tokens{items}, current_token{tokens.begin()} {
    }

    /**
     * Parses the input tokens to construct a concrete syntax tree (CST).
     *
     * This function attempts to parse a program from the provided tokens.
     * It initializes the error count, logs the parsing process, and invokes
     * the `parse_program()` method to process the tokens. If parsing succeeds
     * without errors, it returns a constructed CST. If errors occur during
     * parsing, logs the failure and returns an empty optional.
     *
     * @return A constructed concrete syntax tree (CST) wrapped in std::optional,
     *         or std::nullopt if parsing fails due to errors.
     */
    std::optional<CST> parse() {
        error_count = 0;
        auto cst = CST::create(tokens.front());

        log(LogLevel::INFO, "parse()");
        parse_program();

        if (error_count > 0) {
            log(LogLevel::ERROR, "Parse failed with " + std::to_string(error_count) + " error(s).");
            return std::nullopt;
        }

        log(LogLevel::INFO, "Parse completed successfully");
        return cst;
    }
};
