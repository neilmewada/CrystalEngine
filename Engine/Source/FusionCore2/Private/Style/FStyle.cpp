#include "FusionCore.h"

namespace CE
{
	SubClass<FWidget> FStyle::GetWidgetClass() const
	{
		return FWidget::StaticClass();
	}
} // namespace CE
