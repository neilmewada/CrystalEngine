#include "FusionCore.h"

namespace CE
{

    FWindow::FWindow()
    {
        m_Opacity = 1.0f;
    }

    void FWindow::ShowWindow()
    {
        PlatformWindow* platformWindow = GetPlatformWindow();
        if (platformWindow)
        {
            platformWindow->Show();
        }
    }

    void FWindow::HideWindow()
    {
        PlatformWindow* platformWindow = GetPlatformWindow();
        if (platformWindow)
        {
            platformWindow->Hide();
        }
    }

    PlatformWindow* FWindow::GetPlatformWindow()
    {
        FFusionContext* context = GetContext();
        if (context == nullptr)
            return nullptr;
        if (!context->IsOfType<FNativeContext>())
            return nullptr;

        FNativeContext* nativeContext = static_cast<FNativeContext*>(context);
        return nativeContext->GetPlatformWindow();
    }

    void FWindow::SetWindowContent(FWidget& content)
    {
        Child(content);
    }

    void FWindow::OnPaint(FPainter* painter)
    {
        bool pushed = false;
        if (m_Opacity < 0.999f)
        {
            pushed = true;
            painter->PushOpacity(m_Opacity);
        }

        Super::OnPaint(painter);

        if (pushed)
        {
            painter->PopOpacity();
        }
    }
}

