#include "EditorCore.h"

namespace CE::Editor
{

    EditorDockspace::EditorDockspace()
    {

    }

    void EditorDockspace::AddDockTab(EditorDockTab* tab)
    {
    	if (tab == nullptr || dockedEditors.Exists(tab))
            return;

        tab->dockspace = this;

        dockedEditors.Add(tab);

        UpdateTabWell();

        if (selectedTab < 0)
        {
            SelectTab(tab);
        }
    }

    void EditorDockspace::RemoveDockTab(EditorDockTab* tab)
    {
        if (tab == nullptr)
            return;

        int index = (int)dockedEditors.IndexOf(tab);
        if (index < 0)
            return;

        dockedEditors.RemoveAt(index);

        if (selectedTab == index && selectedTab > 0)
        {
            selectedTab--;
        }

        UpdateTabWell();

        if (selectedTab < dockedEditors.GetSize() && selectedTab >= 0)
        {
            SelectTab(dockedEditors[selectedTab].Get());
        }
    }

    void EditorDockspace::SelectTab(EditorDockTabItem* tabItem)
    {
	    for (int i = 0; i < tabItems.GetSize(); ++i)
	    {
		    if (tabItems[i] == tabItem)
		    {
                selectedTab = i;
                content->Child(*dockedEditors[selectedTab]);

                tabItems[i]->isActive = true;
		        dockedEditors[i]->Enabled(true);
		    }
            else
            {
                tabItems[i]->isActive = false;
                dockedEditors[i]->Enabled(false);

                AttachSubobject(dockedEditors[i].Get());
            }
	    }

        ApplyStyle();
    }

    void EditorDockspace::SelectTab(EditorDockTab* tab)
    {
	    for (int i = 0; i < dockedEditors.GetSize(); ++i)
	    {
		    if (dockedEditors[i] == tab)
		    {
                selectedTab = i;
                content->Child(*tab);

                tabItems[i]->isActive = true;
		        dockedEditors[i]->Enabled(true);
		    }
            else
            {
                tabItems[i]->isActive = false;
                dockedEditors[i]->Enabled(false);

                AttachSubobject(dockedEditors[i].Get());
            }
	    }

        ApplyStyle();
    }

    void EditorDockspace::UpdateTabWell()
    {
        while (tabWell->GetChildCount() > dockedEditors.GetSize())
        {
            Ref<FWidget> child = tabWell->GetChild(tabWell->GetChildCount() - 1);
            tabWell->RemoveChild(child.Get());
            child->BeginDestroy();

            tabItems.RemoveAt(tabItems.GetSize() - 1);
        }

        for (int i = 0; i < dockedEditors.GetSize(); ++i)
        {
	        if (i < (int)tabItems.GetSize())
	        {
                EditorDockTabItem& child = *tabItems[i];

                child.dockTab = dockedEditors[i];
                child.CloseButtonEnabled(dockedEditors[i]->CanBeClosed());

                child
					.Text(dockedEditors[i]->Title() + (dockedEditors[i]->ShowAsterisk() ? "*" : ""))
                ;
	        }
            else
            {
                EditorDockTabItem* child = nullptr;

                FAssignNew(EditorDockTabItem, child)
                .CloseButtonEnabled(dockedEditors[i]->CanBeClosed())
                .Text(dockedEditors[i]->Title() + (dockedEditors[i]->ShowAsterisk() ? "*" : ""))
                .VAlign(VAlign::Fill)
            	;

                child->dockTab = dockedEditors[i];

                tabWell->AddChild(child);
                tabItems.Add(child);
            }
        }

        ApplyStyle();
    }

    bool EditorDockspace::OpenEditor(Ref<Object> targetObject)
    {
        if (!targetObject)
            return false;

        Ref<AssetEditorRegistry> registry = AssetEditorRegistry::Get();
        SubClass<EditorBase> editorClass = registry->GetEditorClass(targetObject->GetClass());
        if (!editorClass.IsValid())
            return false;

        auto editorClassDefaults = (const EditorBase*)editorClass->GetDefaultInstance();
        if (!editorClassDefaults)
            return false;
        if (!editorClassDefaults->CanEdit(targetObject))
            return false;

        bool allowMultipleInstances = editorClassDefaults->AllowMultipleInstances();
        Ref<EditorBase> existingEditor = nullptr;

        for (int i = 0; i < dockedEditors.GetSize(); i++)
        {
            if (dockedEditors[i]->IsOfType<EditorBase>())
            {
                auto editor = (Ref<EditorBase>)dockedEditors[i];
                if (editor->IsOfType(editorClass))
                {
                    if (!allowMultipleInstances)
                    {
                        existingEditor = editor;
                    }

                    if (editor->GetTargetObject() == targetObject)
                    {
                        SelectTab(editor.Get());
                        return true;
                    }
                }
            }
        }

        bool success = false;

        if (existingEditor.IsValid())
        {
            SelectTab(existingEditor.Get());

            success = existingEditor->OpenEditor(targetObject);
        }
        else
        {
            Ref<EditorBase> editor = CreateObject<EditorBase>(this, editorClass->GetName().GetLastComponent(),
                OF_NoFlags, editorClass);
            existingEditor = editor;

            AddDockTab(editor.Get());
            SelectTab(editor.Get());

            success = editor->OpenEditor(targetObject);
        }

        if (success)
        {
            existingEditor->OnEditorOpened(targetObject);
        }

        return success;
    }

