#include "EditorCore.h"

namespace CE::Editor
{


    EditorStyle::EditorStyle()
    {
	    
    }

    void EditorStyle::Initialize()
    {
    	InitializeDefault();
    }

    void EditorStyle::Shutdown()
    {

    }

    Ref<EditorStyle> EditorStyle::Get()
    {
		EditorCoreModule* module = static_cast<EditorCoreModule*>(ModuleManager::Get().GetLoadedModule(MODULE_NAME));
    	if (!module)
    		return nullptr;
    	return module->GetEditorStyle();
    }

    void EditorStyle::InitializeDefault()
    {
        if (!button)
        {
            button = CreateObject<FCustomButtonStyle>(this, "Button");
            Add(button);
        }

		button->background = Color::RGBA(56, 56, 56);
		button->hoveredBackground = Color::RGBA(95, 95, 95);
		button->pressedBackground = Color::RGBA(50, 50, 50);
		button->disabledBackground = Color::RGBA(40, 40, 40);
		button->cornerRadius = Vec4(1, 1, 1, 1) * 2.5f;
		button->borderColor = 
			button->hoveredBorderColor = 
			button->pressedBorderColor = Color::RGBA(24, 24, 24);
		button->borderWidth = 1.0f;

		GetDefaultWidget<FButton>()
			.Padding(Vec4(10, 5, 10, 5))
			.Style(button->GetName())
			;
		GetDefaultWidget<FTextButton>()
			.Padding(Vec4(10, 5, 10, 5))
			.Style(button->GetName())
			;

    	if (!selectableButton)
    	{
    		selectableButton = CreateObject<FSelectableButtonStyle>(this, "SelectableButton");
			Add(selectableButton);
    	}

		if (!clearButton)
		{
			clearButton = CreateObject<FCustomButtonStyle>(this, "Button_Clear");
			Add("Button.Clear", clearButton);
		}

		clearButton->background = 
			clearButton->hoveredBackground =
			clearButton->pressedBackground = 
			clearButton->disabledBackground = Color::Clear();
		clearButton->cornerRadius = Vec4(0, 0, 0, 0);
		clearButton->borderColor = 
			clearButton->hoveredBorderColor =
			clearButton->pressedBorderColor = Color::Clear();
		clearButton->borderWidth = 0.0f;
		clearButton->contentMoveY = 0;

		if (!underlineButton)
		{
			underlineButton = CreateObject<FTextButtonStyle>(this, "UnderlineButton");
			Add("TextButton.Underline", underlineButton);
		}

		underlineButton->background =
			underlineButton->hoveredBackground =
			underlineButton->pressedBackground =
			underlineButton->disabledBackground = Color::Clear();
		underlineButton->cornerRadius = Vec4();
		underlineButton->borderColor =
			underlineButton->hoveredBorderColor =
			underlineButton->pressedBorderColor = Color::Clear();
		underlineButton->borderWidth = 0.0f;
		underlineButton->contentMoveY = 0;

		underlineButton->underline = FPen(Color::White(), 1, FPenStyle::DottedLine);
		underlineButton->hoverUnderline = FPen(Color::White(), 1, FPenStyle::SolidLine);
		underlineButton->pressedUnderline = FPen();

		if (!highlightedButton)
		{
			highlightedButton = CreateObject<FCustomButtonStyle>(this, "Button_Highlighted");
			Add("Button.Highlighted", highlightedButton);
		}

		highlightedButton->background = highlightColor;
		highlightedButton->hoveredBackground = Color::RGBA(14, 134, 255);
		highlightedButton->pressedBackground = Color::RGBA(0, 100, 200);
		highlightedButton->cornerRadius = Vec4(1, 1, 1, 1) * 5;
		highlightedButton->borderColor = 
			highlightedButton->hoveredBorderColor = 
			highlightedButton->pressedBorderColor = Color::RGBA(24, 24, 24);
    	highlightedButton->borderWidth = 1.0f;

		if (!scrollBox)
		{
			scrollBox = CreateObject<FScrollBoxStyle>(this, "ScrollBox");
			Add(scrollBox);
		}

		GetDefaultWidget<FScrollBox>()
			.Style(scrollBox->GetName())
			;

		if (!windowCloseButton)
		{
			windowCloseButton = CreateObject<FWindowControlButtonStyle>(this, "Button_WindowClose");
			Add("Button.WindowClose", windowCloseButton);
		}

		windowCloseButton->background = Color::Clear();
		windowCloseButton->hoveredBackground = Color::RGBA(161, 57, 57);
		windowCloseButton->hoveredBackground = Color::RGBA(196, 43, 28);
		windowCloseButton->pressedBackground = Color::RGBA(178, 43, 30);
		windowCloseButton->borderColor = Color::Clear();
		windowCloseButton->borderWidth = 0.0f;
		windowCloseButton->cornerRadius = Vec4();
		windowCloseButton->contentMoveY = 0;

		if (!windowControlButton)
		{
			windowControlButton = CreateObject<FWindowControlButtonStyle>(this, "Button_WindowControl");
			Add("Button.WindowControl", windowControlButton);
		}

		windowControlButton->background = Color::Clear();
		windowControlButton->hoveredBackground = Color::RGBA(58, 58, 58);
		windowControlButton->pressedBackground = Color::RGBA(48, 48, 48);
		windowControlButton->borderColor = Color::Clear();
		windowControlButton->borderWidth = 0.0f;
		windowControlButton->cornerRadius = Vec4();
		windowControlButton->contentMoveY = 0;

		if (!textInput)
		{
			textInput = CreateObject<FTextInputPlainStyle>(this, "TextInput");
			Add(textInput);
		}

		textInput->background = textInput->hoverBackground = textInput->editingBackground = Color::RGBA(15, 15, 15);
		textInput->borderColor = Color::RGBA(60, 60, 60);
		textInput->editingBorderColor = highlightColor;
		textInput->hoverBorderColor = Color::RGBA(74, 74, 74);
		textInput->borderWidth = 1.0f;
		textInput->cornerRadius = Vec4(5, 5, 5, 5);

		GetDefaultWidget<FTextInput>()
			.Style(textInput->GetName());
		GetDefaultWidget<NumericEditorField>()
			.Style(textInput->GetName());

		if (!expandableSection)
		{
			expandableSection = CreateObject<FExpandableSectionStyle>(this, "ExpandableSection");
			Add(expandableSection);
		}

		GetDefaultWidget<FExpandableSection>()
			.Style(expandableSection->GetName());
			
		if (!comboBox)
		{
			comboBox = CreateObject<FComboBoxStyle>(this, "ComboBox");
			Add(comboBox);
		}

		comboBox->background = Color::RGBA(15, 15, 15);
		comboBox->borderColor = Color::RGBA(60, 60, 60);
		comboBox->pressedBorderColor = comboBox->hoverBorderColor = Color::RGBA(74, 74, 74);
		comboBox->borderWidth = 1.0f;
		comboBox->cornerRadius = Vec4(5, 5, 5, 5);

		if (!comboBoxItem)
		{
			comboBoxItem = CreateObject<FComboBoxItemStyle>(this, "ComboBoxItem");
			Add(comboBoxItem);
		}

		comboBoxItem->background = Color::Clear();
		comboBoxItem->hoverBackground = highlightColor;
		comboBoxItem->selectedBackground = Color::Clear();
		comboBoxItem->selectedShape = FShapeType::RoundedRect;
		comboBoxItem->shapeCornerRadius = Vec4(1, 1, 1, 1) * 3;
		comboBoxItem->selectedBorderColor = highlightColor;
		comboBoxItem->borderWidth = 1.0f;

		GetDefaultWidget<FComboBox>()
			.ItemStyle(comboBoxItem)
			.Style(comboBox->GetName());

		if (!comboBoxPopup)
		{
			comboBoxPopup = CreateObject<FComboBoxPopupStyle>(this, "ComboBoxPopup");
			Add(comboBoxPopup);
		}

		comboBoxPopup->background = Color::RGBA(15, 15, 15);
		comboBoxPopup->borderColor = Color::RGBA(60, 60, 60);
		comboBoxPopup->borderWidth = 1.0f;

		GetDefaultWidget<FComboBoxPopup>()
			.Style(comboBoxPopup->GetName());

		if (!checkbox)
		{
			checkbox = CreateObject<FCheckboxStyle>(this, "Checkbox");
			Add(checkbox);
		}

		checkbox->background = textInput->background;
		checkbox->hoveredBackground = textInput->hoverBackground;
		checkbox->pressedBackground = textInput->editingBackground;
		checkbox->disabledBackground = Color::RGBA(10, 10, 10);

		checkbox->borderColor = textInput->borderColor;
		checkbox->hoveredBorderColor = textInput->hoverBorderColor;
		checkbox->pressedBorderColor = textInput->editingBorderColor;
		checkbox->borderWidth = textInput->borderWidth;

		Color checkboxForeground = highlightColor;

		checkbox->foregroundColor = checkboxForeground;
		checkbox->foregroundDisabledColor = checkboxForeground;
		checkbox->foregroundHoverColor = checkboxForeground;
		checkbox->foregroundPressedColor = checkboxForeground;

		GetDefaultWidget<FCheckbox>()
			.Style(checkbox->GetName());

		if (!tabView)
		{
			tabView = CreateObject<FTabViewStyle>(this, "TabView");
			Add(tabView);
		}

		tabView->tabItemActiveBackground = Color::RGBA(50, 50, 50);
		tabView->tabItemHoverBackground = Color::RGBA(40, 40, 40);
		tabView->tabItemBackground = Color::RGBA(36, 36, 36);
		tabView->tabItemShape = FRoundedRectangle(3, 3, 0, 0);

		GetDefaultWidget<FTabView>()
			.Style(tabView->GetName());

		if (!expandCaretButton)
		{
			expandCaretButton = CreateObject<FImageButtonStyle>(this, "ExpandCaretButton");
			Add(expandCaretButton);
		}

		expandCaretButton->tintColor = Color::RGBA(134, 134, 134);
		expandCaretButton->hoverTintColor = Color::RGBA(180, 180, 180);
		expandCaretButton->pressedTintColor = Color::RGBA(180, 180, 180);

		if (!tabCloseButton)
		{
			tabCloseButton = CreateObject<FImageButtonStyle>(this, "Button_CloseTab");
			Add("Button.CloseTab", tabCloseButton);
		}

		tabCloseButton->tintColor = Color::RGBA(120, 120, 120);
		tabCloseButton->hoverTintColor = Color::RGBA(180, 180, 180);
		tabCloseButton->pressedTintColor = tabCloseButton->tintColor;

		if (!iconButton)
		{
			iconButton = CreateObject<FCustomButtonStyle>(this, "Button_Icon");
			Add("Button.Icon", iconButton);
		}

		iconButton->background = iconButton->disabledBackground = Color::Clear();
		iconButton->hoveredBackground = Color::RGBA(255, 255, 255, 60);
		iconButton->pressedBackground = Color::RGBA(255, 255, 255, 30);
		iconButton->borderColor = Color::Clear();
		iconButton->borderWidth = 0;
		iconButton->cornerRadius = Vec4(1, 1, 1, 1) * 4;

		if (!circleIconButton)
		{
			circleIconButton = CreateObject<FCustomButtonStyle>(this, "Button_CircleIcon");
			Add("Button.CircleIcon", circleIconButton);
		}

		circleIconButton->background = circleIconButton->disabledBackground = Color::Clear();
		circleIconButton->hoveredBackground = Color::RGBA(255, 255, 255, 60);
		circleIconButton->pressedBackground = Color::RGBA(255, 255, 255, 30);
		circleIconButton->borderColor = Color::Clear();
		circleIconButton->borderWidth = 0;
		circleIconButton->backgroundShape = FShapeType::Circle;

		if (!menuPopup)
		{
			menuPopup = CreateObject<FMenuPopupStyle>(this, "MenuPopup");
			Add(menuPopup);
		}

		menuPopup->background = Color::RGBA(56, 56, 56);
		menuPopup->borderColor = Color::RGBA(26, 26, 26);
		menuPopup->borderWidth = 0.75f;
		menuPopup->itemPadding = Vec4(10, 5, 10, 5);
		menuPopup->itemHoverBackground = highlightColor;
		menuPopup->separatorColor = menuPopup->separatorTitleColor = Color::RGBA(106, 106, 106);

		GetDefaultWidget<FMenuPopup>()
			.Style(menuPopup->GetName());

		GetDefaultWidget<EditorMenuPopup>()
			.Style(menuPopup->GetName());

    	GetDefaultWidget<AssetSelectionPopup>()
    		.Style(menuPopup->GetName());

		if (!menuBar)
		{
			menuBar = CreateObject<FMenuBarStyle>(this, "MenuBar");
			Add(menuBar);
		}

		menuBar->background = Color::Black();
		menuBar->itemPadding = Vec4(5, 5, 5, 5);
		menuBar->itemHoverBackground = highlightColor;

		GetDefaultWidget<FMenuBar>()
			.Style(menuBar->GetName());

		if (!toolWindow)
		{
			toolWindow = CreateObject<FToolWindowStyle>(this, "ToolWindow");
			Add(toolWindow);
		}

		GetDefaultWidget<FToolWindow>()
			.Style(toolWindow->GetName());

		InitProjectBrowserStyle();
		InitEditorStyle();
    }

