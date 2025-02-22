
#include "Engine.h"

#include "Resource.h"

namespace CE
{
	static JobManager* gJobManager = nullptr;
	static JobContext* gJobContext = nullptr;

    class EngineModule : public Module
    {
    public:

        void StartupModule() override
        {
			JobManagerDesc desc{};
			desc.defaultTag = JOB_THREAD_WORKER;
			desc.totalThreads = 0; // auto set optimal number of threads
            
			gJobManager = new JobManager("JobSystemManager", desc);
			gJobContext = new JobContext(gJobManager);
			JobContext::PushGlobalContext(gJobContext);

			onClassRegistered = CoreObjectDelegates::onClassRegistered.AddDelegateInstance(MemberDelegate(&EngineModule::OnClassRegistered, this));
        }

        void ShutdownModule() override
        {
			CoreObjectDelegates::onClassRegistered.RemoveDelegateInstance(onClassRegistered);

			gJobManager->DeactivateWorkersAndWait();

			JobContext::PopGlobalContext();
			delete gJobContext;
			gJobContext = nullptr;
			delete gJobManager;
			gJobManager = nullptr;
        }

        void RegisterTypes() override
        {
			
        }

		void OnClassRegistered(ClassType* type)
		{
			if (type == nullptr)
				return;

			if (type->GetTypeId() != TYPEID(EngineSubsystem) && 
				type->IsSubclassOf<EngineSubsystem>())
			{
				if (gEngine == nullptr || !gEngine->IsInitialized()) // Engine has not been initialized yet, cache the subsystem class type.
				{
					Engine::subsystemClassQueue.Add(type);
				}
				else
				{
					EngineSubsystem* engineSubsystem = gEngine->CreateSubsystem(type);
					engineSubsystem->Initialize();
					engineSubsystem->PostInitialize();
				}
			}

			if (type->GetTypeId() != TYPEID(GameInstanceSubsystem) &&
				type->IsSubclassOf<GameInstanceSubsystem>())
			{
				if (gEngine == nullptr || gEngine->GetGameInstance() == nullptr || !gEngine->GetGameInstance()->IsInitialized())
				{
					GameInstance::subsystemClassesQueue.Add(type);
				}
				else
				{
					gEngine->GetGameInstance()->CreateSubsystem(type);
				}
			}
		}

		DelegateHandle onClassRegistered = 0;
    };

	/*
	*	Globals
	*/

	ENGINE_API Engine* gEngine = nullptr;
    
    ENGINE_API JobManager* GetJobManager()
    {
        return gJobManager;
    }

} // namespace CE


#include "Engine.private.h"
CE_IMPLEMENT_MODULE(Engine, CE::EngineModule)

