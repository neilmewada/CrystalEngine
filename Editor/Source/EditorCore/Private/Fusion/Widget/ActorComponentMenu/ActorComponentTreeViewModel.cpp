#include "EditorCore.h"

namespace CE::Editor
{

    ActorComponentTreeViewModel::ActorComponentTreeViewModel()
    {
        
    }

    FModelIndex ActorComponentTreeViewModel::GetParent(const FModelIndex& index)
    {
        if (!index.IsValid())
            return {};

        ClassType* clazz = index.GetData().GetValue<ClassType*>();
        if (!clazz || clazz->GetSuperClassCount() == 0)
            return {};

        ClassType* superClass = clazz->GetSuperClass(0);
        if (!superClass->IsSubclassOf<ActorComponent>())
            return {};

        ClassType* superClassSuper = superClass->GetSuperClass(0);
        auto derivedClasses = superClassSuper->GetDirectDerivedClasses();
        int indexOfSuperClass = derivedClasses.IndexOf(superClass);
        if (indexOfSuperClass < 0)
            return {};

        return CreateIndex(indexOfSuperClass, 0, superClass);
    }

    FModelIndex ActorComponentTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        if (!parent.IsValid())
        {
            if (row != 0)
                return {};
            return CreateIndex(0, 0, ActorComponent::StaticClass());
        }

        ClassType* parentClass = parent.GetData().GetValue<ClassType*>();
        if (!parentClass)
            return {};

        auto derivedClasses = parentClass->GetDirectDerivedClasses();
        if (row >= derivedClasses.GetSize())
            return {};

        return CreateIndex(row, column, derivedClasses[row]);
    }

    u32 ActorComponentTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        if (!parent.IsValid())
        {
            return 1; // ActorComponent
        }

        ClassType* clazz = parent.GetData().GetValue<ClassType*>();
        if (!clazz)
            return 0;

        return clazz->GetDirectDerivedClasses().GetSize();
    }

    u32 ActorComponentTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return 1;
    }

    void ActorComponentTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
    {
        FTreeViewRow& treeViewRow = rowWidget.As<FTreeViewRow>();

        FModelIndex index = GetIndex(row, 0, parent);
        if (!index.IsValid())
            return;

        ClassType* clazz = index.GetData().GetValue<ClassType*>();
        if (!clazz)
            return;

        treeViewRow.GetCell(0)->Text(clazz->GetDisplayName());
    }

} // namespace CE

