
#include "CoreApplication.h"

namespace CE
{
	PlatformApplication* PlatformApplication::instance = nullptr;

	PlatformApplication* PlatformApplication::Get()
	{
		if (instance == nullptr)
			instance = PlatformApplicationImpl::Create();
		return instance;
	}

	PlatformApplication* PlatformApplication::TryGet()
	{
		return instance;
	}

	PlatformApplication::~PlatformApplication()
	{
		if (instance == this)
			instance = nullptr;
	}

	void PlatformApplication::Initialize()
	{

	}

	void PlatformApplication::PreShutdown()
	{

	}

	void PlatformApplication::Shutdown()
	{

	}

	void PlatformApplication::Tick()
	{

	}

}

