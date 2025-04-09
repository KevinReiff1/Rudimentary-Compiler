#pragma once
#include <algorithm>
#include <utility>

#include "Parser.h"
#include "SymbolTable.h"

class AST {
    Node root{NodeType::BLOCK};

    /**
     * Recursively prints the structure of a tree represented by `Node` objects to the standard output.
     *
     * @param node The root node of the tree or subtree to be printed.
     * @param level The current recursion depth, used to determine indentation. Defaults to 0.
     */
    static void print_tree(const Node &node, int level = 0) {
        // Omitting empty nodes for readable output
        if (node.get_children().empty() && node.get_value().empty())
            return;

        for (int i = 0; i < level; ++i) {
            std::cout << "-";
        }
        if (node.get_value().empty())
            std::cout << "<" << node_names[static_cast<size_t>(node.get_node_type())] << ">" << std::endl;
        else
            std::cout << '[' << node.get_value() << ']' << std::endl;
        for (auto &child: node.get_children()) {
            print_tree(child, level + 1);
        }
    }

public:
    AST() = default;

    Node &get_root() {
        return root;
    }

    void print() const {
        print_tree(root, 0);
    }
};

class Analyzer {
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
     * Parses the overall structure of a program represented by an abstract syntax tree (AST).
     *
     * @param ast The abstract syntax tree to parse. The function begins parsing from the root node
     *            of the tree and validates the end of the program.
     */
    void parse_program(AST &ast) {
        parse_block(ast.get_root());
        check(TokenType::EOP);
    }


    /**
     * Parses a block structure from the input and appends it as a child node
     * to the provided parent node. The function ensures proper handling of
     * block delimiters and recursively processes the contained statements.
     *
     * @param parent The parent node to which the parsed block node will be added.
     */
    void parse_block(Node &parent) {
        auto &node = parent.addChild(NodeType::BLOCK);
        check(TokenType::OPEN_BLOCK);
        parse_statement_list(node);
        check(TokenType::CLOSE_BLOCK);
    }


    /**
     * Parses a list of statements starting from the current token and attaches them to the provided parent node.
     * Handles different types of statements and recurses to process subsequent statements in the list.
     *
     * @param parent The node to which the parsed statements will be attached.
     */
    void parse_statement_list(Node &parent) {
        switch (current_token->type) {
            case TokenType::PRINT:
            case TokenType::ID:
            case TokenType::I_TYPE:
            case TokenType::WHILE:
            case TokenType::IF:
            case TokenType::OPEN_BLOCK:
                parse_statement(parent);
                parse_statement_list(parent);
            case TokenType::CLOSE_BLOCK:
                break;
            default:
                report_token_mismatch("statement list", *current_token);
        }
    }


    /**
     * Parses a statement from the current token and adds the resulting structure
     * to the given parent node. The function delegates specific statement parsing
     * based on the type of the current token.
     *
     * @param parent The parent node to which the parsed statement will be attached.
     */
    void parse_statement(Node &parent) {
        switch (current_token->type) {
            case TokenType::PRINT:
                parse_print_statement(parent);
                break;
            case TokenType::ID:
                parse_assignment_statement(parent);
                break;
            case TokenType::I_TYPE:
                parse_var_declaration(parent);
                break;
            case TokenType::WHILE:
                parse_while_statement(parent);
                break;
            case TokenType::IF:
                parse_if_statement(parent);
                break;
            case TokenType::OPEN_BLOCK:
                parse_block(parent);
                break;
            default:
                report_token_mismatch("statement", *current_token);
        }
    }

    /**
     * Parses a print statement in the source code.
     */
    void parse_print_statement(Node &parent) {
        auto &node = parent.addChild(NodeType::PRINT_STATEMENT);
        match(node, TokenType::PRINT);
        check(TokenType::OPEN_PARENTHESIS);
        parse_expression(node);
        check(TokenType::CLOSE_PARENTHESIS);
    }

    /**
     * Parses an assignment statement in the input source code.
     *
     */
    void parse_assignment_statement(Node &parent) {
        auto &node = parent.addChild(NodeType::ASSIGNMENT_STATEMENT);
        parse_id(node);
        check(TokenType::ASSIGN_OP);
        parse_expression(node);
    }

    /**
     * Parses a variable declaration in the input by processing the expected tokens.
     *
     */
    void parse_var_declaration(Node &parent) {
        auto &node = parent.addChild(NodeType::VARIABLE_DECLARATION);
        match(node, TokenType::I_TYPE);
        parse_id(node);
    }

