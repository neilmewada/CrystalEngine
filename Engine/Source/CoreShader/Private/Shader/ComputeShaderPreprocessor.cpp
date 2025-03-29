#include "CoreShader.h"

#include <regex>

namespace CE
{

    ComputeShaderPreprocessor::ComputeShaderPreprocessor(Stream* stream, const Array<IO::Path>& includePaths)
        : stream(stream), includePaths(includePaths)
    {

    }

    ComputeShaderPreprocessor::~ComputeShaderPreprocessor()
    {
    }

    bool ComputeShaderPreprocessor::PreprocessShader(ComputeShaderPreprocessData* outData)
    {
        errorMessage = "";

        defer(&)
        {
            if (errorMessage.NotEmpty())
            {
                CE_LOG(Error, All, "PreprocessShader failed: {}", errorMessage);
            }
        };

        if (stream == nullptr || !stream->CanRead())
        {
            errorMessage = "Invalid input stream";
            return false;
        }

        tokens.Clear();

        Array<Token> tokens{};

        bool success = Tokenize(tokens);
        if (!success)
            return false;

    	curScope = 0;

    	for (int i = 0; i < tokens.GetSize(); i++)
    	{
    		const Token& curToken = tokens[i];
    		if (curToken.token == TK_SCOPE_OPEN)
    			curScope++;
    		else if (curToken.token == TK_SCOPE_CLOSE)
    			curScope--;

    		Token nextToken = {};
    		if (i < tokens.GetSize() - 1)
    			nextToken = tokens[i + 1];

    		if (curToken.token == TK_POUND && nextToken.token == TK_IDENTIFIER && nextToken.lexeme == "pragma")
    		{
    			if (i < tokens.GetSize() - 2)
    			{
    				nextToken = tokens[i + 2];
    				if (nextToken.token == TK_IDENTIFIER && nextToken.lexeme == "kernel")
    				{
    					if (i < tokens.GetSize() - 3)
    					{
    						nextToken = tokens[i + 3];
    						if (nextToken.lexeme.NotEmpty())
    						{
    							outData->kernels.Add(nextToken.lexeme);
    						}
    						i += 3;
    						continue;
    					}

    					i += 2;
    					continue;
    				}
    			}
    		}
    	}

    	return true;
    }

    bool ComputeShaderPreprocessor::Tokenize(Array<Token>& outTokens)
    {
        while (!stream->IsOutOfBounds())
        {
            Token token{};
            bool valid = ReadNextToken(token);

            if (valid)
            {
                outTokens.Add(token);
                prevToken = token;
            }
        }

        return true;
    }

    bool ComputeShaderPreprocessor::ReadNextToken(Token& outToken)
    {
        if (stream->IsOutOfBounds())
			return false;

		char c = stream->Read();

		switch (c)
		{
		case ' ':
			break;
		case '\"': // String literal
		{
			String stringLiteral = "";
			char next = stream->Read();

			while (!stream->IsOutOfBounds() && next != '\"')
			{
				stringLiteral.Append(next);
				next = stream->Read();
			}

			outToken = Token{ .token = TK_LITERAL_STRING, .lexeme = stringLiteral };
			return true;
		}
			break;
		case '{':
			curScope++;
			outToken = Token{ TK_SCOPE_OPEN };
			return true;
		case '}':
			curScope--;
			outToken = Token{ TK_SCOPE_CLOSE };
			return true;
		case '(':
			outToken = Token{ TK_PAREN_OPEN };
			return true;
		case ')':
			outToken = Token{ TK_PAREN_CLOSE };
			return true;
		case '[':
			outToken = Token{ TK_SQUARE_OPEN };
			return true;
		case ']':
			outToken = Token{ TK_SQUARE_CLOSE };
			return true;
		case '=':
			outToken = Token{ TK_EQUAL };
			return true;
		case '#':
			outToken = Token{ TK_POUND };
			return true;
		case ',':
			outToken = Token{ TK_COMMA };
			return true;
		case ':':
			outToken = Token{ TK_COLON };
			return true;
		case ';':
			outToken = Token{ TK_SEMICOLON };
			return true;
		case '\n':
			outToken = Token{ TK_NEWLINE };
			return true;
		default:
		{
			if (String::IsAlphabet(c) || c == '_')
			{
				char nextChar = c;
				std::string identifier{};
				std::regex identifierRegex = std::regex("[a-zA-Z0-9_][a-zA-Z0-9_]*");

				while (!stream->IsOutOfBounds())
				{
					if (!identifier.empty() && !std::regex_match(identifier, identifierRegex))
					{
						identifier = identifier.substr(0, identifier.size() - 1);
						stream->Seek(-2, SeekMode::Current);
						break;
					}

					identifier.append({ nextChar });
					nextChar = stream->Read();
				}

				{
					outToken = Token{ TK_IDENTIFIER, identifier };
					return true;
				}
			}
			else if (String::IsNumeric(c))
			{
				char nextChar = c;
				std::string number = "";
				bool periodFound = false;

				while (!stream->IsOutOfBounds())
				{
					if (nextChar != '.' && !String::IsNumeric(nextChar))
					{
						stream->Seek(-1, SeekMode::Current);
						break;
					}
					if (nextChar == '.' && periodFound)
						break;
					number.append({ nextChar });
					if (nextChar == '.')
						periodFound = true;
					nextChar = stream->Read();
				}

				outToken = Token{ periodFound ? TK_LITERAL_FLOAT : TK_LITERAL_INTEGER, number };
				return true;
			}
		}
			break;
		}


		return false;
    }
} // namespace CE
