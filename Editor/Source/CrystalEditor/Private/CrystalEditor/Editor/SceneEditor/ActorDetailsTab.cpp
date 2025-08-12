#include "CrystalEditor.h"

namespace CE::Editor
{

    ActorDetailsTab::ActorDetailsTab()
    {
        addComponentMenu = CreateDefaultSubobject<AddComponentMenu>("AddComponentMenu");
    }

    void ActorDetailsTab::Construct()
    {
        Super::Construct();

        ConstructMinorDockWindow();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        addComponentMenu->OnComponentClassSelected(FUNCTION_BINDING(this, AddNewComponent));

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

        if (!item)
            return;
        
        if (Actor* actor = item->GetActor())
        {
            editor = ObjectEditorRegistry::Get().Create(actor, GetOwnerEditor()->GetHistory());

            editorContainer->AddChild(editor.Get());
        }
        else if (ActorComponent* component = item->GetComponent())
        {
            editor = ObjectEditorRegistry::Get().Create(component, GetOwnerEditor()->GetHistory());

            editorContainer->AddChild(editor.Get());
        }
    }

    void ActorDetailsTab::OnAddComponentButtonClicked(FButton* button, Vec2 mousePos)
    {
        addComponentMenu->Show(button);
    }

    void ActorDetailsTab::AddNewComponent(ClassType* componentClass)
    {
        if (!componentClass || !componentClass->IsSubclassOf<ActorComponent>())
            return;
        if (!componentClass->CanBeInstantiated())
            return;

        Ref<ComponentTreeItem> selectedItem = treeView->GetSelectedItem();
        if (!selectedItem)
        {
            selectedItem = treeView->GetActorItem();
        }

        String newComponentName = FixObjectName(componentClass->GetName().GetLastComponent());

        if (selectedItem->IsActor())
        {
            Ref<Actor> actor = selectedItem->GetActor();

            if (componentClass->IsSubclassOf<SceneComponent>())
            {
                Ref<SceneComponent> sceneComponent = CreateObject<SceneComponent>(actor.Get(), newComponentName, OF_NoFlags, componentClass);
                if (actor->GetRootComponent() == nullptr)
                {
                    actor->SetRootComponent(sceneComponent.Get());
                }
                else
                {
                    actor->GetRootComponent()->SetupAttachment(sceneComponent.Get());
                }
            }
            else
            {
                Ref<ActorComponent> component = CreateObject<ActorComponent>(actor.Get(), newComponentName, OF_NoFlags, componentClass);
                actor->AttachComponent(component.Get());
            }
        }
        else if (selectedItem->IsSceneComponent())
        {
            Ref<SceneComponent> sceneComponent = (SceneComponent*)selectedItem->GetComponent();

            if (componentClass->IsSubclassOf<SceneComponent>())
            {
                Ref<SceneComponent> newComponent = CreateObject<SceneComponent>(sceneComponent.Get(), newComponentName, OF_NoFlags, componentClass);
                sceneComponent->SetupAttachment(newComponent.Get());
            }
            else if (Ref<Actor> actor = sceneComponent->GetActor())
            {
                Ref<SceneComponent> newComponent = CreateObject<SceneComponent>(actor.Get(), newComponentName, OF_NoFlags, componentClass);
                actor->AttachComponent(newComponent.Get());
            }
        }
        else if (selectedItem->IsActorComponent())
        {
            Ref<ActorComponent> component = selectedItem->GetComponent();

            if (componentClass->IsSubclassOf<SceneComponent>())
            {
                if (Ref<Actor> actor = component->GetActor())
                {
                    Ref<SceneComponent> newComponent = CreateObject<SceneComponent>(component.Get(), newComponentName, OF_NoFlags, componentClass);
                    if (actor->GetRootComponent() == nullptr)
                    {
                        actor->SetRootComponent(newComponent.Get());
                    }
                    else
                    {
                        actor->GetRootComponent()->SetupAttachment(newComponent.Get());
                    }
                }
            }
            else if (Ref<Actor> actor = component->GetActor())
            {
                Ref<ActorComponent> newComponent = CreateObject<ActorComponent>(actor.Get(), newComponentName, OF_NoFlags, componentClass);
                actor->AttachComponent(newComponent.Get());
            }
        }

        treeView->Update();
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

