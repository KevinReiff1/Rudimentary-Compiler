#pragma once
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

public:
    void enterScope() {
        scopes.emplace_back();
        currentScopeLevel++;
        std::cout << "[Verbose] Entering scope " << currentScopeLevel << std::endl;
    }

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

    Symbol *findSymbol(const std::string &symbol) {
        for (int i = currentScopeLevel; i >= 0; --i) {
            auto& map = scopes[i];
            auto it = map.find(symbol);
            if (it != map.end()) {
                it->second.isUsed = true;
                return &it->second;
            }
        }
        return nullptr;
    }

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

    void display() {
        std::cout << "\nSymbol Table:\n";
        std::cout << "Name\tType\tIsInit?\tIsUsed?\tScope\tLine\n";
        for (size_t i = 0; i < scopes.size(); ++i) {

            auto& map = scopes[i];
            for (const auto &pair: map) {

                const auto &[name, type, isInitialized, isUsed, lineNumber] = pair.second;
                std::cout << name << "\t"
                        << type << "\t"
                        << (isInitialized ? "true" : "false") << "\t"
                        << (isUsed ? "true" : "false") << "\t"
                        << i << "\t" << lineNumber << "\n";
            }
        }
    }
};
