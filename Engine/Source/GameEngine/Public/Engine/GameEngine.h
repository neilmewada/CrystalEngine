#pragma once

namespace CE
{
	CLASS(NonSerialized, Config = Game)
	class GAMEENGINE_API GameEngine : public Engine
	{
		CE_CLASS(GameEngine, Engine)
	public:

		GameEngine();
		virtual ~GameEngine();

	protected:

		void Initialize() override;

		void Shutdown() override;
		
		FIELD()
		GameInstance* gameInstance = nullptr;
	};
    
} // namespace CE

#include "GameEngine.rtti.h"
