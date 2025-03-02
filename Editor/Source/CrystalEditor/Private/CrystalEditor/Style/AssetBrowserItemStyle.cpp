#include "CrystalEditor.h"

namespace CE
{

    AssetBrowserItemStyle::AssetBrowserItemStyle()
    {
        
    }

    AssetBrowserItemStyle::~AssetBrowserItemStyle()
    {
        
    }

    SubClass<FWidget> AssetBrowserItemStyle::GetWidgetClass() const
    {
        return AssetBrowserItem::StaticClass();
    }

    void AssetBrowserItemStyle::MakeStyle(FWidget& widget)
    {
        Super::MakeStyle(widget);

        AssetBrowserItem& item = widget.As<AssetBrowserItem>();

        if (item.IsDirectory() && !item.IsHovered() && !item.IsActive())
        {
            item
        	.Background(folderBackground)
        	;
        }
    }

} // namespace CE

