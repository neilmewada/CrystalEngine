#pragma once

#if WINDOWS_DEBUG_MEMORY_LEAKS
// For debugging only!
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "CoreTracy.h"

#include "CoreMinimal.h"

// ******************************************************
// High level

#include "Asset/PathTree.h"

// Resources
#include "Resource/Resource.h"
#include "Resource/ResourceManager.h"

// Settings
#include "Settings/Settings.h"

// Assets
#include "Asset/Asset.h"
#include "Asset/DataAsset.h"
#include "Asset/AssetPath.h"

