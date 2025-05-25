
#include "RTTIGenerator.h"

namespace CE
{
	bool gShouldEmitApiMacro = true;

	Array<IO::Path> RTTIGenerator::includeSearchPaths{};

	void RTTIGenerator::GenerateRTTI(String moduleName, IO::Path moduleHeaderRootPath, IO::Path outputPath, ModuleStamp& moduleStamp)
	{
		fs::path modulePath = moduleHeaderRootPath;
		bool isUnitTest = false;

		if (moduleHeaderRootPath.GetFileName() == "Tests")
		{
			isUnitTest = true;
		}

		ModuleAST moduleAST{ moduleName };

		std::stringstream implStream{};
		CE::Array<String> registrantList{};
		CE::Array<String> sourceHeaderPaths{};

		CE::Array<IO::Path> filesToRemove{};

		bool anyHeaderModified = false;

		outputPath.IterateChildren([&](const IO::Path& entry)
			{
				if (entry.GetFileName().GetString().EndsWith(".private.h"))
				{
					return;
				}

				filesToRemove.Add(entry);
			});

		for (auto entry : fs::recursive_directory_iterator(modulePath))
		{
			if (entry.is_directory() || entry.path().extension() != ".h")
				continue;

			auto headerPath = entry.path();
			auto headerRelPath = fs::relative(headerPath, modulePath);

			if (!isUnitTest && IO::Path::IsSubDirectory(headerPath, moduleHeaderRootPath / "Tests"))
			{
				continue;
			}

			std::ifstream inputHeaderFile{ entry.path(), std::ios_base::in };
			std::string inputHeaderFileContent((std::istreambuf_iterator<char>(inputHeaderFile)),
				(std::istreambuf_iterator<char>()));
			std::string searchString = std::string("#include \"") + headerPath.filename().replace_extension(".rtti.h").string() + "\"";

			if (inputHeaderFileContent.find(searchString) == std::string::npos) // Could not find ".rtti.h"
			{
				continue;
			}

			fs::path headerGeneratedPath = (fs::path)outputPath / headerPath.filename().replace_extension(".rtti.h");

			// Do NOT delete this output header file
			filesToRemove.RemoveAll([headerGeneratedPath](const IO::Path& p) -> bool
			{
				return p.GetFileName() == IO::Path(headerGeneratedPath).GetFileName();
			});

			fs::path headerRelPathFinal{};
			int idx = 0;
			for (auto it = headerRelPath.begin(); it != headerRelPath.end(); ++it)
			{
				if (idx > 0 || (it->string() != "Public" && it->string() != "Private" && it->string() != "Sources"))
				{
					headerRelPathFinal /= *it;
				}
				idx++;
			}

			sourceHeaderPaths.Add(headerRelPathFinal.string());
            
            std::stringstream outStream{};

			moduleAST.ProcessHeader(headerPath, includeSearchPaths, outStream, implStream, registrantList);

            auto outString = outStream.str();

            auto crc = CE::CalculateCRC(outString.c_str(), outString.size());
            bool skipHeaderGen = false;

            if (fs::exists(headerGeneratedPath) && moduleStamp.headers.Exists([crc,headerPath](const HeaderCRC& hCrc) -> bool
              {
                  return hCrc.crc == crc && hCrc.headerPath == IO::Path(headerPath);
              }))
            {
                // Same CRC value => no changes found => skip this header
                skipHeaderGen = true;
            }
            else
            {
                // Remove the entry since we'll be modifying it
                moduleStamp.headers.RemoveAll([headerPath](const HeaderCRC& hCrc) -> bool
                  {
                      return hCrc.headerPath == IO::Path(headerPath);
                  });
            }

			if (skipHeaderGen)
				continue;

			anyHeaderModified = true;
			
			std::ofstream outFile{ headerGeneratedPath, std::ios_base::out };
			if (outFile.is_open())
			{
				outFile << outString;
				outFile.close();
			}

			moduleStamp.headers.Add({ IO::Path(headerPath), crc });
		}

		// Remove unnecessary headers from outputPath
		for (const auto& path : filesToRemove)
		{
			if (path.GetExtension() != ".stamp")
			{
				anyHeaderModified = true;
			}

			IO::Path::Remove(path);
		}
		filesToRemove.Clear();

		IO::Path moduleGenFilePath = outputPath / (moduleName + ".private.h");

		// Generate module file ONLY if either a header was modified or if it doesn't exist
		if (anyHeaderModified || !moduleGenFilePath.Exists())
		{
			if (moduleGenFilePath.Exists())
			{
				IO::Path::Remove(moduleGenFilePath);
			}

			std::ofstream moduleImplFile{ (fs::path)moduleGenFilePath, std::ios::out | std::ios::trunc };
			if (moduleImplFile.is_open())
			{
				moduleImplFile << "#pragma once\n\n";

				moduleImplFile << "#include \"CoreMinimal.h\"\n";

				for (auto headerPath : sourceHeaderPaths)
				{
					moduleImplFile << "#include \"" << headerPath.GetCString() << "\"\n";
				}

				moduleImplFile << implStream.str() << "\n";

				moduleImplFile << "static void CERegisterModuleTypes()\n{\n";
				moduleImplFile << "\tCE_REGISTER_TYPES(\n";
				for (int i = 0; i < registrantList.GetSize(); i++)
				{
					moduleImplFile << "\t\t" << registrantList[i].GetCString();
					if (i < registrantList.GetSize() - 1)
						moduleImplFile << ",";
					moduleImplFile << "\n";
				}
				moduleImplFile << "\t);\n}\n";

				moduleImplFile << "static void CEDeregisterModuleTypes()\n{\n";
				moduleImplFile << "\tCE_DEREGISTER_TYPES(\n";
				for (int i = 0; i < registrantList.GetSize(); i++)
				{
					moduleImplFile << "\t\t" << registrantList[i].GetCString();
					if (i < registrantList.GetSize() - 1)
						moduleImplFile << ",";
					moduleImplFile << "\n";
				}
				moduleImplFile << "\t);\n}\n";

				moduleImplFile.close();
			}
		}
	}

} // namespace CE

CE_RTTI_STRUCT_IMPL(, CE, HeaderCRC)

CE_RTTI_STRUCT_IMPL(, CE, ModuleStamp)
