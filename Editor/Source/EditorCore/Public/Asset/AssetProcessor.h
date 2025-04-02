#pragma once

namespace CE
{
    CLASS()
    class EDITORCORE_API AssetProcessor : public Object
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

        void RunOnce();

    protected:

        bool inProgress = false;

        IO::Path scanPath;
        bool validScanPath = false;

    };
    
} // namespace CE

#include "AssetProcessor.rtti.h"
