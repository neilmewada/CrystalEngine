#include "Fusion.h"

namespace CE
{

    FTreeViewHeader::FTreeViewHeader()
    {

    }

    u32 FTreeViewHeader::GetColumnCount() const
    {
        if (contentStack->GetChildCount() == 0)
            return 0;
        return contentStack->GetChildCount() / 2 + 1;
    }

    FTreeViewHeaderColumn* FTreeViewHeader::GetColumn(u32 index)
    {
        return static_cast<FTreeViewHeaderColumn*>(contentStack->GetChild(index).Get());
    }

    void FTreeViewHeader::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FSplitBox, contentStack)
            .Direction(FSplitDirection::Horizontal)
            .SplitterSize(10)
            .SplitterDrawRatio(0.1f)
            .SplitterBackground(Colors::Black)
            .SplitterHoverBackground(Color::RGBA(160, 160, 160))
            .ContentHAlign(HAlign::Fill)
            .ContentVAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
        );
    }

    void FTreeViewHeader::HandleEvent(FEvent* event)
    {

	    Super::HandleEvent(event);
    }

    FStyledWidget* FTreeViewHeader::CreateSeparator()
    {
        FStyledWidget* separator = nullptr;

        FAssignNew(FStyledWidget, separator)
        .Background(Colors::Black)
        .Width(1)
        .Padding(Vec4())
        .VAlign(VAlign::Fill)
        .Name("Separator")
    	;

        if (treeView)
        {
            treeView->ApplyStyle();
        }

        return separator;
    }

}

