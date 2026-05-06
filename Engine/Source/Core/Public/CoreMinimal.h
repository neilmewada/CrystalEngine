#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

// Basic headers
#include "CoreMemory.h"
#include "CoreTypes.h"
#include "CoreTemplates.h"
#include "CoreContainers.h"

#include "CoreGlobals.h"

// ******************************************************
// Basic headers
#include "Misc/Exception.h"
#include "Templates/IntrusivePtr.h"
#include "Misc/Assert.h"
#include "Misc/ByteUtils.h"
#include "Misc/Defer.h"
#include "Misc/Random.h"
#include "Misc/Platform.h"
#include "Memory/Memory.h"
#include "Memory/IAllocator.h"
#include "Memory/OSAllocator.h"
#include "Memory/FixedSizeAllocator.h"
#include "Logger/Logger.h"
#include "PAL/Common/PlatformMisc.h"
#include "PAL/Common/PlatformDirectories.h"
#include "PAL/Common/PlatformProcess.h"
#include "PAL/Common/PlatformTLS.h"

// Special containers
#include "Containers/PagedDynamicArray.h"

// IO
#include "IO/Path.h"
#include "IO/Stream/GenericStream.h"
#include "IO/Stream/MemoryStream.h"
#include "IO/Stream/FileStream.h"
#include "IO/Archive.h"
#include "IO/EngineDirectories.h"

// *********************************

#include <cpptrace/cpptrace.hpp>

// Module
#include "Module/Module.h"
#include "Module/ModuleManager.h"

// Delegates
#include "RTTI/RTTIDefines.h"
#include "Delegates/Delegate.h"
#include "Delegates/MultiCastDelegate.h"
#include "Delegates/DelegateRTTI.h"
#include "Misc/CoreDelegates.h"

// Threading
#include "Threading/Mutex.h"
#include "Threading/Thread.h"
#include "Threading/ThreadLocalContext.h"
#include "Threading/Async.h"

// ******************************************************
// Math
#include "Math/Math.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"
#include "Math/Matrix.h"
#include "Math/Color.h"
#include "Math/Gradient.h"
#include "Math/Aabb.h"

// Jobs
#include "Jobs/JobContext.h"
#include "Jobs/Job.h"
#include "Jobs/JobFunction.h"
#include "Jobs/JobCompletion.h"
#include "Jobs/WorkQueue.h"
#include "Jobs/WorkThread.h"
#include "Jobs/JobManager.h"

// Config INI
#include "Config/ConfigTypes.h"
#include "Config/ConfigTiers.h"
#include "Config/ConfigFile.h"

// ******************************************************
// RTTI, Object & Serialization

// Serialization
#include "Serialization/Stream.h"
#include "Serialization/MemoryStream.h"
#include "Serialization/FileStream.h"
#include "Serialization/ArchiveStream.h"

// Json
#include "Json/Json.h"

// Reference counting
#include "Object/Lifecycle/RefCounting.h"
#include "Object/Lifecycle/Ref.h"
#include "Object/Lifecycle/WeakRef.h"

// RTTI & Object
#include "RTTI/RTTI.h"
#include "Object/Interface.h"
#include "Object/Object.h"
#include "Object/Serialization.h"
#include "Object/FieldSerializer.h"
#include "Object/JsonSerializer.h"
#include "Object/Serialization/BinarySerializer.h"
#include "Object/ObjectGlobals.h"
#include "Object/ObjectManager.h"
#include "Object/BasePrefs.h"
#include "Object/Prefs.h"
#include "Object/Bundle.h"
#include "Object/Serialization/ObjectSerializer.h"

// RTTI Dependents
#include "Serialization/BinaryBlob.h"
#include "Serialization/TextBlob.h"
#include "Math/MathRTTI.h"
#include "IO/FileWatcher.h"

// ******************************************************
// Messaging
#include "Messaging/MessageBus.h"

