
#include "HeaderAST.h"

#include <stack>

namespace CE
{
	HeaderAST::HeaderAST()
	{

	}

	HeaderAST::~HeaderAST()
	{
		
	}

    HeaderAST* HeaderAST::ProcessHeader(HeaderTokenizer* tokens)
    {
        auto ast = new HeaderAST;
        
        ClassInfo* curClass = nullptr;
        StructInfo* curStruct = nullptr;
        EnumInfo* curEnum = nullptr;
        int curClassScope = 0;
        int curStructScope = 0;
        
        int curScope = 0;
        int curParenthesis = 0;
        bool isCEClassBody = false;
        auto size = tokens->tokens.GetSize();
        
        // namespace
        struct NamespaceInfo
        {
            Name name{};
            int curScope = 0;
        };
        
        std::stack<NamespaceInfo> curNamespace{};
        bool namespaceNameInProgress = false;
        
        for (int i = 0; i < size; i++)
        {
            const auto& token = tokens->tokens[i];
            TokenType prevToken = TK_WHITESPACE;
            if (i > 0)
                prevToken = tokens->tokens[i - 1].type;
            
            if (token.type == TK_KW_NAMESPACE && prevToken != TK_KW_USING) // Make sure it is a valid namespace entry
            {
                int fwdIdx = i;
                bool isValidNamespace = true;
                while (fwdIdx < size)
                {
                    if (tokens->tokens[fwdIdx].type == TK_SEMICOLON)
                    {
                        isValidNamespace = false;
                        i = fwdIdx;
                        break;
                    }
                    else if (tokens->tokens[fwdIdx].type == TK_SCOPE_OPEN)
                    {
                        isValidNamespace = true;
                        break; // valid namespace
                    }
                    fwdIdx++;
                }
                if (isValidNamespace)
                    namespaceNameInProgress = true;
            }
            else if (token.type == TK_SCOPE_OPEN)
            {
                curScope++;
                if (curClass != nullptr)
                {
                    curClassScope++;
                }
                if (curStruct != nullptr)
                {
                    curStructScope++;
                }
                if (namespaceNameInProgress)
                {
                    namespaceNameInProgress = false;
                }

                if (curClass != nullptr && i > 0 && !curClass->name.IsValid())
                {
                    String nameSpace = "";
                    std::stack<NamespaceInfo> copy = curNamespace;
                    while (copy.size() > 0)
                    {
                        if (nameSpace.IsEmpty())
                            nameSpace = copy.top().name.GetString();
                        else
                            nameSpace = copy.top().name.GetString() + "::" + nameSpace;
                        copy.pop();
                    }
                    curClass->nameSpace = nameSpace;
                    int backIdx = i - 1;
                    while (backIdx > 0 && !curClass->name.IsValid())
                    {
                        if (tokens->tokens[backIdx].type == TK_IDENTIFIER)
                            curClass->name = String(tokens->tokens[backIdx].lexeme);
                        backIdx--;
                    }
                }
                else if (curStruct != nullptr && i > 0 && !curStruct->name.IsValid())
                {
                    String nameSpace = "";
                    std::stack<NamespaceInfo> copy = curNamespace;
                    while (copy.size() > 0)
                    {
                        if (nameSpace.IsEmpty())
                            nameSpace = copy.top().name.GetString();
                        else
                            nameSpace = copy.top().name.GetString() + "::" + nameSpace;
                        copy.pop();
                    }
                    curStruct->nameSpace = nameSpace;
                    int backIdx = i - 1;
                    while (backIdx > 0 && !curStruct->name.IsValid())
                    {
                        if (tokens->tokens[backIdx].type == TK_IDENTIFIER)
                            curStruct->name = String(tokens->tokens[backIdx].lexeme);
                        backIdx--;
                    }
                }
            }
            else if (token.type == TK_SCOPE_CLOSE)
            {
                curScope--;
                if (curNamespace.size() > 0 && curNamespace.top().curScope == curScope)
                {
                    curNamespace.pop();
                }
                if (curClass != nullptr)
                {
                    curClassScope--;
                    if (curClassScope <= 0)
                    {
                        curClassScope = 0;
                        ast->classes.Add(*curClass);
                        delete curClass;
                        curClass = nullptr;
                    }
                }
                if (curStruct != nullptr)
                {
                    curStructScope--;
                    if (curStructScope <= 0)
                    {
                        curStructScope = 0;
                        ast->structs.Add(*curStruct);
                        delete curStruct;
                        curStruct = nullptr;
                    }
                }
            }
            else if (token.type == TK_SCOPE_OPERATOR)
            {
                
            }
            else if (token.type == TK_CE_CLASS_BODY && curClass != nullptr)
            {
                int curEntry = 0;
                String baseClassName = "";
                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_COMMA || tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (curEntry > 0 && !baseClassName.IsEmpty())
                        {
                            curClass->superClasses.Add(baseClassName);
                            baseClassName = "";
                        }
                        curEntry++;
                    }
                    else if (curEntry > 0 && tokens->tokens[i].type == TK_IDENTIFIER)
                    {
                        if (!baseClassName.IsEmpty())
                            baseClassName += "::";
                        baseClassName += tokens->tokens[i].lexeme;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                        break;
                    i++;
                }
            }
            else if (token.type == TK_CE_STRUCT_BODY && curStruct != nullptr)
            {
                int curEntry = 0;
                String baseStructName = "";
                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_COMMA || tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (curEntry > 0 && !baseStructName.IsEmpty())
                        {
                            curStruct->superClasses.Add(baseStructName);
                            baseStructName = "";
                        }
                        curEntry++;
                    }
                    else if (curEntry > 0 && tokens->tokens[i].type == TK_IDENTIFIER)
                    {
                        if (!baseStructName.IsEmpty())
                            baseStructName += "::";
                        baseStructName += tokens->tokens[i].lexeme;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                        break;
                    i++;
                }
            }
            else if (token.type == TK_PAREN_OPEN)
            {
                curParenthesis++;
            }
            else if (token.type == TK_PAREN_CLOSE)
            {
                curParenthesis--;
            }
            else if (token.type == TK_CE_STRUCT)
            {
                delete curStruct;
                curStruct = new StructInfo;
                int attribScope = 0;
                String curAttrib = "";
                i++;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
                        break;
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        attribScope++;
                        if (attribScope > 1)
                            curAttrib += "(";
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (attribScope > 1)
                            curAttrib += ")";
                        attribScope--;
                    }
                    if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        if (!curAttrib.IsEmpty())
                            curStruct->attribs.Add(curAttrib);
                        curAttrib = "";
                    }
                    else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        curAttrib += ",";
                    }
                    else if (attribScope >= 1)
                    {
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        curAttrib += String(tokens->tokens[i].lexeme);
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                    }
                    i++;
                }

                if (!curAttrib.IsEmpty())
                {
                    curStruct->attribs.Add(curAttrib);
                    curAttrib = "";
                }
            }
            else if (token.type == TK_CE_CLASS)
            {
                delete curClass;
                curClass = new ClassInfo;
                int attribScope = 0;
                String curAttrib = "";
                i++;
                
                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
                        break;
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        attribScope++;
                        if (attribScope > 1)
                            curAttrib += "(";
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (attribScope > 1)
                            curAttrib += ")";
                        attribScope--;
                    }
                    if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        if (!curAttrib.IsEmpty())
                            curClass->attribs.Add(curAttrib);
                        curAttrib = "";
                    }
                    else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        curAttrib += ",";
                    }
                    else if (attribScope >= 1)
                    {
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        curAttrib += String(tokens->tokens[i].lexeme);
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                    }
                    i++;
                }
                
                if (!curAttrib.IsEmpty())
                {
                    curClass->attribs.Add(curAttrib);
                    curAttrib = "";
                }
            }
			else if (token.type == TK_CE_FUNCTION && curStruct != nullptr)
			{
				FunctionInfo curFunction{};
				int attribScope = 0;
				String curAttrib = "";
				i++;

				while (i < size)
				{
					if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
					{
						i++;
						break;
					}
					if (tokens->tokens[i].type == TK_PAREN_OPEN)
					{
						attribScope++;
						if (attribScope > 1)
							curAttrib += "(";
					}
					if (tokens->tokens[i].type == TK_PAREN_CLOSE)
					{
						if (attribScope > 1)
							curAttrib += ")";
						attribScope--;
					}
					if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
					{
						if (!curAttrib.IsEmpty())
							curFunction.attribs.Add(curAttrib);
						curAttrib = "";
					}
					else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
					{
						curAttrib += ",";
					}
					else if (attribScope >= 1)
					{
						if (tokens->tokens[i].type == TK_LITERAL_STRING)
							curAttrib += "\"";
						if (tokens->tokens[i].type == TK_LITERAL_CHAR)
							curAttrib += "\'";
						curAttrib += String(tokens->tokens[i].lexeme);
						if (tokens->tokens[i].type == TK_LITERAL_CHAR)
							curAttrib += "\'";
						if (tokens->tokens[i].type == TK_LITERAL_STRING)
							curAttrib += "\"";
					}
					i++;
				}

				if (!curAttrib.IsEmpty())
				{
					curFunction.attribs.Add(curAttrib);
					curAttrib = "";
				}

				attribScope = 0;
				int parenScope = 0;
				int funcBodyScope = 0;

				while (i < size)
				{
					if (tokens->tokens[i].type == TK_SCOPE_OPEN)
					{
						funcBodyScope++;
					}
					else if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
					{
						funcBodyScope--;
						if (funcBodyScope <= 0)
							break;
					}

					if (funcBodyScope > 0)
					{
						i++;
						continue;
					}

					if (parenScope == 0 && tokens->tokens[i].type == TK_KW_CONST)
					{
						curFunction.signature += " const";
						curFunction.attribs.Add("Constant");
					}

					if (tokens->tokens[i].type == TK_PAREN_OPEN)
					{
						if (parenScope == 0)
							curFunction.signature += "(";
						parenScope++;
						if (!curFunction.name.IsValid())
						{
							curFunction.name = String(tokens->tokens[i - 1].lexeme);

							String returnType{};

							// Find return type
							int backIdx = i - 2;
							while (backIdx > 1 && tokens->tokens[backIdx].type != TK_SCOPE_CLOSE &&
								tokens->tokens[backIdx].type != TK_SEMICOLON &&
								tokens->tokens[backIdx].type != TK_NEWLINE)
							{
								if (tokens->tokens[backIdx].type == TK_KW_VOID ||
									tokens->tokens[backIdx].type == TK_KW_INLINE ||
									tokens->tokens[backIdx].type == TK_KW_VIRTUAL)
								{
									returnType = "void";
									break;
								}

								if (tokens->tokens[backIdx].type == TK_ASTERISK)
									returnType = "*" + returnType;
								else if (tokens->tokens[backIdx].type == TK_LEFTANGLE)
									returnType = "<" + returnType;
								else if (tokens->tokens[backIdx].type == TK_RIGHTANGLE)
									returnType = ">" + returnType;
								else if (tokens->tokens[backIdx].type == TK_SCOPE_OPERATOR)
									returnType = "::" + returnType;
								else if ((tokens->tokens[backIdx].type == TK_IDENTIFIER && tokens->tokens[backIdx].lexeme != curFunction.name.GetCString()) ||
									(tokens->tokens[backIdx].type >= TK_KW_INT && tokens->tokens[backIdx].type <= TK_KW_VEC4))
								{
									returnType = String(tokens->tokens[backIdx].lexeme) + returnType;
									if (tokens->tokens[backIdx - 1].type != TK_SCOPE_OPERATOR &&
										tokens->tokens[backIdx - 1].type != TK_LEFTANGLE &&
										tokens->tokens[backIdx - 1].type != TK_RIGHTANGLE)
										break;
								}
								else if (tokens->tokens[backIdx].type == TK_AMPERSAND)
									returnType = "&" + returnType;
								else if (tokens->tokens[backIdx].type == TK_KW_ARRAY)
									returnType = "Array" + returnType;

								backIdx--;
							}

							if (!returnType.IsEmpty())
								curFunction.returnType = returnType;
						}
					}
					else if (tokens->tokens[i].type == TK_PAREN_CLOSE)
					{
						parenScope--;
						if (parenScope == 0)
							curFunction.signature += ")";
					}
					else if (tokens->tokens[i].type == TK_SEMICOLON)
					{
						break;
					}
					else if (parenScope > 0)
					{
						if (tokens->tokens[i].type == TK_COMMA)
							curFunction.signature += ",";
						else
							curFunction.signature += String(tokens->tokens[i].lexeme) + " ";
					}

					i++;
				}

				curStruct->functions.Add(curFunction);
			}
            else if (token.type == TK_CE_FUNCTION && curClass != nullptr)
            {
                FunctionInfo curFunction{};
                int attribScope = 0;
                String curAttrib = "";
                i++;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
                    {
                        i++;
                        break;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        attribScope++;
                        if (attribScope > 1)
                            curAttrib += "(";
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (attribScope > 1)
                            curAttrib += ")";
                        attribScope--;
                    }
                    if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        if (!curAttrib.IsEmpty())
                            curFunction.attribs.Add(curAttrib);
                        curAttrib = "";
                    }
                    else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        curAttrib += ",";
                    }
                    else if (attribScope >= 1)
                    {
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        curAttrib += String(tokens->tokens[i].lexeme);
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                    }
                    i++;
                }

                if (!curAttrib.IsEmpty())
                {
                    curFunction.attribs.Add(curAttrib);
                    curAttrib = "";
                }

                attribScope = 0;
                int parenScope = 0;
                int funcBodyScope = 0;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_SCOPE_OPEN)
                    {
                        funcBodyScope++;
                    }
                    else if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
                    {
                        funcBodyScope--;
                        if (funcBodyScope <= 0)
                            break;
                    }

                    if (funcBodyScope > 0)
                    {
                        i++;
                        continue;
                    }

                    if (parenScope == 0 && tokens->tokens[i].type == TK_KW_CONST)
                    {
                        curFunction.signature += " const";
                        curFunction.attribs.Add("Constant");
                    }

                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        if (parenScope == 0)
                            curFunction.signature += "(";
                        parenScope++;
                        if (!curFunction.name.IsValid())
                        {
	                        curFunction.name = String(tokens->tokens[i - 1].lexeme);

                            String returnType{};

                            // Find return type
                            int backIdx = i - 2;
                            while (backIdx > 1 && tokens->tokens[backIdx].type != TK_SCOPE_CLOSE &&
                                tokens->tokens[backIdx].type != TK_SEMICOLON &&
                                tokens->tokens[backIdx].type != TK_NEWLINE)
                            {
                                if (tokens->tokens[backIdx].type == TK_KW_VOID ||
                                    tokens->tokens[backIdx].type == TK_KW_INLINE ||
                                    tokens->tokens[backIdx].type == TK_KW_VIRTUAL)
                                {
                                    returnType = "void";
                                    break;
                                }

                                if (tokens->tokens[backIdx].type == TK_ASTERISK)
                                    returnType = "*" + returnType;
                                else if (tokens->tokens[backIdx].type == TK_LEFTANGLE)
                                    returnType = "<" + returnType;
                                else if (tokens->tokens[backIdx].type == TK_RIGHTANGLE)
                                    returnType = ">" + returnType;
                                else if (tokens->tokens[backIdx].type == TK_SCOPE_OPERATOR)
                                    returnType = "::" + returnType;
                                else if ((tokens->tokens[backIdx].type == TK_IDENTIFIER && tokens->tokens[backIdx].lexeme != curFunction.name.GetCString()) ||
                                    (tokens->tokens[backIdx].type >= TK_KW_INT && tokens->tokens[backIdx].type <= TK_KW_VEC4))
                                {
	                                returnType = String(tokens->tokens[backIdx].lexeme) + returnType;
                                    if (tokens->tokens[backIdx - 1].type != TK_SCOPE_OPERATOR &&
                                        tokens->tokens[backIdx - 1].type != TK_LEFTANGLE &&
                                        tokens->tokens[backIdx - 1].type != TK_RIGHTANGLE)
                                        break;
                                }
                                else if (tokens->tokens[backIdx].type == TK_AMPERSAND)
                                    returnType = "&" + returnType;
                                else if (tokens->tokens[backIdx].type == TK_KW_ARRAY)
                                    returnType = "Array" + returnType;

                                backIdx--;
                            }

                            if (!returnType.IsEmpty())
                                curFunction.returnType = returnType;
                        }
                    }
                    else if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        parenScope--;
                        if (parenScope == 0)
                            curFunction.signature += ")";
                    }
                    else if (tokens->tokens[i].type == TK_SEMICOLON)
                    {
                        break;
                    }
                    else if (parenScope > 0)
                    {
                        if (tokens->tokens[i].type == TK_COMMA)
                            curFunction.signature += ",";
                        else
                            curFunction.signature += String(tokens->tokens[i].lexeme) + " ";
                    }
                    
                    i++;
                }

                curClass->functions.Add(curFunction);
            }
            else if (token.type == TK_FUSION_EVENT && curClass != nullptr)
            {
                i++;
                String name = "";
                int parenScope = 0;
                int paramIndex = 0;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && parenScope <= 1)
                        break;
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        parenScope++;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        parenScope--;
                    }

                    if (parenScope == 1 && paramIndex == 1)
                    {
                        const String& lexeme = tokens->tokens[i].lexeme;
                        if (name.IsEmpty() && lexeme.NotEmpty() && String::IsUpper(lexeme[0]))
                        {
                            name = tokens->tokens[i].lexeme;
                        }
                    }

                    if (parenScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        paramIndex++;
                    }
                    i++;
                }

                if (name.NotEmpty())
                {
                    String fieldName = "m_" + name;

                    FieldInfo field{};
                    field.name = fieldName;
                    field.attribs.Add("FusionEvent");

                    curClass->fields.Add(field);
                }
            }
            else if ((token.type == TK_MODEL_PROPERTY || token.type == TK_MODEL_PROPERTY_EDITABLE) && curClass != nullptr)
            {
                i++;
                String name = "";
                int parenScope = 0;
                int paramIndex = 0;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && parenScope <= 1)
                        break;
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        parenScope++;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        parenScope--;
                    }

                    if (parenScope == 1 && paramIndex == 1)
                    {
                        const String& lexeme = tokens->tokens[i].lexeme;
                        if (name.IsEmpty() && lexeme.NotEmpty() && String::IsUpper(lexeme[0]))
                        {
                            name = tokens->tokens[i].lexeme;
                        }
                    }

                    if (parenScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        paramIndex++;
                    }
                    i++;
                }

                if (name.NotEmpty())
                {
                    String fieldName = "m_" + name;

                    FieldInfo field{};
                    field.name = fieldName;
                    field.attribs.Add("ModelProperty");

                    curClass->fields.Add(field);
                    curClass->fields.Add({ .name = "m_On" + name + "Updated" });

                    String fieldPath = curClass->nameSpace.GetString() + "::" + curClass->name.GetString() + "::" + fieldName;

                    curClass->functions.Add({
                        .name = "Get" + name,
                        .signature = "() const",
                        //.returnType = "const decltype(" + fieldPath + ")&",
                        .returnType = "decltype(" + fieldPath + ")",
                        .isSignal = false,
                        .isEvent = false,
                        .attribs = {}
                    });

                    curClass->functions.Add({
                        .name = "Set" + name,
                        .signature = "(const decltype(" + fieldPath + ")&, Object*)",
                        .returnType = "void",
                        .isSignal = false,
                        .isEvent = false,
                        .attribs = {}
                    });
                }
            }
            else if ((token.type == TK_FUSION_DATA_PROPERTY || token.type == TK_FUSION_PROPERTY || 
                token.type == TK_FUSION_LAYOUT_PROPERTY || token.type == TK_FUSION_DATA_LAYOUT_PROPERTY) && 
                curClass != nullptr)
            {
                i++;
                String name = "";
                int parenScope = 0;
                int paramIndex = 0;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && parenScope <= 1)
                        break;
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        parenScope++;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        parenScope--;
                    }

                    if (parenScope == 1 && paramIndex == 1)
                    {
                        const String& lexeme = tokens->tokens[i].lexeme;
	                    if (name.IsEmpty() && lexeme.NotEmpty() && String::IsUpper(lexeme[0]))
	                    {
                            name = tokens->tokens[i].lexeme;
	                    }
                    }

                    if (parenScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        paramIndex++;
                    }
                    i++;
                }

                if (name.NotEmpty())
                {
                    String fieldName = "m_" + name;

                    FieldInfo field{};
                    field.name = fieldName;
                    switch (token.type)
                    {
                    case TK_FUSION_PROPERTY:
                        field.attribs.Add("FusionProperty");
                        break;
                    case TK_FUSION_DATA_PROPERTY:
                        field.attribs.Add("FusionDataProperty");
                        break;
                    case TK_FUSION_LAYOUT_PROPERTY:
                        field.attribs.Add("FusionLayoutProperty");
                        break;
                    case TK_FUSION_DATA_LAYOUT_PROPERTY:
                        field.attribs.Add("FusionDataLayoutProperty");
                        break;
                    }

                    curClass->fields.Add(field);

                    if (token.type == TK_FUSION_DATA_PROPERTY || token.type == TK_FUSION_DATA_LAYOUT_PROPERTY)
                    {
                        curClass->fields.Add({ .name = fieldName + "Binding", .attribs = { "FusionDataBinding" } });
                        curClass->functions.Add({ .name = "Update_" + name, .signature = "(Object*)", .returnType = "void", .attribs = { "FusionDataUpdate" } });
                    }
                }
            }
            else if (token.type == TK_CE_FIELD && (curClass != nullptr || curStruct != nullptr))
            {
                auto structPtr = curStruct;
                if (structPtr == nullptr)
                    structPtr = curClass;
                FieldInfo curField{};
                int attribScope = 0;
                String curAttrib = "";
                i++;
                
                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
                        break;
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        attribScope++;
                        if (attribScope > 1)
                            curAttrib += "(";
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (attribScope > 1)
                            curAttrib += ")";
                        attribScope--;
                    }
                    if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        if (!curAttrib.IsEmpty())
                            curField.attribs.Add(curAttrib);
                        curAttrib = "";
                    }
                    else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        curAttrib += ",";
                    }
                    else if (attribScope >= 1)
                    {
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        curAttrib += String(tokens->tokens[i].lexeme);
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                    }
                    i++;
                }
                
                if (!curAttrib.IsEmpty())
                {
                    curField.attribs.Add(curAttrib);
                    curAttrib = "";
                }
                
                attribScope = 0;
                
                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_SCOPE_OPEN)
                    {
                        attribScope++;
                    }
                    else if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
                    {
                        attribScope--;
                        break;
                    }
                    else if (attribScope > 0)
                    {
                        i++;
                        continue;
                    }
                    if (tokens->tokens[i].type == TK_SCOPE_OPEN || tokens->tokens[i].type == TK_ASSIGNMENT_OPERATOR ||
                        tokens->tokens[i].type == TK_SEMICOLON || tokens->tokens[i].type == TK_COLON || tokens->tokens[i].type == TK_SQUARE_BRACE_OPEN)
                    {
                        curField.name = String(tokens->tokens[i - 1].lexeme);
                        structPtr->fields.Add(curField);
                        curField.name = "";
                        if (tokens->tokens[i].type == TK_SEMICOLON || tokens->tokens[i].type == TK_ASSIGNMENT_OPERATOR)
                            break;
                    }
                    i++;
                }
            }
            else if (token.type == TK_COLON)
            {
                if (curClass != nullptr && i > 0 && !curClass->name.IsValid())
                {
                    String nameSpace = "";
                    std::stack<NamespaceInfo> copy = curNamespace;
                    while (copy.size() > 0)
                    {
                        if (nameSpace.IsEmpty())
                            nameSpace = copy.top().name.GetString();
                        else
                            nameSpace = copy.top().name.GetString() + "::" + nameSpace;
                        copy.pop();
                    }
                    curClass->nameSpace = nameSpace;
                    int backIdx = i - 1;
                    while (backIdx > 0)
                    {
                        if (tokens->tokens[backIdx].type == TK_IDENTIFIER)
                        {
                            curClass->name = String(tokens->tokens[backIdx].lexeme);
                            break;
                        }
                        backIdx--;
                    }
                }
                else if (curStruct != nullptr && i > 0 && !curStruct->name.IsValid())
                {
                    String nameSpace = "";
                    std::stack<NamespaceInfo> copy = curNamespace;
                    while (copy.size() > 0)
                    {
                        if (nameSpace.IsEmpty())
                            nameSpace = copy.top().name.GetString();
                        else
                            nameSpace = copy.top().name.GetString() + "::" + nameSpace;
                        copy.pop();
                    }
                    curStruct->nameSpace = nameSpace;
                    int backIdx = i - 1;
                    while (backIdx > 0)
                    {
                        if (tokens->tokens[backIdx].type == TK_IDENTIFIER)
                        {
                            curStruct->name = String(tokens->tokens[backIdx].lexeme);
                            break;
                        }
                        backIdx--;
                    }
                }
            }
            else if (token.type == TK_CE_ENUM)
            {
                delete curEnum;
                curEnum = new EnumInfo;
                int attribScope = 0;
                String curAttrib = "";
                EnumConstInfo curConst{};
                i++;

                String nameSpace = "";
                std::stack<NamespaceInfo> copy = curNamespace;
                while (copy.size() > 0)
                {
                    if (nameSpace.IsEmpty())
                        nameSpace = copy.top().name.GetString();
                    else
                        nameSpace = copy.top().name.GetString() + "::" + nameSpace;
                    copy.pop();
                }
                curEnum->nameSpace = nameSpace;

                while (i < size)
                {
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
                    {
                        i++;
                        break;
                    }
                    if (tokens->tokens[i].type == TK_PAREN_OPEN)
                    {
                        attribScope++;
                        if (attribScope > 1)
                            curAttrib += "(";
                    }
                    if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                    {
                        if (attribScope > 1)
                            curAttrib += ")";
                        attribScope--;
                    }
                    if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        if (!curAttrib.IsEmpty())
                            curEnum->attribs.Add(curAttrib);
                        curAttrib = "";
                    }
                    else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
                    {
                        curAttrib += ",";
                    }
                    else if (attribScope >= 1)
                    {
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        curAttrib += String(tokens->tokens[i].lexeme);
                        if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                            curAttrib += "\'";
                        if (tokens->tokens[i].type == TK_LITERAL_STRING)
                            curAttrib += "\"";
                    }
                    i++;
                }

                if (!curAttrib.IsEmpty())
                {
                    curEnum->attribs.Add(curAttrib);
                    curAttrib = "";
                }

                attribScope = 0;
                bool insideEnum = false;
                bool shouldBreak = false;

                while (i < size && !shouldBreak)
                {

                    if ((tokens->tokens[i].type == TK_COLON || tokens->tokens[i].type == TK_SCOPE_OPEN) && !curEnum->name.IsValid())
                    {
                        int backIdx = i - 1;
                        while (backIdx > 0 && !curEnum->name.IsValid())
                        {
                            if (tokens->tokens[backIdx].type == TK_IDENTIFIER)
                            {
                                curEnum->name = String(tokens->tokens[backIdx].lexeme);
                                break;
                            }
                            backIdx--;
                        }
                    }

                    if (tokens->tokens[i].type == TK_SCOPE_OPEN)
                    {
                        insideEnum = true;
                    }
                    if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
                    {
                        insideEnum = false;
                        i++;
                        ast->enums.Add(*curEnum);
                        delete curEnum;
                        curEnum = nullptr;
                        break;
                    }

                    attribScope = 0;
                    curAttrib = "";

                    // ECONST
                    if (tokens->tokens[i].type == TK_CE_ECONST)
                    {
                        curConst = {};
                        i++;

                        while (i < size)
                        {
                            if (tokens->tokens[i].type == TK_PAREN_CLOSE && attribScope <= 1)
                            {
                                i++;
                                if (!curAttrib.IsEmpty())
                                {
                                    curConst.attribs.Add(curAttrib);
                                    curAttrib = "";
                                }
                                break;
                            }
                            if (tokens->tokens[i].type == TK_PAREN_OPEN)
                            {
                                attribScope++;
                                if (attribScope > 1)
                                    curAttrib += "(";
                            }
                            if (tokens->tokens[i].type == TK_PAREN_CLOSE)
                            {
                                if (attribScope > 1)
                                    curAttrib += ")";
                                attribScope--;
                            }
                            if (attribScope == 1 && tokens->tokens[i].type == TK_COMMA)
                            {
                                if (!curAttrib.IsEmpty())
                                {
                                    curConst.attribs.Add(curAttrib);
                                    curAttrib = "";
                                }
                                curAttrib = "";
                            }
                            else if (attribScope > 1 && tokens->tokens[i].type == TK_COMMA)
                            {
                                curAttrib += ",";
                            }
                            else if (attribScope >= 1)
                            {
                                if (tokens->tokens[i].type == TK_LITERAL_STRING)
                                    curAttrib += "\"";
                                if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                                    curAttrib += "\'";
                                curAttrib += String(tokens->tokens[i].lexeme);
                                if (tokens->tokens[i].type == TK_LITERAL_CHAR)
                                    curAttrib += "\'";
                                if (tokens->tokens[i].type == TK_LITERAL_STRING)
                                    curAttrib += "\"";
                            }
                            i++;
                        }
                    }

                    // Identifier
                    if (tokens->tokens[i].type == TK_IDENTIFIER && insideEnum)
                    {
                        const auto& nameToken = tokens->tokens[i];
                        i++;

                        while (i < size)
                        {
                            if (tokens->tokens[i].type == TK_COMMA && curConst.name.IsValid())
                            {
                                curEnum->constants.Add(curConst);
                                curConst = {};
                                break;
                            }

                            if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
                            {
                                shouldBreak = true;
                                if (!curConst.name.IsValid())
                                {
                                    curConst.name = String(nameToken.lexeme);
                                }
                                curEnum->constants.Add(curConst);
                                curConst = {};
                                ast->enums.Add(*curEnum);
                                delete curEnum;
                                curEnum = nullptr;
                                break;
                            }

                            if (!curConst.name.IsValid() && 
                                (tokens->tokens[i].type == TK_ASSIGNMENT_OPERATOR || tokens->tokens[i].type == TK_COMMA || tokens->tokens[i].type == TK_SCOPE_CLOSE))
                            {
                                curConst.name = String(nameToken.lexeme);
                                curEnum->constants.Add(curConst);
                                curConst = {};
                                if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
                                {
                                    shouldBreak = true;
                                    ast->enums.Add(*curEnum);
                                    delete curEnum;
                                    curEnum = nullptr;
                                    break;
                                }

                                while (tokens->tokens[i].type != TK_COMMA)
                                {
                                    if (tokens->tokens[i].type == TK_SCOPE_CLOSE)
                                    {
                                        shouldBreak = true;
                                        ast->enums.Add(*curEnum);
                                        delete curEnum;
                                        curEnum = nullptr;
                                        break;
                                    }
                                    i++;
                                }
                                break;
                            }

                            i++;
                        }
                    }

                    i++;
                }
            }
            else if (token.type == TK_IDENTIFIER)
            {
                if (namespaceNameInProgress) // Namespace name being captured
                {
                    NamespaceInfo namespaceInfo{};
                    namespaceInfo.name = String(token.lexeme);
                    namespaceInfo.curScope = curScope;
                    curNamespace.push(namespaceInfo);
                }
            }
        }
        
        return ast;
    }

} // namespace CE