    void EditorStyle::InitProjectBrowserStyle()
    {
		if (!projectBrowserTabView)
		{
			projectBrowserTabView = CreateObject<FTabViewStyle>(this, "ProjectBrowserTabView");
			Add("TabView.ProjectBrowser", projectBrowserTabView);
		}

		projectBrowserTabView->tabItemActiveBackground = Color::RGBA(40, 40, 40);
		projectBrowserTabView->tabItemHoverBackground = Color::RGBA(36, 36, 36);
		projectBrowserTabView->tabItemBackground = Color::RGBA(26, 26, 26);
		projectBrowserTabView->tabItemShape = FRoundedRectangle(5, 5, 0, 0);

    	projectBrowserTabView->containerBackground = projectBrowserTabView->tabItemActiveBackground;
		projectBrowserTabView->containerBorderColor = Color::Clear();
		projectBrowserTabView->containerBorderWidth = 0.0f;

		if (!projectBrowserWindow)
		{
			projectBrowserWindow = CreateObject<FToolWindowStyle>(this, "ProjectBrowserWindow");
			Add(projectBrowserWindow);
		}

		projectBrowserWindow->background = Color::RGBA(26, 26, 26);

		if (!projectBrowserListView)
		{
			projectBrowserListView = CreateObject<FListWidgetStyle>(this, "ProjectBrowserListView");
			Add("ProjectBrowserWindow.ListView", projectBrowserListView);
		}

		projectBrowserListView->background = Color::RGBA(26, 26, 26);
		projectBrowserListView->borderWidth = 1.0f;
		projectBrowserListView->borderColor = Color::RGBA(56, 56, 56);

		projectBrowserListView->itemBackground = Color::Clear();
		projectBrowserListView->hoveredItemBackground = highlightColor;
		projectBrowserListView->selectedItemBackground = highlightColor;

		projectBrowserListView->itemBorderWidth = 0.0f;
		projectBrowserListView->itemBorderColor = Color::Clear();

    }

