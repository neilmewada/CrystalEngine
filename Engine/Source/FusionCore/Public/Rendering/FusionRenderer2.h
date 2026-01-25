#pragma once

namespace CE
{
    CLASS(Config = Engine)
    class FUSIONCORE_API FusionRenderer2 : public Object
    {
        CE_CLASS(FusionRenderer2, Object)
    protected:

        FusionRenderer2();

        virtual ~FusionRenderer2();

    public:

        // - Constants 0

        static constexpr u32 MaxImageCount = RHI::Limits::MaxSwapChainImageCount;
        static constexpr u32 CoordinateStackItemIncrement = 128;
        static constexpr u32 VertexArrayIncrement = 4096;
        static constexpr u32 IndexArrayIncrement = 4096;
        static constexpr u32 PathArrayIncrement = 512;
        static constexpr u32 DrawCmdArrayIncrement = 128;
        static constexpr u32 ObjectDataArrayIncrement = 128;
        static constexpr u32 ClipRectArrayIncrement = 32;
        static constexpr u32 DrawDataArrayIncrement = 128;
        static constexpr u32 GradientKeyArrayIncrement = 128;
        static constexpr u32 OpacityStackIncrement = 64;

        static constexpr u32 ArcFastTableSize = 48;// 48

        static constexpr f32 MinOpacity = 0.001f;
        static constexpr u32 MaxClipRectStack = 24;

        static constexpr u32 ColorAlphaMask = 0xff000000;

        // - Lifecycle -

        void Init(const FusionRendererInitInfo& initInfo);

        void OnBeginDestroy() override;

        // - General API -

        void SetDrawListTag(RHI::DrawListTag drawListTag)
        {
            this->drawListTag = drawListTag;
        }

        void SetViewConstants(const RPI::PerViewConstants& viewConstants);

        const Array<RHI::DrawPacket*>& FlushDrawPackets(u32 imageIndex);

        Vec2 CalculateCharacterOffsets(Array<Vec2>& outOffsets, const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);
        Vec2 CalculateSDFCharacterOffsets(Array<Vec2>& outOffsets, const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);

        Vec2 CalculateTextQuads(Array<Rect>& outQuads, const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);
        Vec2 CalculateSDFTextQuads(Array<Rect>& outQuads, const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);

        void CalculateUnderlinePositions(Array<Rect>& outLines, const String& text, const FFont& font, f32 width = 0, FWordWrap wordWrap = FWordWrap::Normal);

        FFontMetrics GetFontMetrics(const FFont& font);
        FFontMetrics GetSDFFontMetrics(const FFont& font);

        u64 ComputeMemoryFootprint() override;

        // - State API -

        void SetDebugName(const CE::Name& name);

        void Begin();
        void End();

        void PushChildCoordinateSpace(const Matrix4x4& transform);
        void PushChildCoordinateSpace(Vec2 translation);
        void PopChildCoordinateSpace();

        Matrix4x4 GetTopCoordinateSpace();

        void PushClipRect(const Matrix4x4& clipTransform, Vec2 rectSize);
        void PopClipRect();

        void PushOpacity(f32 opacity);
        void PopOpacity();

        void SetPen(const FPen& pen);
        void SetBrush(const FBrush& brush);
        void SetFont(const FFont& font);

        const FFont& GetFont() const { return currentFont; }
        const FPen& GetPen() const { return currentPen; }

        Vec2i GetImageAtlasSize();
        Vec2i GetFontAtlasSize();

        u32 GetNumImageAtlasLayers();

        //! @brief Used for CPU side culling and is dependent on the state (clips and transforms)!
        bool IsRectClipped(const Rect& rect);

        // - Path API -

