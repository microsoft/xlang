/* Copyright (c) 2012-2017 The ANTLR Project. All rights reserved.
 * Use of this file is governed by the BSD 3-clause license that
 * can be found in the LICENSE.txt file in the project root.
 */


//
//  main.cpp
//  antlr4-cpp-demo
//
//  Created by Mike Lischke on 13.03.16.
//

#include <iostream>

#include "antlr4-runtime.h"
#include "cpp/generated/XlangParser.h"
#include "cpp/generated/XlangLexer.h"

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

    tree::ParseTree *tree = parser.xlang();
    std::string s = tree->toStringTree(&parser);
    std::cout << s << std::endl;

    std::cout << parser.getNumberOfSyntaxErrors() << std::endl;

    // This gets the token stream and prints out all the tokens corresponding to the file
    TokenStream * ts = parser.getTokenStream();
    for (int i = 0; i < ts->size(); i++)
    {
        std::cout << ts->get(i)->getType() << std::endl;
    }
}
