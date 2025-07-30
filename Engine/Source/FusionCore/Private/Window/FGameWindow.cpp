#include "FusionCore.h"

namespace CE
{

    FGameWindow::FGameWindow()
    {

    }

    void FGameWindow::Construct()
    {
        Super::Construct();
        
    }

    void FGameWindow::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (!IsDefaultInstance())
        {
            FusionApplication::Get()->OnRenderViewportDestroyed(this);
        }
    }

    void FGameWindow::SetScene(RPI::Scene* scene)
    {
        this->scene = scene;

        FusionApplication::Get()->RebuildFrameGraph();
    }

    bool FGameWindow::IsEmbeddedViewport() const
    {
        return IsOfType<FViewport>();
    }

}

