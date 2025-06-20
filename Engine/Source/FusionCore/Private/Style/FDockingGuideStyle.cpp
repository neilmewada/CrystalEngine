#include "FusionCore.h"

namespace CE
{

    FDockingGuideStyle::FDockingGuideStyle()
    {
        background = Color::RGBA(36, 36, 36);

        borderColor = Color::RGBA(15, 15, 15);
        borderWidth = 1.5f;
    }

    FDockingGuideStyle::~FDockingGuideStyle()
    {
        
    }

    SubClass<FWidget> FDockingGuideStyle::GetWidgetClass() const
    {
        return FDockingGuide::StaticClass();
    }

    void FDockingGuideStyle::MakeStyle(FWidget& widget)
    {
        Super::MakeStyle(widget);

        FDockingGuide& dockingGuide = widget.As<FDockingGuide>();

        
    }
} // namespace CE

