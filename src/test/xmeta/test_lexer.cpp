#include "pch.h"
#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"
#include "xlang_test_listener.h"
#include "xmeta_idl_reader.h"
#include <Windows.h>

using namespace antlr4;
using namespace xlang::xmeta;

#pragma execution_character_set("utf-8")

/*
    Lexer test
    Primarily test whether the antlr lexer is correctly tokenizing the string and the listener can
    store the correctly string.
*/

TEST_CASE("Namespace Identifier")
{
    std::istringstream test_idl{ R"(
        namespace test { }
    )" };

    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener) == 0);

    auto& namespaces = listener.namespaces;
    REQUIRE(namespaces.find("test") != namespaces.end());
}

TEST_CASE("Token identifier with unicode letter character")
{
    std::istringstream test_idl{ R"(
        namespace test1AÆĦǆＺ{}
        namespace test2aăɶｚ{}
        namespace test3ǅᾜῼ {}
        namespace test4ʰˀﾟ {}
        namespace test5ªကညￜ {}
        namespace test6ᛮⅫⅯ {}
    )" };

    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener) == 0);
    auto& namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test1AÆĦǆＺ") != namespaces.end()); // LU
    REQUIRE(namespaces.find("test2aăɶｚ") != namespaces.end()); // LL
    REQUIRE(namespaces.find("test3ǅᾜῼ") != namespaces.end()); // LT
    REQUIRE(namespaces.find("test4ʰˀﾟ") != namespaces.end()); // LM
    REQUIRE(namespaces.find("test5ªကညￜ") != namespaces.end()); // LO
    REQUIRE(namespaces.find("test6ᛮⅫⅯ") != namespaces.end()); // NL
}

TEST_CASE("Identifer not starting with letter character")
{
    std::istringstream test_idl{ R"(
        namespace 123abc {}
    )" };
    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener, true) != 0);
}

TEST_CASE("Remove comments")
{
    std::istringstream test_idl{ " \
        namespace test {} // this is a comment \n \
        namespace test2 {} /* this is a \n multiline comment */ \
        namespace test3 {}" };

    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener) == 0);
    auto& namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(namespaces.find("test2") != namespaces.end());
    REQUIRE(namespaces.find("test3") != namespaces.end());
}

TEST_CASE("Spacing")
{
    std::istringstream test_idl{ "\
        namespace test    \f {} \
        namespace   test2  \t {} \
        namespace    test3  \v {}" };

    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener) == 0);
    auto& namespaces = listener.namespaces;

    REQUIRE(namespaces.find("test") != namespaces.end());
    REQUIRE(namespaces.find("test2") != namespaces.end());
    REQUIRE(namespaces.find("test3") != namespaces.end());
}


TEST_CASE("Lexer uuid")
{
    std::istringstream test_idl{ R"(
        namespace Windows.UI.ApplicationSettings
        {
            [contract(Windows.Foundation.UniversalApiContract, 1)]
            [uuid(b7de5527-4c8f-42dd-84da-5ec493abdb9a)]
            delegate void WebAccountProviderCommandInvokedHandler(WebAccountProviderCommand command);
        }
    )" };

    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener) == 0);
    auto& expressions = listener.expressions;

    REQUIRE(expressions.find("b7de5527-4c8f-42dd-84da-5ec493abdb9a") != expressions.end());
}

TEST_CASE("Enum assignments")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            enum Color
            {
                Red,
                Green,
                Blue
            }
            enum Alignment
            {
                Center = 0,
                Right = 1
            }
            enum Permissions
            {
                None = 0x0000,
                Camera = 0x0001,
                Microphone = 0x0002,
            }
        }
    )" };

    xlang_test_listener listener;
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl, listener) == 0);
    std::set<std::string> &enums = listener.enums;

    REQUIRE(enums.find("Color") != enums.end());
    REQUIRE(enums.find("Alignment") != enums.end());
    REQUIRE(enums.find("Permissions") != enums.end());

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
    std::istringstream test_idl_string_assignment{ R"(
        namespace Windows.Test
        {
            enum Alignment
            {
                Center = "test",
            }
        }
    )" };

    std::istringstream test_idl_float_assignment{ R"(
        namespace Windows.Test
        {
            enum Alignment
            {
                Right = 1.9
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    xlang_test_listener listener;
    REQUIRE(reader.read(test_idl_string_assignment, listener, true) == 1);
    listener.reset();
    REQUIRE(reader.read(test_idl_float_assignment, listener, true) == 1);
}
