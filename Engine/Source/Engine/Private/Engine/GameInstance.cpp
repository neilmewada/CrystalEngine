#include "Engine.h"

namespace CE
{
	Array<ClassType*> GameInstance::subsystemClassesQueue{};

    GameInstance::GameInstance()
    {

    }

	GameInstanceSubsystem* GameInstance::CreateSubsystem(ClassType* subsystemClass)
	{
		if (subsystemClass == nullptr || 
			subsystemClass->GetTypeId() == TYPEID(GameInstanceSubsystem) ||
			!subsystemClass->IsSubclassOf<GameInstanceSubsystem>())
			return nullptr;

		auto cdi = (GameInstanceSubsystem*)subsystemClass->GetDefaultInstance();
		if (cdi == nullptr || !cdi->ShouldBeCreated(this))
			return nullptr;

		// Check if subsystem already exists. Only 1 subsystem object allowed per class type.
		int index = subsystems.IndexOf([&](GameInstanceSubsystem* subsystem) -> bool
			{ return subsystem != nullptr && subsystem->GetClass()->GetTypeId() == subsystemClass->GetTypeId(); });

		if (index >= 0)
			return subsystems[index];

		String name = subsystemClass->GetName().GetLastComponent();
		GameInstanceSubsystem* subsystem = CreateObject<GameInstanceSubsystem>(this, name, OF_NoFlags, subsystemClass);
		if (subsystem != nullptr)
		{
			subsystems.Add(subsystem);
			if (isInitailized)
				subsystem->Initialize();
		}

		return subsystem;
	}

	GameInstanceSubsystem* GameInstance::GetSubsystem(ClassType* type)
	{
		if (type == nullptr ||
			type->GetTypeId() == TYPEID(GameInstanceSubsystem) ||
			!type->IsSubclassOf<GameInstanceSubsystem>())
			return nullptr;

		// Get the index of the subsystem
		int index = subsystems.IndexOf([&](GameInstanceSubsystem* subsystem) -> bool
			{ return subsystem != nullptr && subsystem->GetClass()->GetTypeId() == type->GetTypeId(); });

		if (index >= 0)
			return subsystems[index];

		return nullptr;
	}

	void GameInstance::Initialize()
	{
		for (auto classType : subsystemClassesQueue)
		{
			CreateSubsystem(classType);
		}
		subsystemClassesQueue.Clear();

		for (auto subsystem : subsystems)
		{
			subsystem->Initialize();
		}

		isInitailized = true;
	}

	void GameInstance::Shutdown()
	{
		isInitailized = false;

		for (auto subsystem : subsystems)
		{
			subsystem->Shutdown();
		}
		for (auto subsystem : subsystems)
		{
			subsystem->BeginDestroy();
		}
		subsystems.Clear();
	}

	void GameInstance::Tick(f32 delta)
	{
		for (auto subsystem : subsystems)
			subsystem->Tick(delta);
	}

} // namespace CE

