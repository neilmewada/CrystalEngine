#pragma once

namespace CE
{
    class CameraComponent;

    CLASS(Abstract)
    class ENGINE_API RenderPipeline : public Object
    {
        CE_CLASS(RenderPipeline, Object)
    public:

        RenderPipeline();

        virtual ~RenderPipeline();

        RPI::RenderPipeline* GetRpiRenderPipeline() const { return renderPipeline; }

        //! @brief Never call this function directly! Use ApplyChanges() instead.
        virtual void ConstructPipeline();

        virtual void Tick();

        bool IsDirty() const { return isDirty; }

        void MarkDirty() { isDirty = true; }

        void SetMainViewTag(const Name& viewTag) { mainViewTag = viewTag; }

        RHI::DrawListTag GetBuiltinDrawListTag(RPI::BuiltinDrawItemTag builtinDrawItemTag);

        void SetOwnerCamera(CameraComponent* owner) { targetCamera = owner; }

        CameraComponent* GetOwnerCamera() const { return targetCamera; }

        void ApplyChanges();
        
    protected:

        RPI::RenderPipeline* renderPipeline = nullptr;

        FIELD()
        Name mainViewTag = "MainCamera";

        FIELD()
        CameraComponent* targetCamera = nullptr;

    public:

        FIELD(EditAnywhere)
        Color clearColor = Color::Black();

        FIELD(EditAnywhere, Category = "Anti-Aliasing")
        MSAA msaa = MSAA2;

        FIELD(EditAnywhere, Category = "Depth Stencil")
        b8 useStencil = false;

        FIELD(EditAnywhere, Category = "Shadows")
        int directionalShadowResolution = 1024;

    private:

        bool isDirty = true;

    };
    
} // namespace CE

#include "RenderPipeline.rtti.h"