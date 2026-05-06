#pragma once

namespace CE
{
    struct FShape;

    struct FusionRendererInitInfo
    {
	    RPI::Shader* fusionShader = nullptr;
        RHI::MultisampleState multisampling{};
    };

    CLASS(Config = Engine)
    class FUSIONCORE_API FusionRenderer : public Object
    {
        CE_CLASS(FusionRenderer, Object)
    public:

        static constexpr u32 MaxImageCount = RHI::Limits::MaxSwapChainImageCount;
        static constexpr u32 DrawItemIncrement = 1_KB;
        static constexpr u32 ShapeItemIncrement = 256;
        static constexpr u32 LineItemIncrement = 128;
        static constexpr u32 ClipItemIncrement = 128;
        static constexpr u32 CoordinateStackItemIncrement = 128;
        static constexpr u32 OpacityStackItemIncrement = 128;

        FusionRenderer();

        void Init(const FusionRendererInitInfo& initInfo);

        void OnBeginDestroy() override;

        void Begin();

        void End();

        const Array<RHI::DrawPacket*>& FlushDrawPackets(u32 imageIndex);

        void SetViewConstants(const RPI::PerViewConstants& viewConstants);

        void SetScreenSize(Vec2i screenSize);

        void SetDrawListTag(RHI::DrawListTag drawListTag);

        u64 ComputeMemoryFootprint() override;

        // - State API -

        void SetItemTransform(const Matrix4x4& transform);

        void SetBrush(const FBrush& brush);

        void SetPen(const FPen& pen);

        void SetFont(const FFont& font);

        void PushOpacity(f32 opacity);
        void PopOpacity();

        Vec2 CalculateTextSize(const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);

        Vec2 CalculateCharacterOffsets(Array<Rect>& outRects, const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);

        FFontMetrics GetFontMetrics(const FFont& font);

        // - Draw API -

        void PushChildCoordinateSpace(const Matrix4x4& transform);
        const Matrix4x4& GetTopCoordinateSpace();
        void PopChildCoordinateSpace();

        void PushClipShape(const Matrix4x4& clipTransform, Vec2 rectSize, const FShape& shape);

        void PopClipShape();

        Vec2 DrawText(const String& text, Vec2 pos, Vec2 size = Vec2(), FWordWrap wordWrap = FWordWrap::Normal);

        void DrawLine(const Vec2& from, const Vec2& to);

        void DrawFrameBuffer(const StaticArray<RPI::Texture*, RHI::Limits::MaxSwapChainImageCount>& frames, Vec2 pos, Vec2 size);

    protected:

        using float4 = Vec4;
        using float2 = Vec2;
        using float4x4 = Matrix4x4;
        using uint = u32;

        // - Data Structures -

        struct alignas(4) FDrawDataConstants
        {
            u32 frameIndex = 0;
        };

        enum FDrawType : u32
        {
	        DRAW_Shape,
            DRAW_Line,
            DRAW_Text,
            DRAW_FrameBuffer
        };

        enum FBrushType : u32
        {
            BRUSH_None = 0,
            BRUSH_Solid,
            BRUSH_Texture,
            BRUSH_LinearGradient
        };

        enum FPenType : u32
        {
	        PEN_None = 0,
            PEN_SolidLine,
            PEN_DashedLine,
            PEN_DottedLine
        };

        struct alignas(16) FShapeItem2D
        {
            float4 cornerRadius;
            float4 brushColor;
            float2 uvMin;
            float2 uvMax;
            FBrushType brushType = BRUSH_None;
            FShapeType shape = FShapeType::None;
            uint textureOrGradientIndex = 0;
            uint samplerIndex = 0;
        };

        using FShapeItemList = StableGrowthArray<FShapeItem2D, ShapeItemIncrement, false>;

        struct alignas(8) FLineItem2D
        {
            float2 lineStart;
            float2 lineEnd;
            float dashLength = 5.0f;
        };

        using FLineItemList = StableGrowthArray<FLineItem2D, LineItemIncrement, false>;

        // Base draw item: Shape or text character
        struct alignas(16) FDrawItem2D
        {
            float4x4 transform;
            float4 penColor;
            float2 quadSize;
            float penThickness;
            float opacity;
            FDrawType drawType;
            FPenType penType = PEN_SolidLine;
            union
            {
                uint shapeIndex = 0;
                uint lineIndex;
                uint charIndex;
            };
            //int clipIndex = -1;
            int startClipIndex = -1;
            int endClipIndex = -1;
        };

