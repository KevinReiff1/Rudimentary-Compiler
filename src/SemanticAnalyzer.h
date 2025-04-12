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
            std::cout << "< " << node_names[static_cast<size_t>(node.get_node_type())] << " >" << std::endl;
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
        if (!root.get_children().empty())
            print_tree(root.get_children().front(), 0);
    }
};

class SemanticAnalyzer {
    std::vector<Token> tokens;
    size_t error_count{0};
    SymbolTable symbol_table;

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
        check(TokenType::PRINT);
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
        match_and_add(node, TokenType::I_TYPE);
        parse_id(node);
    }

    /**
     * Parses a `while` statement in the source code.
     *
     */
    void parse_while_statement(Node &parent) {
        auto &node = parent.addChild(NodeType::WHILE_STATEMENT);
        check(TokenType::WHILE);
        parse_boolean_expression(node);
        parse_block(node);
    }

    /**
     * Parses an `if` statement in the source code.
     *
     */
    void parse_if_statement(Node &parent) {
        auto &node = parent.addChild(NodeType::IF_STATEMENT);
        check(TokenType::IF);
        parse_boolean_expression(node);
        parse_block(node);
    }

    /**
     * Parses an expression based on the current token type and delegates further handling
     * to the appropriate parsing function.
     *
     */
    void parse_expression(Node &parent) {
        switch (current_token->type) {
            case TokenType::NUMBER:
                parse_int_expression(parent);
                break;
            case TokenType::QUOTE:
                parse_string_expression(parent);
                break;
            case TokenType::BOOL_VAL:
            case TokenType::OPEN_PARENTHESIS:
                parse_boolean_expression(parent);
                break;
            case TokenType::ID:
                parse_id(parent);
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
        match_and_add(parent, TokenType::NUMBER);

        if (current_token->type == TokenType::INT_OP) {
            match_and_add(parent, TokenType::INT_OP);
            parse_expression(parent);
        }
    }

    /**
     * Parses a string expression enclosed in quotation marks.
     *
     */
    void parse_string_expression(Node &parent) {
        check(TokenType::QUOTE);
        if (current_token->type == TokenType::QUOTE) {
            check(TokenType::QUOTE);
        } else {
            parse_char_list(parent);
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
        switch (current_token->type) {
            case TokenType::OPEN_PARENTHESIS:
                check(TokenType::OPEN_PARENTHESIS);
                parse_expression(parent);
                parse_boolean_operation(parent);
                parse_expression(parent);
                check(TokenType::CLOSE_PARENTHESIS);
                break;
            case TokenType::BOOL_VAL:
                match_and_add(parent, TokenType::BOOL_VAL);
                break;
            default:
                report_token_mismatch("boolean expression", *current_token);
        }
    }

    void parse_id(Node &parent) {
        match_and_add(parent, TokenType::ID);
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
                match_and_add(node, TokenType::EQUALITY_OP);
                break;
            case TokenType::INEQUALITY_OP:
                match_and_add(node, TokenType::INEQUALITY_OP);
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

    auto evaluate_expression(const Node &node) {
        /*if (node.get_value() == "int")
            return DataType::Int;
        else if (node.get_value() == "string")
            return DataType::String;
        else if (node.get_value() == "boolean")
            return DataType::Boolean;
        else return DataType::Unknown;*/

        switch (node.get_node_type()) {
            case NodeType::INT_EXPRESSION:
                return DataType::Int;
            case NodeType::CHAR_LIST:
                return DataType::String;
            case NodeType::BOOLEAN_EXPRESSION:
                return DataType::Boolean;
            case NodeType::ID: {
                const auto symbol = symbol_table.findSymbol(node.get_value());
                if (!symbol) {
                    ++error_count;
                    return DataType::Unknown;
                }
            }
            default:
                std::cout << "[Error] Invalid expression at line "
                        << node.get_line() << std::endl;
                ++error_count;
                return DataType::Unknown;
        }
    }

    void analyze_node(const Node &node) {
        const auto node_type = node.get_node_type();

        switch (node_type) {
            case NodeType::BLOCK:
                symbol_table.enterScope();
                for (const auto &child: node.get_children()) {
                    analyze_node(child);
                }
                symbol_table.exitScope();
                break;

            case NodeType::VARIABLE_DECLARATION: {
                const auto id_node = node.get_children().front();
                const auto name = node.get_children()[1];
                if (!symbol_table.addSymbol(name.get_value(), id_node.get_value(), id_node.get_line())) {
                    ++error_count;
                }
                break;
            }

            case NodeType::ASSIGNMENT_STATEMENT: {
                const auto id_node = node.get_children()[0];
                const auto expr_node = node.get_children()[1];
                const auto symbol = symbol_table.findSymbol(id_node.get_value());
                if (!symbol) {
                    std::cout << "[Error] Undeclared variable '" << id_node.get_value()
                            << "' used at line " << id_node.get_line() << std::endl;
                    ++error_count;
                    break;
                }
                DataType expr_type = evaluate_expression(expr_node);
                /*if (expr_type != symbol->type) {
                    std::cout << "[Error] Type mismatch in assignment to '"
                            << id_node.get_value() << "' at line "
                            << id_node.get_line() << ": expected "
                            << static_cast<int>(symbol->type) << ", got "
                            << static_cast<int>(expr_type) << std::endl;
                    ++error_count;
                } else {
                    symbol_table.markInitialized(id_node.get_value());
                }*/
                break;
            }

            case NodeType::PRINT_STATEMENT:
                evaluate_expression(node.get_children().front()); // Checks if ID is valid
                break;

            case NodeType::IF_STATEMENT:
            case NodeType::WHILE_STATEMENT: {
                auto bool_expr = node.get_children().front();
                DataType expr_type = evaluate_expression(bool_expr);
                if (expr_type != DataType::Boolean) {
                    std::cout << "[Error] Non-boolean expression in "
                            << (node.get_node_type() == NodeType::IF_STATEMENT ? "if" : "while")
                            << " at line " << node.get_line() << std::endl;
                    ++error_count;
                }
                analyze_node(node.get_children()[1]); // Analyze block
                break;
            }

            default:
                for (const auto &child: node.get_children()) {
                    analyze_node(child);
                }
                break;
        }
    }

public:
    explicit SemanticAnalyzer(const std::vector<Token> &items) : tokens{items}, current_token{tokens.begin()} {
    }

    /**
     * Performs semantic analysis on the program and returns the resulting Abstract Syntax Tree (AST) if successful.
     * Logs the progress and errors encountered during the analysis process.
     *
     * @return An optional containing the constructed AST if analysis is successful, or `std::nullopt` if errors are encountered.
     */
    std::optional<AST> analyze() {
        error_count = 0;
        AST ast;


        parse_program(ast);

        ast.print();


        analyze_node(ast.get_root().get_children().front());

        symbol_table.display();


        log(LogLevel::INFO, "Parse completed successfully");
        return ast;
    }
};
