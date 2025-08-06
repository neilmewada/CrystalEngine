#include "CrystalWidgets.h"

namespace CE::Widgets
{
	constexpr f32 DefaultDecorationSize = 16.0f;

	constexpr f32 ScrollRectWidth = 10.0f;
	constexpr f32 MinScrollRectSize = 20.0f;
	constexpr f32 ScrollSizeBuffer = 1.0f;

	CBaseItemView::CBaseItemView()
	{
		receiveMouseEvents = true;
		receiveDragEvents = true;
		receiveKeyEvents = true;

		defaultSelectionModel = CreateDefaultSubobject<CItemSelectionModel>("ItemSelectionModel");
		delegate = CreateDefaultSubobject<CStandardItemDelegate>("StandardItemDelegate");

		allowVerticalScroll = true;
		allowHorizontalScroll = false;

		if (!IsDefaultInstance())
		{
			SetSelectionModel(defaultSelectionModel);
		}
	}

	CBaseItemView::~CBaseItemView()
	{
		
	}

	Vec2 CBaseItemView::CalculateIntrinsicSize(f32 width, f32 height)
	{
		return Vec2(); // Should never rely on intrinsic size
	}

	void CBaseItemView::SetSelectionModel(CItemSelectionModel* selectionModel)
	{
		if (this->selectionModel == selectionModel)
			return;

		auto functionBinding = FUNCTION_BINDING(this, OnSelectionModelUpdated);

		if (this->selectionModel != nullptr)
		{
			selectionModel->onSelectionChanged.Unbind(functionBinding);
		}

		this->selectionModel = selectionModel;

		selectionModel->onSelectionChanged.Bind(functionBinding);
	}

	void CBaseItemView::SetModel(CBaseItemModel* model)
	{
		if (this->model != nullptr)
		{
			this->model->onModelDataUpdated.UnbindAll(this);
		}

		this->model = model;

		if (selectionModel)
			selectionModel->SetModel(model);

		if (model != nullptr)
		{
			u32 numColumns = model->GetColumnCount({});

			if (columnWidthRatios.GetSize() != numColumns)
			{
				columnWidthRatios.Resize(numColumns);

				for (int i = 0; i < numColumns; ++i)
				{
					columnWidthRatios[i] = 0.0f;
				}
			}

			model->onModelDataUpdated.Bind(FUNCTION_BINDING(this, SetNeedsLayout));
			model->onModelDataUpdated.Bind(FUNCTION_BINDING(this, SetNeedsPaint));
		}
	}

	void CBaseItemView::SetDelegate(CBaseItemDelegate* delegate)
	{
		this->delegate = delegate;
	}

	void CBaseItemView::Construct()
	{
		Super::Construct();

		
	}

