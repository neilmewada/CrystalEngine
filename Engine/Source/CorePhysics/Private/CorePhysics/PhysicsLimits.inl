#pragma once

namespace CE
{

	constexpr SIZE_T TempAllocatorSize = 10_MB;

	constexpr SIZE_T MaxPhysicsJobs = 2048;

	constexpr SIZE_T MaxPhysicsBarriers = 8;

	constexpr SIZE_T NumPhysicsThreads = 2;

	constexpr SIZE_T NumBodyMutexes = 0;

	//! @brief The number of threads that will be excluded from physics. Thereby leaving them for other tasks such as rendering, asset processing, etc.
	constexpr SIZE_T NonPhysicsThreadCount = 8;

	//! @brief The maximum number of physics bodies that can be created in the physics system.
	constexpr SIZE_T MaxPhysicsBodies = 100'000;

	constexpr SIZE_T MaxBodyPairs = 100'000;

	//! @brief Max number of contacts that can be processed.
	constexpr SIZE_T MaxContactConstraints = 10'000;

}
