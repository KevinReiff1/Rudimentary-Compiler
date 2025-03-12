#pragma once

#include <optional>
#include <vector>
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


    void parse_program() {
        parse_block();
    }

    void parse_block() {
        parse_statement_list();
    }

    void parse_statement_list() {
        parse_statement();
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
    explicit Parser(const std::vector<Token> &items) : tokens{items} {
    }

    std::optional<CST> parse() {

        auto cst = CST::create(tokens.front());

        parse_program();
        return {};
    }
};
