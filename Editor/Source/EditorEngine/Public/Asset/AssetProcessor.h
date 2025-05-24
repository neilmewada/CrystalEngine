#pragma once

namespace CE
{
    CLASS()
    class EDITORENGINE_API AssetProcessor : public Object
    {
        CE_CLASS(AssetProcessor, Object)
    protected:

        AssetProcessor();

        void OnAfterConstruct() override;

        void OnBeginDestroy() override;
        
    public:

        virtual ~AssetProcessor();

        bool IsInProgress() const { return inProgress; }

        void TerminateAllJobs();

        void Initialize();

        void Shutdown();

        void RunAll();

    protected:

        bool inProgress = false;

        IO::Path inputRoot;
        IO::Path scanPath;
        IO::Path tempPath;

        Array<IO::Path> includePaths;

        Array<IO::Path> allSourceAssetPaths{};
        Array<IO::Path> allProductAssetPaths{};

        bool validScanPath = false;

    };
    
} // namespace CE

#include "AssetProcessor.rtti.h"
