#include "FusionCore.h"

namespace CE
{

    FSurface::FSurface()
    {

    }

    void FSurface::GetDrawListMask(DrawListMask& drawListMask)
    {
		if (drawListTag.IsValid())
        {
            drawListMask.Set(drawListTag);
        }
    }

    void FSurface::AddChildSurface(Ref<FSurface> childSurface)
    {
		if (childrenSurfaces.Exists(childSurface))
        {
            return;
        }

		childrenSurfaces.Add(childSurface);
		childSurface->parentSurface = this;
    }

    bool FSurface::IsNativeSurface()
    {
		return IsOfType<FNativeSurface>();
    }

} // namespace CE

