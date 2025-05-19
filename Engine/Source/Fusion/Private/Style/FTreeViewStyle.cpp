#include "Fusion.h"

namespace CE
{

    FTreeViewStyle::FTreeViewStyle()
    {
        separatorWidth = 1.5f;
    }

    FTreeViewStyle::~FTreeViewStyle()
    {
        
    }

    SubClass<FWidget> FTreeViewStyle::GetWidgetClass() const
    {
        return FTreeView::StaticType();
    }

    void FTreeViewStyle::MakeStyle(FWidget& widget)
    {
        FTreeView& treeView = widget.As<FTreeView>();

        treeView
			.Background(background)
            ;

        if (treeView.GetHeaderContainer() != nullptr)
        {
	        FStyledWidget& header = *treeView.GetHeaderContainer();

        	header
				.Background(headerBackground)
			;
        }

        if (treeView.header)
        {
            FTreeViewHeader& header = *treeView.header;

            for (FStyledWidget* separator : header.separators)
            {
                (*separator)
					.Background(headerSeparator)
					.Width(separatorWidth)
                    ;
            }
        }

        if (treeView.container && treeView.SelectionModel() != nullptr)
        {
            for (FTreeViewRow* child : treeView.container->children)
            {
                if (!child->Enabled())
                    continue;

                FBrush rowBg = rowBackground;
                if (child->IsAlternate())
                    rowBg = rowAlternateBackground;

                if (treeView.SelectionModel()->IsSelected(child->GetIndex()))
                {
	                rowBg = rowSelectionBackground;
                }
                else if (child->IsHovered())
                {
                    rowBg = rowHoverBackground;
                }

                (*child)
                    .Background(rowBg)
                    ;
            }
        }
    }

} // namespace CE

