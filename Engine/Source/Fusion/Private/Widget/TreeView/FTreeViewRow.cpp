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
        if (event->IsMouseEvent() && treeView)
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MousePress && mouseEvent->sender == this)
            {
                if (mouseEvent->IsLeftButton())
                {
                    bool isCtrl = EnumHasFlag(mouseEvent->keyModifiers, KeyModifier::Ctrl);
#if PLATFORM_MAC
                    isCtrl = EnumHasFlag(mouseEvent->keyModifiers, KeyModifier::Cmd);
#endif

                    treeView->SelectRow(index, isCtrl);

                    if (mouseEvent->isDoubleClick)
                    {
                        int col = treeView->ExpandableColumn();
                        if (col >= 0 && col < GetCellCount() && GetCell(col)->ArrowVisible())
                        {
                            GetCell(col)->OnToggleExpansion().Invoke();
                        }
                    }
                }
                else if (mouseEvent->IsRightButton() && !mouseEvent->isDoubleClick)
                {
                    treeView->OnRowRightClicked(*this, mouseEvent->mousePosition);
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

