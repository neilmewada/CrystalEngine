#pragma once

#include "Core.h"
#include "CoreApplication.h"
#include "CoreRPI.h"


#define FUSION_TESTS


#define FUSION_FRIENDS FUSION_TESTS\
    friend class FWidget;


#include "FusionMacros.h"

// Application
#include "Application/Service/FService.h"
#include "Application/FApplication.h"
#include "Application/Service/FPlatformEventService.h"
#include "Application/Service/FRenderService.h"

// Surface
#include "Surface/FSurface.h"
#include "Surface/FNativeSurface.h"

// Widget
#include "Widget/FWidget.h"


