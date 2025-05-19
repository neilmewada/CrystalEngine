#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API RecentProjectItem : public FListItem
    {
        CE_CLASS(RecentProjectItem, FListItem)
    public:

        RecentProjectItem();

    };

    CLASS()
    class CRYSTALEDITOR_API RecentProjectsListModel : public FListWidgetModel
    {
        CE_CLASS(RecentProjectsListModel, FListWidgetModel)
    public:

        RecentProjectsListModel();

        virtual ~RecentProjectsListModel();

        // - Public API -

        void ModelReset();

        const IO::Path& GetRecentProjectsPath(int index) const
        {
            return recentProjectPaths[index];
        }

    private:

        Array<IO::Path> recentProjectPaths{};

    public:



    };
    
} // namespace CE::Editor

#include "RecentProjectsListModel.rtti.h"
