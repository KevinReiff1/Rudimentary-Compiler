#pragma once
#include <unordered_map>
#include <utility>

#include "Parser.h"
#include "SymbolTable.h"

class CodeBuffer {
    SymbolTable symbol_table;
    std::vector<uint8_t> code;
    uint8_t position = 0;
    uint8_t heap = 0xFF;
    std::unordered_map<uint16_t, std::vector<uint8_t> > temp_addresses;

public:
    CodeBuffer() : code(256, 0) {
    }

    void emit(uint8_t byte) {
        if (position >= 256)
            throw std::runtime_error("Code exceeds 256 bytes");

        code[position++] = byte;
    }

    void emit(uint8_t op, uint8_t operand) {
        emit(op);
        emit(operand);
    }

    void emit_with_temp_address(uint8_t op, uint16_t address) {
        emit(op);
        temp_addresses[address].push_back(position);
        emit(address & 0xFF); // Little-endian
        emit((address >> 8) & 0xFF);
    }

    uint8_t add_string_variable(const std::string &name) {
        code[heap--] = '\0';
        for (auto pos = name.rbegin(); pos != name.rend(); ++pos) {
            code[heap--] = *pos;
        }

        return heap + 1;
    }

    void backpatch() {
        for (auto &[address, offsets]: temp_addresses) {
            for (auto offset: offsets) {
                code[offset] = position;
                position += 2;
            }
        }
    }

    [[nodiscard]] std::vector<uint8_t> getCode() const { return code; }
    [[nodiscard]] size_t getPosition() const { return position; }
};


class VariableTable {
    std::unordered_map<std::string, int> variables;
};

class CodeGen final {
    SymbolTable symbol_table;
    CodeBuffer buffer;

public:
    CodeGen() = default;

    ~CodeGen() = default;

    void generate(const Node &node) {
        visit(node);
        buffer.backpatch();
    }

    void print() const {
        auto code = buffer.getCode();
        int i = 0;
        for (const auto val: code) {
            if (++i == 16) {
                i = 0;
                std::cout << std::endl;
            }
            std::cout << std::hex
                    << std::uppercase
                    << std::setw(2) // Ensure at least 2 characters
                    << std::setfill('0')
                    << static_cast<int>(val) << ' ';
        }

        std::cout << std::endl;
    }

    void handle_var_decl(const Node &node) {
        const auto id_node = node.get_children().front();
        const auto name = node.get_children()[1];
        symbol_table.add_symbol(name.get_value(), node_to_data_type(id_node.get_value()),
                                id_node.get_line());
        // Initialize to 0: LDA #00; STA [temp]
        buffer.emit(0xA9, 0x00);
        auto symbol = symbol_table.findSymbol(name.get_value());
        buffer.emit_with_temp_address(0x8D, symbol->temp_address);
    }

    void handle_assign(const Node &node) {
        auto &children = node.get_children();
        auto *symbol = symbol_table.findSymbol(children.front().get_value());

        if (symbol->type == DataType::Int) {
            auto value = static_cast<uint8_t>(std::stoi(children.back().get_value()));
            buffer.emit(0xA9, value);
            buffer.emit_with_temp_address(0x8D, symbol->temp_address);
        } else if (symbol->type == DataType::Boolean) {
            uint8_t value = children.back().get_value() == "true" ? 1 : 0;
            buffer.emit(0xA9, value);
            buffer.emit_with_temp_address(0x8D, symbol->temp_address);
        } else {
            auto pos = buffer.add_string_variable(children.back().get_value());
            buffer.emit(0xA9, pos);
            buffer.emit_with_temp_address(0x8D, symbol->temp_address);
        }
    }

    void handle_print(const Node &node) {
        const auto &children = node.get_children();
        auto &item = children.front();
        switch (item.get_node_type()) {
            case NodeType::ID:
            case NodeType::INT_EXPRESSION:
            case NodeType::STRING_EXPRESSION:
            case NodeType::BOOLEAN_EXPRESSION:
                break;
        }
        auto symbol = symbol_table.findSymbol(item.get_value());
        buffer.emit_with_temp_address(0xAC, symbol->temp_address); // LDY var
        buffer.emit(0xA2, symbol->type == DataType::String ? 0x02 : 0x01); // LDX #0x
        buffer.emit(0xFF); // SYS
    }

