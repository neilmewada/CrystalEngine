#include "FusionCore.h"

namespace CE
{

    FStyleSet::FStyleSet()
    {
        
    }

    void FStyleSet::Add(const Name& name, FStyle* style)
    {
        if (style && name.IsValid())
        {
            styles.Add(name, style);
        }
    }

    void FStyleSet::Add(FStyle* style)
    {
        if (style)
        {
            Add(style->GetName(), style);
        }
    }

    void FStyleSet::Remove(const Name& styleName)
    {
        styles.Remove(styleName);
    }

    FStyle* FStyleSet::FindStyle(const Name& styleName)
    {
        ZoneScoped;

        if (styles.KeyExists(styleName) && styles[styleName] != nullptr)
        {
            return styles[styleName];
        }
        if (parent != nullptr)
        {
            return parent->FindStyle(styleName);
        }

        return nullptr;
    }

    void FStyleSet::SetParent(FStyleSet* parent)
    {
        this->parent = parent;
    }

} // namespace CE