	void CBaseItemView::OnPaint(CPaintEvent* paintEvent)
	{
		Super::OnPaint(paintEvent);

		if (!model)
			return;

		CPainter* painter = paintEvent->painter;

		u32 numColumns = model->GetColumnCount({});
		u32 numRows = model->GetRowCount({});

		if (numColumns == 0)
			return;

		painter->PushChildCoordinateSpace(GetComputedLayoutTopLeft());
		Vec2 regionSize = GetComputedLayoutSize();
		painter->PushClipRect(Rect::FromSize(Vec2(), regionSize));

		bool hasColumnHeader = true;

		for (int i = 0; i < numColumns; ++i)
		{
			Variant headerData = model->GetHeaderData(i, COrientation::Horizontal);
			if (!headerData.HasValue())
			{
				hasColumnHeader = false;
				break;
			}
		}

		bool hasRowHeader = true;

		for (int i = 0; i < numRows; ++i)
		{
			Variant headerData = model->GetHeaderData(i, COrientation::Vertical);
			if (!headerData.HasValue())
			{
				hasRowHeader = false;
				break;
			}
		}

		if (!canDrawColumnHeader)
			hasColumnHeader = false;
		if (!canDrawRowHeader)
			hasRowHeader = false;

		CStyle headerStyle = styleSheet->SelectStyle(this, CStateFlag::Default, CSubControl::Header);
		CStyle headerHoveredStyle = styleSheet->SelectStyle(this, CStateFlag::Hovered, CSubControl::Header);
		CStyle alternateStyle = styleSheet->SelectStyle(this, CStateFlag::Default, CSubControl::Alternate);
		alternateBgColor = alternateStyle.properties[CStylePropertyType::Background].color;

		if (rowHeightsByParent[{}].GetSize() != numRows)
			recalculateRows = true;

		columnWidths.Resize(numColumns);
		columnWidthRatios.Reserve(numColumns);
		rowHeightsByParent[{}].Reserve(numRows);
		columnHeaderHeight = 0;

		f32 remainingColumnWidth = regionSize.width;

		for (int col = 0; col < numColumns; ++col)
		{
			if (EnumHasFlag(columnHeaderFlags[col], CItemViewHeaderFlags::Resizable))
			{
				columnWidths[col] = 0;
			}
			else
			{
				remainingColumnWidth -= columnWidths[col];
			}

			if (hasColumnHeader)
			{
				CViewItemStyle headerViewStyle{};
				Variant headerText = model->GetHeaderData(col, COrientation::Horizontal, CItemDataUsage::Display);
				if (headerText.IsTextType())
				{
					headerViewStyle.text = headerText.GetTextValue();
				}

				if (hoveredColumnHeader == col) // Hovered column
				{
					headerViewStyle.padding = headerHoveredStyle.properties[CStylePropertyType::Padding].vector;
					headerViewStyle.font.SetFamily(headerHoveredStyle.GetFontName());
					headerViewStyle.font.SetSize(headerHoveredStyle.GetFontSize());
					headerViewStyle.features = CViewItemFeature::HasDisplay;

					Variant headerDisplay = model->GetHeaderData(col, COrientation::Horizontal);
					Variant headerIcon = model->GetHeaderData(col, COrientation::Horizontal, CItemDataUsage::Decoration);

					if (headerIcon.HasValue() && headerIcon.IsTextType())
					{
						headerViewStyle.features |= CViewItemFeature::HasDecoration;
						headerViewStyle.icon = headerIcon.GetNameValue();
						headerViewStyle.decorationRect = Rect::FromSize(0, 0, DefaultDecorationSize, DefaultDecorationSize);
					}

					if (headerDisplay.IsTextType())
					{
						headerViewStyle.text = headerDisplay.GetTextValue();
					}
				}
				else
				{
					headerViewStyle.padding = headerStyle.properties[CStylePropertyType::Padding].vector;
					headerViewStyle.font.SetFamily(headerStyle.GetFontName());
					headerViewStyle.font.SetSize(headerStyle.GetFontSize());
					headerViewStyle.features = CViewItemFeature::HasDisplay;

					Variant headerDisplay = model->GetHeaderData(col, COrientation::Horizontal);
					Variant headerIcon = model->GetHeaderData(col, COrientation::Horizontal, CItemDataUsage::Decoration);

					if (headerIcon.HasValue() && headerIcon.IsTextType())
					{
						headerViewStyle.features |= CViewItemFeature::HasDecoration;
						headerViewStyle.icon = headerIcon.GetNameValue();
						headerViewStyle.decorationRect = Rect::FromSize(0, 0, DefaultDecorationSize, DefaultDecorationSize);
					}

					if (headerDisplay.IsTextType())
					{
						headerViewStyle.text = headerDisplay.GetTextValue();
					}
				}
				
				Vec2 headerSize = delegate->GetHeaderSizeHint(headerViewStyle, col, COrientation::Horizontal, model);

				columnHeaderHeight = Math::Max(columnHeaderHeight, headerSize.height);
			}
		}

		if (recalculateRows)
		{
			cellStyle = styleSheet->SelectStyle(this, CStateFlag::Default, CSubControl::Cell);
			cellHoveredStyle = styleSheet->SelectStyle(this, CStateFlag::Hovered, CSubControl::Cell);
			cellSelectedStyle = styleSheet->SelectStyle(this, CStateFlag::Active | CStateFlag::Focused, CSubControl::Cell);
			cellSelectedUnfocusedStyle = styleSheet->SelectStyle(this, CStateFlag::Active, CSubControl::Cell);
			totalContentHeight = 0.0f;
			expandableColumn = -1;

			CalculateRowHeights(CModelIndex());

			contentSize.height = totalContentHeight;
			contentSize.width = regionSize.width;

			recalculateRows = false;
		}
		else
		{
			contentSize.height = totalContentHeight;
			contentSize.width = regionSize.width;
		}

		for (int col = 0; col < numColumns; ++col)
		{
			if (EnumHasFlag(columnHeaderFlags[col], CItemViewHeaderFlags::Resizable))
			{
				columnWidths[col] = columnWidthRatios[col] * remainingColumnWidth;
			}
		}

		// - Paint rows & cells -

		scrollOffset.y = Math::Clamp(scrollOffset.y, 0.0f, totalContentHeight - regionSize.height - columnHeaderHeight);
		if (totalContentHeight <= regionSize.height - columnHeaderHeight)
		{
			scrollOffset.y = 0.0f;
		}

		painter->PushChildCoordinateSpace(Vec2(0, columnHeaderHeight - scrollOffset.y));
		painter->PushClipRect(Rect::FromSize(0, 0, regionSize.width, contentSize.height));

		PaintRows(painter, Rect::FromSize(0, 0, regionSize.width, regionSize.height - columnHeaderHeight), 0);

		painter->PopClipRect();
		painter->PopChildCoordinateSpace();

		Vec2 headerOrigin = Vec2();

		// - Paint headers -

		for (int col = 0; hasColumnHeader && col < numColumns; ++col)
		{
			CViewItemStyle headerViewStyle{};
			Variant headerText = model->GetHeaderData(col, COrientation::Horizontal, CItemDataUsage::Display);
			if (headerText.IsTextType())
			{
				headerViewStyle.text = headerText.GetTextValue();
			}

			if (hoveredColumnHeader == col) // Hovered column
			{
				headerViewStyle.padding = headerHoveredStyle.properties[CStylePropertyType::Padding].vector;
				headerViewStyle.font.SetFamily(headerHoveredStyle.GetFontName());
				headerViewStyle.font.SetSize(headerHoveredStyle.GetFontSize());
				headerViewStyle.features = CViewItemFeature::HasDisplay;
				headerViewStyle.bgColor = headerHoveredStyle.properties[CStylePropertyType::Background].color;
				headerViewStyle.textColor = headerHoveredStyle.properties[CStylePropertyType::Foreground].color;

				Variant headerDisplay = model->GetHeaderData(col, COrientation::Horizontal);
				Variant headerIcon = model->GetHeaderData(col, COrientation::Horizontal, CItemDataUsage::Decoration);

				if (headerIcon.HasValue() && headerIcon.IsTextType())
				{
					headerViewStyle.features |= CViewItemFeature::HasDecoration;
					headerViewStyle.icon = headerIcon.GetNameValue();
					headerViewStyle.decorationRect = Rect::FromSize(0, 0, DefaultDecorationSize, DefaultDecorationSize);
				}

				if (headerDisplay.IsTextType())
				{
					headerViewStyle.text = headerDisplay.GetTextValue();
				}
			}
			else
			{
				headerViewStyle.padding = headerStyle.properties[CStylePropertyType::Padding].vector;
				headerViewStyle.font.SetFamily(headerStyle.GetFontName());
				headerViewStyle.font.SetSize(headerStyle.GetFontSize());
				headerViewStyle.features = CViewItemFeature::HasDisplay;
				headerViewStyle.bgColor = headerStyle.properties[CStylePropertyType::Background].color;
				headerViewStyle.textColor = headerStyle.properties[CStylePropertyType::Foreground].color;

				Variant headerDisplay = model->GetHeaderData(col, COrientation::Horizontal);
				Variant headerIcon = model->GetHeaderData(col, COrientation::Horizontal, CItemDataUsage::Decoration);

				if (headerIcon.HasValue() && headerIcon.IsTextType())
				{
					headerViewStyle.features |= CViewItemFeature::HasDecoration;
					headerViewStyle.icon = headerIcon.GetNameValue();
					headerViewStyle.decorationRect = Rect::FromSize(0, 0, DefaultDecorationSize, DefaultDecorationSize);
				}

				if (headerDisplay.IsTextType())
				{
					headerViewStyle.text = headerDisplay.GetTextValue();
				}
			}

			Vec2 headerSize = Vec2(columnWidths[col], columnHeaderHeight);
			
			painter->PushChildCoordinateSpace(headerOrigin);
			painter->PushClipRect(Rect::FromSize(Vec2(), headerSize));

			delegate->PaintHeader(painter, headerSize, headerViewStyle, col, COrientation::Horizontal, model);

			painter->PopClipRect();
			painter->PopChildCoordinateSpace();

			headerOrigin += Vec2(headerSize.width, 0);
		}

		// - Paint scroll bar -

		if (allowVerticalScroll) // Draw Vertical Scroll Bar
		{
			Vec2 originalSize = GetComputedLayoutSize();
			f32 originalHeight = originalSize.height;
			f32 contentMaxY = contentSize.height;

			//CE_LOG(Info, All, "Content: {} , {} | {}", contentMaxY, originalHeight, totalContentHeight);

			if (contentMaxY > originalHeight + ScrollSizeBuffer)
			{
				Rect scrollRect = GetVerticalScrollBarRect();

				CPen pen{};
				CBrush brush = CBrush(Color::RGBA(87, 87, 87));

				if (isVerticalScrollHovered || isVerticalScrollPressed)
				{
					brush.SetColor(Color::RGBA(128, 128, 128));
				}

				painter->SetPen(pen);
				painter->SetBrush(brush);

				painter->DrawRoundedRect(scrollRect, Vec4(1, 1, 1, 1) * ScrollRectWidth * 0.5f);
			}
			else
			{
				normalizedScroll = Vec2(0, 0);
			}
		}

		painter->PopClipRect();
		painter->PopChildCoordinateSpace();
	}

