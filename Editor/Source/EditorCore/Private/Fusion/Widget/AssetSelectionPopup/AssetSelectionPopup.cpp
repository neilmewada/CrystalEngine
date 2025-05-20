#include "EditorCore.h"

namespace CE::Editor
{

    AssetSelectionPopup::AssetSelectionPopup()
    {

    }

    void AssetSelectionPopup::OnBeginDestroy()
    {
        Super::OnBeginDestroy();
    }

    void AssetSelectionPopup::Construct()
    {
        Super::Construct();

        AutoClose(true);
        Width(250);

        model = CreateObject<AssetSelectionListViewModel>(this, "Model");

        constexpr f32 gap = 6;
        container->Gap(gap);
        container->Padding(Vec4(1, 1, 1, 1) * gap);

        Content(
            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            .Gap(5)
            .HAlign(HAlign::Fill)
            (
                FAssignNew(FTextInput, searchBox)
                .OnTextEdited([this](FTextInput* input)
                {
                    model->SetFilter(input->Text());
                })
                .FillRatio(1.0f),

                FNew(FImageButton)
                .Image(FBrush("/Editor/Assets/Icons/Settings"))
                .Width(16)
                .Height(16)
                .Style("Button.Icon")
                .Padding(Vec4(1, 1, 1, 1) * 3)
            ),

            FAssignNew(AssetSelectionListView, listView)
            .Model(model)
            .OnSelectionChanged([this](FListView*)
            {
                for (int rowIndex : listView->GetSelectedRowIndices())
                {
                    AssetData* assetData = model->GetRowData(rowIndex);
                    if (assetData)
                    {
                        m_OnAssetSelected.Broadcast(assetData);
                        ClosePopup();
                        break;
                    }
                }
            })
            .HAlign(HAlign::Fill)
            .FillRatio(1.0f)
        );
    }

    void AssetSelectionPopup::HandleEvent(FEvent* event)
    {
        if (event->IsKeyEvent())
        {
            FKeyEvent* keyEvent = static_cast<FKeyEvent*>(event);

            if (event->type == FEventType::KeyPress && keyEvent->key == KeyCode::Escape)
            {
                if (!event->isConsumed)
                {
                    ClosePopup();
                }
            }
        }

        Super::HandleEvent(event);
    }

    void AssetSelectionPopup::SetAssetClass(ClassType* assetClass)
    {
        model->SetAssetClass(assetClass);
    }

    void AssetSelectionPopup::StartEditingSearchBox()
    {
        searchBox->StartEditing(true);
    }
}

