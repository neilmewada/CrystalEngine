#pragma once

namespace CE
{
	class FWidget;

	template<typename T>
	concept WidgetClassType = TIsBaseClassOf<FWidget, T>::Value;

	template<class TCastClassType, typename T, typename = void>
	struct TMemberFunctionCast : TFalseType
	{
		
	};

	template<class TCastClassType, typename TRetType, class TClassType, class... TArgs>
	struct TMemberFunctionCast<TCastClassType, TRetType(TClassType::*)(TArgs...)>
	{
		using TCastedFuncTraits = TFunctionTraits<TRetType(TCastClassType::*)(TArgs...)>;
		using TCastedFuncSignature = TCastedFuncTraits::FuncSignature;

		static constexpr bool Value = TCastedFuncTraits::Value;
	};

}