	void CBaseItemView::PaintRows(CPainter* painter, const Rect& regionRect, int indentLevel)
	{
		if (model == nullptr)
			return;

		f32 rowPosY = 0;
		bool mouseClickedInsideCell = false;
		int rowNumber = 0;

		PaintRowsInternal(painter, regionRect, indentLevel, rowPosY, mouseClickedInsideCell, rowNumber);

		if (isMouseLeftClick && !mouseClickedInsideCell)
		{
			selectionModel->Clear();
		}

		isMouseLeftClick = false;
	}

	void CBaseItemView::PaintRowsInternal(CPainter* painter, const Rect& regionRect, int indentLevel,
	                                      f32& rowPosY, bool& mouseClickedInsideCell,
	                                      int& curRowNumber, const CModelIndex& parentIndex)
	{
		u32 numRows = model->GetRowCount(parentIndex);
		u32 numColumns = model->GetColumnCount(parentIndex);
		if (numColumns == 0)
			return;

		CFont font{};

		font.SetFamily(cellStyle.GetFontName());
		if (!font.GetFamily().IsValid())
		{
			font.SetFamily(computedStyle.GetFontName());
		}

		font.SetSize(cellStyle.GetFontSize());

		for (int row = 0; row < numRows; ++row)
		{
			if (rowPosY - scrollOffset.y > regionRect.GetSize().height)
			{
				break; // Out of bounds
			}

			f32 cellHeight = ceil(rowHeightsByParent[parentIndex][row]);

			bool isClipped = (rowPosY + cellHeight - scrollOffset.y) < 0;

			// - Draw alternate background -

			if (curRowNumber % 2 != 0 && alternateBgColor.a > 0 && !isClipped)
			{
				CBrush brush = CBrush(alternateBgColor);
				painter->SetBrush(brush);
				painter->SetPen(CPen());

				painter->DrawRect(Rect::FromSize(0, rowPosY, regionRect.GetSize().width, cellHeight));
			}

			// - Draw selection background -

			bool isRowSelected = false;

			if (!isClipped && selectionModel != nullptr && selectionType == CItemSelectionType::SelectRow)
			{
				for (int col = 0; col < numColumns; ++col)
				{
					CModelIndex index = model->GetIndex(row, col, parentIndex);
					if (selectionModel->IsSelected(index))
					{
						CBrush brush = CBrush(cellSelectedStyle.GetBackgroundColor());
						if (!IsFocussed())
							brush.SetColor(cellSelectedUnfocusedStyle.GetBackgroundColor());
						painter->SetBrush(brush);
						painter->SetPen(CPen());

						painter->DrawRect(Rect::FromSize(0, rowPosY, regionRect.GetSize().width, cellHeight));
						isRowSelected = true;

						break;
					}
				}
			}

			// - Draw hover background -

			if (!isRowSelected && isMouseHovering && !isMouseLeftHeld && selectionMode != CItemSelectionMode::NoSelection)
			{
				f32 localPosX = 0;
				const Rect fullWidthRect = Rect::FromSize(0, rowPosY, regionRect.GetSize().width, rowHeightsByParent[parentIndex][row]);

				for (int col = 0; !isClipped && col < numColumns; ++col)
				{
					const Rect cellSelectionRect = Rect::FromSize(localPosX, rowPosY, columnWidths[col], rowHeightsByParent[parentIndex][row]);

					Vec2 rowMousePos = localMousePos - Vec2(0, columnHeaderHeight) + Vec2(0, scrollOffset.y);

					if (cellSelectionRect.Contains(rowMousePos))
					{
						painter->SetBrush(CBrush(cellHoveredStyle.GetBackgroundColor()));
						painter->SetPen(CPen());
						painter->DrawRect(fullWidthRect);
						break;
					}

					localPosX += columnWidths[col];
				}
			}

			f32 posX = 0.0f; // Add indentation level

			CModelIndex expandedIndex{};

			for (int col = 0; !isClipped && col < numColumns; ++col)
			{
				CModelIndex index = model->GetIndex(row, col, parentIndex);
				CViewItemStyle itemStyle{};
				itemStyle.font = font;

				Variant display = model->GetData(index, CItemDataUsage::Display);
				if (display.HasValue() && display.IsTextType())
				{
					itemStyle.features |= CViewItemFeature::HasDisplay;
					itemStyle.text = display.GetTextValue();
				}

				Variant icon = model->GetData(index, CItemDataUsage::Decoration);
				if (icon.HasValue() && icon.IsTextType())
				{
					itemStyle.features |= CViewItemFeature::HasDecoration;
					if (icon.HasValue() && icon.IsTextType())
						itemStyle.icon = icon.GetNameValue();
					itemStyle.decorationRect = Rect::FromSize(0, 0, DefaultDecorationSize, DefaultDecorationSize);
				}

				itemStyle.padding = cellStyle.GetPadding();
				itemStyle.textColor = computedStyle.GetForegroundColor();
				itemStyle.bgColor = Colors::Clear;
				itemStyle.expandableColumn = expandableColumn;

				f32 indentationX = 0.0f;
				
				if (parentIndex.IsValid() && expandedRows.Exists(parentIndex) && col == expandableColumn)
				{
					indentationX = indentLevel * 10.0f;
				}

				const Rect cellRect = Rect::FromSize(posX + indentationX, rowPosY, columnWidths[col], rowHeightsByParent[parentIndex][row]);
				const Rect cellSelectionRect = Rect::FromSize(posX, rowPosY, columnWidths[col], rowHeightsByParent[parentIndex][row]);

				itemStyle.cellSize = cellRect.GetSize();

				painter->PushChildCoordinateSpace(cellRect.min);
				painter->PushClipRect(Rect::FromSize(Vec2(0, 0), cellRect.GetSize()));

				bool mouseHoverOnArrow = false;

				// - Draw clickable expansion arrow icon

				if (col == expandableColumn && model->GetRowCount(index) > 0)
				{
					Color triangleColor = Color::RGBA(160, 160, 160);

					if (!expandedRows.Exists(index))
					{
						Vec2 triangleSize = Vec2(1, 0.9f) * cellHeight * 0.5f;
						Vec2 trianglePos = Vec2(20.0f + cellRect.min.x, triangleSize.height * 0.5f);
						Rect triangleRect = Rect::FromSize(trianglePos, triangleSize);
						Vec2 curOrigin = painter->GetCurrentOrigin();
						Rect windowTriangleRect = Rect::FromSize(trianglePos + curOrigin - Vec2(15, 0), triangleSize);
						
						if (isMouseHovering && windowTriangleRect.Contains(windowSpaceMousePos))
						{
							mouseHoverOnArrow = true;
							if (!isMouseLeftClick)
							{
								triangleColor = Color::RGBA(220, 220, 220);
							}
							else
							{
								expandedRows.Add(index);

								recalculateRows = true;
								SetNeedsLayout();
								SetNeedsPaint();

								painter->PopClipRect();
								painter->PopChildCoordinateSpace();

								return;
							}
						}

						painter->SetBrush(CBrush(triangleColor));
						painter->SetRotation(90);
						painter->DrawTriangle(triangleRect);
						painter->SetRotation(0);
					}
					else // Row is expanded
					{
						Vec2 triangleSize = Vec2(1, 0.9f) * cellHeight * 0.5f;
						Vec2 trianglePos = Vec2(17.0f + cellRect.min.x, triangleSize.height * 0.7f + cellRect.GetSize().height * 0.5f);
						Rect triangleRect = Rect::FromSize(trianglePos, triangleSize);
						Vec2 curOrigin = painter->GetCurrentOrigin();
						Rect windowTriangleRect = Rect::FromSize(trianglePos + curOrigin - Vec2(15, cellRect.GetSize().height * 0.5f), triangleSize);

						if (isMouseHovering && windowTriangleRect.Contains(windowSpaceMousePos))
						{
							mouseHoverOnArrow = true;
							if (!isMouseLeftClick)
							{
								triangleColor = Color::RGBA(220, 220, 220);
							}
							else
							{
								expandedRows.Remove(index);

								recalculateRows = true;
								SetNeedsLayout();
								SetNeedsPaint();

								painter->PopClipRect();
								painter->PopChildCoordinateSpace();

								return;
							}
						}

						painter->SetBrush(CBrush(triangleColor));
						painter->SetRotation(180);
						painter->DrawTriangle(triangleRect);
						painter->SetRotation(0);
					}
				}
				
				// - Select clicked row

				if (isMouseHovering)
				{
					Vec2 rowMousePos = localMousePos - Vec2(0, columnHeaderHeight) + Vec2(0, scrollOffset.y);

					if (cellSelectionRect.Contains(rowMousePos) && isMouseLeftClick && selectionMode != CItemSelectionMode::NoSelection)
					{
						if (selectionMode == CItemSelectionMode::SingleSelection)
							selectionModel->Clear();
						else if (selectionMode == CItemSelectionMode::ExtendedSelection && !EnumHasAnyFlags(keyModifier, KeyModifier::Ctrl))
							selectionModel->Clear();

						selectionModel->Select(index);

						mouseClickedInsideCell = true;
					}
				}

				itemStyle.isExpanded = expandedRows.Exists(index);
				itemStyle.padding.left += indentationX;

				if (itemStyle.isExpanded)
				{
					expandedIndex = index;
				}

				delegate->Paint(painter, itemStyle, index); // Paint cell

				painter->PopClipRect();
				painter->PopChildCoordinateSpace();

				posX += columnWidths[col];
			}

			rowPosY += rowHeightsByParent[parentIndex][row];
			curRowNumber++;

			if (expandedIndex.IsValid())
			{
				PaintRowsInternal(painter, regionRect, indentLevel + 1, rowPosY, mouseClickedInsideCell, curRowNumber, expandedIndex);
			}
		}
	}

