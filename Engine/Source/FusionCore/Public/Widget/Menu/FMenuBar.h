#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FMenuBar : public FStyledWidget
    {
        CE_CLASS(FMenuBar, FStyledWidget)
    public:

        FMenuBar();

        u32 GetMenuItemCount() const { return menuItems.GetSize(); }

        FMenuItem* GetMenuItem(u32 index) const { return menuItems[index]; }

        void OnMenuItemHovered(FMenuItem* menuItem);

        u32 GetContainerChildCount() const { return container->GetChildCount(); }

        Ref<FWidget> GetContainerChild(u32 index) const { return container->GetChild(index); }

        void SetFontSizeRecursively(f32 fontSize);

    protected:

        void Construct() override final;

        FIELD()
        Array<FMenuItem*> menuItems;

        FStackBox* container = nullptr;

    public: // - Fusion Properties - 

        template <typename... TArgs> requires TMatchAllBaseClass<FWidget, TArgs...>::Value and (sizeof...(TArgs) > 0)
        Self& Content(const TArgs&... widgets)
        {
            using TupleType = std::tuple<const TArgs&...>;
            TupleType args = { widgets... };

            constexpr_for<0, sizeof...(TArgs), 1>([&](auto i)
                {
                    using ArgTypeBase = std::tuple_element_t<i(), TupleType>;
                    using ArgType = std::remove_cvref_t<ArgTypeBase>;
					
                    if constexpr (TIsBaseClassOf<FWidget, ArgType>::Value)
                    {
                        ArgType* widget = const_cast<ArgType*>(&std::get<i()>(args));
                        if (widget->template IsOfType<FMenuItem>())
                        {
                            FMenuItem* menuItem = static_cast<FMenuItem*>(widget);
                            menuItem->menuOwner = this;
                            menuItems.Add(menuItem);
                        }
                        container->AddChild(widget);
                    }

                    ApplyStyle();
                });

            return *this;
        }

        FUSION_WIDGET;
    };
    
}

#include "FMenuBar.rtti.h"
