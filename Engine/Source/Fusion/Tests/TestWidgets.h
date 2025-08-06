#pragma once

using namespace CE;

namespace WidgetTests
{
    CLASS()
	class MajorDockspaceWindow : public FDockspaceWindow
	{
		CE_CLASS(MajorDockspaceWindow, FDockspaceWindow)
    public:

        MajorDockspaceWindow();

        void Construct() override;

	};

    CLASS()
    class MajorDockspace : public FDockspace
    {
        CE_CLASS(MajorDockspace, FDockspace)
    public:

        MajorDockspace();

        void Construct() override;
        
    };


    CLASS()
    class MinorDockspaceWindow : public FDockspaceWindow
    {
        CE_CLASS(MinorDockspaceWindow, FDockspaceWindow)
    public:

        MinorDockspaceWindow();

        void Construct() override;

    };

    CLASS()
    class MinorDockspace : public FDockspace
    {
        CE_CLASS(MinorDockspace, FDockspace)
    public:

        MinorDockspace();

        void Construct() override;

    };

    CLASS()
    class SceneTreeView : public FTreeView
    {
        CE_CLASS(SceneTreeView, FTreeView)
    public:

        // - Public API -

    protected:

        SceneTreeView();

        void Construct() override;

        FTreeViewRow& GenerateRow();

        FUNCTION()
        FMenuPopup& BuildRowContextMenu(FTreeViewRow& row);

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };

    CLASS()
    class SceneTreeViewModel : public FAbstractItemModel
    {
        CE_CLASS(SceneTreeViewModel, FAbstractItemModel)
    public:

        virtual ~SceneTreeViewModel();

    protected:

        SceneTreeViewModel();

        void OnBeforeDestroy() override;

    public:

        bool IsReady() override { return true; }

        FModelIndex GetIndex(u32 row, u32 column, const FModelIndex& parent) override;

        u32 GetRowCount(const FModelIndex& parent) override;

        u32 GetColumnCount(const FModelIndex& parent) override;

        void SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent) override;

        FModelIndex GetParent(const FModelIndex& index) override;

        PathTree* pathTree = nullptr;
    };
    
} // namespace WidgetTests

#include "TestWidgets.rtti.h"