	void CBaseItemView::CalculateRowHeights(const CModelIndex& parentIndex)
	{
		int numRows = model->GetRowCount(parentIndex);
		int numColumns = model->GetColumnCount(parentIndex);

		if (numColumns == 0 || numRows == 0)
			return;

		rowHeightsByParent[parentIndex].Reserve(numRows);
		CFont font{};

		font.SetFamily(cellStyle.GetFontName());
		if (!font.GetFamily().IsValid())
		{
			font.SetFamily(computedStyle.GetFontName());
		}

		font.SetSize(cellStyle.GetFontSize());

		for (int row = 0; row < numRows; ++row)
		{
			f32 height = 0;

			for (int col = 0; col < numColumns; ++col)
			{
				CModelIndex index = model->GetIndex(row, col, parentIndex);

				if (model->GetRowCount(index) > 0)
				{
					if (expandableColumn == -1)
					{
						expandableColumn = col;
					}

					if (expandedRows.Exists(index))
					{
						CalculateRowHeights(index);
					}
				}

				CViewItemStyle itemStyle{};
				itemStyle.font = font;

				Variant display = model->GetData(index, CItemDataUsage::Display);
				if (display.HasValue() && display.IsTextType())
				{
					itemStyle.features |= CViewItemFeature::HasDisplay;
					itemStyle.text = display.GetTextValue();
				}

				Variant icon = model->GetData(index, CItemDataUsage::Decoration);
				if (icon.HasValue() && icon.IsTextType())
				{
					itemStyle.features |= CViewItemFeature::HasDecoration;
					itemStyle.icon = icon.GetNameValue();
					itemStyle.decorationRect = Rect::FromSize(0, 0, DefaultDecorationSize, DefaultDecorationSize);
				}

				itemStyle.padding = cellStyle.GetPadding();
				itemStyle.textColor = computedStyle.GetForegroundColor();
				itemStyle.bgColor = Colors::Clear;
				itemStyle.expandableColumn = expandableColumn;

				Vec2 size = delegate->GetSizeHint(itemStyle, index);

				height = Math::Max(height, size.height);
			}

			totalContentHeight += height;
			rowHeightsByParent[parentIndex].Add(height);
		}
	}

