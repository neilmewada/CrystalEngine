#include "EditorCore.h"

namespace CE::Editor
{

    ActorComponentMenu::ActorComponentMenu()
    {

    }

    void ActorComponentMenu::Construct()
    {
        Super::Construct();

        MinWidth(200);

        Content(
            FAssignNew(FTextInput, searchBox),

            FNew(FScrollBox)
            .FillRatio(1.0f)
            .Height(300)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FAssignNew(FVerticalStack, contentBox)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Top)
            )
        );
    }

    void ActorComponentMenu::Show(Ref<FButton> senderButton)
    {
        ClosePopup();

        contentBox->DestroyAllChildren();

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

