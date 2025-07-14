
#include "ModuleAST.h"


namespace CE
{

	ModuleAST::ModuleAST(String moduleName)
		: moduleName(moduleName)
	{

	}

	ModuleAST::~ModuleAST()
	{
		for (auto header : processedHeaders)
		{
			delete header;
		}
		processedHeaders.Clear();
	}

	void ModuleAST::ProcessHeader(IO::Path headerPath, const Array<IO::Path>& includeSearchPaths, 
		std::stringstream& outStream, std::stringstream& implStream, CE::Array<String>& registrantList)
	{
		auto tokens = HeaderTokenizer::Tokenize(headerPath, includeSearchPaths);

        outStream << "#pragma once\n\n";

		std::string apiName = moduleName.ToUpper() + "_API";
		if (!gShouldEmitApiMacro)
		{
			apiName = "";
		}

		if (headerPath.GetFileName() == "CWidget.h")
		{
			String::IsAlphabet('a');
		}
        
        auto ast = HeaderAST::ProcessHeader(tokens);

		for (const auto& enumInfo : ast->enums)
		{
			outStream << "CE_RTTI_ENUM(" << apiName << ", " << enumInfo.nameSpace.GetCString() << ", " << enumInfo.name.GetCString() << ",\n";
			outStream << "\t" << "CE_ATTRIBS(";
			for (int i = 0; i < enumInfo.attribs.GetSize(); i++)
			{
				outStream << enumInfo.attribs[i].ToStdString();
				if (i != enumInfo.attribs.GetSize() - 1)
					outStream << ",";
			}
			outStream << "),\n";

			for (int i = 0; i < enumInfo.constants.GetSize(); i++)
			{
				outStream << "\t" << "CE_CONST(";
				outStream << enumInfo.constants[i].name.GetCString();
				for (int j = 0; j < enumInfo.constants[i].attribs.GetSize(); j++)
				{
					outStream << ", " << enumInfo.constants[i].attribs[j].ToStdString();
				}
				outStream << ")";
				if (i != enumInfo.constants.GetSize() - 1)
					outStream << ",";
				outStream << "\n";
			}

			outStream << ")\n";

			implStream << "\nCE_RTTI_ENUM_IMPL(" << apiName << ", " << enumInfo.nameSpace.GetCString() << ", " << enumInfo.name.GetCString() << ")\n";
			
			String fullName = enumInfo.nameSpace.GetString();
			if (!fullName.IsEmpty()) fullName += "::";
			fullName += enumInfo.name.GetString();

			registrantList.Add(fullName);
		}

		for (const auto& classInfo : ast->classes)
		{
			String fullName = classInfo.nameSpace.GetString();
			if (!fullName.IsEmpty()) fullName += "::";
			fullName += classInfo.name.GetString();

			outStream << "#undef __CE_HAS_GENERATED_BODY_" << classInfo.name.GetString() << "\n";

			outStream << "CE_RTTI_CLASS(" << apiName << ", " << classInfo.nameSpace.GetCString() << ", " << classInfo.name.GetCString() << ",\n";

			outStream << "\tCE_SUPER(";
			for (int i = 0; i < classInfo.superClasses.GetSize(); i++)
			{
				outStream << classInfo.superClasses[i].GetCString();
				if (i < classInfo.superClasses.GetSize() - 1)
					outStream << ", ";
			}
			outStream << "),\n";

			std::string attribString = "\tCE_ATTRIBS(";
			bool isAbstract = false;

			for (int i = 0; i < classInfo.attribs.GetSize(); i++)
			{
				if (classInfo.attribs[i] == "Abstract" || classInfo.attribs[i] == "abstract")
					isAbstract = true;
				attribString += classInfo.attribs[i];
				if (i < classInfo.attribs.GetSize() - 1)
					attribString += ", ";
			}

			attribString += ")";

			if (isAbstract)
				outStream << "\tCE_ABSTRACT,\n";
			else
				outStream << "\tCE_NOT_ABSTRACT,\n";

			outStream << attribString << ",\n";

			outStream << "\tCE_FIELD_LIST(\n";
			for (int i = 0; i < classInfo.fields.GetSize(); i++)
			{
				outStream << "\t\tCE_FIELD(" << classInfo.fields[i].name.GetCString();
				for (int j = 0; j < classInfo.fields[i].attribs.GetSize(); j++)
				{
					outStream << ", " << classInfo.fields[i].attribs[j].GetCString();
				}
				outStream << ")\n";
			}
			outStream << "\t),\n";

			outStream << "\tCE_FUNCTION_LIST(\n";
			for (int i = 0; i < classInfo.functions.GetSize(); i++)
			{
				if (!classInfo.functions[i].name.IsValid())
					continue;

				outStream << "\t\tCE_FUNCTION2(" << classInfo.functions[i].name.GetCString()
					<< ", " << (classInfo.functions[i].returnType.IsEmpty() ? "auto" : classInfo.functions[i].returnType.GetCString())
					<< ", " << classInfo.functions[i].signature.GetCString();
				for (int j = 0; j < classInfo.functions[i].attribs.GetSize(); j++)
				{
					outStream << ", " <<  classInfo.functions[i].attribs[j].GetCString();
				}
				outStream << ")\n";
			}
			outStream << "\t)\n";

			outStream << ")\n";

			implStream << "\nCE_RTTI_CLASS_IMPL(" << apiName << ", " << classInfo.nameSpace.GetCString() << ", " << classInfo.name.GetCString() << ")\n";

			registrantList.Add(fullName);
		}

		for (const auto& structInfo : ast->structs)
		{
			outStream << "CE_RTTI_STRUCT(" << apiName << ", " << structInfo.nameSpace.GetCString() << ", " << structInfo.name.GetCString() << ",\n";

			outStream << "\tCE_SUPER(";
			for (int i = 0; i < structInfo.superClasses.GetSize(); i++)
			{
				outStream << structInfo.superClasses[i].GetCString();
				if (i < structInfo.superClasses.GetSize() - 1)
					outStream << ", ";
			}
			outStream << "),\n";

			std::string attribString = "\tCE_ATTRIBS(";

			for (int i = 0; i < structInfo.attribs.GetSize(); i++)
			{
				attribString += structInfo.attribs[i];
				if (i < structInfo.attribs.GetSize() - 1)
					attribString += ", ";
			}

			attribString += ")";

			outStream << attribString << ",\n";

			outStream << "\tCE_FIELD_LIST(\n";
			for (int i = 0; i < structInfo.fields.GetSize(); i++)
			{
				outStream << "\t\tCE_FIELD(" << structInfo.fields[i].name.GetCString();
				for (int j = 0; j < structInfo.fields[i].attribs.GetSize(); j++)
				{
					outStream << ", " << structInfo.fields[i].attribs[j].GetCString();
				}
				outStream << ")\n";
			}
			outStream << "\t),\n";

			outStream << "\tCE_FUNCTION_LIST(\n";
			for (int i = 0; i < structInfo.functions.GetSize(); i++)
			{
				outStream << "\t\tCE_FUNCTION2(" << structInfo.functions[i].name.GetCString()
					<< ", " << (structInfo.functions[i].returnType.IsEmpty() ? "auto" : structInfo.functions[i].returnType.GetCString())
					<< ", " << structInfo.functions[i].signature.GetCString();
				for (int j = 0; j < structInfo.functions[i].attribs.GetSize(); j++)
				{
					outStream << ", " << structInfo.functions[i].attribs[j].GetCString();
				}
				outStream << ")\n";
			}
			outStream << "\t)\n";

			outStream << ")\n";

			implStream << "\nCE_RTTI_STRUCT_IMPL(" << apiName << ", " << structInfo.nameSpace.GetCString() << ", " << structInfo.name.GetCString() << ")\n";

			String fullName = structInfo.nameSpace.GetString();
			if (!fullName.IsEmpty()) fullName += "::";
			fullName += structInfo.name.GetString();

			registrantList.Add(fullName);
		}
        
        delete tokens;
        delete ast;
	}

} // namespace CE
