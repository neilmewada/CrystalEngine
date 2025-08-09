#include "CrystalEditor.h"

namespace CE::Editor
{

    ActorDetailsTab::ActorDetailsTab()
    {
        addComponentMenu = CreateDefaultSubobject<ActorComponentMenu>("AddComponentMenu");
    }

    void ActorDetailsTab::Construct()
    {
        Super::Construct();

        ConstructMinorDockWindow();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        (*this)
		.Title("Details")
		.Child(
            FNew(FOverlayStack)
            .VAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            (
                FAssignNew(FLabel, emptyLabel)
                .Text("Please select an actor in Scene Outliner to see its properties.")
                .VAlign(VAlign::Top)
                .HAlign(HAlign::Center)
                .Margin(Vec4(0, 50, 0, 0)),

                FAssignNew(FCompoundWidget, detailsContainer)
                .VAlign(VAlign::Fill)
                .HAlign(HAlign::Fill)
                .As<FCompoundWidget>()
                .Child(
                    FNew(FSplitBox)
                    .Direction(FSplitDirection::Vertical)
                    .SplitterSize(4.0f)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill)
                    .FillRatio(1.0f)
                    (
                        FNew(FVerticalStack)
                        .HAlign(HAlign::Fill)
                        .FillRatio(0.3f)
                        (
                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .HAlign(HAlign::Fill)
                            .Padding(Vec4(5, 10, 5, 10))
                            (
                                FAssignNew(FLabel, actorName)
                                .Text("Actor Name")
                                .FontSize(11),

                                FNew(FWidget)
                                .FillRatio(1.0f),

                                FNew(FButton)
                                .OnButtonClicked(FUNCTION_BINDING(this, OnAddComponentButtonClicked))
                                .Child(
                                    FNew(FHorizontalStack)
                                    .ContentVAlign(VAlign::Center)
                                    .Gap(5)
                                    .HAlign(HAlign::Center)
                                    (
                                        FNew(FImage)
                                        .Background(FBrush("/Editor/Assets/Icons/Plus").WithTint(Color::RGBHex(0x4CAF50)))
                                        .Width(14)
                                        .Height(14),

                                        FNew(FLabel)
                                        .Text("Add Component")
                                        .FontSize(fontSize - 1)
                                    )
                                )
                                .Height(17)
                            ),

                            FAssignNew(ComponentTreeView, treeView)
                            .OnSelectionChanged(FUNCTION_BINDING(this, OnComponentSelectionChanged))
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            .FillRatio(1.0f)
                        ),

                        FNew(FScrollBox)
                        .VerticalScroll(true)
                        .HorizontalScroll(false)
                        .HAlign(HAlign::Fill)
                        .FillRatio(0.7f)
                        .Margin(Vec4(0, 5, 0, 0))
                        (
                            FAssignNew(FStyledWidget, editorContainer)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Top)
                        )
                    )

                )
            )
        );

        
        SetSelectedActor(nullptr);
    }

    void ActorDetailsTab::OnComponentSelectionChanged(ComponentTreeItem* item)
    {
        editorContainer->RemoveChildWidget();
        f32 splitRatio = -1;

        if (editor.IsValid())
        {
            splitRatio = editor->GetSplitRatio();
            editor->BeginDestroy();
            editor = nullptr;
        }

        if (item && item->GetActor())
        {
            editor = ObjectEditorRegistry::Get().Create(item->GetActor(), GetOwnerEditor()->GetHistory());

            editorContainer->AddChild(editor.Get());
        }
    }

    void ActorDetailsTab::OnAddComponentButtonClicked(FButton* button, Vec2 mousePos)
    {
        addComponentMenu->Show(button);
    }

    void ActorDetailsTab::SetSelectedActor(Actor* actor)
    {
        if (actor)
        {
            treeView->SetActor(actor);
            actorName->Text(actor->GetName().GetString());
        }

        bool actorExists = actor != nullptr;

        emptyLabel->Enabled(!actorExists);
        detailsContainer->Enabled(actorExists);
    }

}

