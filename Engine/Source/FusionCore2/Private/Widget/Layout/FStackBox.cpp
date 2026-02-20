#include "FusionCore.h"

namespace CE
{

    FStackBox::FStackBox()
    {

    }

    void FStackBox::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        Super::SetParentSurfaceRecursive(surface);

        for (int i = 0; i < children.GetSize(); i++)
        {
            children[i]->SetParentSurfaceRecursive(surface);
        }
    }
}

