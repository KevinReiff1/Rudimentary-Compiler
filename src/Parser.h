#pragma once

#include <optional>
#include <vector>
#include "Lexer.h"
#include "Log.h"
#include "Token.h"


struct Node {
private:
    std::string name;
    std::string value;
    std::vector<Node> children;

public:
    [[nodiscard]] std::string get_value() const {
        return value;
    }

    [[nodiscard]] std::string get_name() const {
        return name;
    }

    Node &addChild(const std::string &name, const std::string &value) {
        children.emplace_back(name, value);
        return children.back();
    }

    [[nodiscard]] const std::vector<Node> &get_children() const {
        return children;
    }

    //   Node() = default;

    explicit Node(std::string name_, std::string value_) : name{std::move(name_)}, value{std::move(value_)} {
    }
};

class CST {
    Node root{"Program", ""};

    void print_tree(const Node &node, int level = 0) const {
        for (int i = 0; i < level; ++i) {
            std::cout << "-";
        }
        if (node.get_value().empty())
            std::cout << "<" << node.get_name() << ">" << std::endl;
        else
            std::cout << '[' << node.get_value() << ']' << std::endl;
        for (auto &child: node.get_children()) {
            print_tree(child, level + 1);
        }
    }

public:
    CST() = default;

    Node &get_root() {
        return root;
    }

