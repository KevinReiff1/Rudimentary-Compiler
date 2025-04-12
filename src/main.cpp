#include <fstream>
#include <iostream>
#include <string>

#include "SemanticAnalyzer.h"
#include "Lexer.h"
#include "Parser.h"

std::string read_file(const std::string &filename) {
    std::ifstream file{filename};
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << filename << '\n';

        return {};
    }

    std::string content{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};

    file.close();

    return content;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::cout << "Lexing file " << argv[i] << '\n';

        const auto content = read_file(argv[i]);
        Lexer lexer{content};
        size_t index{0};
        while (!lexer.isEOF()) {
            Logger::log(LogLevel::INFO, "Lexer", "Lexing program " + std::to_string(++index) + "..");
            const auto tokens = lexer.scan();
            if (!tokens.has_value()) {
                Logger::log(LogLevel::ERROR, "PARSER", "Skipped due to LEXER error(s)");
                Logger::log(LogLevel::ERROR, "",
                            "CST for program " + std::to_string(index) + ": Skipped due to LEXER error(s)");

                continue;
            }

            Logger::log(LogLevel::INFO, "Parser", "Parsing program " + std::to_string(index) + "..");
            Parser parser{tokens.value()};
            const auto cst = parser.parse();
            if (!cst.has_value()) {
                Logger::log(LogLevel::ERROR, "",
                            "CST for program " + std::to_string(index) + ": Skipped due to PARSER error(s)");

                continue;
            }

            Logger::log(LogLevel::INFO, "", "CST for program " + std::to_string(index) + ":");
            cst.value().print();

            Logger::log(LogLevel::INFO, "", "Analyzing program " + std::to_string(index) + "..");
            SemanticAnalyzer analyzer{tokens.value()};
            const auto ast = analyzer.analyze();
            Logger::log(LogLevel::INFO, "", "AST for program " + std::to_string(index) + ":");
            ast.value().print();

            Logger::log(LogLevel::INFO, "", "Symbol Table for program " + std::to_string(index) + ":");
            analyzer.display_symbol_table();
        }
    }

    return 0;
}
