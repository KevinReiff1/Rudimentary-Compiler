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

    void advance() {
        if (current_token != tokens.end())
            ++current_token;
    }

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
            case TokenType::CLOSE_BLOCK:
                break;
            default:
                report_token_mismatch("statement list", *current_token);
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
                report_token_mismatch("statement", *current_token);
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
                report_token_mismatch("expression", *current_token);
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
        log(LogLevel::INFO, "parseBooleanExpression()");
        switch (current_token->type) {
            case TokenType::OPEN_BLOCK:
                match(TokenType::OPEN_BLOCK);
                parse_boolean_operation();
                match(TokenType::CLOSE_BLOCK);
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
                report_token_mismatch("statement", *current_token);
        }
    }

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

    void report_token_mismatch(const std::string &expected, Token actual) {
        log(LogLevel::ERROR,
            "ERROR: Expected [" + expected + "] got [" + token_type_names[
                static_cast<size_t>(actual.type)] + "] with value " + actual.value + " on line " + std::to_string(
                actual.line));
    }

public:
    explicit Parser(const std::vector<Token> &items) : tokens{items}, current_token{tokens.begin()} {
    }

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
