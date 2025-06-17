#include  "Core.h"

namespace CE::Internal
{
    int RefCountControl::AddStrongRef()
    {
        return strongReferences.fetch_add(1) + 1;
    }

	int RefCountControl::ReleaseStrongRef()
	{
		// No lock needed to decrement atomic counter
		int refCount = strongReferences.fetch_sub(1) - 1;

		if (refCount == 0) // If no strong references are remaining
		{
			bool destroyThis;

			{
				LockGuard guard{ lock };

				objectState = ObjectState::Destroyed;

				destroyThis = weakReferences == 0;
			}

			object->control = nullptr;
			object->DestroyImmediate();

			if (destroyThis)
			{
				SelfDestroy();
			}
		}

		return refCount;
	}

	int RefCountControl::ReleaseWeakRef()
	{
		lock.Lock();

		int numWeakRefs = weakReferences.fetch_sub(1) - 1;

		if (numWeakRefs == 0 && objectState == ObjectState::Destroyed)
		{
			lock.Unlock();
			SelfDestroy();
		}
		else
		{
			lock.Unlock();
		}

		return numWeakRefs;
	}

	Object* RefCountControl::GetObject()
	{
        LockGuard guard{ lock };
        
		if (objectState != ObjectState::Alive)
			return nullptr;

		Object* retVal = nullptr;

		int strongRefCount = strongReferences.fetch_add(1) + 1;
		if (objectState == ObjectState::Alive && strongRefCount > 1)
		{
			retVal = object;
		}
		strongReferences.fetch_sub(1);

		return retVal;
	}

	bool RefCountControl::IsDestroyed()
	{
		LockGuard guard{ lock };

		return objectState == Destroyed;
	}

	RefCountControl::~RefCountControl()
	{

	}

	void RefCountControl::SelfDestroy()
	{
		object = nullptr;
		delete this;
	}
}
