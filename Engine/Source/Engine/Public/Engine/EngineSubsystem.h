#pragma once

namespace CE
{
    CLASS(Abstract)
	class ENGINE_API EngineSubsystem : public Subsystem
	{
		CE_CLASS(EngineSubsystem, Subsystem)
	public:

		EngineSubsystem();

	protected:


	};

} // namespace CE

#include "EngineSubsystem.rtti.h"