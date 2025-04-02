
#include "AssetProcessorCLI.h"

int main(int argc, char** argv)
{
	AssetProcessorCLI assetProcessor{ argc, argv };

	return assetProcessor.Run();
}
