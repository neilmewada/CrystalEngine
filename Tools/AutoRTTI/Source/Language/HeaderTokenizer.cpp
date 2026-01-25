
#include "HeaderTokenizer.h"

#include <regex>

namespace CE
{
    std::unordered_map<std::string, TokenType> HeaderTokenizer::tokenTypeMap = {
        { "public", TK_ACCESS_SPECIFIER},
        { "private", TK_ACCESS_SPECIFIER },
        { "protected", TK_ACCESS_SPECIFIER },
        { "auto", TK_KW_AUTO },
        { "const", TK_KW_CONST },
        { "if", TK_KW_IF },
        { "else", TK_KW_ELSE },
        { "elif", TK_KW_ELIF },
        { "endif", TK_KW_ENDIF },
        { "class", TK_KW_CLASS },
        { "struct", TK_KW_STRUCT },
        { "enum", TK_KW_ENUM },
        { "CE_CLASS", TK_CE_CLASS_BODY },
        { "CE_STRUCT", TK_CE_STRUCT_BODY },
        { "CLASS", TK_CE_CLASS },
        { "STRUCT", TK_CE_STRUCT },
        { "ENUM", TK_CE_ENUM },
        { "ECONST", TK_CE_ECONST },
        { "FIELD", TK_CE_FIELD },
        { "FUNCTION", TK_CE_FUNCTION },
        { "FUSION_PROPERTY", TK_FUSION_PROPERTY },
        { "FUSION_DATA_LAYOUT_PROPERTY", TK_FUSION_DATA_LAYOUT_PROPERTY },
        { "FUSION_DATA_PROPERTY", TK_FUSION_DATA_PROPERTY },
        { "FUSION_LAYOUT_PROPERTY", TK_FUSION_LAYOUT_PROPERTY },
        { "FUSION_EVENT", TK_FUSION_EVENT },
        { "MODEL_PROPERTY", TK_MODEL_PROPERTY },
        { "MODEL_PROPERTY_EDITABLE", TK_MODEL_PROPERTY_EDITABLE },
        { "true", TK_KW_TRUE },
        { "false", TK_KW_FALSE },
        { "nullptr", TK_KW_NULLPTR },
        { "inline", TK_KW_INLINE },
        { "virtual", TK_KW_VIRTUAL },
        { "override", TK_KW_OVERRIDE },
        { "typedef", TK_KW_TYPEDEF },
        { "template", TK_KW_TEMPLATE },
        { "typename", TK_KW_TYPENAME },
        { "namespace", TK_KW_NAMESPACE },
        { "using", TK_KW_USING },
        { "final", TK_KW_FINAL },
		{ "alignas", TK_KW_ALIGNAS },
		{ "alignof", TK_KW_ALIGNOF },
        // built-in types
        { "void", TK_KW_VOID },
        { "int", TK_KW_INT },
        { "short", TK_KW_SHORT },
        { "long", TK_KW_LONG },
        { "char", TK_KW_CHAR },
        { "Char", TK_KW_CHAR },
        { "float", TK_KW_FLOAT },
        { "f32", TK_KW_FLOAT },
        { "double", TK_KW_DOUBLE },
        { "f64", TK_KW_DOUBLE },
        { "bool", TK_KW_BOOL },
        { "signed", TK_KW_SIGNED },
        { "unsigned", TK_KW_UNSIGNED },
        // Custom types
        { "u8", TK_KW_UINT8 },
        { "u16", TK_KW_UINT16 },
        { "u32", TK_KW_UINT32 },
        { "u64", TK_KW_UINT64 },
        { "s8", TK_KW_INT8 },
        { "s16", TK_KW_INT16 },
        { "s32", TK_KW_INT32 },
        { "s64", TK_KW_INT64 },
        { "String", TK_KW_STRING },
        { "Array", TK_KW_ARRAY },
        { "Vec2", TK_KW_VEC2 },
        { "Vec3", TK_KW_VEC3 },
        { "Vec4", TK_KW_VEC4 },
    };

    inline bool IsAlphabet(char Character)
    {
        return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z');
    }

    inline bool IsAlphaNumeric(char Character)
    {
        return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') ||
            (Character >= '0' && Character <= '9');
    }

    inline bool IsNumeric(char Character)
    {
        return (Character >= '0' && Character <= '9');
    }

    HashMap<IO::Path, HeaderTokenizer*> HeaderTokenizer::headerTokens{};

