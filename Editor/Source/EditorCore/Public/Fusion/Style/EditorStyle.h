#pragma once


namespace CE::Editor
{
	class ComponentTreeViewStyle;

	CLASS()
	class EDITORCORE_API EditorStyle : public FStyleSet
	{
		CE_CLASS(EditorStyle, FStyleSet)
	public:

		EditorStyle();

		void Initialize();
		void Shutdown();

		static Ref<EditorStyle> Get();

	protected:

		virtual void InitializeDefault();

		void InitProjectBrowserStyle();

		virtual void InitEditorStyle();

	public:

		FIELD()
		Color highlightColor = Color::RGBA(0, 112, 224);

		FIELD()
		FCustomButtonStyle* button = nullptr;

		FIELD()
		FCustomButtonStyle* clearButton = nullptr;

		FIELD()
		FCustomButtonStyle* highlightedButton = nullptr;

		FIELD()
		FSelectableButtonStyle* selectableButton = nullptr;

		FIELD()
		FTextButtonStyle* underlineButton = nullptr;

		FIELD()
		FWindowControlButtonStyle* windowCloseButton = nullptr;

		FIELD()
		FWindowControlButtonStyle* windowControlButton = nullptr;

		FIELD()
		FTextInputPlainStyle* textInput = nullptr;

		FIELD()
		FExpandableSectionStyle* expandableSection = nullptr;

		FIELD()
		FComboBoxStyle* comboBox = nullptr;

		FIELD()
		FComboBoxItemStyle* comboBoxItem = nullptr;

		FIELD()
		FComboBoxPopupStyle* comboBoxPopup = nullptr;

		FIELD()
		FTabViewStyle* tabView = nullptr;

		FIELD()
		FMenuPopupStyle* menuPopup = nullptr;

		FIELD()
		FMenuBarStyle* menuBar = nullptr;

		FIELD()
		FScrollBoxStyle* scrollBox = nullptr;

		FIELD()
		FToolWindowStyle* toolWindow = nullptr;

		FIELD()
		FImageButtonStyle* expandCaretButton = nullptr;

		FIELD()
		FImageButtonStyle* tabCloseButton = nullptr;

		FIELD()
		FCustomButtonStyle* iconButton = nullptr;

		FIELD()
		FCustomButtonStyle* circleIconButton = nullptr;

		FIELD()
		FCheckboxStyle* checkbox = nullptr;

		// - Project Browser Window -

		FIELD()
		FToolWindowStyle* projectBrowserWindow = nullptr;

		FIELD()
		FTabViewStyle* projectBrowserTabView = nullptr;

		FIELD()
		FListWidgetStyle* projectBrowserListView = nullptr;

		// - Editor -

		FIELD()
		FStyledWidgetStyle* assetImportProgressPopup = nullptr;

		FIELD()
		ComponentTreeViewStyle* componentTreeView = nullptr;

		FIELD()
		EditorDockspaceStyle* editorDockspace = nullptr;

		FIELD()
		FStyledWidgetStyle* editorMajorDockWindow = nullptr;

		FIELD()
		FStyledWidgetStyle* editorMinorDockWindow = nullptr;

		FIELD()
		FStyledWidgetStyle* editorToolBar = nullptr;

		FIELD()
		EditorMenuBarStyle* editorMenuBar = nullptr;

		FIELD()
		FTreeViewStyle* treeView = nullptr;

		FIELD()
		FListViewStyle* listView = nullptr;

		FIELD()
		FCustomButtonStyle* detailsRow = nullptr;

		FIELD()
		FCustomButtonStyle* objectEditorField = nullptr;

	};

} // namespace CE::Editor

#include "EditorStyle.rtti.h"