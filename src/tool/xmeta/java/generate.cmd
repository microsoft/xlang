cd ../grammar
antlr-4.7.2-complete.jar -o ../java xlang_lexer.g4 xlang_parser.g4
cd ../java
javac Xlang*.java