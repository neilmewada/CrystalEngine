#pragma once

namespace CE::Editor
{

    CLASS(Prefs = Editor)
    class CRYSTALEDITOR_API ProjectBrowser : public FToolWindow
    {
        CE_CLASS(ProjectBrowser, FToolWindow)
    public:

        ProjectBrowser();

        virtual ~ProjectBrowser();

        void OnPostComputeLayout() override;

    protected:

        void Construct() override;

        void CloseWindow();

        FListItemWidget& GenerateRecentProjectRow(FListItem* item, FListWidget* view);

        FListItemWidget& GenerateNewProjectRow(FListItem* item, FListWidget* view);

        FUNCTION()
        void ValidateInputFields(FTextInput* inputField);

        FUNCTION()
        void OnProjectTemplateSelectionChanged();

        FUNCTION()
        void OnRecentProjectSelectionChanged();

        FUNCTION()
        void CreateProject();

        FUNCTION()
        void OpenProject();

        FIELD()
        RecentProjectsListModel* recentProjectsModel = nullptr;

        FIELD()
        NewProjectListModel* newProjectModel = nullptr;

        FTabView* tabView = nullptr;
        FListWidget* recentsList = nullptr;
        FListWidget* newProjectList = nullptr;

        FStyledWidget* recentWarningWidget = nullptr;
        FStyledWidget* newWarningWidget = nullptr;
        FLabel* recentWarningLabel = nullptr;
        FLabel* newWarningLabel = nullptr;

        FTextButton* openButton = nullptr;
        FTextButton* createButton = nullptr;

        FTextInput* openProjectLocation = nullptr;

        FTextInput* newProjectLocation = nullptr;
        FTextInput* newProjectName = nullptr;

        bool isValidInput = true;

        // - Prefs -
        // Preferences are automatically cached and saved to AppData folder and
        // reloaded whenever a ProjectBrowser object is created.

        FIELD(Prefs)
        String defaultOpenProjectLocation = "";

        FIELD(Prefs)
        String defaultNewProjectLocation = "";

        // - Fields -

    };
    
} // namespace CE::Editor

#include "ProjectBrowser.rtti.h"