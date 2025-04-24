#pragma once
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

enum class DataType {
    Int,
    String,
    Boolean,
    Unknown
};

static inline std::array<std::string, static_cast<size_t>(DataType::Unknown) + 1> data_type_names = {
    "int",
    "string",
    "boolean"
};

inline DataType node_to_data_type(const std::string &node_value) {
    for (auto data_type: {DataType::Int, DataType::String, DataType::Boolean}) {
        if (data_type_names[static_cast<size_t>(data_type)] == node_value) {
            return data_type;
        }
    }

    return DataType::Unknown;
}

struct Symbol {
    std::string name{};
    std::string type{};
    bool isInitialized{};
    bool isUsed{};
    int lineNumber{};
};

class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, Symbol> > scopes;
    int currentScopeLevel = -1;

    std::unordered_map<std::string, DataType> type_map{};

    /**
     * Enters a new scope and increments the scope level.
     *
     * This method performs the following:
     * - Creates a new scope by adding an empty symbol map to the `scopes` container.
     * - Increments the `currentScopeLevel` to reflect the new scope.
     * - Logs a verbose message indicating the current scope level entered.
     *
     * This is used to manage nested scopes and helps organize symbol visibility
     * and lifecycle within a specific block of code.
     */
public:
    void enterScope() {
        scopes.emplace_back();
        currentScopeLevel++;
        std::cout << "[Verbose] Entering scope " << currentScopeLevel << std::endl;
    }

    /**
     * Exits the current scope, checks for unused variables, and reduces the scope level.
     *
     * This method performs the following:
     * - Iterates through all the symbols in the current scope.
     * - If a variable was declared and initialized but never used, a warning is logged.
     * - If a variable was declared but neither initialized nor used, a warning is logged.
     * - Reduces the scope level by decrementing `currentScopeLevel`.
     * - Logs an informational message indicating the scope that is exited.
     *
     * Any warnings are issued via standard output, and this assists in identifying
     * potentially unused or unnecessary variables in the program.
     */
    void exitScope() {
        // Check for unused or uninitialized variables
        for (const auto &pair: scopes.back()) {
            if (const Symbol &sym = pair.second; !sym.isUsed && sym.isInitialized) {
                std::cout << "[Warning] Variable '" << sym.name
                        << "' declared and initialized but never used at line "
                        << sym.lineNumber << std::endl;
            } else if (!sym.isUsed) {
                std::cout << "[Warning] Variable '" << sym.name
                        << "' declared but never used at line "
                        << sym.lineNumber << std::endl;
            }
        }
        currentScopeLevel--;
        std::cout << "[Verbose] Exiting scope " << currentScopeLevel + 1 << std::endl;
    }

    /**
     * Adds a new symbol to the current scope if it does not already exist.
     * If the symbol already exists in the current scope, an error is reported.
     *
     * @param name The name of the symbol to add.
     * @param type The type of the symbol (e.g., int, string, etc.).
     * @param line The line number where the symbol is declared.
     * @return True if the symbol was successfully added, false if it already exists in the current scope.
     */
    bool addSymbol(const std::string &name, const std::string &type, int line) {
        auto &current_scope = scopes.back();

        if (current_scope.contains(name)) {
            std::cout << "[Error] Redeclaration of variable '" << name
                    << "' in scope " << currentScopeLevel << " at line "
                    << line << std::endl;
            return false;
        }
        current_scope[name] = {name, type, false, false, line};
        std::cout << "[Verbose] Added symbol '" << name << "' of type "
                << type << " in scope "
                << currentScopeLevel << std::endl;
        return true;
    }

    /**
     * Searches for a symbol with the specified name in the current and parent scopes.
     * If the symbol is found, it marks the symbol as used and returns a pointer to it.
     * If the symbol is not found, it returns nullptr.
     *
     * @param symbol The name of the symbol to search for.
     * @return A pointer to the found Symbol object, or nullptr if not found.
     */
    Symbol *findSymbol(const std::string &symbol) {
        for (int i = currentScopeLevel; i >= 0; --i) {
            auto &map = scopes[i];
            auto it = map.find(symbol);
            if (it != map.end()) {
                it->second.isUsed = true;
                return &it->second;
            }
        }
        return nullptr;
    }

    /**
     * Marks a symbol as initialized by searching for it in the current and parent scopes.
     *
     * This method iterates through the available scopes, starting from the current
     * scope and moving outward to parent scopes, in search of the symbol with the given name.
     * Once found, the symbol's `isInitialized` attribute is set to true.
     * A verbose log message is generated to indicate that the symbol has been marked
     * as initialized and specifies the scope in which this occurred.
     *
     * @param name The name of the symbol to mark as initialized.
     */
    void markInitialized(const std::string &name) {
        for (int i = currentScopeLevel; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) {
                it->second.isInitialized = true;
                std::cout << "[Verbose] Marked '" << name
                        << "' as initialized in scope " << i << std::endl;
                break;
            }
        }
    }

    /**
     * Displays the current state of the symbol table, including all symbols across all scopes.
     *
     * This method prints detailed information about each symbol in the symbol table
     * to the standard output. For every symbol in all the scopes, it outputs:
     * - Symbol name
     * - Data type
     * - Initialization status (true/false)
     * - Usage status (true/false)
     * - Scope level
     * - Line number where the symbol was declared
     *
     * The table is preceded by a header row describing the columns. Each scope is traversed
     * in order, from outermost to innermost. Symbols within a scope are displayed
     * sequentially and their properties are formatted in tabular form.
     *
     * This method is used for debugging and validation purposes, enabling
     * a detailed review of the symbol table's contents at any point
     * during program execution.
     */
    void display() const {
        std::cout << "Symbol Table:\n";
        std::cout << "-------------------------------------------------\n";
        std::cout << "Name          Type     IsInit? IsUsed? Scope Line\n";
        std::cout << "-------------------------------------------------\n";

        for (size_t i = 0; i < scopes.size(); ++i) {
            for (auto &map = scopes[i]; const auto &pair: map) {
                const auto &[name, type, isInitialized, isUsed, lineNumber] = pair.second;
                std::cout << std::left << std::setw(14) << name
                        << std::left << std::setw(9) << type
                        << std::left << std::setw(8) << (isInitialized ? "true" : "false")
                        << std::left << std::setw(8) << (isUsed ? "true" : "false")
                        << std::left << std::setw(6) << i <<  lineNumber << "\n";
            }
        }
    }
};
