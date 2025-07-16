#pragma once

namespace CE::Editor
{

	CLASS()
	class EDITORENGINE_API EditorEngine : public Engine
	{
		CE_CLASS(EditorEngine, Engine)
	protected:

		EditorEngine();
		virtual ~EditorEngine();

		void PostInitialize() override;

		void PreShutdown() override;

		void Tick(f32 deltaTime) override;

	public:

		Ref<AssetProcessor> GetAssetProcessor() { return assetProcessor; }

	protected:

		Ref<AssetProcessor> assetProcessor;
		Ref<ThumbnailSystem> thumbnailSystem;
	};
    
} // namespace CE::Editor

#include "EditorEngine.rtti.h"
