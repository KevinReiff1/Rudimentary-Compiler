#pragma once

#include <optional>
#include <vector>
#include "Token.h"

class CST {

};

class Parser {
    std::vector<Token> tokens;

public:
    explicit Parser(const std::vector<Token> &items) : tokens{items} {
    }

    std::optional<CST> parse() {
        return {};
    }
};
