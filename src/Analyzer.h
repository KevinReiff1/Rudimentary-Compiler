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
