
#include "ResourceCompiler.h"

#include "cxxopts.hpp"

#include <iostream>
#include <chrono>

#define MAX_RESOURCE_SIZE 12_MB

using namespace CE;

struct ResourceStampEntry
{
	CE_STRUCT(ResourceStampEntry)
public:

	IO::Path relativePath{};
	u32 crc{};

};

CE_RTTI_STRUCT(, , ResourceStampEntry,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(relativePath)
		CE_FIELD(crc)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(, , ResourceStampEntry)

struct ResourceStampManifest
{
	CE_STRUCT(ResourceStampManifest)
public:

	Array<ResourceStampEntry> entries{};
};

CE_RTTI_STRUCT(, , ResourceStampManifest,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(entries)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(, , ResourceStampManifest)

static HashSet<Name> textFileExts = {
	".txt", ".json", ".xml", ".html", ".css", ".sass", ".scss",
	".shader", ".hlsl", ".glsl", ".msl"
};

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char** argv)
{
    using namespace CE;

    ModuleManager::Get().LoadModule("Core");
    Logger::Initialize();

	CE_REGISTER_TYPES(ResourceStampEntry, ResourceStampManifest);

    cxxopts::Options options("Resource Compiler", "A command line tool to compile resources to be embedded in a library or executable.");

    options.add_options()
        ("h,help", "Print this help info.")
		("m,module", "Module name", cxxopts::value<std::string>())
        ("d,dir", "Resource root path", cxxopts::value<std::string>())
        ("o,output", "Generated output directory", cxxopts::value<std::string>())
        ("f,force", "Force update output")
        ;

    try
    {   
        cxxopts::ParseResult result = options.parse(argc, argv);

        if (result["h"].as<bool>())
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

		String moduleName = result["m"].as<std::string>();
        IO::Path resourceDirPath = result["d"].as<std::string>();
        IO::Path outPath = result["o"].as<std::string>();
		IO::Path timeStampFile = outPath / "stamp.json";

        if (!outPath.Exists())
        {
            IO::Path::CreateDirectories(outPath);
        }

		// Remove all content
		{
			Array<IO::Path> pathsToRemove{};

			outPath.IterateChildren([&](const IO::Path& subPath)
				{
					pathsToRemove.Add(subPath);
				});

			for (int i = 0; i < pathsToRemove.GetSize(); i++)
			{
				if (pathsToRemove[i].IsDirectory())
				{
					IO::Path::RemoveRecursively(pathsToRemove[i]);
				}
				else
				{
					IO::Path::Remove(pathsToRemove[i]);
				}
			}
		}

		Array<IO::Path> resourceFilesRelative{};

		resourceDirPath.RecursivelyIterateChildren([&](const IO::Path& path)
			{
				IO::Path relativePath = path.GetRelative(path, resourceDirPath);
				if (!path.IsDirectory())
					resourceFilesRelative.Add(relativePath);
			});
		
		if (!timeStampFile.Exists())
		{
			FileStream writer = FileStream(timeStampFile, Stream::Permissions::WriteOnly);
			writer.SetAsciiMode(true);
			writer << "{\n}";
			writer.Close();
		}

		ResourceStampManifest stampManifest{};

		{
			FileStream reader = FileStream(timeStampFile, Stream::Permissions::ReadOnly);
			reader.SetAsciiMode(true);

			JsonFieldDeserializer deserializer{ ResourceStampManifest::Type(), &stampManifest };
			JValue value;

			if (JsonSerializer::Deserialize2(&reader, value))
			{
				while (deserializer.HasNext())
				{
					deserializer.ReadNext(value);
				}
			}

			reader.Close();
		}

		struct ResourceInfo
		{
			IO::Path generatedFilePath{};
			String resourceIdentifier{};
		};
		Array<ResourceInfo> generatedResources{};

		for (const auto& srcFileRelative : resourceFilesRelative)
		{
			String fileName = srcFileRelative.GetFileName().GetString();
			auto fullPath = resourceDirPath / srcFileRelative;
			if (!fullPath.Exists())
				continue;
			String relativePathIdentifier = srcFileRelative.GetString();
			relativePathIdentifier = relativePathIdentifier.Replace({ '.', ':', ';', '-', '+', '/', '\\', '*' }, '_');

			IO::Path outFilePath = outPath / (srcFileRelative.GetString() + ".h");

			if (!outFilePath.GetParentPath().Exists())
			{
				IO::Path::CreateDirectories(outFilePath.GetParentPath());
			}

			FileStream reader = FileStream(fullPath, Stream::Permissions::ReadOnly);
			reader.SetBinaryMode(true);
			auto length = reader.GetLength();
			if (length >= MAX_RESOURCE_SIZE)
			{
				CE_LOG(Error, All, "Failed to compile resource: {}. Size greater than {} MB.", fullPath, MAX_RESOURCE_SIZE / 1024 / 1024);
				reader.Close();
				continue;
			}

			FileStream data = FileStream(outFilePath);
			data.SetAsciiMode(true);

			data << "#pragma once\n\n";

			if (textFileExts.Exists(srcFileRelative.GetExtension().GetString()))
			{
				data << "static const char " << relativePathIdentifier << "_Data[] = R\"(";
				int counter = 0;

				while (!reader.IsOutOfBounds())
				{
					auto byte = reader.ReadByte();
					if (byte == 0)
						break;

					const char append[2] = { (char)byte, 0 };
					data << String(append);
					
					counter++;
				}
				data << ")\";\n";

				data << "static const u32 " << relativePathIdentifier << "_Length = sizeof(" << relativePathIdentifier << "_Data);\n";
			}
			else
			{
				data << "static const char " << relativePathIdentifier << "_Data[] = {\n\t";
				int counter = 0;

				auto t1 = Clock::now();

				while (!reader.IsOutOfBounds())
				{
					if (counter >= 64)
					{
						counter = 0;
						data << "\n\t";
					}

					auto byte = reader.ReadByte();
					data << "(char)" << (int)byte << ", ";
					counter++;
				}
				data << "};\n";

				data << "static const u32 " << relativePathIdentifier << "_Length = " << length << ";\n";
			}
			
			data.Close();
			reader.Close();

			generatedResources.Add({ srcFileRelative.GetString().Replace({ '\\' }, '/'), relativePathIdentifier});
		}

		{
			FileStream resourceHeader = FileStream(outPath / "Resource.h", Stream::Permissions::WriteOnly);
			resourceHeader.SetAsciiMode(true);

			resourceHeader << "#pragma once\n\n";
			resourceHeader << "#include \"Core.h\"\n";

			for (const auto& genPath : generatedResources)
			{
				resourceHeader << "#include \"" << genPath.generatedFilePath.GetString() << ".h\"\n";
			}
			
			resourceHeader << "\n";

			resourceHeader << "static void CERegisterModuleResources()\n{\n";
			for (const auto& info : generatedResources)
			{
				resourceHeader << "\tCE::GetResourceManager()->RegisterResource(";
				resourceHeader << "\"" << moduleName << "\", ";
				resourceHeader << "\"" << info.generatedFilePath.GetString() << "\", ";
				resourceHeader << "(void*)" << info.resourceIdentifier << "_Data, ";
				resourceHeader << info.resourceIdentifier << "_Length);\n";
			}
			resourceHeader << "}\n\n";

			resourceHeader << "static void CEDeregisterModuleResources()\n{\n";
			for (const auto& info : generatedResources)
			{
				resourceHeader << "\tCE::GetResourceManager()->DeregisterResource(";
				resourceHeader << "\"" << moduleName << "\", ";
				resourceHeader << "\"" << info.generatedFilePath.GetString() << "\");\n";
			}
			resourceHeader << "}\n\n";

			resourceHeader.Close();
		}

		CE_LOG(Info, Console, "Resource compilation successful");
    }
    catch (std::exception exc)
    {
        CE_LOG(Critical, Console, "Failed to compile resources: {}", exc.what());
        return -1;
    }

	CE_DEREGISTER_TYPES(ResourceStampEntry, ResourceStampManifest);

    Logger::Shutdown();
    ModuleManager::Get().UnloadModule("Core");

    return 0;
}
