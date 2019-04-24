cd ../java
del Xlang*
cd ../antlr4-runtime
antlr-4.7.2-complete.jar -o ../java ../grammar/XlangLexer.g4 ../grammar/XlangParser.g4
cd ../java
javac Xlang*.java