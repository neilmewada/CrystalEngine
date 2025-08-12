#include "EditorCore.h"

namespace CE::Editor
{

    AddComponentMenu::AddComponentMenu()
    {

    }

    void AddComponentMenu::Construct()
    {
        Super::Construct();

        Width(300);
        Height(300);

        treeViewModel = CreateObject<AddComponentTreeViewModel>(this, "ActorComponentTreeViewModel");

        Content(
            FAssignNew(FTextInput, searchBox)
            .OnTextEdited([this](FTextInput*)
            {
	            treeViewModel->SetFilter(searchBox->Text());
            }),

            FAssignNew(AddComponentTreeView, treeView)
            .Model(treeViewModel)
            .OnSelectionChanged([this]
            {
                FModelIndex index = treeView->SelectionModel()->GetSelectedIndex();
                if (index.IsValid() && index.GetData().HasValue())
                {
	                if (ClassType* clazz = index.GetData().GetValue<ClassType*>())
	                {
                        if (clazz->CanBeInstantiated())
                        {
                            m_OnComponentClassSelected.Invoke(clazz);

                            ClosePopup();
                        }
	                }
                }
            })
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .FillRatio(1.0f)
        );
    }

    void AddComponentMenu::OnPopupClosed()
    {
	    Super::OnPopupClosed();


    }

    void AddComponentMenu::Show(Ref<FButton> senderButton)
    {
        ClosePopup();

        if (!senderButton)
            return;

        Ref<FFusionContext> context = senderButton->GetContext();
        if (!context)
            return;

        treeView->ClearSelection();
        searchBox->Text("");
        treeViewModel->SetFilter(searchBox->Text());

        auto btnSize = senderButton->GetComputedSize();

        context->PushLocalPopup(this, senderButton->GetGlobalPosition() + Vec2(btnSize.width - MinWidth(), btnSize.height),
            Vec2(), btnSize);

        treeView->ExpandAllRows();

        searchBox->StartEditing(true, true);
    }

}

