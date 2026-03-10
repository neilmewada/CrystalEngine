#include "FusionCore.h"

namespace CE
{

    FDecoratedWidget::FDecoratedWidget()
    {
        m_Shape = FRectangle();
    }

    void FDecoratedWidget::Construct()
    {
        Super::Construct();

        
    }

    void FDecoratedWidget::Paint(FPainter& painter)
    {
	    Super::Paint(painter);

        
    }
}

