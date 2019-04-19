#pragma once

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

using namespace antlr4;

int setup_and_run_parser(std::string const& idl, XlangParserBaseListener& listener, bool disable_error_reporting = false)
{
    ANTLRInputStream input(idl);
    XlangLexer lexer(&input);

    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);

    if (disable_error_reporting) {
        lexer.removeErrorListeners();
        parser.removeErrorListeners();
    }

    tree::ParseTree *tree = parser.xlang();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    return parser.getNumberOfSyntaxErrors();
}
