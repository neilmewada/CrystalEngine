#pragma once

namespace CE
{
    class FLayer;

    class FUSIONCORE_API FPainter
    {
        CE_NO_COPY_MOVE(FPainter);
    private:

        FPainter(FLayer* layer);

    public:

        void Begin();

        void End();

    private:

        FLayer* owningLayer = nullptr;

        bool isRecording = false;

        friend class FLayer;
    };
    
} // namespace CE

