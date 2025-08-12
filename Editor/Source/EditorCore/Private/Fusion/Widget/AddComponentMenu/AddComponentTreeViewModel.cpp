#include "EditorCore.h"

namespace CE::Editor
{

    AddComponentTreeViewModel::AddComponentTreeViewModel()
    {
        
    }

    FModelIndex AddComponentTreeViewModel::GetParent(const FModelIndex& index)
    {
        if (IsFiltered())
            return {};

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

    FModelIndex AddComponentTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        if (IsFiltered())
        {
	        if (!parent.IsValid() && row < filteredList.GetSize())
	        {
                return CreateIndex(row, column, filteredList[row]);
	        }
            return {};
        }

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

    u32 AddComponentTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        if (IsFiltered())
        {
	        if (!parent.IsValid())
	        {
                return filteredList.GetSize();
	        }
            return 0;
        }

        if (!parent.IsValid())
        {
            return 1; // ActorComponent
        }

        ClassType* clazz = parent.GetData().GetValue<ClassType*>();
        if (!clazz)
            return 0;

        return clazz->GetDirectDerivedClasses().GetSize();
    }

    u32 AddComponentTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return 1;
    }

    void AddComponentTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
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

    void AddComponentTreeViewModel::SetFilter(const String& filter)
    {
        if (filter == this->filter)
            return;

        defer(&)
        {
            if (auto treeView = this->treeView.Lock())
            {
                treeView->OnModelUpdate();
            }
        };

        filteredList.Clear();

    	this->filter = filter;
        if (this->filter.IsEmpty())
            return;

        String filterLower = filter.ToLower();

        ClassType* baseClass = ActorComponent::StaticClass();

        HashSet<ClassType*> addedClasses;

        std::function<void(ClassType*)> visitor = [&](ClassType* clazz)
            {
                if (!addedClasses.Exists(clazz) && clazz->GetTypeName().GetString().ToLower().Contains(filterLower))
                {
                    filteredList.Add(clazz);
                    addedClasses.Add(clazz);
                }

                auto derivedClasses = clazz->GetDerivedClasses();
                for (ClassType* derivedClass : derivedClasses)
                {
                    visitor(derivedClass);
                }
            };

        visitor(baseClass);
    }

} // namespace CE

