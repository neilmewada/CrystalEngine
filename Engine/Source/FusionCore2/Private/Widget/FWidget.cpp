#include "FusionCore.h"

namespace CE
{

    FWidget::FWidget()
    {
        m_MaxHeight = NumericLimits<f32>::Infinity();
        m_MaxWidth = NumericLimits<f32>::Infinity();
    }

    void FWidget::MarkPaintDirty()
    {
    }

    void FWidget::MarkLayoutDirty()
    {
    }
} // namespace CE

