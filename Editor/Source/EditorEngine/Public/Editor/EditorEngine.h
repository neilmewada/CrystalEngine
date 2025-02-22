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

	public:

	protected:


	};
    
} // namespace CE::Editor

#include "EditorEngine.rtti.h"
