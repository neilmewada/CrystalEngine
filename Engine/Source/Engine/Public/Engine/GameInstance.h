#pragma once

namespace CE
{
	class GameInstanceSubsystem;

	CLASS()
	class ENGINE_API GameInstance : public Object
	{
		CE_CLASS(GameInstance, Object)
	public:

		GameInstance();

		// - Public API -

		inline bool IsInitialized() const { return isInitailized; }

		GameInstanceSubsystem* CreateSubsystem(ClassType* subsystemClass);

		GameInstanceSubsystem* GetSubsystem(ClassType* subsystemClass);

		template<typename TGameInstanceSubsystem> requires TIsBaseClassOf<GameInstanceSubsystem, TGameInstanceSubsystem>::Value
		FORCE_INLINE TGameInstanceSubsystem* GetSubsystem()
		{
			return (TGameInstanceSubsystem*)GetSubsystem(TGameInstanceSubsystem::StaticType());
		}

		virtual void Initialize();
		virtual void Shutdown();

		virtual void Tick(f32 delta);

	protected:

		FIELD()
		Array<GameInstanceSubsystem*> subsystems{};

		bool isInitailized = false;

		static Array<ClassType*> subsystemClassesQueue;

		friend class EngineModule;
	};
    
} // namespace CE

#include "GameInstance.rtti.h"