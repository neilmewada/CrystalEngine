#pragma once

#include "Core.h"
#include "CoreRHI.h"
#include "CoreRPI.h"
#include "CoreApplication.h"
#include "EditorCore.h"
#include "FusionCore.h"
#include "Fusion.h"
#include "EditorEngine.h"

// Common
#include "CrystalEditor/Style/AssetBrowserItemStyle.h"
#include "CrystalEditor/Style/CrystalEditorStyle.h"

// ------------------------------------------------
// Project Browser
// ------------------------------------------------

#include "ProjectBrowser/RecentProjectsListModel.h"
#include "ProjectBrowser/NewProjectListModel.h"
#include "ProjectBrowser/ProjectBrowser.h"


// ------------------------------------------------
// Windows
// ------------------------------------------------

#include "CrystalEditor/Windows/Misc/AboutWindow.h"
#include "CrystalEditor/Windows/Misc/SampleWidgetWindow.h"

// - Debug -
#include "CrystalEditor/Windows/Debug/FusionImageAtlasWindow.h"
#include "CrystalEditor/Windows/Debug/FusionFontAtlasWindow.h"

// ------------------------------------------------
// Crystal Editor
// ------------------------------------------------

// - Viewport -

#include "CrystalEditor/Editor/DockTabs/EditorViewportTab.h"

// - Details -

#include "CrystalEditor/Editor/DockTabs/Details/DetailsTab.h"

// - Asset Browser -

#include "CrystalEditor/Editor/DockTabs/AssetBrowser/AssetBrowser.h"
#include "CrystalEditor/Editor/DockTabs/AssetBrowser/AssetBrowserItem.h"
#include "CrystalEditor/Editor/DockTabs/AssetBrowser/AssetBrowserTreeView.h"
#include "CrystalEditor/Editor/DockTabs/AssetBrowser/AssetBrowserTreeViewModel.h"
#include "CrystalEditor/Editor/DockTabs/AssetBrowser/AssetBrowserGridViewModel.h"
#include "CrystalEditor/Editor/DockTabs/AssetBrowser/AssetBrowserGridView.h"

// - Project Settings Editor -

#include "CrystalEditor/Editor/ProjectSettingsEditor/ProjectSettingsEditor.h"

// - Scene Editor -

#include "CrystalEditor/Editor/SceneEditor/SceneOutliner/SceneTreeViewModel.h"
#include "CrystalEditor/Editor/SceneEditor/SceneOutliner/SceneTreeView.h"
#include "CrystalEditor/Editor/SceneEditor/SceneOutliner/SceneOutlinerTab.h"
#include "CrystalEditor/Editor/SceneEditor/SceneEditor.h"

// - Material Editor -

#include "CrystalEditor/Editor/MaterialEditor/MaterialDetailsTab.h"
#include "CrystalEditor/Editor/MaterialEditor/MaterialEditor.h"

// - Crystal Editor Window -

#include "CrystalEditor/CrystalEditorWindow.h"

namespace CE::Editor
{
    class CrystalEditorModule : public CE::Module
    {
    public:

        static CrystalEditorModule* Get();

        void StartupModule() override;

        void ShutdownModule() override;

        Ref<AssetProcessor> GetAssetProcessor() const { return assetProcessor; }

    private:
        Ref<AssetProcessor> assetProcessor;

    };
}
