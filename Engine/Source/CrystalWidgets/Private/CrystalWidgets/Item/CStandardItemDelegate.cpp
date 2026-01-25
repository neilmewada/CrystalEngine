#include "CrystalWidgets.h"

namespace CE::Widgets
{

    CStandardItemDelegate::CStandardItemDelegate()
    {
	    
    }

    CStandardItemDelegate::~CStandardItemDelegate()
    {
	    
    }

    void CStandardItemDelegate::Paint(CPainter* painter, const CViewItemStyle& itemStyle, const CModelIndex& index)
    {
        if (!index.IsValid())
            return;

        Vec2 size = Vec2();

        CBaseItemModel* model = index.GetModel();

        if (!EnumHasAnyFlags(itemStyle.features, CViewItemFeature::HasDecoration | CViewItemFeature::HasDisplay))
            return;

        if (index.GetColumn() == itemStyle.expandableColumn)
        {
            size.x += 15.0f; // Expand/collapse arrow
        }

        size.x += itemStyle.padding.left;

        if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDecoration))
        {
            size.x += itemStyle.decorationRect.min.x;

            if (itemStyle.icon.IsValid())
            {
                RPI::Texture* icon = CApplication::Get()->LoadImage(itemStyle.icon);
                if (icon)
                {
                    painter->SetBrush(CBrush(Colors::White));

                    painter->DrawTexture(Rect::FromSize(size.x + itemStyle.decorationRect.min.x,
                        itemStyle.padding.top + itemStyle.decorationRect.min.y, 
                        itemStyle.decorationRect.GetSize().width,
                        itemStyle.decorationRect.GetSize().height), icon);
                }
            }

            size.x += itemStyle.decorationRect.GetSize().width;

            if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDisplay))
            {
                size.x += 5.0f; // padding between decoration & text
            }
        }

        size.y = 10.0f;

        if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDisplay))
        {
            String text = "";
            Variant value = model->GetData(index, CItemDataUsage::Display);
            if (value.IsOfType<String>())
            {
                text = value.GetValue<String>();
            }
            else if (value.IsOfType<Name>())
            {
                text = value.GetValue<Name>().GetString();
            }

            CPen pen = CPen(itemStyle.textColor);
            painter->SetPen(pen);

            painter->SetFont(itemStyle.font);

            if (text.NonEmpty())
            {
                painter->DrawText(text, Vec2(size.x, itemStyle.padding.top));
            }
        }
    }

    Vec2 CStandardItemDelegate::GetSizeHint(const CViewItemStyle& itemStyle, const CModelIndex& index)
    {
        if (!index.IsValid())
            return Vec2();

        Vec2 size = Vec2();

        CBaseItemModel* model = index.GetModel();

        CModelIndex parent = index.GetParent();

        if (!EnumHasAnyFlags(itemStyle.features, CViewItemFeature::HasDecoration | CViewItemFeature::HasDisplay))
            return Vec2();

        if (index.GetColumn() == itemStyle.expandableColumn)
        {
            size.x += 15.0f; // Expand/collapse arrow
        }

        if (model->GetRowCount(index) > 0) // Has children
        {
            
        }

        size.x += itemStyle.padding.left;

        if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDecoration) && model->GetData(index, CItemDataUsage::Decoration).HasValue())
        {
            size.x += itemStyle.decorationRect.min.x;
            size.x += itemStyle.decorationRect.GetSize().width;

            if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDisplay))
            {
                size.x += 5.0f; // padding between decoration & text
            }
        }

        size.y = 10.0f;

        if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDisplay))
        {
            String text = "";
            Variant value = model->GetData(index, CItemDataUsage::Display);
            if (value.IsOfType<String>())
            {
                text = value.GetValue<String>();
            }
            else if (value.IsOfType<Name>())
            {
                text = value.GetValue<Name>().GetString();
            }

            if (text.NonEmpty())
            {
                Vec2 textSize = CApplication::Get()->CalculateTextSize(text, itemStyle.font.GetSize(), itemStyle.font.GetFamily());
                size.x += textSize.width;
                size.y = Math::Max(size.y, textSize.height);
            }
        }

        size.x += itemStyle.padding.right;

        size.y += itemStyle.padding.top + itemStyle.padding.bottom;

        return size;
    }

    void CStandardItemDelegate::PaintHeader(CPainter* painter, Vec2 size, const CViewItemStyle& itemStyle,
                                            int position, COrientation orientation, CBaseItemModel* model)
    {
        if (itemStyle.bgColor.a > 0)
        {
            CPen pen = CPen();
            CBrush brush = CBrush(itemStyle.bgColor);

            painter->SetPen(pen);
            painter->SetBrush(brush);

            painter->DrawRect(Rect::FromSize(Vec2(), size));
        }

        if (!itemStyle.text.IsEmpty())
        {
            CPen pen = CPen(itemStyle.textColor);

            painter->SetPen(pen);
            painter->SetFont(itemStyle.font);

            painter->DrawText(itemStyle.text, itemStyle.padding.min);
        }
    }

    Vec2 CStandardItemDelegate::GetHeaderSizeHint(const CViewItemStyle& itemStyle, int position, 
        COrientation orientation, CBaseItemModel* model)
    {
        Vec2 size = Vec2();

        if (!EnumHasAnyFlags(itemStyle.features, CViewItemFeature::HasDecoration | CViewItemFeature::HasDisplay))
            return Vec2();

        size.x += itemStyle.padding.left;

        if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDecoration) && 
            model->GetHeaderData(position, orientation, CItemDataUsage::Decoration).HasValue())
        {
            size.x += itemStyle.decorationRect.min.x;
            size.x += itemStyle.decorationRect.GetSize().width;

            if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDisplay))
            {
                size.x += 2.5f; // padding between decoration & text
            }
        }

        size.y = 10.0f;

        if (EnumHasFlag(itemStyle.features, CViewItemFeature::HasDisplay))
        {
            String text = "";
            Variant value = model->GetHeaderData(position, orientation, CItemDataUsage::Display);
            if (value.IsOfType<String>())
            {
                text = value.GetValue<String>();
            }
            else if (value.IsOfType<Name>())
            {
                text = value.GetValue<Name>().GetString();
            }

            if (text.NonEmpty())
            {
                Vec2 textSize = CApplication::Get()->CalculateTextSize(text, itemStyle.font.GetSize(), itemStyle.font.GetFamily());
                size.x += textSize.width;
                size.y = Math::Max(size.y, textSize.height);
            }
        }

        size.x += itemStyle.padding.right;

        size.y += itemStyle.padding.top + itemStyle.padding.bottom;

        return size;
    }

} // namespace CE::Widgets
