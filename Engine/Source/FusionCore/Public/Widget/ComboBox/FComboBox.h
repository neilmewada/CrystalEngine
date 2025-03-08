#pragma once

namespace CE
{
    class FPopup;
    class FVerticalStack;
    class FComboBoxPopup;
    class FComboBoxItem;
    class FComboBoxItemStyle;
    class FComboBoxStyle;

    typedef ScriptEvent<void(FComboBox*)> FComboBoxEvent;

    ENUM(Flags)
    enum class FComboBoxState
    {
	    Default = 0,
        Hovered = BIT(0),
        Pressed = BIT(1),
        Expanded = BIT(2)
    };
    ENUM_CLASS_FLAGS(FComboBoxState);

    CLASS()
    class FUSIONCORE_API FComboBox : public FStyledWidget
    {
        CE_CLASS(FComboBox, FStyledWidget)
    public:

        FComboBox();

        bool IsHovered() const { return EnumHasFlag(state, FComboBoxState::Hovered); }
        bool IsPressed() const { return EnumHasFlag(state, FComboBoxState::Pressed); }
        bool IsExpanded() const { return EnumHasFlag(state, FComboBoxState::Expanded); }

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsFocusEvents() const override { return true; }

        bool SupportsKeyboardEvents() const override { return true; }

        void ApplyStyle() override;

        FComboBoxPopup& GetPopupMenu() const { return *popupMenu; }

        void SelectItem(FComboBoxItem* item);
        void SelectItem(int index);

        int GetItemCount() const;
        FComboBoxItem* GetItem(int index);
        int GetSelectedItemIndex();

    protected:

        void OnBeforeDestroy() override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        void SelectItemInternal(FComboBoxItem* item);

        void AddItem(FComboBoxItem& item);
        void AddItem(const String& item);

        void Construct() override;

        void HandleEvent(FEvent* event) override;

        FIELD()
        FLabel* selectionText = nullptr;

        FIELD()
        FComboBoxState state = FComboBoxState::Default;

        FIELD()
        Ref<FComboBoxPopup> popupMenu = nullptr;

        FIELD()
        FVerticalStack* popupContent = nullptr;

        FIELD()
        FComboBoxItem* selectedItem = nullptr;

    protected: // - Fusion Fields -

        FIELD()
        FComboBoxItemStyle* m_ItemStyle = nullptr;

    public: // - Fusion Properties -

        FUSION_EVENT(FComboBoxEvent, OnSelectionChanged);

        FUSION_PROPERTY_WRAPPER(Font, selectionText);
        FUSION_PROPERTY_WRAPPER(FontSize, selectionText);
        FUSION_PROPERTY_WRAPPER(FontFamily, selectionText);
        FUSION_PROPERTY_WRAPPER(Bold, selectionText);
        FUSION_PROPERTY_WRAPPER(Italic, selectionText);

        Self& ItemStyle(FComboBoxItemStyle* value);
        Self& ItemStyle(FStyleSet* styleSet, const CE::Name& styleKey);

        void DestroyAllItems();

        FUSION_DATA_PROPERTY(Array<String>, Items);

        template<typename... TArgs> requires (sizeof...(TArgs) > 0)
        Self& Items(const TArgs&... items)
        {
            using TupleType = std::tuple<const TArgs&...>;
            TupleType args = { items... };

            DestroyAllItems();

            constexpr_for<0, sizeof...(TArgs), 1>([&](auto i)
                {
                    using ArgTypeBase = std::tuple_element_t<i(), TupleType>;
                    using ArgType = std::remove_cvref_t<ArgTypeBase>;
					if constexpr (TIsString<ArgType>::Value)
					{
                        this->AddItem(std::get<i()>(args));
					}
                    else if constexpr (TIsBaseClassOf<FComboBoxItem, ArgType>::Value)
                    {
                        this->AddItem(*const_cast<ArgType*>(&std::get<i()>(args)));
                    }
                    else
                    {
                        CE_LOG(Error, All, "Invalid argument in FComboBox::Items(...)");
                        //static_assert(Internal::AlwaysFalse<void>, "Invalid argument in Items(...)");
                    }
                });

            return *this;
        }

        FUSION_WIDGET;
        friend class FComboBoxItem;
    };
    
}

#include "FComboBox.rtti.h"
