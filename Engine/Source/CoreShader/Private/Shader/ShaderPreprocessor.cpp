#include "CoreShader.h"

#include <regex>

namespace CE
{

	ShaderPreprocessor::ShaderPreprocessor(Stream* stream, const Array<IO::Path>& includePaths)
		: stream(stream), includePaths(includePaths)
	{
		
	}

	ShaderPreprocessor::~ShaderPreprocessor()
	{

	}

	void ShaderPreprocessor::PreprocessShader(ShaderPreprocessData* preprocessData)
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
			return;
		}

		tokens.Clear();

		Array<Token> tokens{};

		bool success = Tokenize(tokens);
		if (!success)
			return;

		scopeStack.Clear();
		allScopeStack.Clear();
		curScope = SCOPE_NONE;

		int passIndex = 0;
		int hitGroupIndex = 0;

		defer(&)
		{
			passSources.Clear();
		};

		ShaderPropertyEntry currentProperty{};
		int defaultValueIdx = 0;
		bool isPropertyAttrib = false;
		bool isPropertyAttribInsideParen = false;

		for (int i = 0; i < tokens.GetSize(); i++)
		{
			ScopeType lastScope = SCOPE_NONE;
			if (scopeStack.NotEmpty())
				lastScope = scopeStack.Top();

			const Token& curToken = tokens[i];
			Token nextToken = {};
			Token lastToken = {};
			if (i > 0)
				lastToken = tokens[i - 1];
			if (i < tokens.GetSize() - 1)
				nextToken = tokens[i + 1];

			if (curToken.token == TK_SCOPE_OPEN)
			{
				if (curScope != SCOPE_NONE)
					scopeStack.Push(curScope);
				allScopeStack.Push(curScope);
				curScope = SCOPE_NONE;
			}
			else if (curToken.token == TK_SCOPE_CLOSE)
			{
				if (allScopeStack.NotEmpty())
				{
					if (allScopeStack.Top() != SCOPE_NONE)
					{
						scopeStack.Pop();
					}
					allScopeStack.Pop();
				}
				else
				{
					errorMessage = "Mismatching scope bracket '}' found";
					return;
				}
			}
			else if (lastScope == SCOPE_PROPERTIES) // Parse properties
			{
				if (curToken.token == TK_SQUARE_OPEN && !isPropertyAttrib)
				{
					isPropertyAttrib = true;
				}
				if (curToken.token == TK_SQUARE_CLOSE && isPropertyAttrib)
				{
					isPropertyAttrib = false;
				}
				else if (isPropertyAttrib)
				{
					if (currentProperty.attributes.IsEmpty())
						currentProperty.attributes.Add("");

					if (curToken.token == TK_IDENTIFIER || curToken.token == TK_LITERAL_INTEGER || curToken.token == TK_LITERAL_FLOAT ||
						curToken.token == TK_LITERAL_STRING)
					{
						currentProperty.attributes.Top() += curToken.lexeme;
					}
					else if (curToken.token == TK_COMMA)
					{
						currentProperty.attributes.Add("");
					}
				}
				else if (curToken.token == TK_IDENTIFIER && !currentProperty.name.IsValid())
				{
					currentProperty.name = curToken.lexeme;
					defaultValueIdx = 0;
				}
				else if (curToken.token == TK_LITERAL_STRING && !currentProperty.displayName.IsValid())
				{
					currentProperty.displayName = curToken.lexeme;
					defaultValueIdx = 0;
				}
				else if (curToken.token == TK_IDENTIFIER && currentProperty.name.IsValid() && 
					currentProperty.displayName.IsValid() && 
					currentProperty.propertyType == ShaderPropertyType::None)
				{
					defaultValueIdx = 0;
					if (curToken.lexeme == "Float")
					{
						currentProperty.propertyType = ShaderPropertyType::Float;
					}
					else if (curToken.lexeme == "UInt")
					{
						currentProperty.propertyType = ShaderPropertyType::UInt;
					}
					else if (curToken.lexeme == "Int")
					{
						currentProperty.propertyType = ShaderPropertyType::Int;
					}
					else if (curToken.lexeme == "Color")
					{
						currentProperty.propertyType = ShaderPropertyType::Color;
					}
					else if (curToken.lexeme == "Vector")
					{
						currentProperty.propertyType = ShaderPropertyType::Vector;
					}
					else if (curToken.lexeme == "Tex2D")
					{
						currentProperty.propertyType = ShaderPropertyType::Tex2D;
					}
					else if (curToken.lexeme == "Tex3D")
					{
						currentProperty.propertyType = ShaderPropertyType::Tex3D;
					}
					else if (curToken.lexeme == "TexCube")
					{
						currentProperty.propertyType = ShaderPropertyType::TexCube;
					}
					else
					{
						errorMessage = "Invalid property type: " + curToken.lexeme;
						return;
					}
				}
				else if (curToken.token == TK_LITERAL_STRING)
				{
					currentProperty.defaultStringValue = curToken.lexeme;
				}
				else if (curToken.token == TK_LITERAL_INTEGER)
				{
					int intValue = 0;
					if (String::TryParse(curToken.lexeme, intValue))
					{
						if (defaultValueIdx == 0)
						{
							currentProperty.defaultFloatValue = intValue;
							currentProperty.defaultVectorValue.x = intValue;
							defaultValueIdx++;
						}
						else if (defaultValueIdx == 1)
						{
							currentProperty.defaultVectorValue.y = intValue;
							defaultValueIdx++;
						}
						else if (defaultValueIdx == 2)
						{
							currentProperty.defaultVectorValue.z = intValue;
							defaultValueIdx++;
						}
						else if (defaultValueIdx == 3)
						{
							currentProperty.defaultVectorValue.w = intValue;
							defaultValueIdx++;
						}
					}
				}
				else if (curToken.token == TK_LITERAL_FLOAT)
				{
					f32 floatValue = 0;
					if (String::TryParse(curToken.lexeme, floatValue))
					{
						if (defaultValueIdx == 0)
						{
							currentProperty.defaultFloatValue = floatValue;
							currentProperty.defaultVectorValue.x = floatValue;
							defaultValueIdx++;
						}
						else if (defaultValueIdx == 1)
						{
							currentProperty.defaultVectorValue.y = floatValue;
							defaultValueIdx++;
						}
						else if (defaultValueIdx == 2)
						{
							currentProperty.defaultVectorValue.z = floatValue;
							defaultValueIdx++;
						}
						else if (defaultValueIdx == 3)
						{
							currentProperty.defaultVectorValue.w = floatValue;
							defaultValueIdx++;
						}
					}
				}
				else if (curToken.token == TK_NEWLINE)
				{
					if (currentProperty.IsValid())
					{
						preprocessData->properties.Add(currentProperty);
						currentProperty = {};
					}
					defaultValueIdx = 0;
				}
			}
			else if (lastScope == SCOPE_NONE && curToken.token == TK_KW_SHADER)
			{
				curScope = SCOPE_SHADER;
			}
			else if (curToken.token == TK_LITERAL_STRING && curScope == SCOPE_SHADER)
			{
				preprocessData->shaderName = curToken.lexeme;
			}
			else if (lastScope == SCOPE_SHADER && curToken.token == TK_KW_PROPERTIES)
			{
				curScope = SCOPE_PROPERTIES;
			}
			else if (lastScope == SCOPE_SHADER && curToken.token == TK_KW_SUBSHADER)
			{
				preprocessData->subShaders.Add({});
				curScope = SCOPE_SUBSHADER;
			}
			else if ((lastScope == SCOPE_SUBSHADER || lastScope == SCOPE_PASS || lastScope == SCOPE_HITGROUP) && curToken.token == TK_KW_TAGS)
			{
				curScope = SCOPE_TAGS;
			}
			else if (lastScope == SCOPE_SUBSHADER)
			{
				if (curToken.token == TK_KW_PASS)
				{
					preprocessData->subShaders.Top().passes.Add({});
					curScope = SCOPE_PASS;
				}
				else if (curToken.token == TK_KW_HITGROUP)
				{
					preprocessData->subShaders.Top().hitGroups.Add({});
					curScope = SCOPE_HITGROUP;
				}
				else if (curToken.token == TK_KW_ZTEST && i < tokens.GetSize() - 1)
				{
					Token next = tokens[i + 1];
					if (next.token == TK_IDENTIFIER)
					{
						ShaderTagEntry tagEntry{};
						tagEntry.key = "ZTest";
						tagEntry.value = next.lexeme;
						preprocessData->subShaders.Top().subShaderTags.Add(tagEntry);
					}
				}
				else if (curToken.token == TK_KW_ZWRITE && i < tokens.GetSize() - 1)
				{
					Token next = tokens[i + 1];
					if (next.token == TK_IDENTIFIER)
					{
						ShaderTagEntry tagEntry{};
						tagEntry.key = "ZWrite";
						tagEntry.value = next.lexeme;
						preprocessData->subShaders.Top().subShaderTags.Add(tagEntry);
					}
				}
				else if (curToken.token == TK_KW_CULL && i < tokens.GetSize() - 1)
				{
					Token next = tokens[i + 1];
					if (next.token == TK_IDENTIFIER)
					{
						ShaderTagEntry tagEntry{};
						tagEntry.key = "Cull";
						tagEntry.value = next.lexeme;
						preprocessData->subShaders.Top().subShaderTags.Add(tagEntry);
					}
				}
			}
			else if (lastScope == SCOPE_TAGS)
			{
				auto parentScope = scopeStack[scopeStack.GetSize() - 2];
				if (parentScope == SCOPE_SUBSHADER)
				{
					if (curToken.token == TK_LITERAL_STRING)
					{
						ShaderTagEntry tag{};
						tag.key = curToken.lexeme;

						Token next{};
						while (i < tokens.GetSize() && next.token != TK_LITERAL_STRING)
						{
							i++;
							next = tokens[i];
						}

						if (next.token == TK_LITERAL_STRING)
						{
							tag.value = next.lexeme;
							preprocessData->subShaders.Top().subShaderTags.Add(tag);
						}
					}
				}
				else if (parentScope == SCOPE_PASS)
				{
					if (curToken.token == TK_LITERAL_STRING)
					{
						ShaderTagEntry tag{};
						tag.key = curToken.lexeme;

						Token next{};
						while (i < tokens.GetSize() && next.token != TK_LITERAL_STRING)
						{
							i++;
							next = tokens[i];
						}

						if (next.token == TK_LITERAL_STRING)
						{
							tag.value = next.lexeme;
							preprocessData->subShaders.Top().passes.Top().passTags.Add(tag);

							if (tag.key == "Vertex")
								preprocessData->subShaders.Top().passes.Top().vertexEntry = "VertMain";
							else if (tag.key == "Fragment")
								preprocessData->subShaders.Top().passes.Top().fragmentEntry = "FragMain";
						}
					}
				}
				else if (parentScope == SCOPE_HITGROUP)
				{
					if (curToken.token == TK_LITERAL_STRING)
					{
						ShaderTagEntry tag{};
						tag.key = curToken.lexeme;

						Token next{};
						while (i < tokens.GetSize() && next.token != TK_LITERAL_STRING)
						{
							i++;
							next = tokens[i];
						}

						if (next.token == TK_LITERAL_STRING)
						{
							tag.value = next.lexeme;
							preprocessData->subShaders.Top().hitGroups.Top().passTags.Add(tag);

							if (tag.key == "ClosestHit")
								preprocessData->subShaders.Top().hitGroups.Top().closestHitEntry = tag.value;
							else if (tag.key == "AnyHit")
								preprocessData->subShaders.Top().hitGroups.Top().anyHitEntry = tag.value;
						}
					}
				}
			}
			else if (lastScope == SCOPE_PASS)
			{
				if (curToken.token == TK_IDENTIFIER && curToken.lexeme == "Name")
				{
					Token next{};
					while (i < tokens.GetSize() && next.token != TK_LITERAL_STRING)
					{
						i++;
						next = tokens[i];
					}

					if (next.token == TK_LITERAL_STRING)
					{
						preprocessData->subShaders.Top().passes.Top().passName = next.lexeme;
					}
				}
				else if (curToken.token == TK_KW_ZTEST && i < tokens.GetSize() - 1)
				{
					Token next = tokens[i + 1];
					if (next.token == TK_IDENTIFIER)
					{
						ShaderTagEntry tagEntry{};
						tagEntry.key = "ZTest";
						tagEntry.value = next.lexeme;
						preprocessData->subShaders.Top().passes.Top().passTags.Add(tagEntry);
					}
				}
				else if (curToken.token == TK_KW_ZWRITE && i < tokens.GetSize() - 1)
				{
					Token next = tokens[i + 1];
					if (next.token == TK_IDENTIFIER)
					{
						ShaderTagEntry tagEntry{};
						tagEntry.key = "ZWrite";
						tagEntry.value = next.lexeme;
						preprocessData->subShaders.Top().passes.Top().passTags.Add(tagEntry);
					}
				}
				else if (curToken.token == TK_KW_CULL && i < tokens.GetSize() - 1)
				{
					Token next = tokens[i + 1];
					if (next.token == TK_IDENTIFIER)
					{
						ShaderTagEntry tagEntry{};
						tagEntry.key = "Cull";
						tagEntry.value = next.lexeme;
						preprocessData->subShaders.Top().passes.Top().passTags.Add(tagEntry);
					}
				}
				else if (curToken.token == TK_HLSLPROGRAM)
				{
					BinaryBlob& passSource = passSources[passIndex];
					preprocessData->subShaders.Top().passes.Top().source = passSource;
					preprocessData->subShaders.Top().passes.Top().features.AddRange(passPreprocessData[passIndex].features);
					passIndex++;
				}
			}
			else if (lastScope == SCOPE_HITGROUP)
			{
				if (curToken.token == TK_IDENTIFIER && curToken.lexeme == "Name")
				{
					Token next{};
					while (i < tokens.GetSize() && next.token != TK_LITERAL_STRING)
					{
						i++;
						next = tokens[i];
					}

					if (next.token == TK_LITERAL_STRING)
					{
						preprocessData->subShaders.Top().hitGroups.Top().passName = next.lexeme;
					}
				}
				else if (curToken.token == TK_HLSLPROGRAM)
				{
					BinaryBlob& source = hitGroupSources[hitGroupIndex];
					preprocessData->subShaders.Top().hitGroups.Top().source = source;
					//preprocessData->subShaders.Top().hitGroups.Top().features.AddRange(passPreprocessData[hitGroupIndex].features);
					hitGroupIndex++;
				}
			}
		}

		return;
	}

	bool ShaderPreprocessor::Tokenize(Array<Token>& tokens)
	{
		passSources.Clear();
		hitGroupSources.Clear();

		while (!stream->IsOutOfBounds())
		{
			Token token{};
			bool valid = ReadNextToken(token);

			if (scopeStack.NotEmpty() && scopeStack.GetLast() == SCOPE_PASS)
			{
				if (token.token == TK_HLSLPROGRAM && curPassSource != nullptr)
				{
					auto data = curPassSource->GetRawDataPtr();
					auto dataSize = curPassSource->GetLength();
					passSources.Push({});
					BinaryBlob& blob = passSources.Top();
					blob.LoadData(data, dataSize);

					delete curPassSource;
					curPassSource = nullptr;
				}
			}
			else if (scopeStack.NotEmpty() && scopeStack.GetLast() == SCOPE_HITGROUP)
			{
				if (token.token == TK_HLSLPROGRAM && curPassSource != nullptr)
				{
					auto data = curPassSource->GetRawDataPtr();
					auto dataSize = curPassSource->GetLength();
					hitGroupSources.Push({});
					BinaryBlob& blob = hitGroupSources.Top();
					blob.LoadData(data, dataSize);

					delete curPassSource;
					curPassSource = nullptr;
				}
			}

			if (valid)
			{
				tokens.Add(token);
				prevToken = token;
			}
		}

		return true;
	}

	bool ShaderPreprocessor::ReadNextToken(Token& outToken)
	{
		if (stream->IsOutOfBounds())
			return false;

		char c = stream->Read();

		ScopeType lastScope = SCOPE_NONE;
		if (scopeStack.NotEmpty())
			lastScope = scopeStack.Top();

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
			if (curScope != SCOPE_NONE)
			{
				scopeStack.Push(curScope);
				allScopeStack.Push(curScope);
				curScope = SCOPE_NONE;
			}
			else
			{
				allScopeStack.Push(SCOPE_NONE);
			}
			outToken = Token{ TK_SCOPE_OPEN };
			return true;
		case '}':
			if (allScopeStack.NotEmpty())
			{
				auto top = allScopeStack.Top();
				if (top != SCOPE_NONE)
				{
					scopeStack.Pop();
				}
				allScopeStack.Pop();
			}
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

				if (curScope == SCOPE_NONE && identifier == "Shader")
				{
					curScope = SCOPE_SHADER;
					outToken = Token{ TK_KW_SHADER, identifier };
					return true;
				}
				else if (lastScope == SCOPE_SHADER && identifier == "Properties")
				{
					curScope = SCOPE_PROPERTIES;
					outToken = Token{ TK_KW_PROPERTIES, identifier };
					return true;
				}
				else if (lastScope == SCOPE_SHADER && identifier == "SubShader")
				{
					curScope = SCOPE_SUBSHADER;
					outToken = Token{ TK_KW_SUBSHADER, identifier };
					return true;
				}
				else if ((lastScope == SCOPE_SUBSHADER || lastScope == SCOPE_PASS || lastScope == SCOPE_HITGROUP) && identifier == "Tags")
				{
					curScope = SCOPE_TAGS;
					outToken = Token{ TK_KW_TAGS, identifier };
					return true;
				}
				else if (lastScope == SCOPE_SUBSHADER && identifier == "Pass")
				{
					curScope = SCOPE_PASS;
					outToken = Token{ TK_KW_PASS, identifier };
					return true;
				}
				else if (lastScope == SCOPE_SUBSHADER && identifier == "HitGroup")
				{
					curScope = SCOPE_HITGROUP;
					outToken = Token{ TK_KW_HITGROUP, identifier };
					return true;
				}
				else if ((lastScope == SCOPE_SUBSHADER || lastScope == SCOPE_PASS) && identifier == "ZTest")
				{
					outToken = Token{ TK_KW_ZTEST, identifier };
					return true;
				}
				else if ((lastScope == SCOPE_SUBSHADER || lastScope == SCOPE_PASS) && identifier == "ZWrite")
				{
					outToken = Token{ TK_KW_ZWRITE, identifier };
					return true;
				}
				else if ((lastScope == SCOPE_SUBSHADER || lastScope == SCOPE_PASS) && identifier == "Cull")
				{
					outToken = Token{ TK_KW_CULL, identifier };
					return true;
				}
				else if (lastScope == SCOPE_PASS && identifier == "HLSLPROGRAM")
				{
					curScope = SCOPE_HLSLPROGRAM;
					outToken = Token{ TK_HLSLPROGRAM, identifier };
					delete curPassSource; curPassSource = nullptr;
					curPassPreprocess = {};
					curPassSource = new MemoryStream(1024);
					curPassSource->SetAutoResizeIncrement(1024);
					curPassSource->SetAsciiMode(true);
					bool result = ReadHLSLProgram();
					return true;
				}
				else if (lastScope == SCOPE_HITGROUP && identifier == "HLSLPROGRAM")
				{
					curScope = SCOPE_HLSLPROGRAM;
					outToken = Token{ TK_HLSLPROGRAM, identifier };
					delete curPassSource; curPassSource = nullptr;
					curPassPreprocess = {};
					curPassSource = new MemoryStream(1024);
					curPassSource->SetAutoResizeIncrement(1024);
					curPassSource->SetAsciiMode(true);
					ReadHLSLProgram();
					return true;
				}
				else
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

	bool ShaderPreprocessor::ReadHLSLProgram()
	{
		if (stream->IsOutOfBounds())
			return false;

		Array<IO::Path> alreadyIncludedFiles{};

		bool result = PreprocessHLSL(stream, curPassSource, alreadyIncludedFiles);
		curPassSource->Write('\0'); // Null terminator

		return result;
	}

	bool ShaderPreprocessor::PreprocessHLSL(Stream* stream, MemoryStream* outStream, Array<IO::Path>& alreadyIncludedFiles, const IO::Path& additionalIncludePath)
	{
		if (stream->IsOutOfBounds())
			return false;

		Array<IO::Path> includeSearchPaths = includePaths;
		includeSearchPaths.Add(additionalIncludePath);

		char next = 0;
		char prev = 0;

		while (!stream->IsOutOfBounds())
		{
			next = stream->Read();
			
			if (!String::IsAlphabet(prev) && prev != '_' && next == 'E' && !stream->IsOutOfBounds())
			{
				auto curPos = stream->GetCurrentPosition();
				char next2 = 0;

				String string = "E";

				while (!stream->IsOutOfBounds())
				{
					next2 = stream->Read();
					if (String::IsAlphabet(next2))
					{
						string.Append(next2);
					}
					else
					{
						stream->Seek(-1, SeekMode::Current);
						break;
					}
				}
				
				if (string == "ENDHLSL")
				{
					passPreprocessData.Add(curPassPreprocess);
					curPassPreprocess = {};
					break;
				}

				stream->Seek(curPos, SeekMode::Begin);
			}
			else if (next == '/' && !stream->IsOutOfBounds()) // Check for comment
			{
				auto next2 = stream->Read();
				if (next2 == '*') // Multi-line comment
				{
					while (!stream->IsOutOfBounds())
					{
						next2 = stream->Read();
						if (next2 == '*' && !stream->IsOutOfBounds() && stream->Read() == '/')
						{
							break; // End of multi-line comment
						}
					}
					continue;
				}
				else if (next2 == '/') // Single-line comment
				{
					while (!stream->IsOutOfBounds())
					{
						next2 = stream->Read();
						if (next2 == '\n')
						{
							break; // End of single-line comment
						}
					}
					continue;
				}
				else // NOT a comment, go back before next2 character
				{
					stream->Seek(-1, SeekMode::Current);
				}
			}
			else if (next == '#' && !stream->IsOutOfBounds()) // Check for #include or #pragma
			{
				auto curPos = stream->GetCurrentPosition();
				char next2 = 0;
				bool shouldContinue = false;

				while (!stream->IsOutOfBounds())
				{
					next2 = stream->Read();
					if (next2 == ' ')
						continue;
					if (String::IsAlphabet(next2))
					{
						String keyword = "";

						while (String::IsAlphabet(next2))
						{
							keyword.Append(next2);
							next2 = stream->Read();
						}

						stream->Seek(-1, SeekMode::Current);

						if (keyword == "include")
						{
							bool isStringLiteral = false;
							String stringLiteral = "";

							while (!stream->IsOutOfBounds())
							{
								next2 = stream->Read();
								if (!isStringLiteral && next2 == ' ')
									continue;
								if (next2 == '\"')
								{
									isStringLiteral = !isStringLiteral;
									if (!isStringLiteral)
										break;
								}
								else if (isStringLiteral)
								{
									stringLiteral.Append(next2);
								}
							}

							for (const auto& includePath : includeSearchPaths)
							{
								auto includeFile = includePath / stringLiteral;
								if (includeFile.Exists() && !includeFile.IsDirectory())
								{
									FileStream fileReader = FileStream(includeFile, Stream::Permissions::ReadOnly);
									fileReader.SetAsciiMode(true);
									shouldContinue = true;
									if (alreadyIncludedFiles.Exists(includeFile)) // If we have already include'ed this file
										break;
									alreadyIncludedFiles.Add(includeFile);
									PreprocessHLSL(&fileReader, outStream, alreadyIncludedFiles, includeFile.GetParentPath());
									break;
								}
							}

							break;
						}
						else if (keyword == "pragma")
						{
							String value = "";

							while (!stream->IsOutOfBounds())
							{
								next2 = stream->Read();
								if (next2 == '\n')
									break;
								if (next2 != '\r')
									value.Append(next2);
							}

							Array<String> splits = value.Split(' ');

							if (splits.GetSize() >= 2 && splits[0] == "shader_feature")
							{
								curPassPreprocess.features.Add(splits[1]);
							}

							shouldContinue = true;
						}

						break;
					}
				}

				if (shouldContinue)
					continue;

				stream->Seek(curPos, SeekMode::Begin);
			}

			prev = next;
			outStream->Write(next);
		}

		return true;
	}

	bool ShaderPropertyEntry::IsValid()
	{
		return name.IsValid() && displayName.IsValid() && propertyType != ShaderPropertyType::None;
	}

} // namespace CE
