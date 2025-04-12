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
    DataType type{};
    bool isInitialized{};
    bool isUsed{};
    int lineNumber{};
};

class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    int currentScopeLevel = -1;

public:
    void enterScope() {
        scopes.emplace_back();
        currentScopeLevel++;
        std::cout << "[Verbose] Entering scope " << currentScopeLevel << std::endl;
    }

    void exitScope() {
        // Check for unused or uninitialized variables
        for (const auto& pair : scopes.back()) {
            if (const Symbol& sym = pair.second; !sym.isUsed && sym.isInitialized) {
                std::cout << "[Warning] Variable '" << sym.name
                          << "' declared and initialized but never used at line "
                          << sym.lineNumber << std::endl;
            } else if (!sym.isUsed) {
                std::cout << "[Warning] Variable '" << sym.name
                          << "' declared but never used at line "
                          << sym.lineNumber << std::endl;
            }
        }
        scopes.pop_back();
        currentScopeLevel--;
        std::cout << "[Verbose] Exiting scope " << currentScopeLevel + 1 << std::endl;
    }

    bool addSymbol(const std::string& name, DataType type, int line) {
        if (scopes.back().find(name) != scopes.back().end()) {
            std::cout << "[Error] Redeclaration of variable '" << name
                      << "' in scope " << currentScopeLevel << " at line "
                      << line << std::endl;
            return false;
        }
        scopes.back()[name] = {name, type, false, false, line};
        std::cout << "[Verbose] Added symbol '" << name << "' of type "
                  << static_cast<int>(type) << " in scope "
                  << currentScopeLevel << std::endl;
        return true;
    }

    Symbol* findSymbol(const std::string& name, int line) {
        for (int i = currentScopeLevel; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) {
                it->second.isUsed = true;
                return &it->second;
            }
        }
        std::cout << "[Error] Undeclared variable '" << name
                  << "' used at line " << line << std::endl;
        return nullptr;
    }

    void markInitialized(const std::string& name) {
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
            for (const auto& pair : scopes[i]) {
                const auto&[name, type, isInitialized, isUsed, lineNumber] = pair.second;
                std::cout << name << "\t"
                          << static_cast<int>(type) << "\t"
                          << (isInitialized ? "true" : "false") << "\t"
                          << (isUsed ? "true" : "false") << "\t"
                          << i << "\t" << lineNumber << "\n";
            }
        }
    }
};