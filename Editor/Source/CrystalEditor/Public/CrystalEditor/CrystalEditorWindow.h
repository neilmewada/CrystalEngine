#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API CrystalEditorWindow : public EditorMajorDockspaceWindow
    {
        CE_CLASS(CrystalEditorWindow, EditorMajorDockspaceWindow)
    public:

        // - Public API -

        static CrystalEditorWindow* Get();

        //! Internal use only!
        static void Launch();

    protected:

        CrystalEditorWindow();

        void Construct() override;

        void OnBeginDestroy() override;

        FUNCTION()
        void OnAssetProcessorUpdate(AssetProcessor* assetProcessor);

        Ref<AssetImportProgressPopup> assetImportProgressPopup;
        bool progressPopupShown = false;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "CrystalEditorWindow.rtti.h"
