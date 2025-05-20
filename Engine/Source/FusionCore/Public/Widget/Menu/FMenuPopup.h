#pragma once

namespace CE
{
    class FMenuItem;

    CLASS()
    class FUSIONCORE_API FMenuPopup : public FPopup
    {
        CE_CLASS(FMenuPopup, FPopup)
    public:

        FMenuPopup();

        u32 GetMenuItemCount() const { return menuItems.GetSize(); }

        FMenuItem* GetMenuItem(u32 index) const { return menuItems[index]; }

        bool FocusParentExistsRecursive(FWidget* parent) override;

        void QueueDestroyAllItems();
        void DestroyAllItems();

        u32 GetContainerChildCount() const { return container->GetChildCount(); }

        Ref<FWidget> GetContainerChild(u32 index) const { return container->GetChild(index); }

    protected:

        void HandleEvent(FEvent* event) override;

        void Construct() override;

        void OnPopupClosed() override;

        FIELD()
        Array<FMenuItem*> menuItems;

        FIELD()
        FMenuItem* ownerItem = nullptr;

        FVerticalStack* container = nullptr;

    public: // - Fusion Properties -

        FUSION_PROPERTY_WRAPPER(Gap, container);
        FUSION_PROPERTY_WRAPPER2(Padding, container, ContentPadding);

        template <typename... TArgs> requires TMatchAllBaseClass<FWidget, TArgs...>::Value and (sizeof...(TArgs) > 0)
        Self& Content(const TArgs&... widgets)
        {
            using TupleType = std::tuple<const TArgs&...>;
            TupleType args = { widgets... };

            constexpr_for<0, sizeof...(TArgs), 1>([&](auto i)
                {
                    using ArgTypeBase = std::tuple_element_t<i(), TupleType>;
                    using ArgType = std::remove_cvref_t<ArgTypeBase>;

					if constexpr (TIsBaseClassOf<FMenuItem, ArgType>::Value)
					{
                        const_cast<ArgType*>(&std::get<i()>(args))->menuOwner = this;
                        menuItems.Add(const_cast<ArgType*>(&std::get<i()>(args)));
					}
                    if constexpr (TIsBaseClassOf<FWidget, ArgType>::Value)
                    {
                        container->AddChild(const_cast<ArgType*>(&std::get<i()>(args)));
                    }
                });

            return *this;
        }

        FUSION_WIDGET;
        friend class FMenuItem;
    };
    
}

#include "FMenuPopup.rtti.h"
