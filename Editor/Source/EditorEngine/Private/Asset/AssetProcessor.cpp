#include "EditorCore.h"

namespace CE
{
    constexpr u32 StampFileVersion = 2;

    static WeakRef<AssetProcessor> instance = nullptr;

    AssetProcessor::AssetProcessor()
    {

    }

    AssetProcessor::~AssetProcessor()
    {

    }

    void AssetProcessor::OnAfterConstruct()
    {
        Super::OnAfterConstruct();

        if (instance == nullptr)
        {
            instance = this;
        }
    }

    void AssetProcessor::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        if (instance == this)
        {
            instance = nullptr;
        }
    }

    void AssetProcessor::TerminateAllJobs()
    {

    }

    void AssetProcessor::Initialize()
    {
        scanPath = gProjectPath / "Game/Assets";
        validScanPath = true;

        if (!scanPath.Exists())
        {
            scanPath = "";
            validScanPath = false;
            return;
        }

        tempPath = gProjectPath / "Temp/AssetCache";

        if (!tempPath.Exists())
        {
            IO::Path::CreateDirectories(tempPath);
        }
    }

    void AssetProcessor::RunAll()
    {
        if (!validScanPath)
            return;

        allSourceAssetPaths.Clear();
        allProductAssetPaths.Clear();

        scanPath.RecursivelyIterateChildren([&](const IO::Path& path)
        {
            if (path.IsDirectory())
                return;

            String fileName = path.GetFileName().GetString();
            String extension = path.GetExtension().GetString();
            if (extension == ".cmake" || fileName == "CMakeLists.txt")
                return;

            DateTime lastWriteTime = path.GetLastWriteTime();

            IO::Path relativePath = IO::Path::GetRelative(path, scanPath);
            IO::Path stampFilePath = tempPath / relativePath.ReplaceExtension(".stamp");

            if (!stampFilePath.GetParentPath().Exists())
            {
                IO::Path::CreateDirectories(stampFilePath.GetParentPath());
            }

            IO::Path productPath = path.ReplaceExtension(".casset");

            if (!stampFilePath.Exists() || !productPath.Exists())
            {
                allSourceAssetPaths.Add(path);
                allProductAssetPaths.Add(productPath);
            }
            else
            {
                FileStream reader = FileStream(stampFilePath, Stream::Permissions::ReadOnly);
                reader.SetBinaryMode(true);

                bool needsProcessing = false;

                u32 version = 0;
                reader >> version;

                if (version != StampFileVersion)
                {
                    needsProcessing = true;
                }

                u64 stampedTimeInt = 0;
                reader >> stampedTimeInt;

                DateTime stampedTime = DateTime::FromNumber(stampedTimeInt);
                if (lastWriteTime != stampedTime)
                {
                    needsProcessing = true;
                }

                if (version >= 1)
                {
                    u32 major = 0, minor = 0, patch = 0;

                    reader >> major;
                    reader >> minor;
                    reader >> patch;

                    u32 assetDefinitionVersion = 0;
                    reader >> assetDefinitionVersion;

                    AssetDefinition* assetDefinition = GetAssetDefinitionRegistry()->FindAssetDefinition(extension);
                    if (assetDefinition && assetDefinition->GetAssetVersion() != assetDefinitionVersion)
                    {
                        needsProcessing = true;
                    }

                    if (major != Bundle::GetCurrentMajor() || minor != Bundle::GetCurrentMinor() || patch != Bundle::GetCurrentPatch())
                    {
                        needsProcessing = true;
                    }

                    if (version >= 2)
                    {
                        u32 assetImporterVersion = 0;
                        reader >> assetImporterVersion;

                        if (assetDefinition)
                        {
                            AssetImporter* importerCDI = (AssetImporter*)assetDefinition->GetAssetImporterClass()->GetDefaultInstance();
                            if (importerCDI != nullptr && importerCDI->GetImporterVersion() != assetImporterVersion)
                            {
                                needsProcessing = true;
                            }
                        }
                    }
                }

                if (needsProcessing)
                {
                    allSourceAssetPaths.Add(path);
                    allProductAssetPaths.Add(productPath);
                }
            }
        });

        HashMap<AssetDefinition*, Array<IO::Path>> sourcePathsByAssetDef{};
        HashMap<AssetDefinition*, Array<IO::Path>> productPathsByAssetDef{};

        // Paths to product assets to be generated in this run
        HashSet<IO::Path> productAssetPaths{};

        AssetDefinitionRegistry* assetDefRegistry = GetAssetDefinitionRegistry();

        for (int i = 0; i < allSourceAssetPaths.GetSize(); i++)
        {
            auto sourcePath = allSourceAssetPaths[i];
            auto productPath = allProductAssetPaths[i];
            sourcePath = sourcePath.GetString().Replace({ '\\' }, '/');
            productPath = productPath.GetString().Replace({ '\\' }, '/');

            if (!sourcePath.Exists())
                continue;

            String extension = sourcePath.GetExtension().GetString().ToLower();

            AssetDefinition* assetDef = assetDefRegistry->FindAssetDefinition(extension);

            if (assetDef != nullptr)
            {
                sourcePathsByAssetDef[assetDef].Add(sourcePath);
                productPathsByAssetDef[assetDef].Add(productPath);
                productAssetPaths.Add(productPath);
            }
        }
    }

    
} // namespace CE

