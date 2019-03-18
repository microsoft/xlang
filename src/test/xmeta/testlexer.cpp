#include "pch.h"
#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"
#include "XlangBasicListener.h"
#include <Windows.h>

using namespace antlr4;

#pragma execution_character_set("utf-8")

TEST_CASE("Namespace Identifier")
{   
    std::string test_idl =
        "namespace test{}";
    ANTLRInputStream input(test_idl);
    XlangLexer lexer(&input);       
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    XlangBasicListener listener;

    tree::ParseTree *tree = parser.xlang();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    std::set<std::string> namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(parser.getNumberOfSyntaxErrors() == 0);
}

TEST_CASE("Token identifier with unicode letter character")
{
    std::string test_idl =
        "namespace test1AÆĦǆＺ{} \
        namespace test2aăɶｚ{} \
        namespace test3ǅᾜῼ {} \
        namespace test4ʰˀﾟ {} \
        namespace test5ªကညￜ {} \
        namespace test6ᛮⅫⅯ {}";

    ANTLRInputStream input(test_idl);
    XlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    XlangBasicListener listener;

    tree::ParseTree *tree = parser.xlang();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    std::set<std::string> namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test1AÆĦǆＺ") != namespaces.end()); // LU 
    REQUIRE(namespaces.find("test2aăɶｚ") != namespaces.end()); // LL
    REQUIRE(namespaces.find("test3ǅᾜῼ") != namespaces.end()); // LT
    REQUIRE(namespaces.find("test4ʰˀﾟ") != namespaces.end()); // LM
    REQUIRE(namespaces.find("test5ªကညￜ") != namespaces.end()); // LO
    REQUIRE(namespaces.find("test6ᛮⅫⅯ") != namespaces.end()); // NL
    REQUIRE(parser.getNumberOfSyntaxErrors() == 0);
}

TEST_CASE("Identifer not starting with letter character")
{
    std::string test_idl =
        "namespace 123abc {}";

    ANTLRInputStream input(test_idl);
    XlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    XlangBasicListener listener;

    tree::ParseTree *tree = parser.xlang();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    std::set<std::string> namespaces = listener.namespaces;

    REQUIRE(parser.getNumberOfSyntaxErrors() == 1);
}

TEST_CASE("Remove comments")
{
    std::string test_idl =
        "namespace test {} // this is a comment \n \
        namespace test2 {} /* this is a \n multiline comment */ \n \
        namespace test3 {}";

    ANTLRInputStream input(test_idl);
    XlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    XlangBasicListener listener;
                
    tree::ParseTree *tree = parser.xlang();
    std::cout << tree->toStringTree(&parser) << std::endl;
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    std::set<std::string> namespaces = listener.namespaces;
    std::cout << tree->toStringTree(&parser) << std::endl;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(namespaces.find("test2") != namespaces.end());
    REQUIRE(namespaces.find("test3") != namespaces.end());
    REQUIRE(parser.getNumberOfSyntaxErrors() == 0);
}

TEST_CASE("Spacing")
{
    std::string test_idl =
        "namespace test    \f {} \
        namespace   test2  \t {} \
        namespace    test3  \v {}";

    ANTLRInputStream input(test_idl);
    XlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    XlangBasicListener listener;

    tree::ParseTree *tree = parser.xlang();
    std::cout << tree->toStringTree(&parser) << std::endl;
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    std::set<std::string> namespaces = listener.namespaces;
    std::cout << tree->toStringTree(&parser) << std::endl;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(namespaces.find("test2") != namespaces.end());
    REQUIRE(namespaces.find("test3") != namespaces.end());
    REQUIRE(parser.getNumberOfSyntaxErrors() == 0);
}




