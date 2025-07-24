#include "FusionTest.h"


namespace WidgetTests
{

	FusionTestWindow::FusionTestWindow()
	{
        m_DockspaceClass = MajorDockspace::StaticClass();
	}

    void FusionTestWindow::Construct()
    {
        Super::Construct();

        for (int i = 1; i <= 3; i++)
        {
            Ref<MinorDockspace> minorDockspace;

            dockspace->AddDockWindow(
                FNew(FDockWindow)
                .CanBeUndocked(i != 1)
                .AllowedDockspaces(FDockspaceFilter().WithDockTypeMask(FDockTypeMask::Major))
                .Title(String::Format("Major {}", i))
                .Background(Color::RGBA(26, 26, 26))
                .Child(
                    FNew(FVerticalStack)
                    .ContentHAlign(HAlign::Fill)
                    .ContentVAlign(VAlign::Top)
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Fill)
                    (
                        FNew(FStyledWidget)
                        .Background(Color::RGBA(36, 36, 36))
                        .HAlign(HAlign::Fill)
                        .Height(40),

                        FNew(FStyledWidget)
                        .Background(Color::RGBA(26, 26, 26))
                        .HAlign(HAlign::Fill)
                        .Height(1.0f),

                        FAssignNew(MinorDockspace, minorDockspace)
                        .HAlign(HAlign::Fill)
                        .FillRatio(1.0f)
                    )
                )
                .Name(String::Format("Major{}", i))
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                .As<FDockWindow>()
            );

            if (i == 3)
            {
                
            }
            else
            {
                for (int j = 1; j <= 5; j++)
                {
                    minorDockspace->AddDockWindow(
                        FNew(FDockWindow)
                        .AllowedDockspaces(FDockspaceFilter().WithDockTypeMask(FDockTypeMask::All))
                        .Title(String::Format("Minor {} ({})", j, i))
                        .Background(Color::RGBA(36, 36, 36))
                        .Child(
                            FNew(FVerticalStack)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Top)
                            (
                                FNew(FLabel)
                                .Text(String::Format("This is {} minor window in {} major window", j, i))
                                .FontSize(16)
                                .HAlign(HAlign::Fill)
                                .VAlign(VAlign::Fill),

                                FNew(FTextButton)
                                .Text(Flipped() ? "SDF, PNG" : "PNG, SDF")
                                .OnButtonClicked([this](FButton* button, Vec2)
                                {
	                                Flipped(!Flipped());
                                    ((FTextButton*)button)->Text(Flipped() ? "SDF, PNG" : "PNG, SDF");
                                }),

                                FNew(FVerticalStack)
                                .ContentVAlign(VAlign::Top)
                                .Margin(Vec4(0, 150, 0, 0))
                                .Scale(Vec2(1, 1) * 15)
                                (
                                    FAssignNew(FLabel, label)
                                    .FontSize(8)
                                    .Text("Hello World!")
                                    .HAlign(HAlign::Center)
                                )
                            )
                        )
                        .Name(String::Format("Minor{}_{}", j, i))
                        .HAlign(HAlign::Fill)
                        .VAlign(VAlign::Fill)
                        .As<FDockWindow>()
                    );
                }
            }
        }
    }

    void FusionTestWindow::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        return;

        String text = "quick fox jumped over last";

        FixedArray<u32, 8> fontSizes = { 30, 24, 16, 13, 10, 7, 7, 7 };

        auto func1 = &FPainter::DrawText;
        auto func2 = &FPainter::DrawSDFText;

        if (m_Flipped)
        {
            std::swap(func1, func2);
        }

        Vec2 topLeft = Vec2(25, 200);
        for (int i = 0; i < fontSizes.GetSize(); i++)
        {
            painter->SetFontSize(fontSizes[i]);
            (painter->*func1)(text, topLeft, Vec2(), FWordWrap::Normal);

            topLeft.y += fontSizes[i] * 2;
        }

        Vec2 topRight = Vec2(550, 200);
        for (int i = 0; i < fontSizes.GetSize(); i++)
        {
            painter->SetFontSize(fontSizes[i]);
            (painter->*func2)(text, topRight, Vec2(), FWordWrap::Normal);

            topRight.y += fontSizes[i] * 2;
        }
    }

}

