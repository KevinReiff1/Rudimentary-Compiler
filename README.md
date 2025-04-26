# KR-CMPT-432-Compilers

This repository contains a semantic analyzer for a compiler written in C++. The project includes a Makefile for compilation and a set of test files to verify its functionality.

## **Branch to Grade**
The branch to grade is **`code_generator`**.

## **How to Run the Project**

### **Step 1: Navigate to the Project Directory**
```sh
cd KR-CMPT-432-Compilers-parser
```


### **Step 2: Compile the Project**
```
el@192 Downloads % cd KR-CMPT-432-Compilers-parser
el@192 KR-CMPT-432-Compilers-parser % make clean
rm -rf obj/*.o bin/compiler
el@192 KR-CMPT-432-Compilers-parser % make all
g++ -std=c++17 -std=c++17   -c src/Lexer.cpp -o obj/Lexer.o
g++ -std=c++17 -std=c++17   -c src/main.cpp -o obj/main.o
g++ -std=c++17 -std=c++17   -c src/Parser.cpp -o obj/Parser.o
g++ obj/Lexer.o obj/main.o obj/Parser.o -o bin/compiler
el@192 KR-CMPT-432-Compilers-lexer %
```
### **Step 3: Run the Compiler on Test Files**
```sh
bin/compiler tests/16
```
This executes the compiled program on the provided test cases.

## **File Structure**
```
KR-CMPT-432-Compilers-lexer/
├── Makefile
├── README.md
├── src
├── CodeGen.h
├── Lexer.cpp
├── Lexer.h
├── Log.h
├── main.cpp
├── Parser.cpp
├── Parser.h
├── SemanticAnalyzer.cpp
├── SemanticAnalyzer.h
├── SymbolTable.h
└── Token.h

├── bin/
│   └── compiler (generated after compilation)
├── obj/
│   ├── Lexer.o (generated after compilation)
│   ├── main.o (generated after compilation)
│   └── Parser.o (generated after compilation)

├── tests/
│   ├── 1
│   ├── 2
│   ├── 3
│   ├── pass
│   ├── fail
```

## **Notes**
- The `Makefile` is structured to handle compilation and cleanup efficiently.
- The test files in the `tests/` directory provide input for the compiler to analyze.
- If there are errors during lexing, they will be displayed in the output.


