del Xlang*
cd ../grammar
antlr-4.7.2-complete.jar -o ../java XlangLexer.g4 XlangParser.g4
cd ../java
javac Xlang*.java