    bool EditorDockspace::OpenEditor(const CE::Name& assetPath)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return false;

        AssetData* primaryAssetData = registry->GetPrimaryAssetByPath(assetPath);
        if (!primaryAssetData)
            return false;

        Ref<Asset> asset = AssetManager::Get()->LoadAssetAtPath(assetPath);
        return OpenEditor(asset);
    }

    void EditorDockspace::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FStyledWidget, borderWidget)
            .Background(FBrush(Color::RGBA(36, 36, 36)))
            .BorderWidth(1.0f)
            .BorderColor(Color::RGBA(15, 15, 15))
            .Padding(Vec4(1, 1, 1, 1) * 2.0f)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .Name("RootStyle")
            (
                FAssignNew(FVerticalStack, rootBox)
                .ContentHAlign(HAlign::Fill)
                .ContentVAlign(VAlign::Top)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                .Name("RootBox")
                (
                    // - Title Bar Begin -

                    FAssignNew(FTitleBar, titleBar)
                    .Background(FBrush(Color::RGBA(26, 26, 26)))
                    .Height(40)
                    .HAlign(HAlign::Fill)
                    (
                        FAssignNew(FHorizontalStack, titleBarContainer)
                        .ContentVAlign(VAlign::Center)
                        .HAlign(HAlign::Fill)
                        .VAlign(VAlign::Fill)
                        (
                            // - Logo -

                            FAssignNew(FImage, logo)
                            .Background(FBrush("/Editor/Assets/Icons/Logo128"))
                            .Width(32)
                            .Height(32)
                            .VAlign(VAlign::Center)
                            .Margin(Vec4(10, 0, 10, 0)),

                            // - Tabs -

                            FAssignNew(FHorizontalStack, tabWell)
                            .ContentHAlign(HAlign::Left)
                            .VAlign(VAlign::Fill)
                            .FillRatio(1.0f)
                            .Name("TabWell")
                            .Padding(Vec4(5, 5, 0, 0)),

                            // - Project Title -

                            FAssignNew(FStyledWidget, projectLabelParent)
                            .VAlign(VAlign::Top)
                            .Height(16)
                            .Padding(Vec4(2, 1, 2, 1) * 6)
                            .Margin(Vec4(0, 0, 60, 0))
                            .Enabled(false)
                            (
                                FAssignNew(FLabel, projectLabel)
                                .FontSize(11)
                                .Text("Project Name")
                                .VAlign(VAlign::Center)
                            ),

                            // - Window Controls -

                            FAssignNew(FWindowControlButton, minimizeButton)
                            .ControlType(FWindowControlType::Minimize)
                            .OnClicked([this]
                            {
                                CastTo<FNativeContext>(GetContext())->Minimize();
                            })
                            .Name("WindowMinimizeButton")
                            .Style("Button.WindowControl"),

                            FAssignNew(FWindowControlButton, maximizeButton)
                            .ControlType(FWindowControlType::Maximize)
                            .OnClicked([this]
                            {
                                Ref<FNativeContext> nativeContext = CastTo<FNativeContext>(GetContext());
                                if (nativeContext->IsMaximized())
                                {
                                    nativeContext->Restore();
                                }
                                else
                                {
                                    nativeContext->Maximize();
                                }
                            })
                            .Name("WindowMaximizeButton")
                            .Style("Button.WindowControl"),

                            FAssignNew(FWindowControlButton, closeButton)
                            .ControlType(FWindowControlType::Close)
                            .OnClicked([this]
                            {
                                GetContext()->QueueDestroy();
                            })
                            .Name("WindowCloseButton")
                            .Style("Button.WindowClose")
                        )
                    ),

                    // - Title Bar End -

                    FAssignNew(FStyledWidget, content)
                    .Padding(Vec4(0, 0, 0, 0))
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Fill)
                    .FillRatio(1.0f)
                    .Name("Content")
                )

            ) // End of Child
        );

        Array<WeakRef<FWindowControlButton>> controlGroup = {
            closeButton,
            minimizeButton,
            maximizeButton
        };

        closeButton->SetControlGroup(controlGroup);
        minimizeButton->SetControlGroup(controlGroup);
        maximizeButton->SetControlGroup(controlGroup);

        if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
        {
            titleBarContainer->MoveChildToIndex(closeButton, 0);
            titleBarContainer->MoveChildToIndex(minimizeButton, 1);
            titleBarContainer->MoveChildToIndex(maximizeButton, 2);

            closeButton->Margin(Vec4(10, 0, -1, 0));
            minimizeButton->Margin(Vec4(0, 0, -1, 0));
        }
    }

    void EditorDockspace::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        
    }

    void EditorDockspace::OnMaximized()
    {
	    Super::OnMaximized();

        maximizeButton->SetMaximizedState(true);

#if PLATFORM_WINDOWS
        // This is needed on Windows to prevent things from rendering outside the screen edges when maximized
        borderWidget->Padding(Vec4(1, 1, 1, 1) * 7);
#endif
    }

    void EditorDockspace::OnRestored()
    {
        Super::OnRestored();

        maximizeButton->SetMaximizedState(false);

#if PLATFORM_WINDOWS
        borderWidget->Padding(Vec4(1, 1, 1, 1) * 1);
#endif
    }
}

