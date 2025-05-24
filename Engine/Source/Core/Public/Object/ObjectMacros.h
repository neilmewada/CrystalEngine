#pragma once

#include "Misc/CoreDefines.h"

namespace CE
{

	enum ObjectFlags : u32
	{
		OF_NoFlags = 0,

		OF_Transient = BIT(0),
		OF_ClassDefaultInstance = BIT(1),
		OF_TemplateInstance = BIT(2),
		OF_DefaultSubobject = BIT(3),
		// Internal flag!
		OF_SubobjectPending = BIT(4),
		// Internal flag!
		OF_InsideConstructor = BIT(5),
		// Internal flag!
		OF_PendingDestroy = BIT(6),
		// Internal flag!
		OF_TemporaryBundleInstance = BIT(7),
	};
	ENUM_CLASS_FLAGS(ObjectFlags);

	enum LoadFlags : u32
	{
		LOAD_Default = 0,
		LOAD_Full = BIT(0),
	};
	ENUM_CLASS_FLAGS(LoadFlags);

	
}
