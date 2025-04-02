#include "EditorCore.h"

namespace CE
{
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
        }


    }

    void AssetProcessor::RunOnce()
    {
        if (!validScanPath)
            return;


    }

    
} // namespace CE

