#include <iostream>

#include "antlr4-runtime.h"
#include "XlangLexer.h"
#include "XlangParser.h"

#include <Windows.h>

#pragma execution_character_set("utf-8")

using namespace antlrcpptest;
using namespace antlr4;

int main(int argc, const char * argv[]) {

  ANTLRInputStream input("🍴 = 🍐 + \"😎\";(((x * π))) * µ + ∰; a + (x * (y ? 0 : 1) + z);");
  XlangLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  XLangParser parser(&tokens);
  return 0;
}
