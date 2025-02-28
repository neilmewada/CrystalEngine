#pragma once

namespace CE::Editor
{
    class AssetBrowserItem;

    CLASS()
    class CRYSTALEDITOR_API AssetBrowserGridViewModel : public Object, public IAssetRegistryListener
    {
        CE_CLASS(AssetBrowserGridViewModel, Object)
    protected:

        AssetBrowserGridViewModel();

        void OnAssetPathTreeUpdated(PathTree& pathTree) override;

        void OnBeforeDestroy() override;

    public:

        virtual ~AssetBrowserGridViewModel();

        void Init();

        void SetCurrentDirectory(const Name& path);

        PathTreeNode* GetCurrentDirectory() const { return currentDirectory; }

        void SetData(AssetBrowserItem& item, PathTreeNode* node);

    private:
        Name currentPath;
        PathTreeNode* currentDirectory = nullptr;

        friend class AssetBrowserTreeView;
    };
    
} // namespace CE

#include "AssetBrowserGridViewModel.rtti.h"
