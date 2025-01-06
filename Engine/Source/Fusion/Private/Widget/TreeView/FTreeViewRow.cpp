#include "Fusion.h"

namespace CE
{

    FTreeViewRow::FTreeViewRow()
    {

    }

    FTreeViewCell* FTreeViewRow::GetCell(u32 index) const
    {
        return static_cast<FTreeViewCell*>(contentStack->GetChild(index).Get());
    }

    void FTreeViewRow::AddCell(FTreeViewCell& cell)
    {
        contentStack->AddChild(&cell);
    }

    void FTreeViewRow::AddCell(FTreeViewCell* cell)
    {
        contentStack->AddChild(cell);
    }

    void FTreeViewRow::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FHorizontalStack, contentStack)
            .ContentHAlign(HAlign::Fill)
            .ContentVAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
        );
    }

    void FTreeViewRow::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MousePress && mouseEvent->IsLeftButton())
            {
                treeView->SelectRow(index);

                if (mouseEvent->isDoubleClick && mouseEvent->sender == this)
                {
                    int col = treeView->ExpandableColumn();
                    if (col >= 0 && col < GetCellCount() && GetCell(col)->ArrowVisible())
                    {
                        GetCell(col)->OnToggleExpansion().Invoke();
                    }
                }
            }
            else if (mouseEvent->type == FEventType::MouseEnter)
            {
                isHovered = true;
                treeView->ApplyStyle();
            }
            else if (mouseEvent->type == FEventType::MouseLeave)
            {
                isHovered = false;
                treeView->ApplyStyle();
            }
        }

	    Super::HandleEvent(event);
    }
}

