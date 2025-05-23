#pragma once

#include "Core.h"
#include "CoreMedia.h"
#include "CoreRHI.h"
#include "CoreShader.h"
#include "CoreApplication.h"
#include "Engine.h"

#include "EditorCore.h"

/////////////////////////////////////////////

#include "Asset/AssetProcessor.h"

#include "Editor/EditorEngine.h"
#include "Editor/EditorAssetManager.h"


namespace CE::Editor
{
	extern EDITORENGINE_API Ref<EditorEngine> gEditor;

	EDITORENGINE_API AssetDefinitionRegistry* GetAssetDefinitionRegistry();

}
