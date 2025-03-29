#pragma once

#include "Core.h"
#include "CoreRHI.h"
#include "CoreRPI.h"
#include "CoreShader.h"
#include "CoreSettings.h"
#include "FusionCore.h"
#include "Fusion.h"

// Asset Meta
#include "Asset/AssetData.h"
#include "Asset/AssetRegistry.h"
#include "Engine/AssetManager.h"

///////////////////////////////////////////////////////
// Engine
///////////////////////////////////////////////////////

#if PAL_TRAIT_BUILD_EDITOR
namespace CE::Editor
{
	class ComputeShaderAssetImportJob;
}
#endif

// Asset Types
#include "Engine/TextureDefines.h"
#include "Engine/SamplerState.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/Shader.h"
#include "Engine/ComputeShader.h"
#include "Engine/MaterialInterface.h"
#include "Engine/Material.h"
#include "Engine/MaterialInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/Font.h"

// Engine
#include "Engine/Subsystem.h"
#include "Engine/EngineSubsystem.h"
#include "Engine/Subsystems/SceneSubsystem.h"
#include "Engine/Subsystems/RendererSubsystem.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

///////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////

#include "Renderer/RenderPipeline.h"
#include "Renderer/MainRenderPipeline/MainRenderPipeline.h"

///////////////////////////////////////////////////////
// GameFramework
///////////////////////////////////////////////////////

// Components
#include "GameFramework/ActorComponent.h"
#include "GameFramework/SceneComponent.h"
#include "GameFramework/LightComponent.h"
#include "GameFramework/DirectionalLightComponent.h"
#include "GameFramework/GeometryComponent.h"
#include "GameFramework/MeshComponent.h"
#include "GameFramework/StaticMeshComponent.h"
#include "GameFramework/CameraComponent.h"

// Actors
#include "GameFramework/Actor.h"
#include "GameFramework/StaticMeshActor.h"
#include "GameFramework/CameraActor.h"
#include "GameFramework/Light.h"
#include "GameFramework/DirectionalLight.h"

// Scene
#include "Engine/Scene.h"

namespace CE
{

	/*
	*	Globals
	*/

	class Engine;

	extern ENGINE_API Engine* gEngine;

	ENGINE_API JobManager* GetJobManager();

}