	void CBaseItemView::OnPaintOverlay(CPaintEvent* paintEvent)
	{
		Super::OnPaintOverlay(paintEvent);

		if (!model)
			return;
	}

	void CBaseItemView::SetColumFixedWidth(u32 column, f32 width)
	{
		columnWidths.Resize(column + 1, 0);
		columnWidths[column] = width;

		columnHeaderFlags.Resize(column + 1, CItemViewHeaderFlags::None);
		columnHeaderFlags[column] &= ~CItemViewHeaderFlags::Resizable;
	}

	void CBaseItemView::SetColumnResizable(u32 column, f32 widthRatio)
	{
		columnWidthRatios.Resize(column + 1);
		columnWidthRatios[column] = widthRatio;

		columnHeaderFlags.Resize(column + 1, CItemViewHeaderFlags::None);
		columnHeaderFlags[column] |= CItemViewHeaderFlags::Resizable;
	}

	void CBaseItemView::OnSelectionModelUpdated()
	{
		SetNeedsPaint();
	}

	Rect CBaseItemView::GetVerticalScrollBarRect()
	{
		if (allowVerticalScroll)
		{
			Vec2 originalSize = GetComputedLayoutSize();
			f32 originalHeight = originalSize.height - columnHeaderHeight;
			f32 contentMaxY = contentSize.height;

			if (contentMaxY > originalHeight + ScrollSizeBuffer)
			{
				Rect scrollRegion = Rect::FromSize(Vec2(originalSize.width - ScrollRectWidth, columnHeaderHeight), 
					Vec2(ScrollRectWidth, originalHeight));
				f32 scrollRectHeightRatio = originalHeight / contentMaxY;
				
				if (contentMaxY != originalHeight)
					normalizedScroll.y = Math::Clamp01(scrollOffset.y / (contentMaxY - originalHeight));
				else
					normalizedScroll.y = 0.0f;

				Rect scrollRect = Rect::FromSize(scrollRegion.min,
					Vec2(scrollRegion.GetSize().width, Math::Max(scrollRegion.GetSize().height * scrollRectHeightRatio, MinScrollRectSize)));
				scrollRect = scrollRect.Translate(Vec2(0, (originalHeight - scrollRect.GetSize().height) * normalizedScroll.y));
				
				return scrollRect;
			}
		}

		return Rect();
	}

