#include "EditorCore.h"

namespace CE::Editor
{

    AssetThumbnailGen::AssetThumbnailGen()
    {

    }

    AssetThumbnailGen::~AssetThumbnailGen()
    {
        
    }

    void AssetThumbnailGen::OnFinish()
    {
        onFinishEvent.Broadcast(this);
    }
} // namespace CE

