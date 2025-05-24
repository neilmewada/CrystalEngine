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

    FMenuItem& AssetSelectionPopup::NewMenuItem()
    {
        return
        FNew(FMenuItem)
        .IconEnabled(true)
        .IconVisible(true)
        .FontSize(9)
        .ContentPadding(Vec4(5, -2, 5, -2));
    }

    void AssetSelectionPopup::Construct()
    {
        Super::Construct();

        AutoClose(true);
        Width(250);

        model = CreateObject<AssetSelectionListViewModel>(this, "Model");

        constexpr f32 gap = 6;

        Content(
            FNew(FMenuItemSeparator)
            .Title("BASIC"),

            NewMenuItem()
            .Text("Copy Reference")
            .Icon(FBrush("/Editor/Assets/Icons/Copy"))
            .OnClick([this]
            {
                if (Ref<Object> object = curValue.Lock())
                {
                    if (Ref<Bundle> bundle = object->GetBundle())
                    {
                        // TODO: Copy object reference to be able to paste it somewhere else in the engine
                    }
                }
            }),

            NewMenuItem()
            .Text("Copy Asset Path")
            .Icon(FBrush("/Editor/Assets/Icons/Copy"))
            .OnClick([this]
            {
                if (Ref<Object> object = curValue.Lock())
                {
                    if (Ref<Bundle> bundle = object->GetBundle())
                    {
                        PlatformApplication::Get()->SetClipboardText(bundle->GetBundlePath().GetString());
                    }
                }
            }),

            NewMenuItem()
            .Text("Paste Reference")
            .Icon(FBrush("/Editor/Assets/Icons/Paste"))
            .OnClick([this]
            {
                // TODO: Paste behaviour
            }),

            FNew(FVerticalStack)
            .Gap(gap)
            .ContentHAlign(HAlign::Fill)
            .HAlign(HAlign::Fill)
            .FillRatio(1.0f)
            .Padding(Vec4(1, 1, 1, 1) * gap)
            (
                FNew(FHorizontalStack)
                .ContentVAlign(VAlign::Center)
                .Gap(5)
                .HAlign(HAlign::Fill)
                (
                    FAssignNew(FTextInput, searchBox)
                    .OnTextEdited([this] (FTextInput* input)
                    {
                        model->SetFilter(input->Text());
                    })
                    .OnTextEditingFinished([this] (FTextInput* input)
                    {
                        model->SetFilter(input->Text());
                    })
                    .FillRatio(1.0f),

                    FNew(FImageButton)
                    .Image(FBrush("/Editor/Assets/Icons/Settings"))
                    .DropDownMenu(
                        FNew(FMenuPopup)
                        .Content(
                            FNew(FMenuItem)
                            .Text("Option 1"),

                            FNew(FMenuItem)
                            .Text("Option 2")
                        )
                        .BlockInteraction(false)
                        .AutoClose(true)
                        .As<FMenuPopup>()
                    )
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
            )
        );
    }

    void AssetSelectionPopup::HandleEvent(FEvent* event)
    {
        if (event->IsKeyEvent())
        {
            auto keyEvent = static_cast<FKeyEvent*>(event);

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

    void AssetSelectionPopup::SetCurrentValue(WeakRef<Object> curValue)
    {
        this->curValue = curValue;
    }

    void AssetSelectionPopup::StartEditingSearchBox()
    {
        searchBox->StartEditing(true);
    }

}