    HeaderTokenizer* HeaderTokenizer::Tokenize(IO::Path headerFilePath, const Array<IO::Path>& includeSearchPaths)
    {
        if (headerTokens.KeyExists(headerFilePath))
            return headerTokens[headerFilePath];

        HeaderTokenizer* self = new HeaderTokenizer;
        headerTokens.Add({ headerFilePath, self });

        self->headerPath = headerFilePath;

        std::ifstream file{ (fs::path)headerFilePath };
        std::stringstream stringStream;
        stringStream << file.rdbuf();
        file.close();

        auto source = stringStream.str();
        auto length = source.length();

        int cursor = 0;
        int curLine = 0;
        int positionInLine = 0;

        // Regular Expressions
        std::regex identifierRegex = std::regex("[a-zA-Z_][a-zA-Z0-9_]*");

        while (cursor < length) // Changed <= to <
        {
            char c = source[cursor];
            std::string_view substring(source.c_str() + cursor, length - cursor - 1);

            switch (source[cursor])
            {
            case ' ': // No token (whitespace)
                break;
            case '.':
                self->AddToken(TK_PERIOD, curLine, ".");
                break;
            case '#':
                self->AddToken(TK_POUNDSIGN, curLine, "#");
                break;
            case ':':
                if (cursor < length - 1 && source[cursor + 1] == ':')
                {
                    self->AddToken(TK_SCOPE_OPERATOR, curLine, "::");
                    cursor++;
                }
                else
                {
                    self->AddToken(TK_COLON, curLine);
                }
                break;
            case ';':
                self->AddToken(TK_SEMICOLON, curLine, ";");
                break;
            case ',':
                self->AddToken(TK_COMMA, curLine, ",");
                break;
            case '\\':
                self->AddToken(TK_BACKSLASH, curLine, "\\");
                break;
            case '/':
            {
                if (cursor < length - 1 && source[cursor + 1] == '/') // Comment: // ... \n
                {
                    int endCursor = cursor;
                    while (endCursor < length && source[endCursor] != '\n')
                    {
                        endCursor++;
                    }
                    self->AddToken(TK_COMMENT, curLine, std::string(source.begin() + cursor, source.begin() + endCursor));

                    cursor = endCursor;
                }
                else if (cursor < length - 1 && source[cursor + 1] == '*') // Comment: /* .... */
                {
                    int endCursor = cursor;
                    while (endCursor < length - 2 && source[endCursor] != '*' && source[endCursor + 1] != '/')
                    {
                        endCursor++;
                    }
                    self->AddToken(TK_COMMENT, curLine, std::string(source.begin() + cursor, source.begin() + endCursor));

                    cursor = endCursor;
                }
                else // Just a forward slash
                {
                    self->AddToken(TK_FORWARDSLASH, curLine);
                }
            }
                break;
            case '{':
                self->AddToken(TK_SCOPE_OPEN, curLine);
                break;
            case '}':
                self->AddToken(TK_SCOPE_CLOSE, curLine);
                break;
            case '(':
                self->AddToken(TK_PAREN_OPEN, curLine);
                break;
            case ')':
                self->AddToken(TK_PAREN_CLOSE, curLine);
                break;
            case '[':
                self->AddToken(TK_SQUARE_BRACE_OPEN, curLine);
                break;
            case ']':
                self->AddToken(TK_SQUARE_BRACE_CLOSE, curLine);
                break;
            case '<':
                self->AddToken(TK_LEFTANGLE, curLine, "<");
                break;
            case '>':
                self->AddToken(TK_RIGHTANGLE, curLine, ">");
                break;
            case '*':
                self->AddToken(TK_ASTERISK, curLine, "*");
                break;
            case '+':
            case '-':
            case '%':
            case '=':
            case '~':
            case '!':
            {
                if (cursor < length - 1 && source[cursor] == source[cursor + 1] && (c == '+' || c == '-' || c == '='))
                {
                    if (source[cursor + 1] == '=') // ==
                    {
                        self->AddToken(TK_LOGICAL_OPERATOR, curLine, "==");
                        cursor++;
                        break;
                    }
                    else if (source[cursor + 1] == '+') // ++
                    {
                        self->AddToken(TK_ARTIHMETIC_OPERATOR, curLine, "++");
                        cursor++;
                        break;
                    }
                    else if (source[cursor + 1] == '-') // --
                    {
                        self->AddToken(TK_ARTIHMETIC_OPERATOR, curLine, "--");
                        cursor++;
                        break;
                    }
                }
                else if (c == '!')
                {
                    self->AddToken(TK_LOGICAL_OPERATOR, curLine, "!");
                    break;
                }
                else if (c == '=')
                {
                    self->AddToken(TK_ASSIGNMENT_OPERATOR, curLine, "=");
                    break;
                }

                self->AddToken(TK_ARTIHMETIC_OPERATOR, curLine, std::to_string(c));
            }
                break;
            case '&':
            case '|':
            {
                int endCursor = cursor + 1;
                if (endCursor < length && source[endCursor] == c) // &&, ||
                {
                    self->AddToken(TK_LOGICAL_OPERATOR, curLine, std::string(source.begin() + cursor, source.begin() + endCursor + 1));
                    cursor = endCursor;
                }
                else if (c == '&')
                {
                    self->AddToken(TK_AMPERSAND, curLine, "&");
                }
                else // |
                {
                    self->AddToken(TK_ARTIHMETIC_OPERATOR, curLine, std::to_string(c));
                }
            }
                break;
            case '\'': // Char Literal
            {
                int endCursor = cursor + 1;
                while (endCursor < length && source[endCursor] != '\'' && source[endCursor] != '\n')
                {
                    endCursor++;
                }
                if (source[endCursor] == '\'')
                {
                    self->AddToken(TK_LITERAL_CHAR, curLine, std::string(source.begin() + cursor + 1, source.begin() + endCursor));
                }

                cursor = endCursor;
            }
                break;
            case '\"': // String Literal
            {
                int endCursor = cursor + 1;
                while (endCursor < length && source[endCursor] != '\"' && source[endCursor] != '\n')
                {
                    endCursor++;
                }
                if (source[endCursor] == '\"')
                {
                    self->AddToken(TK_LITERAL_STRING, curLine, std::string(source.begin() + cursor + 1, source.begin() + endCursor));

                    if (self->GetTokenFromEnd(1).type == TK_KW_INCLUDE && self->GetTokenFromEnd(2).type == TK_POUNDSIGN)
                    {
                        self->headerIncludes.emplace_back(source.begin() + cursor + 1, source.begin() + endCursor);
                    }
                }
                cursor = endCursor;
            }
                break;
            default:
            {
                int endCursor = cursor;

                if (!IsAlphabet(c) && c != '_') // NOT an identifier
                {
                    if (IsNumeric(c)) // If a numeric character
                    {
                        while (endCursor < length && (source[endCursor] == '.' || IsNumeric(source[endCursor]) || IsAlphabet(source[endCursor])))
                        {
                            endCursor++;
                        }
                        if (endCursor > cursor)
                            endCursor--;

                        self->AddToken(TK_LITERAL_NUMBER, curLine, std::string(source.begin() + cursor, source.begin() + endCursor + 1));

                        cursor = endCursor;
                    }

                    break;
                }

                // Continue ONLY if it's an Alphabet or underscore character

                while (endCursor < length && (IsAlphaNumeric(source[endCursor]) || source[endCursor] == '_')) // Wait till identifier/keyword is valid
                {
                    endCursor++;
                }
                if (endCursor > cursor)
                    endCursor--;

                std::string_view tokenView(source.c_str() + cursor, endCursor - cursor + 1);
                std::string TokenString = std::string(tokenView);

                // Check for keywords first
                if (tokenTypeMap.count(TokenString) > 0)
                {
                    auto type = tokenTypeMap[TokenString];
                    self->AddToken(type, curLine, TokenString);
                }
                else if (tokenView == "include" && self->GetLastToken().type == TK_POUNDSIGN)
                {
                    self->AddToken(TK_KW_INCLUDE, curLine, TokenString);
                }
                else if (tokenView == "define" && self->GetLastToken().type == TK_POUNDSIGN) // Macro definition
                {
                    self->AddToken(TK_KW_DEFINE, curLine, TokenString);
                }
                else if (tokenView == "defined" && (self->GetLastToken().type == TK_KW_IF))
                {
                    self->AddToken(TK_KW_DEFINED, curLine, TokenString);
                }
                else if (std::regex_match(TokenString, identifierRegex)) // Identifier
                {
                    self->AddToken(TK_IDENTIFIER, curLine, TokenString);
                }
                else // Unknown token
                {
                    self->AddToken(TK_UNKNOWN, curLine, TokenString);
                }

                cursor = endCursor;
            }
                break;
            }

            positionInLine++;

            if (c == '\n') // New line
            {
                self->AddToken(TK_NEWLINE, curLine);
                curLine++;
                positionInLine = 0;
            }

            cursor++;
        }

        //for (const auto& headerInclude : self->headerIncludes)
        //{
        //    fs::path includedHeaderRelativePath = headerInclude;

        //    for (const auto& includeDir : includeSearchPaths)
        //    {
        //        fs::path includeDirPath = includeDir;

        //        if (fs::exists(includeDirPath / includedHeaderRelativePath)) // Found the header
        //        {
        //            Tokenize(includeDirPath / includedHeaderRelativePath, includeSearchPaths);
        //            break;
        //        }
        //    }
        //}

        return self;
    }

    void HeaderTokenizer::AddToken(TokenType type, int line, std::string lexeme)
    {
        tokens.Add({line, type, lexeme});
    }

} // namespace CE

