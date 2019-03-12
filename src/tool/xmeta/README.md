#This directory contains the tool to parse xlang idl file.
xmeta/grammar contains all the grammar file and a generate.cmd 
xmeta/java is the target directory for java code generation from the antlr tool. It is currently used to manually check the grammar and view the parse tree in the GUI.
xmeta/cpp is the target directory for cpp code generation.

#Generating Parser and Lexer
1) Run the generate.cmd in xmeta/grammar. This will generate the xlang lexer, parser and all else you need in cpp. 

