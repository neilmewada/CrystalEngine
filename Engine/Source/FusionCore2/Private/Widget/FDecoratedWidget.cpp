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

        painter.SetBrush(m_Background);
        painter.SetPen(m_Border);

        painter.FillAndStrokeShape(Rect(0, 0, layoutSize.width, layoutSize.height), m_Shape);
    }
}