        void PathClear();
        void PathLineTo(const Vec2& point);
        void PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle);
        void PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, int numSegments);
        // 0: East, 3: South, 6: West, 9: North, 12: East
        void PathArcToFast(const Vec2& center, float radius, int angleMinOf12, int angleMaxOf12);
        void PathRect(const Rect& rect, const Vec4& cornerRadius = {});
        void PathBezierCubicCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, int numSegments = 0);
        void PathQuadraticCubicCurveTo(const Vec2& p2, const Vec2& p3, int numSegments = 0);

        bool PathFill(bool antiAliased = true);
        bool PathStroke(bool closed = false, bool antiAliased = true);
        bool PathFillStroke(bool closed = false, bool antiAliased = true);

        // - Draw API -

        bool FillRect(const Rect& rect, const Vec4& cornerRadius = {}, bool antiAliased = true);
        bool StrokeRect(const Rect& rect, const Vec4& cornerRadius = {}, bool antiAliased = true);

        bool FillCircle(const Vec2& center, f32 radius, bool antiAliased = true);
        bool StrokeCircle(const Vec2& center, f32 radius, bool antiAliased = true);

        Vec2 DrawText(const String& text, Vec2 textPos, Vec2 size = Vec2(), FWordWrap wordWrap = FWordWrap::Normal);
        Vec2 DrawSDFText(const String& text, Vec2 textPos, Vec2 size = Vec2(), FWordWrap wordWrap = FWordWrap::Normal);

        Vec2 DrawSDFTextCached(Uuid cacheId, const String& text, Vec2 textPos, Vec2 size = Vec2(), FWordWrap wordWrap = FWordWrap::Normal);
        void ResetSDFTextCache(Uuid cacheId);

        void DrawViewport(const Rect& rect, FViewport* viewport);

        void DrawImageAtlas(const Rect& rect, int layerIndex);

        void DrawFontAtlas(const Rect& rect, const Ref<FFontAtlas>& fontAtlas, int layerIndex);

    private:

        enum FDrawType : int
        {
            DRAW_Geometry = 0,
            DRAW_Text,
            DRAW_SDFText,
            DRAW_TextureNoTile,
            DRAW_TextureTileX,
            DRAW_TextureTileY,
            DRAW_TextureTileXY,
            DRAW_Viewport,
            DRAW_TextureAtlas,
            DRAW_FontAtlas,
            DRAW_LinearGradient,
            DRAW_RadialGradient
        };

        // - Internal Draw API -

        void DrawTextInternal(const Rect* quads, char* text, int length, const FFont& font, Vec2 textPos);
        void DrawSDFTextInternal(const Rect* quads, char* text, int length, const FFont& font, Vec2 textPos);

        int CalculateNumCircleSegments(float radius) const;

        void PathArcToFastInternal(const Vec2& center, float radius, int sampleMin, int sampleMax, int step);

        void PrimReserve(int vertexCount, int indexCount);
        void PrimUnreserve(int vertexCount, int indexCount);

        void PrimRect(const Rect& rect, u32 color, Vec2* uvs = nullptr, FDrawType drawType = DRAW_Geometry, int index = -1);

        void AddDrawCmd();

        void PathInsert(const Vec2& point);
        void PathMinMax(const Vec2& point);

        bool AddRect(const Rect& rect, const Vec4& cornerRadius = {}, bool antiAliased = true);
        bool AddRectFilled(Rect rect, const Vec4& cornerRadius = {}, bool antiAliased = true);
        void AddConvexPolyFilled(const Vec2* points, int numPoints, bool antiAliased, Rect* minMaxPos = nullptr);
        void AddPolyLine(const Vec2* points, int numPoints, f32 thickness, bool closed, bool antiAliased);
        bool AddCircle(const Vec2& center, f32 radius, int numSegments, bool antiAliased);
        bool AddCircleFilled(const Vec2& center, f32 radius, int numSegments, bool antiAliased);

        // - Utility API -

        void GrowQuadBuffer(u64 newTotalSize);

        void QueueDestroy(RHI::Buffer* buffer);

        // - Config -

        FIELD(Config)
        u32 initialMeshBufferSize = 24'000;

        FIELD(Config)
        f32 meshBufferGrowRatio = 0.2f;

        FIELD(Config)
        u32 initialObjectCount = 128;

        FIELD(Config)
        u32 objectDataGrowCount = 128;

        FIELD(Config)
        u32 initialClipRectCount = 128;

        FIELD(Config)
        u32 clipRectGrowCount = 128;

        FIELD(Config)
        u32 initialDrawDataCount = 512;

        FIELD(Config)
        f32 drawDataGrowRatio = 0.3f;

        FIELD(Config)
        u32 initialGradientCount = 512;

        FIELD(Config)
        f32 gradientGrowRatio = 0.3f;

        FIELD(Config)
        f32 circleSegmentMaxError = 0.3f;

        FIELD(Config)
        f32 curveTessellationTolerance = 1.25f;

        // - Data Structures -

        using float4x4 = Matrix4x4;
        using float2 = Vec2;
        using float4 = Vec4;
        using uint = u32;

        struct alignas(16) FObjectData
        {
            Matrix4x4 transform = Matrix4x4::Identity();
        };

        struct alignas(8) FDrawData
        {
            float2 rectSize;
            float2 uvMin;
            float2 uvMax;
            float2 brushPos;
            float2 brushSize;
            float userAngle = 0;
            // Index into texture Array
            int index = 0;
            int endIndex = 0;
            int imageFit = 0;
        };

        struct DestroyItem
        {
            RHI::Buffer* buffer = nullptr;
            RPI::Material* material = nullptr;
            RHI::ShaderResourceGroup* srg = nullptr;
            int frameCounter = 0;
        };

        struct FRootConstants
        {
            float2 transparentUV;
            u32 numClipRects = 0;
            u32 clipRectIndices[MaxClipRectStack] = {};
        };

        struct FDrawCmd
        {
            u32 vertexOffset = 0;
            u32 indexOffset = 0;
            u32 numIndices = 0;
            u32 firstInstance = 0;

            RHI::ShaderResourceGroup* fontSrg = nullptr;
            RHI::ShaderResourceGroup* textureSrgOverride = nullptr;
            FRootConstants rootConstants{};
        };

        struct alignas(16) FClipRect
        {
            Matrix4x4 clipTransform = Matrix4x4::Identity();
            Vec2 size = Vec2();
        };

        struct alignas(16) FColorStop
        {
            float4 color;
            float position = 0;
        };

        struct FVertex
        {
            Vec2 position;
            Vec2 uv;
            u32 color = Colors::White.ToU32();
            FDrawType drawType = DRAW_Geometry;
            int index = -1;
        };

        struct FCoordinateSpace
        {
            Matrix4x4 transform;
            Vec2 translation;

            bool isTranslationOnly = false;
            int indexInObjectArray = 0;
        };

        using FDrawIndex = u16;

        using FCoordinateSpaceStack = StableDynamicArray<FCoordinateSpace, CoordinateStackItemIncrement, false>;
        using FVertexArray = StableDynamicArray<FVertex, VertexArrayIncrement, false>;
        using FIndexArray = StableDynamicArray<FDrawIndex, IndexArrayIncrement, false>;
        using FPathArray = StableDynamicArray<Vec2, PathArrayIncrement, false>;
        using FDrawCmdArray = StableDynamicArray<FDrawCmd, DrawCmdArrayIncrement, false>;
        using FObjectDataArray = StableDynamicArray<FObjectData, ObjectDataArrayIncrement, false>;
        using FClipRectArray = StableDynamicArray<FClipRect, ClipRectArrayIncrement, false>;
        using FClipRectStack = StableDynamicArray<int, ClipRectArrayIncrement, false>;
        using FDrawDataArray = StableDynamicArray<FDrawData, DrawDataArrayIncrement, false>;
        using FOpacityStack = StableDynamicArray<f32, OpacityStackIncrement, false>;
        using FGradientKeyArray = StableDynamicArray<FColorStop, GradientKeyArrayIncrement, false>;

        // - Draw Data -

        FVertexArray vertexArray;
        FIndexArray indexArray;
        FPathArray path;
        FPathArray temporaryPoints;
        Vec2 pathMin, pathMax;

        FVertex* vertexWritePtr = nullptr;
        FDrawIndex* indexWritePtr = nullptr;

    	// Start offset of current vertex
        FDrawIndex vertexCurrentIdx = 0;

        float fringeScale = 1.0f;

        FDrawCmdArray drawCmdList;
        FObjectDataArray objectDataArray;
        FGradientKeyArray gradientKeyArray;

        FDrawDataArray drawDataArray;

        StaticArray<bool, MaxImageCount> quadUpdatesRequired{};

        // - Cache -

        struct FTextCacheEntry
        {
            Uuid cacheId;
            Vec2 finalSize;
            Array<Rect> quads;
		};

		HashMap<Uuid, FTextCacheEntry> sdfTextCache;

        // - Setup -

        RHI::DrawListTag drawListTag = RHI::DrawListTag::NullValue;

		CE::Name debugName;
        u32 numFrames = 0;
        int curImageIndex = 0;
        bool pixelPerfect = true;
        Vec2i screenSize = Vec2i(0, 0);
        Vec2 whitePixelUV = Vec2(0, 0);
        Vec2 transparentPixelUV = Vec2(0, 0);

        RPI::Shader* fusionShader = nullptr;
        RHI::MultisampleState multisampling{};

        // - State -

        FBrush currentBrush;
        FPen currentPen;
        FFont currentFont;

        FCoordinateSpaceStack coordinateSpaceStack;

        FClipRectArray clipRectArray;
        FClipRectStack clipStack;
        FOpacityStack opacityStack;

        // - View Constants -

        RPI::PerViewConstants viewConstants{};
        RHI::ShaderResourceGroup* perViewSrg = nullptr;
        RHI::ShaderResourceGroup* perObjectSrg = nullptr;
    	StaticArray<bool, MaxImageCount> viewConstantsUpdateRequired{};

        // - GPU Buffers -

        StaticArray<RHI::Buffer*, MaxImageCount> viewConstantsBuffer{};
        StaticArray<RHI::Buffer*, MaxImageCount> quadsBuffer{};
        DynamicStructuredBuffer<FObjectData> objectDataBuffer{};
        DynamicStructuredBuffer<FClipRect> clipRectBuffer{};
        DynamicStructuredBuffer<FDrawData> drawDataBuffer{};
        DynamicStructuredBuffer<FColorStop> gradientBuffer{};

        // - Draw List -

        StaticArray<Array<RHI::DrawPacket*>, MaxImageCount> drawPacketsPerImage{};

        // - Utils -

        Array<DestroyItem> destructionQueue{};
        Array<RHI::DrawPacket*> freePackets;

        Vec2 arcFastVertex[ArcFastTableSize] = {};
        float arcFastRadiusCutoff = 0;

        friend class FNativeContext;
        friend void PathBezierCubicCurveToCasteljau(FusionRenderer2* renderer,
            float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level);
        friend void PathBezierQuadraticCurveToCasteljau(FusionRenderer2* renderer,
            float x1, float y1, float x2, float y2, float x3, float y3, float tess_tol, int level);
    };

} // namespace CE

#include "FusionRenderer2.rtti.h"
