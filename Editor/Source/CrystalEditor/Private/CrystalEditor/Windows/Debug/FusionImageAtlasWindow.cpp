#include "CrystalEditor.h"

namespace CE::Editor
{
    static WeakRef<FusionImageAtlasWindow> instance = nullptr;

    FusionImageAtlasWindow::FusionImageAtlasWindow()
    {

    }

    void FusionImageAtlasWindow::Construct()
    {
        Super::Construct();

        auto imageAtlas = FusionApplication::Get()->GetImageAtlas();

        FBrush defaultImage = FBrush("__ImageAtlas_0");
        defaultImage.SetBrushTiling(FBrushTiling::None);
        defaultImage.SetImageFit(FImageFit::Cover);
        defaultImage.SetBrushPosition(Vec2(0, 0));

        const CE::Name gridSmall = "/Engine/Resources/Images/GridSmall";

        FBrush gridBg = FBrush(gridSmall);
        gridBg.SetBrushSize(Vec2(16, 16));
        gridBg.SetBrushTiling(FBrushTiling::TileXY);
        gridBg.SetImageFit(FImageFit::Fill);

        (*this)
            .Title("Fusion Image Atlas")
            .MinimizeEnabled(true)
            .MaximizeEnabled(true)
            .ContentPadding(Vec4(1, 1, 1, 1) * 10.0f)
			.ContentGap(10)
            .Content(
                FNew(FHorizontalStack)
                .ContentHAlign(HAlign::Left)
                .ContentVAlign(VAlign::Center)
                .Height(35)
                .HAlign(HAlign::Fill)
                (
                    FAssignNew(FComboBox, comboBox)
                    .Bind_Items(BIND_PROPERTY_R(imageAtlas, Pages))
                    .OnSelectionChanged([this](FComboBox*)
                    {
                        int selectionIndex = comboBox->GetSelectedItemIndex();

                        FBrush image = FBrush(String::Format("__ImageAtlas_{}", selectionIndex));
                        image.SetBrushTiling(FBrushTiling::None);
                        image.SetImageFit(FImageFit::Cover);
                        image.SetBrushPosition(Vec2(0, 0));

                        atlasViewport->Background(image);
                    })
                    .Width(60)
                ),

                FNew(FOverlayStack)
                .ClipChildren(true)
                .FillRatio(1.0f)
                .HAlign(HAlign::Fill)
                (
                    FAssignNew(FStyledWidget, background)
                    .Background(gridBg)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill),

                    FAssignNew(FStyledWidget, atlasViewport)
                    .Background(defaultImage)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill)
                )
            );

        comboBox->SelectItem(0);
    }

    void FusionImageAtlasWindow::OnPostComputeLayout()
    {
        Super::OnPostComputeLayout();

    }

    void FusionImageAtlasWindow::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        imageAtlasSize = painter->GetImageAtlasSize();
    }

    Ref<FusionImageAtlasWindow> FusionImageAtlasWindow::Show()
    {
        if (Ref<FusionImageAtlasWindow> lock = instance.Lock())
        {
            Ref<FNativeContext> nativeContext = CastTo<FNativeContext>(lock->GetContext());
            PlatformWindow* window = nativeContext->GetPlatformWindow();
            window->Show();
            return lock;
        }

        PlatformWindowInfo info{
            .maximised = false,
            .fullscreen = false,
            .resizable = true,
            .hidden = false,
            .windowFlags = PlatformWindowFlags::DestroyOnClose
        };

        Ref<FusionImageAtlasWindow> window = Object::CastTo<FusionImageAtlasWindow>(FusionApplication::Get()->CreateNativeWindow(
            "FusionImageAtlasWindow", 
            "Fusion Image Atlas",
            800, 600,
            Self::StaticClass(), info));

        instance = window;

        window->ShowWindow();

        return window;
    }

}

