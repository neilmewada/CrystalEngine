#include "CrystalEditor.h"

namespace CE::Editor
{
    static WeakRef<FusionFontAtlasWindow> instance = nullptr;

    FusionFontAtlasWindow::FusionFontAtlasWindow()
    {

    }

    void FusionFontAtlasWindow::Construct()
    {
        Super::Construct();

        FFontManager* fontManager = FusionApplication::Get()->GetFontManager();
        Array<String> fontNames{};
        Array<WeakRef<FFontAtlas>> fontAtlases;

        fontManager->IterateFontAtlases([&](FFontAtlas* fontAtlas)
            {
                fontNames.Add(fontAtlas->GetName().GetString());
                fontAtlases.Add(fontAtlas);
            });

        FBrush defaultImage = FBrush(String::Format("__FontAtlas_{}_0", fontNames[0]));
        defaultImage.SetBrushTiling(FBrushTiling::None);
        defaultImage.SetImageFit(FImageFit::Cover);
        defaultImage.SetBrushPosition(Vec2(0, 0));

        (*this)
            .Title("Fusion Font Atlas")
            .MinimizeEnabled(true)
            .MaximizeEnabled(true)
            .ContentPadding(Vec4(1, 1, 1, 1) * 10.0f)
			.ContentGap(10)
            .Content(
                FNew(FHorizontalStack)
                .Gap(15)
                .ContentHAlign(HAlign::Left)
                .ContentVAlign(VAlign::Center)
                .Height(35)
                .HAlign(HAlign::Fill)
                (
                    FAssignNew(FComboBox, fontComboBox)
                    .Items(fontNames)
                    .OnSelectionChanged([this, fontNames, fontAtlases](FComboBox*)
                    {
                        int selectionIndex = fontComboBox->GetSelectedItemIndex();

                        FBrush image = FBrush(String::Format("__FontAtlas_{}_0", fontNames[selectionIndex]));
                        image.SetBrushTiling(FBrushTiling::None);
                        image.SetImageFit(FImageFit::Cover);
                        image.SetBrushPosition(Vec2(0, 0));

                        if (auto selectedAtlas = fontAtlases[selectionIndex].Lock())
                        {
                            fontPageComboBox->Items(selectedAtlas->GetPages());
                        }

                        fontPageComboBox->SelectItem(0);

                        atlasViewport->Background(image);
                    })
                    .Width(60),

                    FAssignNew(FComboBox, fontPageComboBox)
                    .Items(fontAtlases[0]->GetPages())
                    .OnSelectionChanged([this, fontNames, fontAtlases](FComboBox*)
                    {
                        int fontIndex = fontComboBox->GetSelectedItemIndex();
                        int page = fontPageComboBox->GetSelectedItemIndex();

                        FBrush image = FBrush(String::Format("__FontAtlas_{}_{}", fontNames[fontIndex], page));
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
                    .Background(Colors::Black)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill),

                    FAssignNew(FStyledWidget, atlasViewport)
                    .Background(defaultImage)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill)
                )
            );

        fontComboBox->SelectItem(0);
        fontPageComboBox->SelectItem(0);
    }

    void FusionFontAtlasWindow::OnPostComputeLayout()
    {
        Super::OnPostComputeLayout();

    }

    void FusionFontAtlasWindow::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        imageAtlasSize = painter->GetImageAtlasSize();
    }

    Ref<FusionFontAtlasWindow> FusionFontAtlasWindow::Show()
    {
        if (Ref<FusionFontAtlasWindow> lock = instance.Lock())
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

        Ref<FusionFontAtlasWindow> window = (Ref<FusionFontAtlasWindow>)FusionApplication::Get()->CreateNativeWindow(
            "FusionFontAtlasWindow", 
            "Fusion Font Atlas",
            800, 600,
            Self::StaticClass(), info);

        instance = window;

        window->ShowWindow();

        return window;
    }

}

