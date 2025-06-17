#include "Fusion.h"

namespace CE
{

    FTreeViewContainer::FTreeViewContainer()
    {
        
    }

    void FTreeViewContainer::Construct()
    {
        Super::Construct();

        
    }

    void FTreeViewContainer::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

        if (children.IsEmpty() || !Enabled())
            return;

        FScrollBox* parentScroll = static_cast<FScrollBox*>(GetParent());
        Vec2 translation = Translation();
        Rect visibleRect = Rect::FromSize(-translation, GetComputedSize());

        for (FTreeViewRow* child : children)
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

    void FTreeViewContainer::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

        OnModelUpdate();
    }

    void FTreeViewContainer::SetContextRecursively(FFusionContext* context)
    {
	    Super::SetContextRecursively(context);

        for (FTreeViewRow* child : children)
        {
            child->SetContextRecursively(context);
        }
    }

    void FTreeViewContainer::ExpandRow(const FModelIndex& index, bool recursive)
    {
        if (!index.IsValid())
            return;

        expandedRows.Add(index);

        if (recursive && treeView->m_Model)
        {
            FModelIndex parent = treeView->m_Model->GetParent(index);

            while (parent.IsValid())
            {
                expandedRows.Add(parent);

                parent = treeView->m_Model->GetParent(parent);
            }
        }
    }

    FWidget* FTreeViewContainer::HitTest(Vec2 localMousePos)
    {
        FWidget* thisHitTest = Super::HitTest(localMousePos);
        if (thisHitTest == nullptr)
            return nullptr;
        if (children.IsEmpty())
            return thisHitTest;

        Vec2 transformedMousePos = mouseTransform * Matrix4x4::Translation(-Translation()) * Vec4(localMousePos.x, localMousePos.y, 0, 1);

        for (int i = children.GetCount() - 1; i >= 0; --i)
        {
            FTreeViewRow* child = children[i];
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

    bool FTreeViewContainer::ChildExistsRecursive(FWidget* child)
    {
        if (this == child)
            return true;

        for (FTreeViewRow* widget : children)
        {
            if (widget->ChildExistsRecursive(child))
                return true;
        }

        return false;
    }

    void FTreeViewContainer::ApplyStyleRecursively()
    {
        Super::ApplyStyleRecursively();

        for (FTreeViewRow* widget : children)
        {
            widget->ApplyStyleRecursively();
        }
    }

    void FTreeViewContainer::HandleEvent(FEvent* event)
    {
        if (event->stopPropagation)
            return;

        if (event->direction == FEventDirection::TopToBottom)
        {
            for (FTreeViewRow* child : children)
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
                treeView->SelectionModel()->ClearSelection();
                treeView->ApplyStyle();
            }
        }

        Super::HandleEvent(event);
    }

    void FTreeViewContainer::OnModelUpdate()
    {
        if (treeView == nullptr || treeView->m_Model == nullptr || !treeView->m_Model->IsReady())
            return;
        if (!treeView->m_GenerateRowDelegate.IsValid())
            return;

        auto model = treeView->m_Model;

        int childIndex = 0;
        f32 curPosY = 0;
        f32 scrollY = -Translation().y;
        int globalRowIdx = 0;

        if (treeView->header)
        {
            model->SetHeaderData(*treeView->header);
        }

        Delegate<void(const FModelIndex&, int)> visitor = [&](const FModelIndex& parent, int indentLevel) -> void
            {
                int rowCount = model->GetRowCount(parent);
                if (rowCount == 0)
                    return;

                for (int i = 0; i < rowCount; ++i)
                {
                    FModelIndex index = model->GetIndex(i, 0, parent);
                    if (!index.IsValid())
                        continue;

                    f32 rowHeight = 0;

                    if (!treeView->m_RowHeightDelegate.IsValid())
                    {
                        rowHeight = treeView->m_RowHeight;
                    }
                    else
                    {
                        rowHeight = treeView->m_RowHeightDelegate(index);
                    }

                    f32 topY = curPosY;
                    f32 bottomY = curPosY + rowHeight;

                    if (bottomY < scrollY)
                    {
                        //curPosY += rowHeight; // We are above the scroll view
                        //globalRowIdx++;
                        //continue;
                    }
                    else if (topY > scrollY + computedSize.y)
                    {
                        //break; // We are below the scroll view
                    }

                    FTreeViewRow* rowWidget = nullptr;
                    if (childIndex < children.GetCount())
                    {
                        rowWidget = children[childIndex];
                    }
                    else
                    {
                        rowWidget = &treeView->m_GenerateRowDelegate();
                        children.Insert(rowWidget);
                    }

                    rowWidget->SetParent(this);
                    rowWidget->index = index;
                    rowWidget->Enabled(true);
                    rowWidget->isAlternate = (globalRowIdx % 2 != 0);
                    rowWidget->treeView = treeView;
                    rowWidget->isHovered = false;

                    Ref<FFusionContext> ctx = GetContext();
                    rowWidget->SetContextRecursively(ctx.Get());
                    rowWidget->ApplyStyleRecursively();

                    childIndex++;
                    curPosY += rowHeight;

                    model->SetData(i, *rowWidget, parent);

                    int childrenCount = model->GetRowCount(index);

                    int headerCount = rowWidget->GetCellCount();
                    if (treeView->header)
                    {
                        headerCount = treeView->header->GetColumnCount();
                    }

                    f32 availWidth = treeView->GetComputedSize().width - 
                        treeView->Padding().left - treeView->Padding().right - 
                        Margin().left - Margin().right;

                    for (int c = 0; c < headerCount && c < rowWidget->GetCellCount(); ++c)
                    {
                        f32 minWidth = availWidth / Math::Min<f32>(headerCount, rowWidget->GetCellCount());
                        if (treeView->header)
                        {
	                        minWidth = treeView->header->GetColumn(c)->GetComputedSize().x;
                        }
                        rowWidget->Visible(true);

                        FTreeViewCell& cell = *rowWidget->GetCell(c);

                        cell
							.ArrowVisible(treeView->m_ExpandableColumn == c && childrenCount > 0)
							.ArrowEnabled(treeView->m_ExpandableColumn == c)
							.ArrowExpanded(expandedRows.Exists(index))
							.OnToggleExpansion([index, this]
							{
                                if (!expandedRows.Exists(index))
                                {
	                                expandedRows.Add(index);
                                }
                                else
                                {
                                    expandedRows.Remove(index);
                                }
                                MarkLayoutDirty();
							})
                        ;

                        if (treeView->m_ExpandableColumn == c && indentLevel > 0)
                        {
                            f32 indentOffset = treeView->m_Indentation * indentLevel;
                            if (indentOffset < minWidth)
                            {
                                cell.Margin(Vec4(indentOffset, 0, 0, 0));
                                cell.Width(minWidth - indentOffset);
                            }
                            else
                            {
                                rowWidget->Visible(false);
                            }
                        }
                        else
                        {
                            // Reset indentation
                            cell.Margin(Vec4(0, 0, 0, 0));
                            cell.Width(minWidth);
                        }
                    }

                    globalRowIdx++;

                    if (childrenCount > 0 && expandedRows.Exists(index))
                    {
                        visitor(index, indentLevel + 1);
                    }
                }
            };

        visitor(FModelIndex(), 0);

        while (childIndex < children.GetCount())
        {
            children[childIndex]->Enabled(false);

            childIndex++;
        }

        treeView->ApplyStyle();
        MarkDirty();
    }

    FTreeViewContainer::Self& FTreeViewContainer::NormalizedScrollY(f32 value)
    {
        value = Math::Clamp01(value);

        Vec2 translation = Translation();
        translation.y = -value * (totalRowHeight - computedSize.y);
        if (totalRowHeight < computedSize.y)
        {
            translation.y = 0;
        }
        Translation(translation);
        return *this;
    }

    f32 FTreeViewContainer::NormalizedScrollY()
    {
        if (totalRowHeight < computedSize.y - 1)
            return 0;
        return -Translation().y / (totalRowHeight - computedSize.y);
    }

    void FTreeViewContainer::CalculateIntrinsicSize()
    {
        ZoneScoped;

        if (treeView == nullptr || treeView->m_Model == nullptr || !treeView->m_Model->IsReady())
        {
            Super::CalculateIntrinsicSize();
            return;
        }

        intrinsicSize.width = m_Padding.left + m_Padding.right;
        intrinsicSize.height = m_Padding.top + m_Padding.bottom;

        Vec2 contentSize = Vec2();

        auto model = treeView->m_Model;

        totalRowHeight = 0;

        Delegate<void(const FModelIndex&)> visitor = [&](const FModelIndex& parent) -> void
            {
                int rowCount = model->GetRowCount(parent);
                if (rowCount == 0)
                    return;

                for (int i = 0; i < rowCount; ++i)
                {
                    FModelIndex index = model->GetIndex(i, 0, parent);

                    if (!treeView->m_RowHeightDelegate.IsValid())
                    {
	                    totalRowHeight += treeView->m_RowHeight;
                    }
                    else
                    {
                        totalRowHeight += treeView->m_RowHeightDelegate(index);
                    }

                    if (expandedRows.Exists(index))
                    {
                        visitor(index);
                    }
                }
            };

        visitor(FModelIndex());
        if (treeView->AutoHeight())
        {
            contentSize.height = totalRowHeight;
        }

        for (int i = 0; i < children.GetCount(); ++i)
        {
            if (!children[i]->Enabled())
                continue;

            children[i]->CalculateIntrinsicSize();
            contentSize.width = Math::Max(contentSize.width, children[i]->GetIntrinsicSize().width);

            if (!treeView->AutoHeight())
            {
                f32 rowHeight = 0;
                if (!treeView->m_RowHeightDelegate.IsValid())
                {
                    rowHeight = treeView->m_RowHeight;
                }
                else
                {
                    rowHeight = treeView->m_RowHeightDelegate(children[i]->index);
                }

                contentSize.height += Math::Max(rowHeight, children[i]->GetIntrinsicSize().height);
            }
        }

        if (totalRowHeight >= computedSize.y)
        {
            Padding(Vec4(0, 0, treeView->m_ScrollBarWidth + treeView->m_ScrollBarMargin, 0));
        }
        else
        {
            Padding(Vec4(0, 0, 0, 0));
        }

        intrinsicSize.width += contentSize.width;
        intrinsicSize.height += contentSize.height;

        ApplyIntrinsicSizeConstraints();
    }

    void FTreeViewContainer::PlaceSubWidgets()
    {
        ZoneScoped;

        Super::PlaceSubWidgets();

        if (children.IsEmpty())
        {
            return;
        }

        f32 scrollY = -Translation().y;

        f32 remainingSize = 0;
        Vec2 curPos = Vec2(m_Padding.left, m_Padding.top);
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
            m_Padding.top + m_Padding.bottom);

        for (int i = 0; i < children.GetCount(); ++i)
        {
            auto child = children[i];

	        if (!child->Enabled())
                continue;

            f32 rowHeight = 0;
            if (!treeView->m_RowHeightDelegate.IsValid())
            {
                rowHeight = treeView->m_RowHeight;
            }
            else
            {
                rowHeight = treeView->m_RowHeightDelegate(child->index);
            }

            Vec2 childPos = curPos + Vec2(child->Margin().left, child->Margin().top);

            child->SetComputedPosition(childPos);
            child->SetComputedSize(Vec2(availableSize.x, Math::Max(rowHeight, child->GetIntrinsicSize().height)));

        	child->PlaceSubWidgets();

            curPos.y += child->computedSize.y;
        }
    }
    
}

