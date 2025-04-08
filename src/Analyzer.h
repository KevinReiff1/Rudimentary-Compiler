#pragma once
#include <algorithm>
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

    std::optional<ASTNode> convert_var_declaration(const Node &node) {
        return {};
    }

    std::optional<ASTNode> convert_assignment_statement(const Node &node) {
        return {};
    }

    std::optional<ASTNode> convert_if_statement(const Node &node) {
        return std::nullopt;
    }

    std::optional<ASTNode> convert_while_statement(const Node &node) {
        return std::nullopt;
    }

    std::optional<ASTNode> convert_print_statement(const Node &node) {
        return std::nullopt;
    }

    std::optional<ASTNode> convert_node(const Node &node) {
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
                if (node.get_children().empty())
                    return std::nullopt;
                else
                    return convert_node(node.get_children().front());
        }

        return std::nullopt;
    }

    std::optional<ASTNode> convert_block(const Node &node) {
        ASTNode block{NodeType::BLOCK};
        for (const auto &child: node.get_children()) {
            auto child_node = convert_node(child);
            if (child_node.has_value())
                block.children.push_back(child_node.value());
        }
        return block;
    }

    Node convert(const Node &node) {
        switch (node.get_node_type()) {
            case NodeType::STATEMENT:
            case NodeType::STATEMENT_LIST:
                for (auto &child: node.get_children())
                    convert(child);
            case NodeType::PROGRAM:
                return node.get_children().front();
            default:
                return node;
        }
    }

    void remove_useless_nodes(Node &node) {
        auto nodes = node.get_children();
        if (nodes.empty())
            return;

        std::vector<Node> new_nodes;

        for (auto &child: nodes) {
            remove_useless_nodes(child);
            if (child.get_node_type() == NodeType::STATEMENT || child.get_node_type() == NodeType::STATEMENT_LIST) {

                std::copy(child.get_children().begin(), child.get_children().end(), std::back_inserter(new_nodes));
            }
        }

        std::erase_if(nodes, [](const auto &node) {return node.get_node_type() == NodeType::STATEMENT || node.get_node_type() == NodeType::STATEMENT_LIST;});
    }

    std::optional<Node&> findNode(Node &node, NodeType nodeType) {
        if (node.get_node_type() == nodeType)
            return node;



    }

    void convert_all(const Node &node) {

        // Omitting empty nodes for readable output
        /*if (node.get_children().empty() && node.get_value().empty())
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
        }*/
    }

public:
    explicit ASTBuilder(CST cst_) : cst(std::move(cst_)) {
    }


    std::optional<ASTNode> build() {
        if (const auto root = cst.get_root(); root.get_children().empty())
            return std::nullopt;
        else {
            remove_useless_nodes(cst.get_root());
            return convert_node(root);
        }
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