    void handle_if(const Node &node) {
        // Compare variables (simplified)
        //std::string lhs = node->left->left->value;
        //std::string rhs = node->left->right->value;

        //buffer.emit(0xAE, symtab.getAddress(lhs)); // LDX lhs
        //buffer.emit(0xEC, symtab.getAddress(rhs)); // CPX rhs
        //buffer.emit(0xD0); // BNE operand (temp)
        size_t patch_addr = buffer.getPosition();
        //buffer.emit(0x00); // Placeholder
        //backpatches["IF"] = patch_addr;

        //traverse(node->right); // Generate 'then' block
    }

    // Generate code for if statement
    /*void generateIf(const Node &node) {
        // Condition: Assume form (var == value)
        Symbol *sym = findSymbol(node->children[0]->children[0]->value, node->children[0]->children[0]->scope);
        int value = std::stoi(node->children[0]->children[1]->value);
        emit({0xEC, static_cast<uint8_t>(sym->offset & 0xFF), static_cast<uint8_t>(sym->offset >> 8)});
        emit({0xD0, 0x00}); // Placeholder for jump
        int jumpAddr = machineCode.size() - 1;
        // Body
        for (const auto *child: node->children[1]->children) {
            generateNode(ASTchild);
        }
        jumpBackpatch.push_back({jumpAddr, machineCode.size() - jumpAddr});
    }*/

    // Generate code for while loop
    /* void generateWhile(const ASTNode *node) {
         int loopStart = machineCode.size();
         // Condition: Assume form (var != value)
         Symbol *sym = findSymbol(node->children[0]->children[0]->value, node->children[0]->children[0]->scope);
         int value = std::stoi(node->children[0]->children[1]->value);
         emit({
             0xAD, static_cast<uint8_t>(sym->offset & 0xFF), static_cast<uint8_t>(sym->offset >> 8), 0x8D, 0x54, 0x00
         });
         emit({0xA9, static_cast<uint8_t>(value), 0x8D, 0x53, 0x00});
         emit({0xAE, 0x54, 0x00, 0xEC, 0x53, 0x00});
         emit({0xD0, 0x00}); // Placeholder for jump
         int jumpAddr = machineCode.size() - 1;
         // Body
         for (const auto *child: node->children[1]->children) {
             generateNode(child);
         }
         // Jump back to start
         int distance = loopStart - machineCode.size() - 2;
         emit({0xD0, static_cast<uint8_t>(distance & 0xFF)});
         jumpBackpatch.push_back({jumpAddr, machineCode.size() - jumpAddr});
     }*/

    /*void resolveBackpatches() {
        // Example: Calculate jump distance for IF
        for (auto &[label, addr]: backpatches) {
            // Simplified: Assume 6 bytes to skip (as in document example)
            buffer.backpatch(addr, 0x06);
        }
    }*/

    [[nodiscard]] std::vector<uint8_t> getMachineCode() const {
        return buffer.getCode();
    }

    void visit(const Node &node) {
        auto children = node.get_children();

        switch (node.get_node_type()) {
            case NodeType::BLOCK: {
                symbol_table.enter_scope();
                for (auto &child: children) {
                    visit(child);
                }
                symbol_table.exit_scope();
                break;
            }
            case NodeType::VARIABLE_DECLARATION: {
                handle_var_decl(node);

                break;
            }
            case NodeType::ASSIGNMENT_STATEMENT:
                handle_assign(node);
                break;
            case NodeType::IF_STATEMENT:
                handle_if(node);
                break;
            case NodeType::WHILE_STATEMENT:
                //generate_while(node);
                break;
            case NodeType::PRINT_STATEMENT: {
                handle_print(node);
                auto val = children.front().get_value();
                break;
            }
        }
    }
};
