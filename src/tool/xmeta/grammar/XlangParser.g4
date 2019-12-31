parser grammar XlangParser;

options { tokenVocab=XlangLexer; }

/* Entry Point */
xlang
    : compilation_unit
    ;

compilation_unit
    : using_directive* namespace_declaration*
    ;

/* Basic Concepts */ //This could be namespace or type name
type_name
    : IDENTIFIER type_argument_list?
    | qualified_alias_member
    ;

// Type is used in parameters and return types
// It can refer to another type that was declared via type_name
type
    : value_type
    | class_type
    | array_type
    ;

value_type
    : BOOLEAN
    | STRING
    | INT8
    | INT16
    | INT32
    | INT64
    | UINT8
    | UINT16
    | UINT32
    | UINT64
    | CHAR16
    | SINGLE
    | DOUBLE
    ;

class_type
    : type_name
    | OBJECT
    ;

array_type
    : non_array_type OPEN_BRACKET CLOSE_BRACKET
    ;

non_array_type
    : value_type
    | class_type
    ;

enum_integral_type // Bool is intentionally excluded
    : INT8
    | UINT8
    | INT16
    | UINT16
    | INT32
    | UINT32
    | INT64
    | UINT64
    ;

type_argument_list
    : LESS_THAN type (COMMA type)* GREATER_THAN
    ;

/* Expression */
expression //Maybe separate expression and literal later when we do real expression
    : decimal_literal_expression
    | hexadecimal_literal_expression
    | boolean_literal_expression
    | real_literal_expression
    | character_literal_expression
    | string_expression
    | IDENTIFIER
    | uuid_expression
    | null_expression
    ;

string_expression
    : STRING_LITERAL
    ;

uuid_expression
    : UUID
    ;

null_expression
    : NILL
    ;

character_literal_expression
    : CHARACTER_LITERAL
    ;

real_literal_expression
    : REAL_LITERAL
    ;

boolean_literal_expression
    : BOOLEAN_LITERAL
    ;

hexadecimal_literal_expression
    : HEXADECIMAL_INTEGER_LITERAL
    ;

decimal_literal_expression
    : DECIMAL_INTEGER_LITERAL
    ;

// /* Variables */ // this is not used anywhere
// variable_declarator
//     : IDENTIFIER
//     | IDENTIFIER EQUAL variable_initializer
//     ;

// variable_initializer
//     : expression
//     | array_type
//     ;

/* Attributes */
attributes
    : attribute_section*
    ;

attribute_section
    : OPEN_BRACKET (attribute_target COLON)? attribute_list CLOSE_BRACKET
    ;

// Attributes targets are subject to change
attribute_target // Method, Param, Type, Attribute
    : ASSEMBLY
    | MODULE
    | FIELD
    | EVENT
    | PROPERTY
    | RETURN
    | PUBLIC
    ;

attribute_list
    : attribute (COMMA attribute)*
    ;

attribute
    : type_name attribute_arguments?
    ;

attribute_arguments
    : OPEN_PARENS positional_argument_list? CLOSE_PARENS
    | OPEN_PARENS positional_argument_list COMMA named_argument_list CLOSE_PARENS
    | OPEN_PARENS named_argument_list? CLOSE_PARENS
    ;

positional_argument_list
    : positional_argument (COMMA positional_argument)*
    ;

positional_argument
    : IDENTIFIER? expression
    ;

named_argument_list // Name arguments may not be needed in xlang. 
    : named_argument (COMMA named_argument)*
    ;

named_argument
    : IDENTIFIER EQUAL expression
    ;

/* Namespaces */
namespace_declaration // add (. IDentifier)
    : NAMESPACE IDENTIFIER namespace_body SEMICOLON? // namespace windows.name.test {}
    ;

namespace_body
    : OPEN_BRACE using_directive* namespace_member_declaration* CLOSE_BRACE
    ;

using_directive
    : using_alias_directive
    | using_namespace_directive
    ;

using_alias_directive
    : USING IDENTIFIER EQUAL type_name SEMICOLON
    ;

using_namespace_directive
    : USING (IDENTIFIER .)* IDENTIFIER SEMICOLON
    ;

namespace_member_declaration
    : namespace_declaration
    | class_declaration
    | struct_declaration
    | interface_declaration
    | enum_declaration
    | delegate_declaration
    | apicontract_declaration 
    | attribute_declaration
    ;

qualified_alias_member
    : IDENTIFIER DOUBLE_COLON IDENTIFIER type_argument_list?
    ;

apicontract_declaration
    : attributes? APICONTRACT_LXR IDENTIFIER OPEN_BRACE CLOSE_BRACE SEMICOLON?
    ;

//* Class members *//
attribute_declaration
    : attributes? ATTRIBUTE IDENTIFIER
        attribute_body SEMICOLON
    ;

attribute_body
    : OPEN_BRACE attribute_constructor_declaration? field_declaration* CLOSE_BRACE
    ;

attribute_constructor_declaration
    : IDENTIFIER OPEN_PARENS attribute_parameter_list? CLOSE_PARENS SEMICOLON
    ;

attribute_parameter_list
    : positional_parameter (COMMA positional_parameter)*
    ;

positional_parameter
    : type IDENTIFIER
    ;

class_declaration
    : attributes? class_modifiers* CLASS IDENTIFIER type_parameter_list?
        class_base? class_body SEMICOLON?
    ;

