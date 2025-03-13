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
    }

    void parse_print_statement() {
    }

    void parse_assignment_statement() {
    }

    void parse_var_declaration() {
    }

    void parse_while_statement() {
    }

    void parse_if_statement() {
    }

    void parse_expression() {
    }

    void parse_int_expression() {
    }

    void parse_string_expression() {
    }

    void parse_boolean_expression() {
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
