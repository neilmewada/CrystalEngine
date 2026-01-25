#include "CrystalEditor.h"

namespace CE::Editor
{

    ProjectBrowser::ProjectBrowser()
    {
        
    }

    ProjectBrowser::~ProjectBrowser()
    {
	    
    }

    void ProjectBrowser::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

        
    }

    void ProjectBrowser::Construct()
    {
	    Super::Construct();
        
        ProjectManager* projectManager = ProjectManager::Get();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        recentProjectsModel = CreateObject<RecentProjectsListModel>(this, "RecentProjectsListModel");
        newProjectModel = CreateObject<NewProjectListModel>(this, "NewProjectListModel");

        static const Array<EditorPlatform::FileType> fileTypes = { {.desc = "Crystal Project File", .extensions = { "*.cproject" } } };

        auto& self = *this;

        self
			.Title("Project Browser")
    		.ContentDirection(FStackBoxDirection::Vertical)
			.ContentPadding(Vec4(0, 0, 0, 0))
    		.Content(
	            FAssignNew(FTabView, tabView)
	            .FillRatio(1.0f)
                .Style("TabView.ProjectBrowser")
	            .As<FTabView>()
	            .TabItems(

                    // - Recent Projects -

	                FNew(FLabelTabItem)
                    .FontSize(14)
                    .Bold(true)
	                .Text("Recent Projects")
                    .ContentWidget(
                        FNew(FVerticalStack)
                        .ContentHAlign(HAlign::Fill)
                        .VAlign(VAlign::Fill)
                        .HAlign(HAlign::Fill)
                        .Padding(Vec4(1, 1, 1, 1) * 20)
                        (
                            FAssignNew(FListWidget, recentsList)
                            .GenerateRowDelegate(MemberDelegate(&Self::GenerateRecentProjectRow, this))
                            .Bind_ItemList(BIND_PROPERTY_R(recentProjectsModel, ItemList))
                            .SelectionMode(FSelectionMode::Single)
                            .OnSelectionChanged([this](FListWidget*)
                            {
                                int index = recentsList->GetSelectedItemIndex();
                                if (index >= 0)
                                {
                                    IO::Path path = recentProjectsModel->GetRecentProjectsPath(index);
                                    openProjectLocation->Text(path.GetString());

                                    ValidateInputFields(nullptr);
                                }
                            })
                            .HAlign(HAlign::Fill)
                            .Height(400)
                            .Style("ProjectBrowserWindow.ListView")
                            .Margin(Vec4(0, 0, 0, 25)),

                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Top)
                            (
                                FNew(FLabel)
                                .Text("Project Location")
                                .FontSize(10)
                                .Width(200)
                                .VAlign(VAlign::Center),

                                FAssignNew(FTextInput, openProjectLocation)
                                .Text(defaultOpenProjectLocation)
                                .OnTextEdited(FUNCTION_BINDING(this, ValidateInputFields))
                                .FillRatio(1.0f),

                                FNew(FTextButton)
                                .Text("...")
                                .FontSize(fontSize)
                                .OnClicked([this]
                                {
                                    String openLocation = EditorPlatform::ShowFileSelectionDialog(defaultOpenProjectLocation, fileTypes)
                                        .GetString()
                                        .Replace({ '\\' }, '/');

                                    if (openLocation.IsEmpty())
                                        return;

                                    defaultOpenProjectLocation = openLocation;
                                    openProjectLocation->Text(defaultOpenProjectLocation);
                                })
                            )
                            .Margin(Vec4(0, 0, 0, 50)),

                            FNew(FHorizontalStack)
                            .Gap(5)
                            .ContentVAlign(VAlign::Center)
                            .HAlign(HAlign::Fill)
                            (
                                FAssignNew(FStyledWidget, recentWarningWidget)
                                .Border(Colors::Red, 0.75f)
                                .BackgroundShape(FRoundedRectangle(2.5f))
                                .Child(
                                    FNew(FHorizontalStack)
                                    .Gap(5)
                                    .ContentVAlign(VAlign::Center)
                                    (
                                        FNew(FImage)
                                        .Background(FBrush("/Engine/Resources/Icons/Warning", Colors::Red))
                                        .Width(11)
                                        .Height(11)
                                        .HAlign(HAlign::Center)
                                        .VAlign(VAlign::Center),

                                        FAssignNew(FLabel, recentWarningLabel)
                                        .Text("This is a warning!")
                                    )
                                )
                                .FillRatio(1.0f)
                                .Padding(Vec4(2, 1, 2, 1) * 2.5f)
                                .Visible(false),

                                FAssignNew(FTextButton, openButton)
                                .TextHAlign(HAlign::Center)
                                .Text("Open")
                                .FontSize(fontSize)
                                .OnClicked([this]
                                {
                                    OpenProject();
                                })
                                .Width(40),

                                FNew(FTextButton)
                                .TextHAlign(HAlign::Center)
                                .Text("Cancel")
                                .FontSize(fontSize)
                                .OnClicked([this]
                                {
                                    CloseWindow();
                                })
                                .Width(40)
                            )
                        )
                    )
                    .Padding(Vec4(1.5f, 1, 1.5f, 1) * 10),

                    // - New Project -

	                FNew(FLabelTabItem)
                    .FontSize(14)
                    .Bold(true)
	                .Text("New Project")
                    .ContentWidget(
                        FNew(FVerticalStack)
                        .ContentHAlign(HAlign::Fill)
                        .VAlign(VAlign::Fill)
                        .HAlign(HAlign::Fill)
                        .Padding(Vec4(1, 1, 1, 1) * 20)
                        (
                            FAssignNew(FListWidget, newProjectList)
                            .GenerateRowDelegate(MemberDelegate(&Self::GenerateNewProjectRow, this))
                            .Bind_ItemList(BIND_PROPERTY_R(newProjectModel, ItemList))
                            .SelectionMode(FSelectionMode::Single)
                            .HAlign(HAlign::Fill)
                            .Height(400)
                            .Style("ProjectBrowserWindow.ListView")
                            .Name("NewProjectListView")
                            .Margin(Vec4(0, 0, 0, 25)),

                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .Margin(Vec4(0, 0, 0, 10))
                            (
                                FNew(FLabel)
                                .Text("Project Location")
                                .FontSize(10)
                                .Width(200)
                                .VAlign(VAlign::Center),

                                FAssignNew(FTextInput, newProjectLocation)
                                .Text(defaultNewProjectLocation)
                                .OnTextEdited(FUNCTION_BINDING(this, ValidateInputFields))
                                .FillRatio(1.0f),

                                FNew(FTextButton)
                                .Text("...")
                                .FontSize(fontSize)
                                .OnClicked([this]
                                {
                                    String newLocation = EditorPlatform::ShowSelectDirectoryDialog(defaultNewProjectLocation)
                                        .GetString()
                                        .Replace({ '\\' }, '/');

                                    if (newLocation.IsEmpty())
                                        return;

                                    defaultNewProjectLocation = newLocation;
                                    newProjectLocation->Text(defaultNewProjectLocation);
                                })
                            ),

                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .Margin(Vec4(0, 0, 0, 15))
                            (
								FNew(FLabel)
                                .Text("Project Name")
                                .FontSize(10)
                                .Width(200)
                                .VAlign(VAlign::Center),

                                FAssignNew(FTextInput, newProjectName)
                                .OnTextEdited(FUNCTION_BINDING(this, ValidateInputFields))
                                .FillRatio(1.0f)
                            ),

                            FNew(FHorizontalStack)
                            .Gap(5)
                            .ContentVAlign(VAlign::Center)
                            .HAlign(HAlign::Fill)
                            (
								FAssignNew(FStyledWidget, newWarningWidget)
                                .Border(Colors::Red, 0.75f)
                                .BackgroundShape(FRoundedRectangle(2.5f))
                                .Child(
									FNew(FHorizontalStack)
                                    .Gap(5)
                                    .ContentVAlign(VAlign::Center)
                                    (
										FNew(FImage)
                                        .Background(FBrush("/Engine/Resources/Icons/Warning", Colors::Red))
                                        .Width(11)
                                        .Height(11)
                                        .HAlign(HAlign::Center)
                                        .VAlign(VAlign::Center),

                                        FAssignNew(FLabel, newWarningLabel)
                                        .Text("This is a warning!")
                                    )
                                )
                                .FillRatio(1.0f)
                                .Padding(Vec4(2, 1, 2, 1) * 2.5f)
                                .Visible(false),

                                FAssignNew(FTextButton, createButton)
                                .TextHAlign(HAlign::Center)
                                .Text("Create")
                                .FontSize(fontSize)
                                .OnClicked([this]
                                {
                                    CreateProject();
                                })
                                .Width(40),

                                FNew(FTextButton)
                                .TextHAlign(HAlign::Center)
                                .Text("Cancel")
                                .FontSize(fontSize)
                                .OnClicked([this]
                                {
                                    CloseWindow();
                                })
                                .Width(40)
                            )
                        )
                    )
                    .Padding(Vec4(1.5f, 1, 1.5f, 1) * 10)
				)
			);

        this->Style("ProjectBrowserWindow");
        
        tabView->GetTabWell()->WindowDragHitTest(true);

        recentProjectsModel->ModelReset();
        newProjectModel->ModelReset();

        newProjectList->SelectItem(0);

        ValidateInputFields(openProjectLocation);

        recentWarningWidget->Visible(false);
        newWarningWidget->Visible(false);
    }

    void ProjectBrowser::CloseWindow()
    {
        GetContext()->QueueDestroy();
    }

    FListItemWidget& ProjectBrowser::GenerateRecentProjectRow(FListItem* item, FListWidget* view)
    {
	    auto projectItem = static_cast<RecentProjectItem*>(item);

        return FNew(FListItemWidget)
            .Child(
                FNew(FVerticalStack)
                .Padding(Vec4(1, 1, 1, 1) * 10)
                (
                    FNew(FLabel)
                    .Text(projectItem->title)
                    .FontSize(14)
                    .Margin(Vec4(0, 0, 0, 2.5f)),

                    FNew(FLabel)
                    .Text(projectItem->description)
                )
            )
            .As<FListItemWidget>()
    	;
    }

    FListItemWidget& ProjectBrowser::GenerateNewProjectRow(FListItem* item, FListWidget* view)
    {
        return FNew(FListItemWidget)
			.Child(
                FNew(FVerticalStack)
                .Padding(Vec4(1, 1, 1, 1) * 10)
                (
                    FNew(FLabel)
                    .Text(item->title)
                    .FontSize(14)
                    .Margin(Vec4(0, 0, 0, 2.5f)),

                    FNew(FLabel)
                    .Text(item->description)
                )
            )
			.As<FListItemWidget>()
        ;
    }

    void ProjectBrowser::ValidateInputFields(FTextInput*)
    {
        static const HashSet<CE::Name> reservedNames = {
            "Game", "Engine", "Editor", "Plugin"
        };

        auto projectManager = ProjectManager::Get();

        // Recent projects tab
        {
            recentWarningWidget->Visible(false);
            openButton->Interactable(true);

	        IO::Path projectLocation = openProjectLocation->Text();
            if (!projectLocation.Exists() || 
                projectLocation.GetExtension().GetString() != ProjectManager::GetProjectFileExtension() ||
                !projectManager->IsValidProjectFile(projectLocation))
            {
                openButton->Interactable(false);
                recentWarningWidget->Visible(true);
                recentWarningLabel->Text("Invalid project path. Please enter a path to a valid project file!");
            }
        }

        // New project tab
        {
            newWarningWidget->Visible(false);
            createButton->Interactable(true);

            IO::Path projectLocation = newProjectLocation->Text();
            String projectName = newProjectName->Text();

            if (!projectLocation.Exists())
            {
                createButton->Interactable(false);
                newWarningWidget->Visible(true);
                newWarningLabel->Text("Project location directory does not exist!");
            }
            else if (!projectLocation.IsDirectory())
            {
                createButton->Interactable(false);
                newWarningWidget->Visible(true);
                newWarningLabel->Text("Project location is not a directory!");
            }
            else if (projectName.IsEmpty())
            {
                createButton->Interactable(false);
                newWarningWidget->Visible(true);
                newWarningLabel->Text("Project name should not be empty!");
            }
            else if (!IsValidObjectName(projectName))
            {
                createButton->Interactable(false);
                newWarningWidget->Visible(true);
                newWarningLabel->Text("Project name should only contain alphabets, numbers and underscores (_)!");
            }
            else if ((projectLocation / projectName).Exists())
            {
                createButton->Interactable(false);
                newWarningWidget->Visible(true);
                newWarningLabel->Text("A project with given name already exists at the Project Location!");
            }
            else if (reservedNames.Exists(projectName))
            {
                createButton->Interactable(false);
                newWarningWidget->Visible(true);
                newWarningLabel->Text("The project name is a reserved word. Please use something else!");
            }
        }
    }

    void ProjectBrowser::OnProjectTemplateSelectionChanged()
    {
        
    }

    void ProjectBrowser::OnRecentProjectSelectionChanged()
    {
        
    }

    void ProjectBrowser::CreateProject()
    {
        ValidateInputFields(newProjectLocation);

        if (!newWarningWidget->Visible())
        {
	        IO::Path location = newProjectLocation->Text();
            if (!location.Exists())
            {
                IO::Path::CreateDirectories(location);
            }

            String projectName = newProjectName->Text();

            if (gEditorMode == EditorMode::ProjectBrowser)
            {
                ProjectManager::Get()->CreateEmptyProject(location / projectName, projectName);

                gExitLaunchProcess = EditorPlatform::GetEditorExecutablePath();
                gExitLaunchArguments = location / projectName / (projectName + ProjectManager::GetProjectFileExtension());
            }
            else if (gEditorMode == EditorMode::Default)
            {
                // TODO: Open project in the Crystal Editor
            }

            CloseWindow();
        }
    }

    void ProjectBrowser::OpenProject()
    {
        ValidateInputFields(openProjectLocation);

        if (!recentWarningWidget->Visible())
        {
	        IO::Path location = openProjectLocation->Text();
            if (!location.Exists() || location.IsDirectory() || location.GetExtension() != ProjectManager::GetProjectFileExtension())
                return;

            if (gEditorMode == EditorMode::ProjectBrowser)
            {
                gExitLaunchProcess = EditorPlatform::GetEditorExecutablePath();
                gExitLaunchArguments = location;

                CloseWindow();
            }
            else if (gEditorMode == EditorMode::Default)
            {
	            // TODO: Open project in the Crystal Editor
            }
        }
    }

} // namespace CE::Editor