    void EditorStyle::InitEditorStyle()
    {
		if (!componentTreeView)
		{
			componentTreeView = CreateObject<ComponentTreeViewStyle>(this, "ComponentTreeView");
			Add(componentTreeView);
		}

		GetDefaultWidget<ComponentTreeView>()
			.Style(componentTreeView->GetName());

		if (!editorDockspace)
		{
			editorDockspace = CreateObject<EditorDockspaceStyle>(this, "EditorDockspace");
			Add(editorDockspace);
		}

		GetDefaultWidget<EditorDockspace>()
			.Style(editorDockspace->GetName());

		if (!editorMinorDockspace)
		{
			editorMinorDockspace = CreateObject<EditorMinorDockspaceStyle>(this, "EditorMinorDockspace");
			Add(editorMinorDockspace);
		}

		GetDefaultWidget<EditorMinorDockspace>()
			.Style(editorMinorDockspace->GetName());

		if (!editorDockTab)
		{
			editorDockTab = CreateObject<EditorDockTabStyle>(this, "EditorDockTab");
			Add(editorDockTab);
		}

		editorDockTab->background = Color::RGBA(26, 26, 26);

		GetDefaultWidget<EditorDockTab>()
			.Style(editorDockTab->GetName());

		if (!editorMinorDockTab)
		{
			editorMinorDockTab = CreateObject<EditorDockTabStyle>(this, "EditorMinorDockTab");
			Add(editorMinorDockTab);
		}

		GetDefaultWidget<EditorMinorDockTab>()
			.Style(editorMinorDockTab->GetName());

		if (!editorToolBar)
		{
			editorToolBar = CreateObject<FStyledWidgetStyle>(this, "EditorToolBar");
			Add(editorToolBar);
		}

		editorToolBar->background = Color::RGBA(36, 36, 36);
		editorToolBar->borderColor = Color::Clear();
		editorToolBar->borderWidth = 0.0f;

		GetDefaultWidget<EditorToolBar>()
			.Style(editorToolBar->GetName());

		if (!editorMenuBar)
		{
			editorMenuBar = CreateObject<EditorMenuBarStyle>(this, "EditorMenuBar");
			Add(editorMenuBar);
		}

		editorMenuBar->background = editorToolBar->background;
		editorMenuBar->borderColor = Color::Clear();
		editorMenuBar->borderWidth = 0.0f;

    	editorMenuBar->itemBackground = Color::Clear();
    	editorMenuBar->itemHoverBackground = Color::RGBA(64, 64, 64);
		editorMenuBar->itemActiveBackground = highlightColor;
		editorMenuBar->itemBorderColor = Color::Clear();
		editorMenuBar->itemHoverBorderColor = Color::Clear();
		editorMenuBar->itemPadding = Vec4(2, 1, 2, 1) * 5;
		
		GetDefaultWidget<EditorMenuBar>()
			.Style(editorMenuBar->GetName());

		if (!treeView)
		{
			treeView = CreateObject<FTreeViewStyle>(this, "TreeView");
			Add(treeView);
		}

		GetDefaultWidget<FTreeView>()
			.Style(treeView->GetName());

    	if (!listView)
    	{
    		listView = CreateObject<FListViewStyle>(this, "ListView");
    		Add(listView);
    	}

    	GetDefaultWidget<FListView>()
    		.Style(listView->GetName());
    	GetDefaultWidget<AssetSelectionListView>()
    		.Style(listView->GetName());

		if (!detailsRow)
		{
			detailsRow = CreateObject<FCustomButtonStyle>(this, "Button_DetailsRow");
			Add("Button.DetailsRow", detailsRow);
		}

		detailsRow->background = detailsRow->disabledBackground = Color::Clear();
		detailsRow->hoveredBackground = detailsRow->pressedBackground = Color::RGBA(47, 47, 47);
		detailsRow->cornerRadius = Vec4();
		detailsRow->contentMoveY = 0;
		detailsRow->borderColor = Color::Clear();
		detailsRow->borderWidth = 0;

    	if (!objectEditorField)
    	{
    		objectEditorField = CreateObject<FCustomButtonStyle>(this, "Button_ObjectEditorField");
    		Add("Button.ObjectEditorField", objectEditorField);
    	}

    	objectEditorField->background = objectEditorField->hoveredBackground = objectEditorField->pressedBackground = Color::RGBA(28, 28, 28);
    	objectEditorField->cornerRadius = Vec4(5, 5, 5, 5);
    	objectEditorField->borderColor = Color::RGBA(60, 60, 60);
    	objectEditorField->hoveredBorderColor = Color::RGBA(74, 74, 74);
    	objectEditorField->pressedBorderColor = highlightColor;
    	objectEditorField->borderWidth = 1.0f;
    }

} // namespace CE::Editor
