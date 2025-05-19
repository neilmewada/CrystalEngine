#include "Fusion.h"

namespace CE
{
    constexpr f32 RowHideDistance = 5;

    FListViewContainer::FListViewContainer()
    {

    }

    void FListViewContainer::Construct()
    {
        Super::Construct();
        
    }

    void FListViewContainer::SetContextRecursively(FFusionContext* context)
    {
        Super::SetContextRecursively(context);

        for (FListViewRow* child : children)
        {
            child->SetContextRecursively(context);
        }
    }

    void FListViewContainer::OnPostComputeLayout()
    {
        Super::OnPostComputeLayout();

        UpdateRows();
    }

    void FListViewContainer::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

        if (children.IsEmpty() || !Enabled())
            return;

        FScrollBox* parentScroll = static_cast<FScrollBox*>(GetParent());
        Vec2 translation = Translation();
        Rect visibleRect = Rect::FromSize(-translation, GetComputedSize());

        for (FListViewRow* child : children)
        {
            if (!child->Enabled() || !child->Visible())
                continue;

            Rect childRect = Rect::FromSize(child->GetComputedPosition(), child->GetComputedSize());
            if (!childRect.Overlaps(visibleRect))
                continue;

            if (child->IsTranslationOnly())
            {
                painter->PushChildCoordinateSpace(child->GetComputedPosition() + child->Translation());
            }
            else
            {
                painter->PushChildCoordinateSpace(child->GetLocalTransform());
            }

            child->OnPaint(painter);

            painter->PopChildCoordinateSpace();
        }
    }

    FWidget* FListViewContainer::HitTest(Vec2 localMousePos)
    {
        FWidget* thisHitTest = Super::HitTest(localMousePos);
        if (thisHitTest == nullptr)
            return nullptr;
        if (children.IsEmpty())
            return thisHitTest;

        Vec2 transformedMousePos = mouseTransform * Matrix4x4::Translation(-Translation()) * Vec4(localMousePos.x, localMousePos.y, 0, 1);

        for (int i = children.GetCount() - 1; i >= 0; --i)
        {
            FListViewRow* child = children[i];
            if (!child->Enabled())
                continue;

            FWidget* result = child->HitTest(transformedMousePos);
            if (result)
            {
                return result;
            }
        }

        return thisHitTest;
    }

    bool FListViewContainer::ChildExistsRecursive(FWidget* child)
    {
        if (this == child)
            return true;

        for (FListViewRow* widget : children)
        {
            if (widget->ChildExistsRecursive(child))
                return true;
        }

        return false;
    }

    void FListViewContainer::CalculateIntrinsicSize()
    {
        ZoneScoped;

        Ref<FListView> listView = this->listView.Lock();

        if (listView == nullptr || listView->Model() == nullptr || !listView->m_GenerateRowCallback.IsValid())
        {
            Super::CalculateIntrinsicSize();
            return;
        }

        Ref<FListViewModel> model = listView->Model();

        intrinsicSize.width = m_Padding.left + m_Padding.right;
        intrinsicSize.height = m_Padding.top + m_Padding.bottom;

        Vec2 contentSize = Vec2();
        totalRowHeight = 0;

        int rowCount = model->GetRowCount();

        if (listView->m_RowHeightDelegate.IsValid())
        {
            for (int i = 0; i < rowCount; ++i)
            {
                totalRowHeight += listView->m_RowHeightDelegate(i);
            }
        }
        else
        {
            totalRowHeight = rowCount * listView->m_RowHeight;
        }

        contentSize.height = totalRowHeight;

        for (int i = 0; i < children.GetCount(); ++i)
        {
            if (!children[i]->Enabled())
                continue;

            children[i]->CalculateIntrinsicSize();
            contentSize.width = Math::Max(contentSize.width, children[i]->GetIntrinsicSize().width);
        }

        if (totalRowHeight >= computedSize.y)
        {
            //Padding(Vec4(0, 0, listView->scrollBox->ScrollBarWidth() + listView->scrollBox->ScrollBarMargin(), 0));
            Padding(Vec4(0, 0, 0, 0));
        }
        else
        {
            Padding(Vec4(0, 0, 0, 0));
        }

        intrinsicSize.width += contentSize.width;
        intrinsicSize.height += contentSize.height;

        ApplyIntrinsicSizeConstraints();
    }

    void FListViewContainer::PlaceSubWidgets()
    {
        ZoneScoped;

        Super::PlaceSubWidgets();

        if (children.IsEmpty())
        {
            return;
        }

        Vec2 curPos = Vec2(m_Padding.left, m_Padding.top);
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
            m_Padding.top + m_Padding.bottom);

        if (listView->m_RowHeightDelegate.IsValid())
        {

        }
        else
        {
            for (int i = 0; i < children.GetCount(); ++i)
            {
                FListViewRow* child = children[i];

                if (!child->Enabled())
                    continue;

                f32 rowHeight = listView->m_RowHeight;

                curPos.y = child->rowIndex * rowHeight;

                child->SetComputedPosition(curPos);
                child->SetComputedSize(Vec2(availableSize.x, Math::Max(rowHeight, child->GetIntrinsicSize().height)));

                child->PlaceSubWidgets();

                //curPos.y += child->computedSize.y;
            }
        }
    }

    void FListViewContainer::ApplyStyleRecursively()
    {
        Super::ApplyStyleRecursively();

        for (FListViewRow* widget : children)
        {
            widget->ApplyStyleRecursively();
        }
    }

    void FListViewContainer::HandleEvent(FEvent* event)
    {
        if (event->stopPropagation)
            return;

        if (event->direction == FEventDirection::TopToBottom)
        {
            for (FListViewRow* child : children)
            {
                if (!child->Enabled())
                    continue;

                child->HandleEvent(event);

                if (event->stopPropagation)
                {
                    event->stopPropagation = false;
                }
            }
        }

        if (event->IsMouseEvent() && event->sender == this)
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MousePress && mouseEvent->IsLeftButton())
            {
                // TODO: Selection
                //treeView->SelectionModel()->ClearSelection();
                //treeView->ApplyStyle();
            }
        }

        Super::HandleEvent(event);
    }

    void FListViewContainer::UpdateRows()
    {
        ZoneScoped;

        Ref<FListView> listView = this->listView.Lock();

        if (listView == nullptr || listView->Model() == nullptr || !listView->m_GenerateRowCallback.IsValid())
        {
            Super::CalculateIntrinsicSize();
            return;
        }

        Ref<FWidget> parent = this->GetParent();
        if (!parent)
            return;

        Ref<FListViewModel> model = listView->Model();

        int rowCount = model->GetRowCount();

        int childIndex = 0;
        f32 curPosY = 0;
        f32 scrollY = -Translation().y;

        if (listView->m_RowHeightDelegate.IsValid())
        {
            for (int i = 0; i < rowCount; ++i)
            {
                f32 rowHeight = 0;

                rowHeight = listView->m_RowHeightDelegate(i);

                f32 topY = curPosY;
                f32 bottomY = curPosY + rowHeight;

                if (bottomY + RowHideDistance < scrollY)
                {
                    continue; // Above the bounds
                }
                else if (topY > scrollY + computedSize.y + RowHideDistance)
                {
                    break; // Below the bounds
                }

                curPosY += rowHeight;

                // TODO: Finish this
            }
        }
        else
        {
            f32 rowHeight = listView->m_RowHeight;
            int startIndex = Math::Max(0, (int)floor((scrollY - RowHideDistance) / rowHeight));

            curPosY = startIndex * rowHeight;

            for (int rowIndex = startIndex; rowIndex < rowCount; rowIndex++)
            {
                f32 topY = curPosY;
                f32 bottomY = curPosY + rowHeight;

                if (topY > scrollY + parent->GetComputedSize().y + RowHideDistance)
                {
                    break; // Below the bounds
                }

                FListViewRow* rowWidget = nullptr;

                if (childIndex < children.GetCount())
                {
                    rowWidget = children[childIndex];
                }
                else
                {
                    rowWidget = &listView->m_GenerateRowCallback();
                    children.Insert(rowWidget);
                }

                rowWidget->SetParent(this);
                rowWidget->rowIndex = rowIndex;
                rowWidget->listView = listView;
                rowWidget->isAlternate = rowIndex % 2 != 0;
                rowWidget->isSelected = listView->selectedRows.Exists(rowIndex);

                auto ctx = GetContext();
                rowWidget->SetContextRecursively(ctx);
                rowWidget->ApplyStyleRecursively();

                rowWidget->Enabled(true);

                childIndex++;
                curPosY += rowHeight;

                model->SetData(rowIndex, *rowWidget);
            }
        }

        while (childIndex < children.GetCount())
        {
            children[childIndex]->Enabled(false);
            children[childIndex]->isHovered = false;
            children[childIndex]->isSelected = false;

            childIndex++;
        }

        listView->ApplyStyle();
        MarkDirty();
    }

    void FListViewContainer::OnSelectionChanged()
    {

    }

    FListViewContainer::Self& FListViewContainer::ListView(Ref<FListView> listView)
    {
        this->listView = listView;
        return *this;
    }
}

