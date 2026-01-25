#include "Engine.h"

namespace CE
{

    SceneRenderer::SceneRenderer()
    {

    }

    SceneRenderer::~SceneRenderer()
    {
        
    }

    void SceneRenderer::SetScene(Ref<CE::Scene> scene)
    {
        this->targetScene = scene;

        if (this->targetScene && outputImages[0] != nullptr)
        {
            targetScene->GetRpiScene()->SetPrimaryViewportSize(Vec2i(outputImages[0]->GetWidth(), outputImages[0]->GetHeight()));
        }
    }

    void SceneRenderer::SetOutputImage(int imageIndex, RHI::Texture* outputImage)
    {
        this->outputImages[imageIndex] = outputImage;

        if (targetScene)
        {
	        targetScene->GetRpiScene()->SetPrimaryViewportSize(Vec2i(outputImage->GetWidth(), outputImage->GetHeight()));
        }
    }
} // namespace CE

