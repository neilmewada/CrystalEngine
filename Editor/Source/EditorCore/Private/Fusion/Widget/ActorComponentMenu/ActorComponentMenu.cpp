#include "EditorCore.h"

namespace CE::Editor
{

    ActorComponentMenu::ActorComponentMenu()
    {

    }

    void ActorComponentMenu::Construct()
    {
        Super::Construct();

        Width(200);
        Height(150);

        treeViewModel = CreateObject<ActorComponentTreeViewModel>(this, "ActorComponentTreeViewModel");

        Content(
            FAssignNew(FTextInput, searchBox),

            FAssignNew(ActorComponentTreeView, treeView)
            .Model(treeViewModel)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .FillRatio(1.0f)
        );
    }

    void ActorComponentMenu::OnPopupClosed()
    {
	    Super::OnPopupClosed();


    }

    void ActorComponentMenu::Show(Ref<FButton> senderButton)
    {
        ClosePopup();

        if (!senderButton)
            return;

        Ref<FFusionContext> context = senderButton->GetContext();
        if (!context)
            return;

        // TODO: Build content

        ClassType* base = ActorComponent::StaticClass();

        context->PushLocalPopup(this, senderButton->GetGlobalPosition() + Vec2(0, senderButton->GetComputedSize().height),
            Vec2(), senderButton->GetComputedSize());
    }

}

