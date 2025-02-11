#include <iostream>
#include "Lexer.h"

int main(int argc, char *argv[]) {
    std::vector tests = {
        "{}$",
        "{{{{{{}}}}}}$",
        "{{{{{{}}} /* comments are ignored */ }}}}$",
        "{ /* comments are still ignored */ int @}$"
    };

    for (const auto &test : tests) {
        Lexer lexer{test};

        const auto tokens = lexer.scan();
    }


    std::cout << "Lexer" << std::endl;


    return 0;
}