class_modifiers // Explicit Seal? allow it to be explicitly sealed
    : SEALED
    | STATIC
    ;

type_parameter_list // attributes on generic types? Something<[Attribute] K> 
    : LESS_THAN attributes? IDENTIFIER (COMMA attributes? IDENTIFIER)* GREATER_THAN
    ;

// Semantically check for one class and multiple interfaces restriction
class_base
    : (COLON type_base)? interface_base?
    ;

type_base
    : attributes? class_type
    ;

class_body
    : OPEN_BRACE class_member_declarations* CLOSE_BRACE
    ;

class_member_declarations
    : attributes OPEN_BRACE class_member_declaration* CLOSE_BRACE
    | class_member_declaration
    ;

class_member_declaration
    : class_method_declaration
    | class_property_declaration
    | class_event_declaration
    | class_constructor_declaration
    ;

class_method_declaration //Do not need generics in method
    : attributes? method_modifier* return_type IDENTIFIER OPEN_PARENS formal_parameter_list? CLOSE_PARENS SEMICOLON
    ;

class_event_declaration // this has to be a delegate type aka class_type
    : attributes? event_modifier* EVENT type IDENTIFIER SEMICOLON
    ;

class_property_declaration // Restrict property_identifier first ///TODO: Property accessors are optional
    : attributes? property_modifier* type IDENTIFIER property_accessors
    ;

class_constructor_declaration
    : attributes? class_constructor_modifier? IDENTIFIER OPEN_PARENS formal_parameter_list? CLOSE_PARENS SEMICOLON
    ;

method_modifier
    : OVERRIDE
    | OVERRIDABLE
    | PROTECTED
    | STATIC
    ;

formal_parameter_list
    : fixed_parameter (COMMA fixed_parameter)*
    ;

fixed_parameter
    : attributes? parameter_modifier? type IDENTIFIER
    ;

parameter_modifier // More restrictive?
    : CONST_LXR REF
    | REF CONST_LXR
    | REF
    | OUT_LXR
    ;

return_type
    : type
    | VOID_LXR
    ;

property_modifier
    : PROTECTED
    | STATIC
    | OVERRIDABLE
    ;

property_accessors
    : OPEN_BRACE attributes? property_accessor_method+ CLOSE_BRACE SEMICOLON?
    | SEMICOLON
    ;

property_accessor_method
    : GET SEMICOLON
    | SET SEMICOLON
    ;

event_modifier
    : PROTECTED
    | STATIC
    ;

event_accessors // add and remove implicit?
    : OPEN_BRACE attributes? ADD SEMICOLON attributes? REMOVE SEMICOLON CLOSE_BRACE SEMICOLON?
    | SEMICOLON
    ;

class_constructor_modifier
    : PROTECTED
    ;

/* Structs */
struct_declaration // we don't have paramaterized structs yet in V1
    : attributes? STRUCT IDENTIFIER // type_parameter_list?
        struct_body SEMICOLON?
    ;

struct_body
    : OPEN_BRACE field_declaration* CLOSE_BRACE
    ;

field_declaration
    : type IDENTIFIER SEMICOLON
    ;

/* Interfaces */
interface_declaration
    : attributes? INTERFACE
        IDENTIFIER type_parameter_list? interface_base? interface_body SEMICOLON?
    ;

interface_base
    : REQUIRES type_base (COMMA type_base)*
    ;

interface_body
    : OPEN_BRACE interface_member_declaration* CLOSE_BRACE
    ;

interface_member_declaration
    : interface_method_declaration
    | interface_property_declaration
    | interface_event_declaration
    ;

interface_method_declaration
    : attributes? return_type IDENTIFIER type_parameter_list? OPEN_PARENS formal_parameter_list? CLOSE_PARENS SEMICOLON
    ;

interface_property_declaration
    : attributes? type IDENTIFIER property_accessors
    ;

interface_event_declaration
    : attributes? EVENT type IDENTIFIER SEMICOLON
    ;

/* Enums */
enum_declaration
    : attributes? ENUM IDENTIFIER enum_base? enum_body SEMICOLON?
    ;

enum_base
    : COLON enum_integral_type
    ;

enum_body
    : OPEN_BRACE CLOSE_BRACE
    | OPEN_BRACE enum_member_declaration (COMMA enum_member_declaration)* COMMA? CLOSE_BRACE
    ;

enum_member_declaration
    : attributes? enum_identifier
    | attributes? enum_identifier EQUAL enum_expression
    ;

enum_expression
    : enum_decimal_integer
    | enum_hexdecimal_integer
    | enum_expresssion_identifier;

enum_decimal_integer
    : MINUS? DECIMAL_INTEGER_LITERAL;

enum_expresssion_identifier
    : IDENTIFIER;

enum_hexdecimal_integer
    : HEXADECIMAL_INTEGER_LITERAL;

enum_identifier
    : IDENTIFIER
    | BOOLEAN
    | STRING
    | INT8
    | INT16
    | INT32
    | INT64
    | UINT8
    | UINT16
    | UINT32
    | UINT64
    | CHAR16
    | SINGLE
    | DOUBLE
    | OBJECT
    | NILL;

/* Delegates */
delegate_declaration
    : attributes? DELEGATE return_type
        IDENTIFIER type_parameter_list?
        OPEN_PARENS formal_parameter_list? CLOSE_PARENS SEMICOLON
    ;
