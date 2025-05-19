#include "Fusion.h"

namespace CE
{

    FTreeView::FTreeView()
    {
        m_SelectionModel = CreateDefaultSubobject<FItemSelectionModel>("SelectionModel");
        m_SelectionModel->SetSelectionMode(FSelectionMode::Single);

        m_ScrollBarWidth = 8;
        m_ScrollBarShape = FRoundedRectangle(4);
        m_ScrollBarMargin = 2.5f;

        m_VerticalScrollSensitivity = 15.0f;

        m_ScrollBarBackground = Color::RGBA(50, 50, 50);
        m_ScrollBarBrush = Color::RGBA(130, 130, 130);
        m_ScrollBarHoverBrush = Color::RGBA(163, 163, 163);

        m_AutoHeight = false;
        m_MinHeight = 30;
        m_Indentation = 10;
        m_RowHeight = 25;
    }

    void FTreeView::Construct()
    {
        Super::Construct();

        Child(
            FNew(FVerticalStack)
            .VAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            (
                FAssignNew(FStyledWidget, headerContainer)
                .HAlign(HAlign::Fill),

                FAssignNew(FStyledWidget, containerStyle)
                .HAlign(HAlign::Fill)
                .FillRatio(1.0f)
                (
                    FAssignNew(FScrollBox, scrollBox)
                    .VerticalScroll(true)
                    .HorizontalScroll(false)
                    .Child(
                        FAssignNew(FTreeViewContainer, container)
                        .TreeView(this)
                        .HAlign(HAlign::Left)
                        .VAlign(VAlign::Top)
                    )
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Fill)
                )
            )
        );
    }

    void FTreeView::ExpandRow(const FModelIndex& index, bool recursive)
    {
        container->ExpandRow(index, recursive);
    }

    void FTreeView::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        static const CE::Name model = "Model";
        static const CE::Name indentation = "Indentation";

        if (propertyName == model)
        {
            MarkLayoutDirty();
        }
    }

    FTreeView::Self& FTreeView::Header(FTreeViewHeader& header)
    {
        this->header = &header;
        header.treeView = this;
        
        header
			.HAlign(HAlign::Fill)
			.VAlign(VAlign::Fill)
    	;

        headerContainer->Child(header);

        ApplyStyle();
        return *this;
    }

    void FTreeView::OnModelUpdate()
    {
        container->OnModelUpdate();
    }

    void FTreeView::SelectRow(const FModelIndex& index)
    {
        m_SelectionModel->Select(index);

        ApplyStyle();
    }

}

