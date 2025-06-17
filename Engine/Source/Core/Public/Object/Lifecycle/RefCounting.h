#pragma once

namespace CE
{

	class Object;

	namespace Internal
	{
		class CORE_API RefCountControl
		{
		public:

            int AddStrongRef();

			int AddWeakRef()
			{
				return weakReferences.fetch_add(1) + 1;
			}

			int GetNumStrongRefs()
			{
				return strongReferences;
			}

			int GetNumWeakRefs()
			{
				return weakReferences;
			}

			int ReleaseStrongRef();

			int ReleaseWeakRef();

			Object* GetObject();

			bool IsDestroyed();

		private:

			~RefCountControl();

			void SelfDestroy();

			enum ObjectState : int
			{
				NotInitialized,
				Alive,
				Destroyed
			};

			std::atomic<int> strongReferences = 0;
			std::atomic<int> weakReferences = 0;
			Object* object = nullptr;
			SharedMutex lock{};

			ObjectState objectState = ObjectState::NotInitialized;
            
            friend class CE::Object;
		};
	}

} // namespace CE
