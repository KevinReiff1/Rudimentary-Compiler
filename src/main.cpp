#include <iostream>
#include "Lexer.h"

int main(int argc, char *argv[]) {
    std::vector tests = {
        //"{}$",
        //"{{{{{{}}}}}}$",
        //"{{{\t{{{}}}}}}$",
        //"{{{{{{}}}}}}$",
        "{{{{\n\r"
        "{{}}}}}}$",
        "{{{{{{ } }}}}}$",

        "{{{{{{}}} /* comments are ignored */ }}}}$",
        "{ /* comments are still ignored */ int @}$",
        "{ print(5 + 3)}$",

        "{ print( 5 + 3)}$",

        "{ print  (5 + 3)}$",
        "{ print    (   5+3)}$"


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

    for (const auto &test: tests) {
        Lexer lexer{test};

        std::cout << "INFO  Lexer - Lexing program" << '\n';
        const auto tokens = lexer.scan();
        const auto error_count = lexer.getErrorCount();
        const auto has_errors = error_count > 0;

        std::cout << (has_errors ? "ERROR" : "INFO") << " Lexer - Lex " << (has_errors ? "failed" : "completed") <<
                " with " << error_count << " error" << (has_errors ? "(s)" : "s") << "\n";
    }


    std::cout << "Lexer" << std::endl;


    return 0;
}