	void CBaseItemView::HandleEvent(CEvent* event)
	{
		if (event->IsMouseEvent())
		{
			CMouseEvent* mouseEvent = static_cast<CMouseEvent*>(event);
			Vec2 globalMousePos = mouseEvent->mousePos;
			Vec2 mouseDelta = mouseEvent->mousePos - mouseEvent->prevMousePos;
			keyModifier = mouseEvent->keyModifiers;

			Vec2 originalSize = GetComputedLayoutSize();
			f32 originalHeight = originalSize.height - columnHeaderHeight;
			f32 contentMaxY = contentSize.height;

			Rect scrollBarRect = GetVerticalScrollBarRect();
			scrollBarRect = LocalToScreenSpaceRect(scrollBarRect);

			if (mouseEvent->type == CEventType::MouseEnter || mouseEvent->type == CEventType::MouseMove || 
				mouseEvent->type == CEventType::MousePress || mouseEvent->type == CEventType::MouseRelease)
			{
				mouseEvent->Consume(this);

				isMouseLeftClick = mouseEvent->type == CEventType::MousePress && mouseEvent->button == MouseButton::Left;
				isMouseLeftHeld = mouseEvent->type == CEventType::MousePress && mouseEvent->button == MouseButton::Left;

				if (contentMaxY > originalHeight + ScrollSizeBuffer && scrollBarRect.Contains(globalMousePos))
				{
					isVerticalScrollHovered = true;

					localMousePos = windowSpaceMousePos = Vec2(-1, -1);
					isMouseHovering = false;
				}
				else
				{
					isVerticalScrollHovered = false;
					
					localMousePos = ScreenToLocalSpacePoint(globalMousePos);
					windowSpaceMousePos = ScreenToWindowSpacePoint(globalMousePos);
					isMouseHovering = true;
				}
			}
			else if (mouseEvent->type == CEventType::MouseLeave)
			{
				mouseEvent->Consume(this);

				isVerticalScrollHovered = false;
				isMouseHovering = false;
			}
			else if (mouseEvent->type == CEventType::MouseWheel)
			{
				mouseEvent->Consume(this);

				if (contentMaxY > originalHeight + ScrollSizeBuffer) // If scrolling is possible
				{
					normalizedScroll.y += -mouseEvent->wheelDelta.y * scrollSensitivity / (contentMaxY - originalHeight);
					normalizedScroll.y = Math::Clamp01(normalizedScroll.y);

					scrollOffset.y = normalizedScroll.y * (contentMaxY - originalHeight);
					scrollOffset.y = Math::Clamp(scrollOffset.y, 0.0f, (contentMaxY - originalHeight));

					SetNeedsLayout();
					SetNeedsPaint();
				}
			}

			if (event->IsDragEvent())
			{
				CDragEvent* dragEvent = (CDragEvent*)mouseEvent;

				if (dragEvent->type == CEventType::DragBegin)
				{
					if (isVerticalScrollHovered)
					{
						isVerticalScrollPressed = true;

						dragEvent->Consume(this);
					}
				}
				else if (dragEvent->type == CEventType::DragMove)
				{
					if (isVerticalScrollPressed)
					{
						dragEvent->Consume(this);

						normalizedScroll.y += mouseDelta.y / (originalHeight - GetVerticalScrollBarRect().GetSize().height);
						normalizedScroll.y = Math::Clamp01(normalizedScroll.y);

						scrollOffset.y = normalizedScroll.y * (contentMaxY - originalHeight);
						scrollOffset.y = Math::Clamp(scrollOffset.y, 0.0f, (contentMaxY - originalHeight));
					}
				}
				else if (dragEvent->type == CEventType::DragEnd)
				{
					if (isVerticalScrollPressed)
					{
						isVerticalScrollPressed = false;

						dragEvent->Consume(this);
					}
				}
			}

			SetNeedsPaint();
		}
		else if (event->IsKeyEvent())
		{
			CKeyEvent* keyEvent = static_cast<CKeyEvent*>(event);

			keyEvent->Consume(this);

			SetNeedsPaint();
		}

		Super::HandleEvent(event);
	}

} // namespace CE::Widgets
