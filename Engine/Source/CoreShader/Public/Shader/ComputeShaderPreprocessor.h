#pragma once

namespace CE
{
	struct ComputeShaderPreprocessData
	{
		Array<Name> kernels;
	};

	class CORESHADER_API ComputeShaderPreprocessor
	{
	public:

		ComputeShaderPreprocessor(Stream* stream, const Array<IO::Path>& includePaths);
		~ComputeShaderPreprocessor();

		bool PreprocessShader(ComputeShaderPreprocessData* outData);

		inline const String& GetErrorMessage() const { return errorMessage; }

	private:

		enum TokenType
		{
			TK_WHITESPACE = 0,
			TK_IDENTIFIER,
			TK_SCOPE_OPEN, TK_SCOPE_CLOSE,
			TK_PAREN_OPEN, TK_PAREN_CLOSE,
			TK_SQUARE_OPEN, TK_SQUARE_CLOSE,
			TK_KW_SHADER,
			TK_KW_TAGS,
			TK_KW_PASS,
			TK_KW_ZTEST,
			TK_KW_PROPERTIES,
			TK_KW_INCLUDE,
			TK_LITERAL_STRING,
			TK_LITERAL_INTEGER,
			TK_LITERAL_FLOAT,
			TK_POUND,
			TK_EQUAL,
			TK_COMMA,
			TK_COLON,
			TK_SEMICOLON,
			TK_NEWLINE,
			TK_KW_PRAGMA,
			TK_KW_CULL
		};

		struct Token
		{
			TokenType token = TK_WHITESPACE;
			String lexeme{};
		};

		bool Tokenize(Array<Token>& outTokens);
		bool ReadNextToken(Token& outToken);

		Stream* stream = nullptr;
		Array<IO::Path> includePaths{};
		String errorMessage = "";

		Array<Token> tokens{};
		Token prevToken{};

		int curScope = 0;
	};

} // namespace CE

#include "ComputeShaderPreprocessor.rtti.h"
