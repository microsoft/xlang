#include "pch.h"
#include <iostream>
#include <string>
#include <filesystem>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"
#include "xlangtestlistener.h"
#include <Windows.h>

using namespace antlr4;

#pragma execution_character_set("utf-8")

TEST_CASE("Midl3 compat test", "[!hide]") {
    std::string path = std::filesystem::current_path().string() + "\\MoIdlConversion";
    std::cout << "Starting test on Midl3 Idl Files" << std::endl;
    int num_errors = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        std::cout << "Parsing File:" << std::endl;
        std::cout << entry.path() << std::endl;
        std::ifstream stream;
        stream.open(entry.path());

        ANTLRInputStream input(stream);
        XlangLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        XlangParser parser(&tokens);
        
        xlangtestlistener listener;
        tree::ParseTree *tree = parser.xlang();
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
        if (parser.getNumberOfSyntaxErrors() != 0)
        {
            num_errors++;
            std::cout << "Syntax errors: " << parser.getNumberOfSyntaxErrors() << std::endl;
        }
    }
    REQUIRE(num_errors == 0);
}