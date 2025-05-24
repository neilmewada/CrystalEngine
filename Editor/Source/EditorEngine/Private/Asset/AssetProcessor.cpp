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
    	inputRoot = gProjectPath;

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

    	includePaths = {
        	EngineDirectories::GetEngineInstallDirectory() / "Engine/Shaders"
    	};

    	RunAll();
    }

    void AssetProcessor::Shutdown()
    {
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
            if (extension == ".cmake" || fileName == "CMakeLists.txt" || extension == ".casset")
                return;

            DateTime lastWriteTime = path.GetLastWriteTime();

            IO::Path relativePath = IO::Path::GetRelative(path, inputRoot);
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

        Array<Ref<AssetImporter>> importers{};
        Array<AssetDefinition*> assetDefinitions{};

        struct ImportJobEntry
        {
            HashSet<IO::Path> productAssetDependencies{};
            AssetImportJob* job = nullptr;
        };

        Array<ImportJobEntry> importJobs{};
        HashMap<IO::Path, AssetImportJob*> importJobsByProductPath{};

    	JobCompletion jobCompletion = JobCompletion();

        for (const auto& [assetDef, sourcePaths] : sourcePathsByAssetDef)
		{
			if (assetDef == nullptr || sourcePaths.IsEmpty())
				continue;
			if (!productPathsByAssetDef.KeyExists(assetDef) ||
				productPathsByAssetDef[assetDef].GetSize() != sourcePaths.GetSize())
				continue;

			ClassType* assetImporterClass = assetDef->GetAssetImporterClass();
			if (assetImporterClass == nullptr || !assetImporterClass->CanBeInstantiated())
				continue;
			Ref<AssetImporter> assetImporter = CreateObject<AssetImporter>(nullptr, "AssetImporter", OF_Transient, assetImporterClass);
			if (assetImporter == nullptr)
				continue;

			// Get product asset dependencies required by the asset importer.

			Array<Name> productAssetDependencies = assetImporter->GetProductAssetDependencies();
			HashSet<IO::Path> productDependencies{};
			for (const Name& productAssetName : productAssetDependencies)
			{
				IO::Path assetPath = Bundle::GetAbsoluteBundlePath(productAssetName);
				if (productAssetPaths.Exists(assetPath))
				{
					productDependencies.Add(assetPath);
				}
				else if (assetPath.Exists()) // Nothing needed to be done
				{

				}
				else
				{
					CE_LOG(Warn, All, "Could not process assets of type {}. Product asset dependency not found: {}.",
						assetImporterClass->GetName(), productAssetName);
					continue;
				}
			}

			assetImporter->SetIncludePaths(includePaths);
			assetImporter->SetLogging(true);
			assetImporter->SetTargetPlatform(PlatformMisc::GetCurrentPlatform());
			assetImporter->SetTempDirectoryPath(tempPath);

			importers.Add(assetImporter);
			assetDefinitions.Add(assetDef);

			Array<AssetImportJob*> jobs = assetImporter->PrepareImportJobs(sourcePaths, productPathsByAssetDef[assetDef]);

			for (AssetImportJob* job : jobs)
			{
				job->SetAutoDelete(true);
				importJobsByProductPath[job->GetProductPath()] = job;

				Array<Name> perJobProductDependencies = job->PrepareProductAssetDependencies();
				HashSet<IO::Path> finalProductDependencies{};

				for (const auto& dependency : perJobProductDependencies)
				{
					finalProductDependencies.Add(dependency.GetString());
				}
				for (const auto& dependency : productDependencies)
				{
					finalProductDependencies.Add(dependency);
				}

				importJobs.Add({ finalProductDependencies, job });

				job->SetDependent(&jobCompletion);
			}
		}

		// Setup job dependencies
		for (const ImportJobEntry& entry : importJobs)
		{
			for (const IO::Path& productDependency : entry.productAssetDependencies)
			{
				if (importJobsByProductPath[productDependency] != nullptr)
				{
					importJobsByProductPath[productDependency]->SetDependent(entry.job);
				}
			}
		}

		// Launch the jobs
		for (const ImportJobEntry& entry : importJobs)
		{
			entry.job->Start();
		}

		// Wait for all jobs to complete
    	jobCompletion.StartAndWaitForCompletion();

		Array<AssetImportJobResult> assetImportResults{};
		Array<u32> assetDefinitionVersions{};
		Array<u32> assetImporterVersions{};

		for (int i = 0; i < importers.GetSize(); ++i)
		{
			int count = importers[i]->GetResults().GetSize();
			assetImportResults.AddRange(importers[i]->GetResults());
			for (int j = 0; j < count; ++j)
			{
				assetDefinitionVersions.Add(assetDefinitions[i]->GetAssetVersion());
				assetImporterVersions.Add(importers[i]->GetImporterVersion());
			}
			importers[i]->BeginDestroy();
		}
		importers.Clear();

		int failCounter = 0;
		CE_ASSERT(assetImportResults.GetSize() == assetDefinitionVersions.GetSize(), "Invalid number of asset import results!");

		// Update time stamps
		{
			for (int i = 0; i < assetImportResults.GetSize(); i++)
			{
				const auto& [success, sourcePath, productPath, errorMessage] = assetImportResults[i];

				IO::Path relativePath = IO::Path::GetRelative(sourcePath, inputRoot);
				IO::Path stampFilePath = tempPath / relativePath.ReplaceExtension(".stamp");
				if (success)
				{
					FileStream writer = FileStream(stampFilePath, Stream::Permissions::WriteOnly);
					writer.SetBinaryMode(true);

					writer << StampFileVersion;
					writer << sourcePath.GetLastWriteTime().ToNumber();

					writer << Bundle::GetCurrentMajor();
					writer << Bundle::GetCurrentMinor();
					writer << Bundle::GetCurrentPatch();

					writer << assetDefinitionVersions[i];

					writer << assetImporterVersions[i];
				}
				else
				{
					failCounter++;
				}
			}
		}
    }

    
} // namespace CE

