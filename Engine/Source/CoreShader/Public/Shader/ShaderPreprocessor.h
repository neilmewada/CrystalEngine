#pragma once

namespace CE
{
	ENUM()
	enum class ShaderSourceLanguage
	{
		HLSL
	};
	ENUM_CLASS_FLAGS(ShaderSourceLanguage)

	ENUM()
	enum class ShaderPropertyType
	{
		None = 0,
		UInt, Int,
		Float,
		Color,
		Vector,
		Tex2D,
		Tex3D,
		TexCube
	};
	ENUM_CLASS_FLAGS(ShaderPropertyType);

	STRUCT()
	struct CORESHADER_API ShaderPropertyEntry
	{
		CE_STRUCT(ShaderPropertyEntry)
	public:

		FIELD()
		Name name{};

		FIELD()
		Name displayName{};

		FIELD()
		ShaderPropertyType propertyType = ShaderPropertyType::None;

		FIELD()
		Array<String> attributes{};

		FIELD()
		String defaultStringValue = "";

		FIELD()
		Vec4 defaultVectorValue = {};

		FIELD()
		f32 defaultFloatValue = 0;

		bool IsValid();
	};

	STRUCT()
	struct CORESHADER_API ShaderTagEntry final
	{
		CE_STRUCT(ShaderTagEntry)
	public:

		ShaderTagEntry() = default;

		ShaderTagEntry(Name key, const String& value) : key(key), value(value)
		{}

		FIELD()
		Name key{};

		FIELD()
		String value{};
	};

	STRUCT()
	struct CORESHADER_API SubShaderPassEntry final
	{
		CE_STRUCT(SubShaderPassEntry)
	public:

		FIELD()
		Name passName{};
		
		FIELD()
		Array<ShaderTagEntry> passTags{};

		FIELD()
		ShaderSourceLanguage language = ShaderSourceLanguage::HLSL;

		FIELD()
		Name vertexEntry = "VertMain";

		FIELD()
		Name fragmentEntry = "FragMain";

		FIELD()
		BinaryBlob source{};

		FIELD()
		Array<String> features{};

		inline bool TagExists(const Name& key) const
		{
			return passTags.Exists([&](const ShaderTagEntry& entry) { return entry.key == key; });
		}

		inline String GetTagValue(const Name& key) const
		{
			int index = passTags.IndexOf([&](const ShaderTagEntry& entry) { return entry.key == key; });
			if (index >= 0 && index < passTags.GetSize())
			{
				return passTags[index].value;
			}
			return "";
		}
	};

	STRUCT()
	struct CORESHADER_API SubShaderEntry final
	{
		CE_STRUCT(SubShaderEntry)
	public:

		FIELD()
		Array<ShaderTagEntry> subShaderTags{};

		FIELD()
		Array<SubShaderPassEntry> passes{};

		inline bool TagExists(const Name& key) const
		{
			return subShaderTags.Exists([&](const ShaderTagEntry& entry) { return entry.key == key; });
		}

		inline String GetTagValue(const Name& key) const
		{
			int index = subShaderTags.IndexOf([&](const ShaderTagEntry& entry) { return entry.key == key; });
			if (index >= 0 && index < subShaderTags.GetSize())
			{
				return subShaderTags[index].value;
			}
			return "";
		}
	};

	STRUCT()
	struct CORESHADER_API ShaderPreprocessData final
	{
		CE_STRUCT(ShaderPreprocessData)
	public:

		FIELD()
		Name shaderName{};

		FIELD()
		Array<ShaderPropertyEntry> properties{};

		FIELD()
		Array<SubShaderEntry> subShaders{};

	};

    class CORESHADER_API ShaderPreprocessor
    {
    public:

		ShaderPreprocessor(Stream* stream, const Array<IO::Path>& includePaths);
		~ShaderPreprocessor();
        
		void PreprocessShader(ShaderPreprocessData* outData);

		inline const String& GetErrorMessage() const { return errorMessage; }

	private:

		struct HlslPreprocessData
		{
			Array<String> features{};
		};

		enum TokenType
		{
			TK_WHITESPACE = 0,
			TK_IDENTIFIER,
			TK_SCOPE_OPEN, TK_SCOPE_CLOSE,
			TK_PAREN_OPEN, TK_PAREN_CLOSE,
			TK_SQUARE_OPEN, TK_SQUARE_CLOSE,
			TK_KW_SHADER,
			TK_KW_SUBSHADER,
			TK_KW_TAGS,
			TK_KW_PASS,
			TK_KW_ZTEST,
			TK_KW_ZWRITE,
			TK_KW_PROPERTIES,
			TK_KW_INCLUDE,
			TK_LITERAL_STRING,
			TK_LITERAL_INTEGER,
			TK_LITERAL_FLOAT,
			TK_HLSLPROGRAM,
			TK_HLSLINCLUDE,
			TK_POUND,
			TK_EQUAL,
			TK_COMMA,
			TK_COLON,
			TK_SEMICOLON,
			TK_NEWLINE,
			TK_KW_PRAGMA,
			TK_KW_CULL
		};

		enum ScopeType
		{
			SCOPE_NONE,
			SCOPE_SHADER,
			SCOPE_PROPERTIES,
			SCOPE_SUBSHADER,
			SCOPE_TAGS,
			SCOPE_PASS,
			SCOPE_HLSLPROGRAM,
			SCOPE_HLSLINCLUDE,
		};

		struct Token
		{
			TokenType token = TK_WHITESPACE;
			String lexeme{};

			ScopeType scopeType{};
		};

		bool Tokenize(Array<Token>& outTokens);
		bool ReadNextToken(Token& outToken);

		bool ReadHLSLProgram();

		bool PreprocessHLSL(Stream* inStream, MemoryStream* outStream, Array<IO::Path>& alreadyIncludedFiles, const IO::Path& additionalIncludePath = {});

		Array<Token> tokens{};
		
		Stream* stream = nullptr;
		Array<IO::Path> includePaths{};
		String errorMessage = "";

		ScopeType curScope = SCOPE_NONE;
		Array<ScopeType> scopeStack{};
		Array<ScopeType> allScopeStack{};
		Token prevToken{};

		Array<BinaryBlob> passSources{};
		Array<HlslPreprocessData> passPreprocessData{};

		// Per HLSL program
		MemoryStream* curPassSource = nullptr;
		HlslPreprocessData curPassPreprocess{};
    };

} // namespace CE

#include "ShaderPreprocessor.rtti.h"
