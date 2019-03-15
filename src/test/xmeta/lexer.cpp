#include "pch.h"
#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include <Windows.h>

using namespace antlr4;

#pragma execution_character_set("utf-8")

TEST_CASE("Simple lexer")
{
    std::ifstream stream;
    stream.open("example/namespace_test.idl");

    ANTLRInputStream input(stream);
    XlangLexer lexer(&input);       
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    tree::ParseTree *tree = parser.xlang();

    REQUIRE(parser.getNumberOfSyntaxErrors() == 0);
}

