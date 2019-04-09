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
    std::string path = std::filesystem::current_path().string() + "\\midl3test";
    std::cout << "Starting test on Midl3 Idl Files" << std::endl;
    int num_errors = 0;
    try {
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
    }
    catch (...)
    {
        std::cout << "Cannot find Midl3 Idl Files. They might not be avaibale on your machine." << std::endl;
        std::cout << "They might not be avaibale on your machine because the files are internal only." << std::endl;
        std::cout << "A subset of these test files may become avaibale at a later time" << std::endl;
    }
    REQUIRE(num_errors == 0);
}