    void print() const {
        print_tree(root, 0);
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
    void parse_program(CST &cst) {
        log(LogLevel::INFO, "parseProgram()");
        parse_block(cst.get_root());
        match(cst.get_root(), TokenType::EOP);
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
    void parse_block(Node &parent) {
        log(LogLevel::INFO, "parseBlock()");

        auto &node = parent.addChild("Block", "");
        match(node, TokenType::OPEN_BLOCK);
        parse_statement_list(node);
        match(node, TokenType::CLOSE_BLOCK);
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
    void parse_statement_list(Node &parent) {
        log(LogLevel::INFO, "parseStatementList()");
        auto &node = parent.addChild("Statement List", "");
        switch (current_token->type) {
            case TokenType::PRINT:
            case TokenType::ID:
            case TokenType::I_TYPE:
            case TokenType::WHILE:
            case TokenType::IF:
            case TokenType::OPEN_BLOCK:
                parse_statement(node);
                parse_statement_list(node);
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
    void parse_statement(Node &parent) {
        log(LogLevel::INFO, "parseStatement()");
        auto &node = parent.addChild("Statement", "");
        switch (current_token->type) {
            case TokenType::PRINT:
                parse_print_statement(node);
                break;
            case TokenType::ID:
                parse_assignment_statement(node);
                break;
            case TokenType::I_TYPE:
                parse_var_declaration(node);
                break;
            case TokenType::WHILE:
                parse_while_statement(node);
                break;
            case TokenType::IF:
                parse_if_statement(node);
                break;
            case TokenType::OPEN_BLOCK:
                parse_block(node);
                break;
            default:
                report_token_mismatch("statement", *current_token);
        }
    }

    /**
     * Parses a print statement in the source code.
     */
    void parse_print_statement(Node &parent) {
        log(LogLevel::INFO, "parsePrintStatement()");
        auto &node = parent.addChild("Print Statement", "");
        match(node, TokenType::PRINT);
        match(node, TokenType::OPEN_PARENTHESIS);
        parse_expression(node);
        match(node, TokenType::CLOSE_PARENTHESIS);
    }

    /**
     * Parses an assignment statement in the input source code.
     *
     */
    void parse_assignment_statement(Node &parent) {
        log(LogLevel::INFO, "parseAssignmentStatement()");
        auto &node = parent.addChild("Assignment Statement", "");
        parse_id(node);
        match(node, TokenType::ASSIGN_OP);
        parse_expression(node);
    }

    /**
     * Parses a variable declaration in the input by processing the expected tokens.
     *
     */
    void parse_var_declaration(Node &parent) {
        log(LogLevel::INFO, "parseVarDeclaration()");
        auto &node = parent.addChild("Variable Declaration", "");
        match(node, TokenType::I_TYPE);
        parse_id(node);
    }

    /**
     * Parses a `while` statement in the source code.
     *
     */
    void parse_while_statement(Node &parent) {
        log(LogLevel::INFO, "parseWhileStatement()");
        auto &node = parent.addChild("While Statement", "");
        match(node, TokenType::WHILE);
        parse_boolean_expression(node);
        parse_block(node);
    }

    /**
     * Parses an `if` statement in the source code.
     *
     */
    void parse_if_statement(Node &parent) {
        log(LogLevel::INFO, "parseIfStatement()");
        auto &node = parent.addChild("If Statement", "");
        match(node, TokenType::IF);
        parse_boolean_expression(node);
        parse_block(node);
    }

    /**
     * Parses an expression based on the current token type and delegates further handling
     * to the appropriate parsing function.
     *
     */
    void parse_expression(Node &parent) {
        log(LogLevel::INFO, "parseExpression()");
        auto &node = parent.addChild("Expression", "");

        switch (current_token->type) {
            case TokenType::NUMBER:
                parse_int_expression(node);
                break;
            case TokenType::QUOTE:
                parse_string_expression(node);
                break;
            case TokenType::BOOL_VAL:
            case TokenType::OPEN_PARENTHESIS:
                parse_boolean_expression(node);
                break;
            case TokenType::ID:
                parse_id(node);
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
    void parse_int_expression(Node &parent) {
        log(LogLevel::INFO, "parseIntExpression()");
        auto &node = parent.addChild("Int Expression", "");
        match(node, TokenType::NUMBER);

        if (current_token->type == TokenType::INT_OP) {
            match(node, TokenType::INT_OP);
            parse_expression(node);
        }
    }

    /**
     * Parses a string expression enclosed in quotation marks.
     *
     */
    void parse_string_expression(Node &parent) {
        log(LogLevel::INFO, "parseStringExpression()");
        auto &node = parent.addChild("String Expression", "");

        match(node, TokenType::QUOTE);
        if (current_token->type == TokenType::QUOTE) {
            match(node, TokenType::QUOTE);
        } else {
            parse_char_list(node);
            match(node, TokenType::QUOTE);
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
    void parse_boolean_expression(Node &parent) {
        log(LogLevel::INFO, "parseBooleanExpression()");
        auto &node = parent.addChild("Boolean Expression", "");
        switch (current_token->type) {
            case TokenType::OPEN_PARENTHESIS:
                match(node, TokenType::OPEN_PARENTHESIS);
                parse_expression(node);
                parse_boolean_operation(node);
                parse_expression(node);
                match(node, TokenType::CLOSE_PARENTHESIS);
                break;
            case TokenType::BOOL_VAL:
                match(node, TokenType::BOOL_VAL);
                break;
            default:
                report_token_mismatch("boolean expression", *current_token);
        }
    }

    void parse_id(Node &parent) {
        log(LogLevel::INFO, "parseId()");
        auto &node = parent.addChild("Id", "");
        match(node, TokenType::ID);
    }

    void parse_char_list(Node &parent) {
        log(LogLevel::INFO, "parseCharList()");
        auto &node = parent.addChild("Char List", "");
        match(node, TokenType::CHAR);
        while (current_token->type == TokenType::CHAR) {
            match(node, TokenType::CHAR);
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
    void parse_boolean_operation(Node &parent) {
        log(LogLevel::INFO, "parseBooleanOperation()");
        auto &node = parent.addChild("Boolean Operation", "");

        switch (current_token->type) {
            case TokenType::EQUALITY_OP:
                match(node, TokenType::EQUALITY_OP);
                break;
            case TokenType::INEQUALITY_OP:
                match(node, TokenType::INEQUALITY_OP);
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
    void match(Node &parent, TokenType token) {
        if (current_token->type == token) {
            parent.addChild("", current_token->value);
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
        CST cst;

        log(LogLevel::INFO, "parse()");
        parse_program(cst);

        if (error_count > 0) {
            log(LogLevel::ERROR, "Parse failed with " + std::to_string(error_count) + " error(s).");
            return std::nullopt;
        }

        log(LogLevel::INFO, "Parse completed successfully");
        return cst;
    }
};