    /**
     * Parses a `while` statement in the source code.
     *
     */
    void parse_while_statement(Node &parent) {
        auto &node = parent.addChild(NodeType::WHILE_STATEMENT);
        match(node, TokenType::WHILE);
        parse_boolean_expression(node);
        parse_block(node);
    }

    /**
     * Parses an `if` statement in the source code.
     *
     */
    void parse_if_statement(Node &parent) {
        auto &node = parent.addChild(NodeType::IF_STATEMENT);
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
        auto &node = parent.addChild(NodeType::EXPRESSION);

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
        auto &node = parent.addChild(NodeType::INT_EXPRESSION);
        match_and_add(node, TokenType::NUMBER);

        if (current_token->type == TokenType::INT_OP) {
            match_and_add(node, TokenType::INT_OP);
            parse_expression(node);
        }
    }

    /**
     * Parses a string expression enclosed in quotation marks.
     *
     */
    void parse_string_expression(Node &parent) {
        auto &node = parent.addChild(NodeType::STRING_EXPRESSION);

        check(TokenType::QUOTE);
        if (current_token->type == TokenType::QUOTE) {
            check(TokenType::QUOTE);
        } else {
            parse_char_list(node);
            check(TokenType::QUOTE);
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
        auto &node = parent.addChild(NodeType::BOOLEAN_EXPRESSION);
        switch (current_token->type) {
            case TokenType::OPEN_PARENTHESIS:
                check(TokenType::OPEN_PARENTHESIS);
                parse_expression(node);
                parse_boolean_operation(node);
                parse_expression(node);
                check(TokenType::CLOSE_PARENTHESIS);
                break;
            case TokenType::BOOL_VAL:
                match_and_add(node, TokenType::BOOL_VAL);
                break;
            default:
                report_token_mismatch("boolean expression", *current_token);
        }
    }

    void parse_id(Node &parent) {
        auto &node = parent.addChild(NodeType::ID);
        match_and_add(node, TokenType::ID);
    }

    void parse_char_list(Node &parent) {
        std::string chars;
        while (current_token->type == TokenType::CHAR) {
            chars += current_token->value;
            advance();
        }

        auto &node = parent.addChild(NodeType::CHAR_LIST, chars);
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
        auto &node = parent.addChild(NodeType::BOOLEAN_OPERATION);

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
     * Matches the current token type with the expected type and adds it as a child node under the specified parent.
     * If the token type does not match, an error is reported for the mismatch.
     *
     * @param parent The parent node to which a new child node will be added if the token matches.
     * @param token The expected token type to be matched with the current token.
     */
    void match_and_add(Node &parent, TokenType token) {
        if (current_token->type == token) {
            parent.addChild(NodeType::UNKNOWN, *current_token);
            advance();
        } else {
            // Report error for type mismatch
            report_token_mismatch(token_type_names[static_cast<size_t>(token)], *current_token);
        }
    }

    void check(TokenType token) {
        if (current_token->type == token)
            advance();
    }

    void match_and_add() {
    }

    void log(LogLevel level, const std::string &message) {
        Logger::log(level, "SEMANTIC ANALYZER", message);

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
    void report_token_mismatch(const std::string &expected, const Token &actual) {
        log(LogLevel::ERROR,
            "ERROR: Expected [" + expected + "] got [" + token_type_names[
                static_cast<size_t>(actual.type)] + "] with value " + actual.value + " on line " + std::to_string(
                actual.line));
    }

public:
    explicit Anayzer(const std::vector<Token> &items) : tokens{items}, current_token{tokens.begin()} {
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
    std::optional<AST> parse() {
        error_count = 0;
        AST ast;

        log(LogLevel::INFO, "parse()");
        parse_program(ast);

        if (error_count > 0) {
            log(LogLevel::ERROR, "Parse failed with " + std::to_string(error_count) + " error(s).");
            return std::nullopt;
        }

        log(LogLevel::INFO, "Parse completed successfully");
        return ast;
    }
};

class SemanticAnalyzer {
    CST cst;
    SymbolTable symbol_table;

public:
    explicit SemanticAnalyzer(CST cst_) : cst(std::move(cst_)) {
    }

    auto analyze() {
        ASTBuilder ast_builder(cst);
        return ast_builder.build();;
    }
};
