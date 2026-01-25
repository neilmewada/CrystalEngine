#pragma once

#include "FTextValidator.h"

namespace CE
{
    class FTextInput;
    class FStackBox;
    DECLARE_SCRIPT_EVENT(FTextInputEvent, FTextInput*);

    ENUM(Flags)
    enum class FTextInputState : u8
    {
        None = 0,
        Hovered = BIT(0),
        Editing = BIT(1),
        InteractionDisabled = BIT(2),
        Highlighted = BIT(3)
    };
    ENUM_CLASS_FLAGS(FTextInputState);

    CLASS()
    class FUSIONCORE_API FTextInputLabel : public FLabel
    {
        CE_CLASS(FTextInputLabel, FLabel)
    public:

        FTextInputLabel();

        void CalculateIntrinsicSize() override;

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsKeyboardEvents() const override { return true; }

        bool SupportsFocusEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

        bool IsTextSelected() const { return isSelectionActive; }

    protected:

        void Construct() override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        void OnPaint(FPainter* painter) override;

        void HandleEvent(FEvent* event) override;

        FUNCTION()
        void RecalculateCharacterOffsets();

        FUNCTION()
        void OnTimeOut();

        // - Internal API -

        void OnMouseClick(Vec2 localPos, bool isDoubleClick);

        Range GetCharacterMinMax(int characterIndex);

        bool IsEditing();

        void StartEditing();
        void StopEditing(bool restoreOriginal = false);

        void SetCursorPos(int newCursorPos);

        void ScrollTo(int charIndex);

        void InsertAt(const String& string, int insertPos);
        void InsertAt(char character, int insertPos);
        bool CanInsertAt(const String& string, int insertPos);

        void RemoveRange(int startIndex, int count);
        void RemoveSelectedRange();
        bool CanRemoveSelectedText();
        bool CanRemoveRange(int startIndex, int count);

        String GetSelectedText();

        void SelectAll();
        void SelectRange(int startIndex, int endIndex);
        void DeselectAll();

    public: // - Fusion Properties -

        FUSION_PROPERTY(Color, SelectionColor);

        // - Property Bindings -

    private:

        FIELD()
        FTimer* cursorTimer = nullptr;

        FTextInput* textInput = nullptr;

        String originalText;
        Array<Vec2> characterOffsets;
        Vec2 textSize;
        bool cursorState : 1 = false;
        bool isSelectionActive : 1 = false;
        int cursorPos = 0;
        int selectionStart = -1;
        int selectionEnd = -1;
        f32 textScrollOffset = 0;
        f32 selectionDistance = 0;
        bool isCursorPushed = false;

        friend class FTextInput;
        FUSION_WIDGET;
    };

    CLASS()
    class FUSIONCORE_API FTextInput : public FStyledWidget
    {
        CE_CLASS(FTextInput, FStyledWidget)
    public:

        FTextInput();

        bool IsHovered() const { return EnumHasFlag(state, FTextInputState::Hovered); }
        bool IsEditing() const { return EnumHasFlag(state, FTextInputState::Editing); }
        bool IsHighlighted() const { return EnumHasFlag(state, FTextInputState::Highlighted); }
        bool IsInteractionDisabled() const { return EnumHasFlag(state, FTextInputState::InteractionDisabled); }

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsKeyboardEvents() const override { return true; }

        bool SupportsFocusEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

        FHorizontalStack* GetContentStack() const { return contentStack; }

        void StartEditing(bool selectAll, bool cursorAtEnd = false);
        void StopEditing(bool restoreOriginal = false);

        void HandleEvent(FEvent* event) override;

        // Internal use only!
        void SetHoveredInternal(bool hovered);

        // Internal use only!
        void SetHighlightedInternal(bool highlighted);

    protected:

        void OnPaintContent(FPainter* painter) override;

        void OnLostFocus() override;

        void Construct() override;

        virtual void OnFinishEdit() {}

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        void SetEditingInternal(bool editing);

        FIELD()
        FTextInputState state = FTextInputState::None;

        FIELD()
        FHorizontalStack* contentStack = nullptr;

        FIELD()
        FTextInputLabel* inputLabel = nullptr;

        bool isCursorPushed = false;

    public: // - Fusion Properties -

        FUSION_PROPERTY_WRAPPER(SelectionColor, inputLabel);
        FUSION_PROPERTY_WRAPPER(Foreground, inputLabel);
        FUSION_PROPERTY_WRAPPER(FontSize, inputLabel);
        FUSION_PROPERTY_WRAPPER(FontFamily, inputLabel);
        FUSION_PROPERTY(FTextInputValidator, Validator);

        FUSION_DATA_PROPERTY_WRAPPER(Text, inputLabel);

        FUSION_EVENT(FTextInputEvent, OnBeginEditing);
        FUSION_EVENT(FTextInputEvent, OnTextEdited);
        FUSION_EVENT(FTextInputEvent, OnTextEditingFinished);

        FUSION_EVENT(FPaintEvent, OnBeforeTextPaint);

        Self& LeftSlot(FWidget& content);

    private: // - Internal Data -

        friend class FTextInputLabel;
        FUSION_FRIENDS;
        FUSION_WIDGET;
    };

} // namespace CE

#include "FTextInput.rtti.h"