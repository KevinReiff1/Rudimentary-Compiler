#pragma once

#include <optional>
#include <vector>
#include "Lexer.h"
#include "Log.h"
#include "Token.h"


struct Node {
    Token token;
    std::vector<Node> children;

    Node(Token token) : token{token} {
    }
};

class CST {
    Node root;

    CST(const Token &token) : root{token} {
    }

public:
    static CST create(const Token &token) {
        return {token};
    }
};

class Parser {
    std::vector<Token> tokens;
    size_t error_count{0};

    std::vector<Token>::iterator current_token;

    void parse_program() {
        log(LogLevel::INFO, "parseProgram()");
        parse_block();
        match(TokenType::EOP);
    }

    void parse_block() {
        log(LogLevel::INFO, "parseBlock()");

        match(TokenType::OPEN_BLOCK);
        parse_statement_list();
        match(TokenType::CLOSE_BLOCK);
    }

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
            default: ;
        }
    }

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
                log(LogLevel::ERROR,
                    "Expected statement, got unexpected token [" + token_type_names[static_cast<size_t>(current_token->
                        type)] + " with value '" + current_token->value + "' on line " + std::to_string(
                        current_token->line));
        }
    }

    void parse_print_statement() {
        log(LogLevel::INFO, "parsePrintStatement()");
        match(TokenType::PRINT);
        match(TokenType::OPEN_PARENTHESIS);
        parse_expression();
        match(TokenType::CLOSE_PARENTHESIS);
    }

    void parse_assignment_statement() {
        log(LogLevel::INFO, "parseAssignmentStatement()");
        parse_id();
        match(TokenType::ASSIGN_OP);
        parse_expression();
    }

    void parse_var_declaration() {
        log(LogLevel::INFO, "parseVarDeclaration()");
        match(TokenType::I_TYPE);
        parse_id();
    }

    void parse_while_statement() {
        log(LogLevel::INFO, "parseWhileStatement()");
        match(TokenType::WHILE);
        parse_boolean_expression();
        parse_block();
    }

    void parse_if_statement() {
        log(LogLevel::INFO, "parseIfStatement()");
        match(TokenType::IF);
        parse_boolean_expression();
        parse_block();
    }

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
                parse_boolean_expression();
                break;
            case TokenType::ID:
                parse_id();
                break;
            default:
                log(LogLevel::ERROR,
                    "Expected statement, got unexpected token [" + token_type_names[static_cast<size_t>(current_token->
                        type)] + " with value '" + current_token->value + "' on line " + std::to_string(
                        current_token->line));
        }
    }

    void parse_int_expression() {
        log(LogLevel::INFO, "parseIntExpression()");
        match(TokenType::NUMBER);

        if (current_token->type == TokenType::INT_OP) {
            match(TokenType::INT_OP);
            parse_expression();
        }
    }

    void parse_string_expression() {
        log(LogLevel::INFO, "parseStringExpression()");
        match(TokenType::QUOTE);
        parse_char_list();
        match(TokenType::QUOTE);
    }

    void parse_boolean_expression() {
        log(LogLevel::INFO, "parseBooleanExpression()");switch (current_token->type) {
            case TokenType::OPEN_BLOCK:
                match(TokenType::OPEN_BLOCK);
            parse_expression();
                match(TokenType::CLOSE_BLOCK);
                break;
            case TokenType::BOOL_VAL:
                match(TokenType::BOOL_VAL);
                break;
            default:
                log(LogLevel::ERROR,
                    "Expected statement, got unexpected token [" + token_type_names[static_cast<size_t>(current_token->
                        type)] + " with value '" + current_token->value + "' on line " + std::to_string(
                        current_token->line));
        }
    }

    void match(TokenType token) {
        if (current_token->type == token) {
        } else {
            // Report error for type mismatch
            log(LogLevel::ERROR,
                "ERROR: Expected [" + token_type_names[static_cast<size_t>(token)] + "] got [" + token_type_names[
                    static_cast<size_t>(current_token->type)] + "] with value " + current_token->value + " on line " +
                std::to_string(current_token->line));
        }
    }

    void log(LogLevel level, const std::string &message) {
        Logger::log(level, "PARSER", message);

        if (level == LogLevel::ERROR)
            ++error_count;
    }

public:
    explicit Parser(const std::vector<Token> &items) : tokens{items}, current_token{tokens.begin()} {
    }

    std::optional<CST> parse() {
        error_count = 0;
        auto cst = CST::create(tokens.front());

        log(LogLevel::INFO, "parse()");
        parse_program();
        return {};
    }
};
