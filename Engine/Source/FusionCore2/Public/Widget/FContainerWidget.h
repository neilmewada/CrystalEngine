#pragma once

namespace CE
{
    CLASS(Abstract)
    class FUSIONCORE_API FContainerWidget : public FWidget
    {
        CE_CLASS(FContainerWidget, FWidget)
    protected:

        FContainerWidget();

    public: // - Public API -

        void SetParentSurfaceRecursive(Ref<FSurface> surface) override;

        void AddChildWidget(Ref<FWidget> childWidget);
    	
    	void RemoveChildWidget(Ref<FWidget> childWidget);

        void DetachChild(Ref<FWidget> child) override;

		u32 GetChildCount() override { return (int)children.GetSize(); }

		Ref<FWidget> GetChildAt(u32 index) override
		{
			if (index >= (u32)children.GetSize())
            {
                return nullptr;
            }
			return children[index];
		}

        template<typename... TArgs>
        struct TValidate_Children : TFalseType
        {
        };

        template<typename T>
        struct TValidate_Children<T> : TBoolConst<TIsBaseClassOf<FWidget, T>::Value or TIsBaseClassOf<FWidgetBuilder, T>::Value>
        {
        };

        template<typename TFirst, typename... TRest>
        struct TValidate_Children<TFirst, TRest...> : TBoolConst<TValidate_Children<TFirst>::Value and TValidate_Children<TRest...>::Value>
        {
        };

        template<typename TSelf, typename... TArgs> requires TValidate_Children<TArgs...>::Value and (sizeof...(TArgs) > 0)
        TSelf& operator()(this TSelf& self, const TArgs&... childWidget)
        {
            using TupleType = std::tuple<const TArgs&...>;
            TupleType args = { childWidget... };

            constexpr_for<0, sizeof...(TArgs), 1>([&](auto i)
                {
                    using ArgTypeBase = std::tuple_element_t<i(), TupleType>;
                    using ArgType = std::remove_cvref_t<ArgTypeBase>;
                    if constexpr (TIsBaseClassOf<FWidget, ArgType>::Value)
                    {
                        self.AddChildWidget(const_cast<ArgType*>(&std::get<i()>(args)));
                    }
                    else if constexpr (TIsBaseClassOf<FWidgetBuilder, ArgType>::Value)
                    {
                        const_cast<ArgType*>(&std::get<i()>(args))->Build(&self);
                    }
                });

            return self;
        }

    private: // - Internal -

        void SetWidgetFlagInternal(FWidgetFlags flag, bool set);

        FIELD()
        Array<Ref<FWidget>> children;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_SET(bool, ForcePaintBoundary)
        {
            if (self.TestWidgetFlags(FWidgetFlags::ForcePaintBoundary) == value)
                return self;

            self.SetWidgetFlagInternal(FWidgetFlags::ForcePaintBoundary, value);
            self.UpdateBoundaryFlags();
            return self;
        }

        FUSION_WIDGET;
    };

}

#include "FContainerWidget.rtti.h"
