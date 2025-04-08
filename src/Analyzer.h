#pragma once
#include <utility>

#include "Parser.h"
#include "SymbolTable.h"

struct ASTNode {
    NodeType type;
    DataType data_type = DataType::Unknown;
    std::string value{}; // For IDs, literals
    int line_number{};
    std::vector<ASTNode> children{};

    ASTNode(const NodeType node_type, int line) : type(node_type), line_number(line) {
    }

    explicit ASTNode(const NodeType node_type) : type(node_type) {
    }

    ASTNode(NodeType node_type, std::string val, int line = 0)
        : type(node_type), value(std::move(val)), line_number(line) {
    }
};

class ASTBuilder {
    CST cst;

    ASTNode root{NodeType::BLOCK};

    std::optional<ASTNode> convert_var_declaration(const Node& node) {

    }

    std::optional<ASTNode> convert_node(const Node& node) {
        switch (node.get_node_type()) {
            case NodeType::BLOCK:
                return convert_block(node);
            case NodeType::VARIABLE_DECLARATION:
                return convert_var_declaration(node);
            case NodeType::ASSIGNMENT_STATEMENT:
                return convert_assignment_statement(node);
            case NodeType::IF_STATEMENT:
                return convert_if_statement(node);
            case NodeType::WHILE_STATEMENT:
                return convert_while_statement(node);
            case NodeType::PRINT_STATEMENT:
                return convert_print_statement(node);
            default:
        }

        return {};
    }

    std::optional<ASTNode> convert_block(const Node& node) {
        ASTNode block{NodeType::BLOCK};
        for (const auto &child: node.get_children()) {
            auto child_node = convert_node(child);
            if (child_node.has_value())
                block.children.push_back(child_node.value());
        }
        return block;
    }

public:
    explicit ASTBuilder(CST cst_) : cst(std::move(cst_)) {
    }


    ASTNode build() {
        if (const auto root = cst.get_root(); root.get_children().empty())
            throw std::runtime_error("Empty AST");
        else
            traverse_node(root);

        return root;
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
