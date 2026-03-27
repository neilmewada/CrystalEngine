#pragma once

#include "Core.h"

namespace CE
{
    
    enum TokenType
    {
		TK_WHITESPACE = 0,
		TK_UNKNOWN, // Unkown or don't care

		TK_IDENTIFIER,
		TK_COMMENT,
		TK_LITERAL_STRING, // "
		TK_LITERAL_CHAR, // '
		TK_LITERAL_NUMBER, // Integer, float, double, etc
		TK_ASSIGNMENT_OPERATOR, // =
		TK_ARTIHMETIC_OPERATOR, // +, -, +=, >=, <<, >>, &, |, ~, etc (arithmetic + bitwise)
		TK_LOGICAL_OPERATOR, // &&, ||, !
		TK_SCOPE_OPERATOR, // ::
		TK_SCOPE_OPEN, // {
		TK_SCOPE_CLOSE, // }
		TK_PAREN_OPEN, // (
		TK_PAREN_CLOSE, // )
		TK_SQUARE_BRACE_OPEN, // [
		TK_SQUARE_BRACE_CLOSE, // ]
		TK_ACCESS_SPECIFIER, // public, private, protected

		// Custom tokens
		TK_FUNCTION_BODY, // We don't care what's in the functions
		TK_MACRO_BODY, // We don't care about macro bodies
		// RTTI tokens
        TK_CE_CLASS_BODY, // CE_CLASS(...)
		TK_CE_STRUCT_BODY, // CE_STRUCT(...)
		TK_CE_CLASS,
		TK_CE_STRUCT,
		TK_CE_FIELD,
		TK_CE_ENUM,
		TK_CE_ECONST,
		TK_CE_FUNCTION,
		TK_FUSION_PROPERTY,
		TK_FUSION_STATE_PROPERTY,
		TK_FUSION_DATA_PROPERTY,
		TK_FUSION_LAYOUT_PROPERTY,
		TK_FUSION_DATA_LAYOUT_PROPERTY,
		TK_FUSION_DATA_PAINT_PROPERTY,
		TK_FUSION_EVENT,
		TK_MODEL_PROPERTY,
		TK_MODEL_PROPERTY_EDITABLE,

		TK_LEFTANGLE, // <
		TK_RIGHTANGLE, // >
		TK_SEMICOLON, // ;
		TK_COLON, // :
		TK_COMMA, // ,
		TK_BACKSLASH, // `\`
		TK_FORWARDSLASH, // `/`
		TK_POUNDSIGN, // #
		TK_ASTERISK, // *
		TK_AMPERSAND, // &
		TK_PERIOD, // .

		TK_KW_INCLUDE,
		TK_KW_DEFINE,
		TK_KW_IF,
		TK_KW_IFDEF,
		TK_KW_ELSE,
		TK_KW_ELIF,
		TK_KW_ENDIF,
		TK_KW_DEFINED,

		TK_KW_INLINE,
		TK_KW_VIRTUAL,
		TK_KW_OVERRIDE,
		TK_KW_FINAL,

		TK_KW_FALSE,
		TK_KW_TRUE,
		TK_KW_NULLPTR,

		TK_KW_NAMESPACE,
		TK_KW_USING,
		TK_KW_CLASS,
		TK_KW_STRUCT,
		TK_KW_ENUM,
		TK_KW_CONST,
		TK_KW_AUTO,
		TK_KW_ALIGNAS,
		TK_KW_ALIGNOF,

		TK_KW_UNSIGNED,
		TK_KW_SIGNED,
		// Types
		TK_KW_VOID,
		TK_KW_INT,
		TK_KW_SHORT,
		TK_KW_LONG,
		TK_KW_CHAR,
		TK_KW_FLOAT,
		TK_KW_DOUBLE,
		TK_KW_BOOL,

		// Custom types
		TK_KW_UINT8,
		TK_KW_UINT16,
		TK_KW_UINT32,
		TK_KW_UINT64,
		TK_KW_INT8,
		TK_KW_INT16,
		TK_KW_INT32,
		TK_KW_INT64,
		TK_KW_STRING,
		TK_KW_ARRAY,
		TK_KW_VEC2,
		TK_KW_VEC3,
		TK_KW_VEC4,

		TK_KW_TYPEDEF,
		TK_KW_TEMPLATE,
		TK_KW_TYPENAME,

		TK_NEWLINE,
		TK_ERROR_TYPE,
		TK_END_OF_FILE,
    };

	struct Token
	{
		int line = 0;
		TokenType type = TK_WHITESPACE;
		std::string lexeme = "";
	};

} // namespace CE
