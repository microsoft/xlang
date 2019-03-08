parser grammar XlangParser;

options { tokenVocab=XlangLexer; }

xlang
	: identifier comment
    | unicodeescape
    | xlang comment
	;


identifier
    : IDENTIFIER
    ;
    
unicodeescape
    : UNICODE_ESCAPE_SEQUENCE*
    ;

comment: COMMENT;