#pragma once

namespace CE
{
	class FWidget;

	template<typename T>
	concept WidgetClassType = TIsBaseClassOf<FWidget, T>::Value;

}