        using FDrawItemList = StableGrowthArray<FDrawItem2D, DrawItemIncrement, false>;

        struct alignas(16) FClipItem2D
        {
            Matrix4x4 clipTransform;
            Vec4 cornerRadius;
            Vec2 size;
            FShapeType shapeType = FShapeType::Rect;
        };

        struct alignas(4) FClipItemIndexData
        {
            int clipItemIndex = -1;
        };

        using FClipItemList = StableGrowthArray<FClipItem2D, ClipItemIncrement, false>;
        using FClipItemStack = StableGrowthArray<int, ClipItemIncrement, false>;
        using FClipItemIndexList = StableGrowthArray<FClipItemIndexData, ClipItemIncrement, false>;

        using FCoordinateSpaceStack = StableGrowthArray<Matrix4x4, CoordinateStackItemIncrement, false>;
        using FOpacityStack = StableGrowthArray<f32, OpacityStackItemIncrement, false>;

        struct FDrawBatch
        {
            RHI::ShaderResourceGroup* fontSrg = nullptr;
            u32 firstDrawItemIndex = 0;
            u32 drawItemCount = 0;
        };

        // - Internal Methods -

        FDrawItem2D& DrawCustomItem(FDrawType drawType, Vec2 pos, Vec2 size);
        bool DrawShape(const FShape& shape, Vec2 pos, Vec2 size);

        bool IsCulled(Vec2 pos, Vec2 quadSize);

        // - Config -

        FIELD(Config)
        u32 initialDrawItemCapacity = 5000;

        FIELD(Config)
        f32 drawItemGrowRatio = 0.25f;

        FIELD(Config)
        u32 initialClipItemCapacity = 256;

        FIELD(Config)
        f32 clipItemGrowRatio = 0.25f;

        FIELD(Config)
        u32 initialShapeItemCapacity = 2500;

        FIELD(Config)
        f32 shapeItemGrowRatio = 0.25f;

        FIELD(Config)
        u32 initialLineItemCapacity = 500;

        // - Rendering Setup -

        u32 numFrames = 0;
        int curImageIndex = 0;
        Vec2i screenSize = Vec2i(0, 0);
        RHI::MultisampleState multisampling{};


        // - State -

        FBrush currentBrush;
        FPen currentPen;
        FFont currentFont;
        Matrix4x4 itemTransform = Matrix4x4::Identity();

        FCoordinateSpaceStack coordinateSpaceStack{};
        FOpacityStack opacityStack{};

        // - Constants -

    	RPI::PerViewConstants viewConstants{};
        StaticArray<RHI::Buffer*, MaxImageCount> viewConstantsBuffer{};
        RHI::ShaderResourceGroup* perViewSrg = nullptr;

        StaticArray<RHI::Buffer*, MaxImageCount> drawDataConstantsBuffer{};

        // - Drawing -

        RHI::ShaderResourceGroup* drawItemSrg = nullptr;

        RPI::DynamicStructuredBuffer<FDrawItem2D> drawItemsBuffer;
        FDrawItemList drawItemList;

        RPI::DynamicStructuredBuffer<FClipItem2D> clipItemsBuffer;
        FClipItemList clipItemList;
        FClipItemStack clipItemStack;
        FClipItemIndexList clipItemIndexList;
        RPI::DynamicStructuredBuffer<FClipItemIndexData> clipItemIndexListBuffer;

        RPI::DynamicStructuredBuffer<FShapeItem2D> shapeItemsBuffer;
        FShapeItemList shapeItemList;

        RPI::DynamicStructuredBuffer<FLineItem2D> lineItemsBuffer;
        FLineItemList lineItemList;

        Array<FDrawBatch> drawBatches{};
        bool createNewDrawBatch = false;

        // - Flush Draw Packets -

        Array<RHI::DrawPacket*> drawPackets{};
        StaticArray<bool, MaxImageCount> resubmitDrawItems{};
        StaticArray<bool, MaxImageCount> viewConstantsUpdateRequired{};
        RHI::DrawListTag drawListTag = RHI::DrawListTag::NullValue;

        // - Utils -

        RPI::Shader* fusionShader = nullptr;

        // - Cache -

        friend class FNativeContext;
        friend class FPainter;
    };
    
} // namespace CE

#include "FusionRenderer.rtti.h"
