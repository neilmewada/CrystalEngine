#include "FusionCore.h"

namespace CE
{

    FStackBox::FStackBox()
    {
        m_HAlign = HAlign::Fill;
        m_VAlign = VAlign::Fill;
    }

    void FStackBox::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        Super::SetParentSurfaceRecursive(surface);

        for (int i = 0; i < GetChildCount(); i++)
        {
            GetChildAt(i)->SetParentSurfaceRecursive(surface);
        }
    }

    Vec2 FStackBox::MeasureContent(Vec2 availableSize)
    {
        ZoneScoped;

        Vec2 contentAvailable = Vec2(
            Math::Max(0.0f, availableSize.x - m_Padding.left - m_Padding.right),
            Math::Max(0.0f, availableSize.y - m_Padding.top  - m_Padding.bottom)
        );

        f32 totalMainAxis = 0.0f;
        f32 maxCrossAxis  = 0.0f;
        int enabledCount  = 0;

        for (int i = 0; i < GetChildCount(); i++)
        {
            Ref<FWidget> child = GetChildAt(i);
            if (!child->Enabled())
                continue;

            FMargin childMargin = child->Margin();

            Vec2 childAvailable = Vec2(
                Math::Max(0.0f, contentAvailable.x - childMargin.left - childMargin.right),
                Math::Max(0.0f, contentAvailable.y - childMargin.top  - childMargin.bottom)
            );

            Vec2 childDesired = child->MeasureContent(childAvailable);

            if (m_StackDirection == FStackDirection::Horizontal)
            {
                totalMainAxis += childDesired.x + childMargin.left + childMargin.right;
                maxCrossAxis = Math::Max(maxCrossAxis, childDesired.y + childMargin.top + childMargin.bottom);
            }
            else
            {
                totalMainAxis += childDesired.y + childMargin.top + childMargin.bottom;
                maxCrossAxis = Math::Max(maxCrossAxis, childDesired.x + childMargin.left + childMargin.right);
            }

            enabledCount++;
        }

        if (enabledCount > 1)
        {
	        totalMainAxis += (enabledCount - 1) * m_Spacing;
        }

        Vec2 desired;
        if (m_StackDirection == FStackDirection::Horizontal)
        {
            desired = Vec2(
                totalMainAxis + m_Padding.left + m_Padding.right,
                maxCrossAxis  + m_Padding.top  + m_Padding.bottom
            );
        }
        else
        {
            desired = Vec2(
                maxCrossAxis  + m_Padding.left + m_Padding.right,
                totalMainAxis + m_Padding.top  + m_Padding.bottom
            );
        }

        return desiredSize = ApplyLayoutConstraints(desired);
    }

    void FStackBox::ArrangeContent(Vec2 finalSize)
    {
        ZoneScoped;

        Super::ArrangeContent(finalSize);

        bool isHorizontal = (m_StackDirection == FStackDirection::Horizontal);

        f32 contentWidth  = Math::Max(0.0f, layoutSize.x - m_Padding.left - m_Padding.right);
        f32 contentHeight = Math::Max(0.0f, layoutSize.y - m_Padding.top  - m_Padding.bottom);
        f32 contentMain   = isHorizontal ? contentWidth  : contentHeight;
        f32 contentCross  = isHorizontal ? contentHeight : contentWidth;

        // ── Pass 1: fixed main-axis budget + fill-child collection ───────────
        int enabledCount = 0;
        f32 fixedMain    = 0.0f;

        struct FillEntry
        {
            int  childIdx;
            f32  ratio;
            f32  allocatedMain; // resolved main-axis size (post-clamping)
            bool frozen;        // true once clamped by MaxWidth/MaxHeight
        };

        // Collect fill children in iteration order (indices into children array)
        Array<FillEntry> fillEntries;

        for (int i = 0; i < GetChildCount(); i++)
        {
            Ref<FWidget> child = GetChildAt(i);
            if (!child->Enabled()) continue;

            FMargin m      = child->Margin();
            f32 fillRatio  = child->FillRatio();
            f32 mainMargin = isHorizontal ? (m.left + m.right) : (m.top + m.bottom);

            if (fillRatio > 0.0f)
            {
                fillEntries.Add({ i, fillRatio, 0.0f, false });
                fixedMain += mainMargin; // only margin is fixed; size comes from the pool
            }
            else
            {
                Vec2 d = child->GetDesiredSize();
                fixedMain += isHorizontal ? (d.x + mainMargin) : (d.y + mainMargin);
            }

            enabledCount++;
        }

        if (enabledCount > 1)
            fixedMain += (enabledCount - 1) * m_Spacing;

        // ── Iterative fill distribution (CSS flex-grow semantics) ────────────
        // Each iteration freezes any fill child whose proportional share exceeds
        // its MaxWidth/MaxHeight, returns the excess to the pool, and repeats
        // until no new child is frozen.  This mirrors the CSS flexbox algorithm.
        f32 availableForFill = Math::Max(0.0f, contentMain - fixedMain);

        bool anyFrozenThisRound = true;
        while (anyFrozenThisRound && fillEntries.GetSize() > 0)
        {
            anyFrozenThisRound = false;

            f32 sumRatios = 0.0f;
            for (int e = 0; e < fillEntries.GetSize(); e++)
                if (!fillEntries[e].frozen) sumRatios += fillEntries[e].ratio;

            if (sumRatios <= 0.0f) break;

            for (int e = 0; e < fillEntries.GetSize(); e++)
            {
                FillEntry& entry = fillEntries[e];
                if (entry.frozen) continue;

                f32 allocated = availableForFill * (entry.ratio / sumRatios);

                // Check whether this child's constraints clamp the main axis.
                // We probe with the full contentCross for the cross dimension;
                // only the main-axis result of ApplyLayoutConstraints matters here.
                Ref<FWidget> child = GetChildAt(entry.childIdx);
                Vec2 probe   = isHorizontal ? Vec2(allocated, contentCross)
                                            : Vec2(contentCross, allocated);
                Vec2 clamped = child->ApplyLayoutConstraints(probe);
                f32  clampedMain = isHorizontal ? clamped.x : clamped.y;

                if (clampedMain < allocated - 0.001f) // clamped — freeze at max
                {
                    entry.allocatedMain = clampedMain;
                    entry.frozen        = true;
                    availableForFill    = Math::Max(0.0f, availableForFill - clampedMain);
                    anyFrozenThisRound  = true;
                }
            }
        }

        // Distribute whatever is left to the still-unfrozen fill children
        {
            f32 sumRatios = 0.0f;
            for (int e = 0; e < fillEntries.GetSize(); e++)
                if (!fillEntries[e].frozen) sumRatios += fillEntries[e].ratio;

            for (int e = 0; e < fillEntries.GetSize(); e++)
            {
                FillEntry& entry = fillEntries[e];
                if (!entry.frozen)
                    entry.allocatedMain = (sumRatios > 0.0f)
                        ? Math::Max(0.0f, availableForFill * (entry.ratio / sumRatios))
                        : 0.0f;
            }
        }

        // ── Pass 2: position and arrange each child ──────────────────────────
        f32  cursor   = isHorizontal ? m_Padding.left : m_Padding.top;
        bool isFirst  = true;
        int  fillSlot = 0; // walking index into fillEntries (same order as children)

        for (int i = 0; i < GetChildCount(); i++)
        {
            Ref<FWidget> child = GetChildAt(i);
            if (!child->Enabled()) continue;

            if (!isFirst) cursor += m_Spacing;
            isFirst = false;

            FMargin childMargin  = child->Margin();
            Vec2    childDesired = child->GetDesiredSize();
            f32     fillRatio    = child->FillRatio();

            // ── Main axis ────────────────────────────────────────────────────
            f32 mainMarginStart = isHorizontal ? childMargin.left  : childMargin.top;
            f32 mainMarginEnd   = isHorizontal ? childMargin.right : childMargin.bottom;

            f32 childMainSize;
            if (fillRatio > 0.0f)
            {
                // Advance the fill slot to match this child index
                while (fillSlot < fillEntries.GetSize() && fillEntries[fillSlot].childIdx < i)
                    fillSlot++;
                childMainSize = (fillSlot < fillEntries.GetSize() && fillEntries[fillSlot].childIdx == i)
                    ? fillEntries[fillSlot].allocatedMain : 0.0f;
            }
            else
            {
                childMainSize = isHorizontal ? childDesired.x : childDesired.y;
            }

            f32 childMainPos = cursor + mainMarginStart;

            // ── Cross axis ───────────────────────────────────────────────────
            f32 crossMarginStart  = isHorizontal ? childMargin.top    : childMargin.left;
            f32 crossMarginEnd    = isHorizontal ? childMargin.bottom : childMargin.right;
            f32 crossDesired      = isHorizontal ? childDesired.y     : childDesired.x;
            f32 crossAreaSize     = Math::Max(0.0f, contentCross - crossMarginStart - crossMarginEnd);
            f32 crossPaddingStart = isHorizontal ? m_Padding.top : m_Padding.left;

            f32 childCrossSize;
            f32 childCrossPos;

            if (isHorizontal)
            {
                CE::VAlign vAlign = child->VAlign();
                if (vAlign == VAlign::Auto) vAlign = m_ContentVAlign;

                switch (vAlign)
                {
                default:
                case VAlign::Auto:
                case VAlign::Fill:
                    childCrossSize = crossAreaSize;
                    childCrossPos  = crossPaddingStart + crossMarginStart;
                    break;
                case VAlign::Top:
                    childCrossSize = Math::Min(crossDesired, crossAreaSize);
                    childCrossPos  = crossPaddingStart + crossMarginStart;
                    break;
                case VAlign::Center:
                    childCrossSize = Math::Min(crossDesired, crossAreaSize);
                    childCrossPos  = crossPaddingStart + crossMarginStart + (crossAreaSize - childCrossSize) / 2.0f;
                    break;
                case VAlign::Bottom:
                    childCrossSize = Math::Min(crossDesired, crossAreaSize);
                    childCrossPos  = crossPaddingStart + crossMarginStart + (crossAreaSize - childCrossSize);
                    break;
                }
            }
            else
            {
                CE::HAlign hAlign = child->HAlign();
                if (hAlign == HAlign::Auto) hAlign = m_ContentHAlign;

                switch (hAlign)
                {
                default:
                case HAlign::Auto:
                case HAlign::Fill:
                    childCrossSize = crossAreaSize;
                    childCrossPos  = crossPaddingStart + crossMarginStart;
                    break;
                case HAlign::Left:
                    childCrossSize = Math::Min(crossDesired, crossAreaSize);
                    childCrossPos  = crossPaddingStart + crossMarginStart;
                    break;
                case HAlign::Center:
                    childCrossSize = Math::Min(crossDesired, crossAreaSize);
                    childCrossPos  = crossPaddingStart + crossMarginStart + (crossAreaSize - childCrossSize) / 2.0f;
                    break;
                case HAlign::Right:
                    childCrossSize = Math::Min(crossDesired, crossAreaSize);
                    childCrossPos  = crossPaddingStart + crossMarginStart + (crossAreaSize - childCrossSize);
                    break;
                }
            }

            // Arrange and advance the cursor using the child's actual layout size
            // (post-constraint) rather than our pre-computed estimate.
            if (isHorizontal)
            {
                child->SetLayoutPosition(Vec2(childMainPos, childCrossPos));
                child->ArrangeContent(Vec2(childMainSize, childCrossSize));
                cursor += mainMarginStart + child->GetLayoutSize().x + mainMarginEnd;
            }
            else
            {
                child->SetLayoutPosition(Vec2(childCrossPos, childMainPos));
                child->ArrangeContent(Vec2(childCrossSize, childMainSize));
                cursor += mainMarginStart + child->GetLayoutSize().y + mainMarginEnd;
            }
        }
    }

    void FVerticalStack::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

		thread_local const CE::Name stackDirectionName = "StackDirection";

		if (propertyName == stackDirectionName)
        {
			m_StackDirection = FStackDirection::Vertical;
        }
    }

    void FHorizontalStack::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        thread_local const CE::Name stackDirectionName = "StackDirection";

        if (propertyName == stackDirectionName)
        {
            m_StackDirection = FStackDirection::Horizontal;
        }
    }

}

