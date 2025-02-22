#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API CameraActor : public Actor
    {
        CE_CLASS(CameraActor, Actor)
    public:

        CameraActor();

		CameraComponent* GetCameraComponent() const { return cameraComponent; }

		void SetCameraComponent(CameraComponent* cameraComponent) { this->cameraComponent = cameraComponent; }

    protected:

		FIELD()
		CameraComponent* cameraComponent = nullptr;
    };

} // namespace CE

#include "CameraActor.rtti.h"
