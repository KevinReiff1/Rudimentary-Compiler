#pragma once
#include <utility>

#include "Parser.h"

class SemanticAnalyzer {
    CST cst;
public:
    explicit SemanticAnalyzer(CST  cst_) :cst(std::move(cst_)) {

    }

    bool analyze() {
        return true;
    }
};
