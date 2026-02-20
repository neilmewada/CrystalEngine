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

		int GetChildCount() const { return (int)children.GetSize(); }

		Ref<FWidget> GetChildAt(int index) const
		{
			if (index < 0 || index >= (int)children.GetSize())
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

        template<typename... TArgs> requires TValidate_Children<TArgs...>::Value and (sizeof...(TArgs) > 0)
        FContainerWidget& operator()(const TArgs&... childWidget)
        {
            using TupleType = std::tuple<const TArgs&...>;
            TupleType args = { childWidget... };

            constexpr_for<0, sizeof...(TArgs), 1>([&](auto i)
                {
                    using ArgTypeBase = std::tuple_element_t<i(), TupleType>;
                    using ArgType = std::remove_cvref_t<ArgTypeBase>;
                    if constexpr (TIsBaseClassOf<FWidget, ArgType>::Value)
                    {
                        this->AddChildWidget(const_cast<ArgType*>(&std::get<i()>(args)));
                    }
                    else if constexpr (TIsBaseClassOf<FWidgetBuilder, ArgType>::Value)
                    {
                        const_cast<ArgType*>(&std::get<i()>(args))->Build(this);
                    }
                });

            return *this;
        }

    protected: // - Internal -

        FIELD()
        Array<Ref<FWidget>> children;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };

}

#include "FContainerWidget.rtti.h"
