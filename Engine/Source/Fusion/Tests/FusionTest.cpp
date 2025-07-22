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
                            FNew(FLabel)
                            .Text(String::Format("This is {} minor window in {} major window", j, i))
                            .FontSize(16)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
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

        painter->SetFontSize(20);

        String text = "quick fox last";

        Vec2 pos = Vec2(50, 250);
        painter->DrawSDFText(text, pos);

        Vec2 normalPos = Vec2(250, 250);
        painter->DrawText(text, normalPos);

        painter->SetFontSize(10);

        pos = Vec2(50, 280);
        painter->DrawSDFText(text, pos);

        normalPos = Vec2(250, 280);
        painter->DrawText(text, normalPos);

        painter->SetFontSize(8);

        pos = Vec2(50, 300);
        painter->DrawSDFText(text, pos);

        normalPos = Vec2(250, 300);
        painter->DrawText(text, normalPos);
    }

}

