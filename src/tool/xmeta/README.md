#This directory contains the tool to parse xlang idl file.
xmeta/grammar contains all the grammar file and a generate.cmd 
xmeta/java is the target directory for java code generation from the antlr tool. It is currently used to manually check the grammar and view the parse tree in the GUI.
xmeta/cpp is the target directory for cpp code generation.

Requirements to building xmeta.exe
1) Install the latest JDK. 

2) Run the generate.cmd in xmeta/grammar. This will generate the xlang lexer, parser and all else you need in cpp. 

3) Build antlr4-runtime.dll and antlr4-runtime.lib from https://github.com/antlr/antlr4 
    - use target compile options "/MD$<$<CONFIG:Debug>:d>
        You will need to override it in their Cmakelist.txt with these. This is a stop gap measure
```
target_compile_options(antlr4_shared PRIVATE "/MD$<$<CONFIG:Debug>:d>")
target_compile_options(antlr4_static PRIVATE "/MD$<$<CONFIG:Debug>:d>")
```
4) Put your dll and lib file in the folder named x86debug or x86release depending on if the build is debug or release
Put that folder inside antlr4-runtime folder. Our cmakelist will look for the file path
/tool/xmeta/antlr4-runtime/antlrlib/x86debug/
/tool/xmeta/antlr4-runtime/antlrlib/x86release/

