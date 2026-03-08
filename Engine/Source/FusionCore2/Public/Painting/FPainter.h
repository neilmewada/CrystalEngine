#pragma once

namespace CE
{
    class FWidget;

    class FUSIONCORE_API FPainter
    {
        CE_NO_COPY_MOVE(FPainter);
    public:

        FPainter(FWidget* widget);

        // - Public API -


    private:

        FPen currentPen;
        FBrush currentBrush;

    };
    
} // namespace CE

