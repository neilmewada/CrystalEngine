#pragma once

#include "Core.h"
#include "CoreRHI.h"
#include "Engine.h"

#include "EditorGlobals.h"

// Platform
#include "PAL/Common/EditorPlatform.h"

// Project
#include "Project/CrystalProject.h"
#include "Project/ProjectManager.h"

// Asset
#include "Asset/AssetDefinition.h"
#include "Asset/AssetDefinitionRegistry.h"
#include "Asset/AssetImporter.h"

// Assets
#include "Asset/ShaderAssetDefinition.h"
#include "Asset/ShaderAssetImporter.h"
#include "Asset/ComputeShaderAssetDefinition.h"
#include "Asset/ComputeShaderAssetImporter.h"
#include "Asset/TextureAssetDefinition.h"
#include "Asset/TextureAssetImporter.h"
#include "Asset/StaticMeshDefinition.h"
#include "Asset/StaticMeshAssetImporter.h"
#include "Asset/FontAssetDefinition.h"
#include "Asset/FontAssetImporter.h"

// History
#include "History/EditorOperation.h"
#include "History/EditorHistory.h"

///////////////////////////////////////////////////////
/// Widgets

#include "Fusion/Style/EditorDockspaceStyle.h"
#include "Fusion/Style/EditorMinorDockspaceStyle.h"
#include "Fusion/Style/EditorDockTabStyle.h"
#include "Fusion/Style/EditorMenuBarStyle.h"
#include "Fusion/Style/EditorStyle.h"

#include "Fusion/PropertyEditor/DetailsRow.h"
#include "Fusion/PropertyEditor/Fields/EditorField.h"
#include "Fusion/PropertyEditor/Fields/EnumEditorField.h"
#include "Fusion/PropertyEditor/Fields/TextEditorField.h"
#include "Fusion/PropertyEditor/Fields/BoolEditorField.h"
#include "Fusion/PropertyEditor/Fields/NumericEditorField.h"
#include "Fusion/PropertyEditor/Fields/VectorEditorField.h"
#include "Fusion/PropertyEditor/Fields/ColorEditorField.h"
#include "Fusion/PropertyEditor/Fields/ColorComponentField.h"

#include "Fusion/PropertyEditor/ObjectEditor.h"
#include "Fusion/PropertyEditor/ObjectEditorRegistry.h"
#include "Fusion/PropertyEditor/PropertyEditor.h"
#include "Fusion/PropertyEditor/ArrayPropertyEditor.h"
#include "Fusion/PropertyEditor/PropertyEditorRegistry.h"

#include "Fusion/Widget/Editor/EditorToolBar.h"
#include "Fusion/Widget/Editor/EditorMenuBar.h"
#include "Fusion/Widget/Editor/EditorMenuPopup.h"
#include "Fusion/Widget/Editor/EditorViewport.h"
#include "Fusion/Widget/Editor/EditorWindow.h"

#include "Fusion/Widget/ComponentTreeView/ComponentTreeItem.h"
#include "Fusion/Widget/ComponentTreeView/ComponentTreeViewRow.h"
#include "Fusion/Widget/ComponentTreeView/ComponentTreeView.h"
#include "Fusion/Widget/ComponentTreeView/ComponentTreeViewStyle.h"

// Docking
#include "Fusion/Docking/EditorDockspace.h"
#include "Fusion/Docking/EditorDockTabItem.h"
#include "Fusion/Docking/EditorDockTab.h"
#include "Fusion/Widget/Editor/EditorBase.h"

#include "Fusion/Docking/EditorMinorDockspace.h"

// - Tools -
#include "Fusion/Widget/Tools/ColorPickerPreview.h"
#include "Fusion/Widget/Tools/ColorPickerTool.h"

namespace CE::Editor
{

    class EDITORCORE_API EditorCoreModule : public Module
    {
    public:

        void StartupModule() override;

        void ShutdownModule() override;

        void RegisterTypes() override;

        void DeregisterTypes() override;

        void InitializeStyle(EditorStyle* style);
        void ShutdownStyle();

        Ref<EditorStyle> GetEditorStyle();

    };
    
} // namespace CE::Editor


