#include <iostream>
#include "Lexer.h"

int main(int argc, char *argv[]) {
    std::vector tests = {
        "{}$",
        "{{{{{{}}}}}}$",
        "{{{\t{{{}}}}}}$",
        "{{{{{{"
        "}}}}}}$",
        "{{{{"
        "\r{{}}}}}}$",
        "{{{{{{ } }}}}}$",

        "{{{{{{}}} /* comments are ignored */ }}}}$",
        // "{ /* comments are still ignored */ int @}$",
        // ""
        // "{"
        // "   int a"
        // "   a=a"
        // "   string b"
        // "   a=b"
        // "}$",
        // ""
        // "{"
        // "   int a"
        // "   a=a"
        // "   string b"
        // "   a=b"
        // "}$",
    };

    for (const auto &test : tests) {
        Lexer lexer{test};

        std::cout << "INFO  Lexer - Lexing " << '\n';
        const auto tokens = lexer.scan();
        std::cout << "INFO Lexer - Lex completed with " << lexer.getErrorCount() << " errors\n";
    }


    std::cout << "Lexer" << std::endl;


    return 0;
}
