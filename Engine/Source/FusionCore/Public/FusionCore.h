#pragma once

#include "Core.h"
#include "CoreApplication.h"
#include "CoreRPI.h"

#if PAL_TRAIT_BUILD_TESTS

// Test Classes
class FusionCore_Construction_Test;
class FusionCore_Layout_Test;
namespace RenderingTests
{
    class RendererSystem;
}

#define FUSION_TESTS \
    friend class FusionCore_Construction_Test;\
    friend class FusionCore_Layout_Test;\
    friend class RenderingTests::RendererSystem;

#else

#define FUSION_TESTS

#endif

#define FUSION_FRIENDS FUSION_TESTS\
    friend class FWidget;\
    friend class FStackBox;\
    friend class FSplitBox;\
    friend class FWindow;\
    friend class FFusionContext;\
    friend class FRootContext;\
    friend class FNativeContext;\
    friend class FusionApplication;\
    friend class FCompoundWidget;\
    friend class FStyleManager;\
    friend class FusionRenderer;\
    friend class FPainter;\
    friend class FScrollBox;\
    friend class FReorderableStack;

namespace CE
{
    // Forward Declarations
    class FStyleManager;

}

#include "FusionMacros.h"
#include "FusionDefines.h"
#include "Exception/FusionException.h"
#include "Font/FFontAtlas.h"
#include "Font/FFontManager.h"

#include "Event/FEvent.h"

#include "Style/FShape.h"
#include "Style/FPen.h"
#include "Style/FBrush.h"
#include "Style/FFont.h"

#include "Application/FTimer.h"
#include "Application/FusionApplication.h"
#include "Rendering/FImageAtlas.h"
#include "Rendering/FusionRenderer.h"
#include "Rendering/FusionRenderer2.h"
#include "Rendering/FPainter.h"
#include "Context/FFusionContext.h"
#include "Context/FRootContext.h"
#include "Context/FNativeContext.h"

#include "Layout/LayoutTypes.h"
#include "Layout/FLayoutManager.h"

#include "Style/FStyle.h"
#include "Style/FButtonStyle.h"
#include "Style/FSelectableButtonStyle.h"
#include "Style/FImageButtonStyle.h"
#include "Style/FTextInputStyle.h"
#include "Style/FComboBoxStyle.h"
#include "Style/FTabViewStyle.h"
#include "Style/FMenuStyle.h"
#include "Style/FCheckboxStyle.h"
#include "Style/FStyledWidgetStyle.h"
#include "Style/FScrollBoxStyle.h"
#include "Style/FDockTabWellStyle.h"
#include "Style/FDockspaceStyle.h"
#include "Style/FDockingGuideStyle.h"
#include "Style/FStyleSet.h"
#include "Style/FStyleManager.h"

#include "Data/FDataModel.h"

#include "Widget/FWidget.h"
#include "Widget/FLabel.h"
#include "Widget/FCompoundWidget.h"
#include "Widget/FStyledWidget.h"
#include "Widget/FContainerWidget.h"

#include "Widget/Layout/FStackBox.h"
#include "Widget/Layout/FWrapBox.h"
#include "Widget/Layout/FOverlayStack.h"
#include "Widget/Layout/FSplitBox.h"
#include "Widget/Layout/FScrollBox.h"
#include "Widget/Layout/FReorderableStackItem.h"
#include "Widget/Layout/FReorderableStack.h"

#include "Widget/FPopup.h"
#include "Widget/FImage.h"
#include "Widget/FButton.h"
#include "Widget/FSelectableButton.h"
#include "Widget/FImageButton.h"
#include "Widget/FTextButton.h"
#include "Widget/FCheckbox.h"
#include "Widget/Input/FTextInput.h"
#include "Widget/ComboBox/FComboBoxPopup.h"
#include "Widget/ComboBox/FComboBoxItem.h"
#include "Widget/ComboBox/FComboBox.h"

#include "Widget/Menu/FMenuPopup.h"
#include "Widget/Menu/FMenuItem.h"
#include "Widget/Menu/FMenuItemSeparator.h"
#include "Widget/Menu/FMenuBar.h"

#include "Widget/Tab/FTabItem.h"
#include "Widget/Tab/FTabView.h"

#include "Window/FTitleBar.h"
#include "Window/FWindow.h"
#include "Window/FWindowControlButton.h"
#include "Window/FToolWindow.h"
#include "Window/FGameWindow.h"
#include "Widget/FViewport.h"

#include "Widget/Docking/FDockTabWell.h"
#include "Widget/Docking/FDockingHint.h"
#include "Widget/Docking/FDockingGuide.h"
#include "Widget/Docking/FDockspace.h"
#include "Widget/Docking/FDockspaceSplitView.h"
#include "Widget/Docking/FDockWindow.h"
#include "Widget/Docking/FDockTabItem.h"

#include "Window/FDockspaceWindow.h"
