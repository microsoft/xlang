#include "pch.h"
#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"
#include "xlangtestlistener.h"
#include <Windows.h>

using namespace antlr4;

#pragma execution_character_set("utf-8")

/* 
    Lexer test
    Primarily test whether the antlr lexer is correctly tokenizing the string and the listener can 
    store the correctly string. 
*/


/*
    setup_and_run_parser

    This helper method sets up the tokenizer and parser from the idl string and
    walks through the AST with the xlangtestlistener. The test listener is used to checked
    whether the idk string was lexed and parsed correctly by simply adding the string to a 
    set which we check inside the check. This method also returns the number of syntax errors. 
*/
int setup_and_run_parser(std::string idl, xlangtestlistener &listener)
{
    ANTLRInputStream input(idl);
    XlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);

    tree::ParseTree *tree = parser.xlang();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    return parser.getNumberOfSyntaxErrors();
}

TEST_CASE("Namespace Identifier")
{   
    std::string test_idl =
        "namespace test{}";
        
    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) == 0);

    std::set<std::string> namespaces = listener.namespaces;
    REQUIRE(namespaces.find("test") != namespaces.end());
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

    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) == 0);
    std::set<std::string> namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test1AÆĦǆＺ") != namespaces.end()); // LU 
    REQUIRE(namespaces.find("test2aăɶｚ") != namespaces.end()); // LL
    REQUIRE(namespaces.find("test3ǅᾜῼ") != namespaces.end()); // LT
    REQUIRE(namespaces.find("test4ʰˀﾟ") != namespaces.end()); // LM
    REQUIRE(namespaces.find("test5ªကညￜ") != namespaces.end()); // LO
    REQUIRE(namespaces.find("test6ᛮⅫⅯ") != namespaces.end()); // NL
}

TEST_CASE("Identifer not starting with letter character")
{
    std::string test_idl =
        "namespace 123abc {}";
    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) != 0);
}

TEST_CASE("Remove comments")
{
    std::string test_idl =
        "namespace test {} // this is a comment \n \
        namespace test2 {} /* this is a \n multiline comment */ \n \
        namespace test3 {}";

    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) == 0);
    std::set<std::string> namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(namespaces.find("test2") != namespaces.end());
    REQUIRE(namespaces.find("test3") != namespaces.end());
}

TEST_CASE("Spacing")
{
    std::string test_idl =
        "namespace test    \f {} \
        namespace   test2  \t {} \
        namespace    test3  \v {}";

    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) == 0);
    std::set<std::string> namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(namespaces.find("test2") != namespaces.end());
    REQUIRE(namespaces.find("test3") != namespaces.end());
}


TEST_CASE("Lexer uuid")
{
    std::string test_idl =
        "namespace Windows.UI.ApplicationSettings \
        { \
            [contract(Windows.Foundation.UniversalApiContract, 1)] \
            [uuid(b7de5527-4c8f-42dd-84da-5ec493abdb9a)] \
            delegate void WebAccountProviderCommandInvokedHandler(WebAccountProviderCommand command); \
        }";

    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) == 0);
    std::set<std::string> expressions = listener.expressions;

    REQUIRE(expressions.find("b7de5527-4c8f-42dd-84da-5ec493abdb9a") != expressions.end());
}

TEST_CASE("Enum assignments")
{
    std::string test_idl =
        "namespace Windows.Test \
        { \
            enum Color \
            { \
                Red, \
                Green, \
                Blue \
            } \
            enum Alignment \
            { \
                Center = 0, \
                Right = 1 \
            } \
            enum Permissions \
            { \
                None = 0x0000, \
                Camera = 0x0001, \
                Microphone = 0x0002, \
            } \
        }";

    xlangtestlistener listener;
    REQUIRE(setup_and_run_parser(test_idl, listener) == 0);
    std::set<std::string> enums = listener.enums;

    REQUIRE(enums.find("Red") != enums.end());
    REQUIRE(enums.find("Green") != enums.end());
    REQUIRE(enums.find("Blue") != enums.end());
    REQUIRE(enums.find("Center") != enums.end());
    REQUIRE(enums.find("Right") != enums.end());
    REQUIRE(enums.find("None") != enums.end());
    REQUIRE(enums.find("Camera") != enums.end());
    REQUIRE(enums.find("Blue") != enums.end());
    REQUIRE(enums.find("0") != enums.end());
    REQUIRE(enums.find("0x0000") != enums.end());
}

TEST_CASE("Enum illegal assignments")
{
    std::string test_idl_string_assignment =
        "namespace Windows.Test { \
            enum Alignment \
            { \
                Center = \"test\", \
            } \
        }";
    
    xlangtestlistener listener1;
    REQUIRE(setup_and_run_parser(test_idl_string_assignment, listener1) == 1);

    std::string test_idl_float_assignment =
        "namespace Windows.Test { \
            enum Alignment \
            { \
                Right = 1.9 \
            } \
        }";

    xlangtestlistener listener2;
    REQUIRE(setup_and_run_parser(test_idl_float_assignment, listener2) == 1);
}
