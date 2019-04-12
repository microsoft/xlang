#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"
#include "ast_to_st_listener.h"
#include "symbol_table_helpers.h"

#include <Windows.h>

#pragma execution_character_set("utf-8")

using namespace antlr4;

int main(int argc, const char * argv[]) {

    std::ifstream stream;
    printf("Opening %s \n", argv[1]);
    stream.open(argv[1]);

    ANTLRInputStream input(stream);
    XlangLexer lexer(&input);       
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    parser.setBuildParseTree(true);
    xlang::xmeta::symbol_table_helper;
    // ast_to_st_listener listener;
    
    tree::ParseTree *tree = parser.xlang();
    // tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    std::string s = tree->toStringTree(&parser);

}
