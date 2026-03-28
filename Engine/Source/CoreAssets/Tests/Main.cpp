

#include "CoreAssets.h"


#include <iostream>
#include <any>

#include <gtest/gtest.h>

#define TEST_BEGIN BeginTest();
    

#define TEST_END EndTest();

#define LOG(x) std::cout << x << std::endl
#define LOG_ERR(x) std::cerr << x << std::endl;

using namespace CE;

static void BeginTest()
{
	CE::gProjectPath = PlatformDirectories::GetLaunchDir();
	CE::gProjectName = "CoreMedia_Test";
	CE::ModuleManager::Get().LoadModule("Core");
	CE::ModuleManager::Get().LoadModule("CoreMedia");
}

static void EndTest()
{
	CE::ModuleManager::Get().UnloadModule("CoreMedia");
	CE::ModuleManager::Get().UnloadModule("Core");
}


TEST(CoreMedia, Font)
{
	TEST_BEGIN;

	CAFontAtlasGenerateInfo genInfo{};
	genInfo.charSetRanges.Add(CACharRange('a', 'z'));
	genInfo.charSetRanges.Add(CACharRange('A', 'Z'));
	genInfo.charSetRanges.Add(CACharRange('0', '9'));
	genInfo.charSetRanges.Add(CACharRange(32, 47));
	genInfo.charSetRanges.Add(CACharRange(58, 64));
	genInfo.charSetRanges.Add(CACharRange(91, 96));
	genInfo.charSetRanges.Add(CACharRange(123, 126));

	genInfo.padding = 8;
	genInfo.fontSize = 16;

	CAFontAtlas* atlas = CAFontAtlas::GenerateFromFontFile(PlatformDirectories::GetLaunchDir() / "Temp/Roboto-Medium.ttf", genInfo);
	
	delete atlas;

	TEST_END;
}

