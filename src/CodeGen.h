#pragma once
#include "Parser.h"
#include "SemanticAnalyzer.h"

class AST;

class CodeGen {
public:
    CodeGen() {
    }

    virtual ~CodeGen() {
    }

    void generate(Node &node) {
        visit(node);
        //ast.accept(this);
    }

    void visit(const Node &node) {
        switch (node.get_node_type()) {
            case NodeType::PROGRAM:
                break;
            case NodeType::BLOCK:
                for (auto &child: node.get_children()) {
                    visit(child);
                }
                break;
            case NodeType::VARIABLE_DECLARATION:
                break;
            case NodeType::ASSIGNMENT_STATEMENT:
                break;
            case NodeType::STATEMENT_LIST:
                break;
            case NodeType::STATEMENT:
                break;
            case NodeType::IF_STATEMENT:
                break;
            case NodeType::WHILE_STATEMENT:
                break;
            case NodeType::PRINT_STATEMENT:
                break;
            case NodeType::EXPRESSION:
                break;
            case NodeType::INT_EXPRESSION:
                break;
            case NodeType::STRING_EXPRESSION:
                break;
            case NodeType::BOOLEAN_EXPRESSION:
                break;
            case NodeType::ID:
                break;
            case NodeType::CHAR_LIST:
                break;
            case NodeType::BOOLEAN_OPERATION:
                break;
            case NodeType::UNKNOWN:
                break;
        }
    }
};
