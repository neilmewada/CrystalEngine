#include "FusionCore.h"

namespace CE
{
    FTextInputLabel::FTextInputLabel()
    {
        m_SelectionColor = Color::RGBA(0, 112, 224);

        cursorTimer = CreateDefaultSubobject<FTimer>("Timer");
    }

    void FTextInputLabel::Construct()
    {
        Super::Construct();

        cursorTimer->OnTimeOut(FUNCTION_BINDING(this, OnTimeOut));
    }

    void FTextInputLabel::CalculateIntrinsicSize()
    {
	    Super::CalculateIntrinsicSize();

    }

    void FTextInputLabel::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        static const CE::Name TextName = "Text";
        static const CE::Name EnabledName = "Enabled";

        if (propertyName == TextName)
        {
            RecalculateCharacterOffsets();
        }
    }

    void FTextInputLabel::OnPaint(FPainter* painter)
    {
        if (characterOffsets.GetSize() != m_Text.GetLength())
        {
            RecalculateCharacterOffsets();
        }

        painter->PushChildCoordinateSpace(Vec2(-textScrollOffset, 0));

        if (IsEditing() && IsTextSelected() && selectionStart <= selectionEnd)
        {
            painter->SetPen(FPen());
            painter->SetBrush(m_SelectionColor);

            Range start = GetCharacterMinMax(selectionStart);
            Range end = GetCharacterMinMax(selectionEnd);

            Rect rect = Rect(start.min, 0, end.max, GetComputedSize().height);

            painter->DrawRect(rect);
        }

	    Super::OnPaint(painter);

        if (IsEditing() && cursorState)
        {
            painter->SetPen(FPen(Colors::White, 1.2f));
            painter->SetBrush(FBrush());

            f32 posX = GetCharacterMinMax(cursorPos).min;
            Vec2 lineStart = Vec2(posX, 0);
            Vec2 lineEnd = lineStart + Vec2(0, GetComputedSize().height - m_Padding.bottom - m_Padding.top);

            painter->DrawLine(lineStart, lineEnd);
        }

        painter->PopChildCoordinateSpace();
    }

    void FTextInputLabel::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);
            auto app = FusionApplication::Get();

            Vec2 mousePos = mouseEvent->mousePosition;
            Vec2 localMousePos = mousePos - globalPosition;

            if (event->type == FEventType::MouseEnter && !isCursorPushed)
            {
                app->PushCursor(SystemCursor::IBeam);
                isCursorPushed = true;
            }
            else if (event->type == FEventType::MouseLeave && isCursorPushed)
            {
                isCursorPushed = false;
                app->PopCursor();
            }
            else if (event->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left)
            {
                OnMouseClick(localMousePos, mouseEvent->isDoubleClick);
            }
        }
        else if (!event->isConsumed && (event->type == FEventType::KeyHeld || event->type == FEventType::KeyPress))
        {
            FKeyEvent* keyEvent = static_cast<FKeyEvent*>(event);

            event->Consume(this);

            bool shiftPressed = EnumHasAnyFlags(keyEvent->modifiers, KeyModifier::LShift | KeyModifier::RShift);
            bool capslock = EnumHasAnyFlags(keyEvent->modifiers, KeyModifier::Caps);
            bool ctrl = EnumHasFlag(keyEvent->modifiers, KeyModifier::Ctrl);
            bool gui = EnumHasFlag(keyEvent->modifiers, KeyModifier::Gui);
#if PLATFORM_MAC
            ctrl = ctrl || gui;
#endif
            
            bool isUpperCase = (shiftPressed != capslock);
            char c = 0;

            if ((int)keyEvent->key >= (int)KeyCode::Space && (int)keyEvent->key <= (int)KeyCode::Z && IsEditing())
            {
                c = (char)keyEvent->key;

                if (isUpperCase && c >= 'a' && c <= 'z')
                {
                    c += ('A' - 'a');
                }

                if (shiftPressed)
                {
                    switch (keyEvent->key)
                    {
                    case KeyCode::N0:
                        c = ')';
                        break;
                    case KeyCode::N1:
                        c = '!';
                        break;
                    case KeyCode::N2:
                        c = '@';
                        break;
                    case KeyCode::N3:
                        c = '#';
                        break;
                    case KeyCode::N4:
                        c = '$';
                        break;
                    case KeyCode::N5:
                        c = '%';
                        break;
                    case KeyCode::N6:
                        c = '^';
                        break;
                    case KeyCode::N7:
                        c = '&';
                        break;
                    case KeyCode::N8:
                        c = '*';
                        break;
                    case KeyCode::N9:
                        c = '(';
                        break;
                    case KeyCode::Semicolon:
                        c = ':';
                        break;
                    case KeyCode::LeftBracket:
                        c = '{';
                        break;
                    case KeyCode::RightBracket:
                        c = '}';
                        break;
                    case KeyCode::Equals:
                        c = '+';
                        break;
                    case KeyCode::Minus:
                        c = '_';
                        break;
                    case KeyCode::Backquote:
                        c = '~';
                        break;
                    case KeyCode::Comma:
                        c = '<';
                        break;
                    case KeyCode::Period:
                        c = '>';
                        break;
                    case KeyCode::Slash:
                        c = '?';
                        break;
                    case KeyCode::Quote:
                        c = '\"';
                        break;
                    }
                }

                bool isCommand = false;

                if (ctrl) // Check for commands
                {
                    if (keyEvent->key == KeyCode::X && IsTextSelected()) // Cut
                    {
                        String selected = GetSelectedText();
                        PlatformApplication::Get()->SetClipboardText(selected);
                        RemoveSelectedRange();

                        isCommand = true;
                    }
                    else if (keyEvent->key == KeyCode::C && IsTextSelected()) // Copy
                    {
                        String selected = GetSelectedText();
                        PlatformApplication::Get()->SetClipboardText(selected);

                        isCommand = true;
                    }
                    else if (keyEvent->key == KeyCode::V) // Paste
                    {
                        String textToPaste = "";
                        if (PlatformApplication::Get()->HasClipboardText())
                        {
                            textToPaste = PlatformApplication::Get()->GetClipboardText();
                            bool canInsert = true;
                            if (IsTextSelected() && CanRemoveSelectedText())
                            {
	                            RemoveSelectedRange();
                            }
                            else if (IsTextSelected())
                            {
                                canInsert = false;
                            }

                            if (canInsert && !CanInsertAt(textToPaste, cursorPos))
                            {
                                canInsert = false;
                            }

                            if (canInsert)
                            {
	                            InsertAt(textToPaste, cursorPos);
                            }
                        }

                        isCommand = true;
                    }
                    else if (keyEvent->key == KeyCode::A)
                    {
                        SelectAll();

                        isCommand = true;
                    }
                    else if (IsTextSelected() && CanRemoveSelectedText())
                    {
                        RemoveSelectedRange();
                    }
                }
                else if (IsTextSelected() && CanRemoveSelectedText())
                {
                    RemoveSelectedRange();
                }

                if (!isCommand)
                {
                    String str = "";
                    str.Append(c);

                    if (CanInsertAt(str, cursorPos))
                    {
                        cursorState = true;
                        cursorTimer->Reset();

                        InsertAt(str, cursorPos);
                        ScrollTo(cursorPos);
                    }
                }
            }
            else if ((int)keyEvent->key >= (int)KeyCode::KeypadDivide && (int)keyEvent->key <= (int)KeyCode::KeypadPeriod &&
                    keyEvent->key != KeyCode::KeypadEnter
#if PLATFORM_WINDOWS
                     && EnumHasFlag(keyEvent->modifiers, KeyModifier::Num)
#endif
                     )
            {
                switch (keyEvent->key)
                {
                case KeyCode::KeypadDivide:
                    c = '/';
                    break;
                case KeyCode::KeypadPlus:
                    c = '+';
                    break;
                case KeyCode::KeypadMinus:
                    c = '-';
                    break;
                case KeyCode::KeypadMultiply:
                    c = '*';
                    break;
                case KeyCode::Keypad0:
                    c = '0';
                    break;
                case KeyCode::Keypad1:
                    c = '1';
                    break;
                case KeyCode::Keypad2:
                    c = '2';
                    break;
                case KeyCode::Keypad3:
                    c = '3';
                    break;
                case KeyCode::Keypad4:
                    c = '4';
                    break;
                case KeyCode::Keypad5:
                    c = '5';
                    break;
                case KeyCode::Keypad6:
                    c = '6';
                    break;
                case KeyCode::Keypad7:
                    c = '7';
                    break;
                case KeyCode::Keypad8:
                    c = '8';
                    break;
                case KeyCode::Keypad9:
                    c = '9';
                    break;
                case KeyCode::KeypadPeriod:
                    c = '.';
                    break;
                }

                if (c != 0)
                {
                    if (IsTextSelected() && CanRemoveSelectedText())
                    {
                        cursorState = true;
                        cursorTimer->Reset();

                        RemoveSelectedRange();
                    }

                    String str = "";
                    str.Append(c);

                    if (CanInsertAt(str, cursorPos))
                    {
                        cursorState = true;
                        cursorTimer->Reset();

                        InsertAt(str, cursorPos);
                    }
                }
            }
            else if ((keyEvent->key == KeyCode::Backspace || keyEvent->key == KeyCode::Delete) 
                && IsTextSelected() && CanRemoveSelectedText())
            {
                cursorState = true;
                cursorTimer->Reset();

                RemoveSelectedRange();
            }
            else if (keyEvent->key == KeyCode::Backspace && CanRemoveRange(cursorPos - 1, 1))
            {
                cursorState = true;
                cursorTimer->Reset();

                RemoveRange(cursorPos - 1, 1);
            }
            else if (keyEvent->key == KeyCode::Delete && CanRemoveRange(cursorPos, 1))
            {
                cursorState = true;
                cursorTimer->Reset();

                RemoveRange(cursorPos, 1);
            }
            else if (keyEvent->key == KeyCode::Left)
            {
                int newCursorPos = cursorPos - 1;

                if (shiftPressed)
                {
                    int startIdx = selectionStart;
                    int endIdx = selectionEnd;
                    if (!IsTextSelected())
                    {
                        startIdx = cursorPos;
                        endIdx = cursorPos - 1;
                    }

                    if (IsTextSelected() && cursorPos > startIdx)
                    {
                        SelectRange(startIdx, endIdx - 1);
                        ScrollTo(endIdx - 1);
                    }
                    else
                    {
                        SelectRange(startIdx - 1, endIdx);
                        ScrollTo(startIdx - 1);

                        cursorPos = endIdx;
                    }
                }
                else
                {
	                if (IsTextSelected())
	                {
                        newCursorPos = selectionStart - 1;
                        DeselectAll();
	                }

                    ScrollTo(newCursorPos);
                }

                cursorState = true;
                cursorTimer->Reset();

                SetCursorPos(newCursorPos);
            }
            else if (keyEvent->key == KeyCode::Right)
            {
                int newCursorPos = cursorPos + 1;

                if (shiftPressed)
                {
                    int startIdx = selectionStart;
                    int endIdx = selectionEnd;
                    if (!IsTextSelected())
                    {
                        startIdx = cursorPos;
                        endIdx = cursorPos - 1;
                    }

                    if (IsTextSelected() && cursorPos <= endIdx && endIdx < characterOffsets.GetSize())
                    {
                        SelectRange(startIdx + 1, endIdx);
                        ScrollTo(startIdx + 1);
                    }
                    else
                    {
                        SelectRange(startIdx, endIdx + 1);
                        ScrollTo(endIdx + 1);
                    }

                    SetCursorPos(selectionEnd);
                }
                else
                {
                    if (IsTextSelected())
                    {
                        DeselectAll();
                    }

                    ScrollTo(newCursorPos);
                }

                cursorState = true;
                cursorTimer->Reset();

                SetCursorPos(newCursorPos);
            }
            else if (keyEvent->key == KeyCode::Return || keyEvent->key == KeyCode::KeypadEnter)
            {
                StopEditing(false);
                Unfocus();
            }
            else if (keyEvent->key == KeyCode::Escape)
            {
                StopEditing(true);
                textInput->Unfocus(); // Added: textInput->
            }
        }
        else if (event->type == FEventType::FocusChanged)
        {
            FFocusEvent* focusEvent = static_cast<FFocusEvent*>(event);

            if (focusEvent->LostFocus())
            {
                StopEditing();
            }
        }

	    Super::HandleEvent(event);
    }

    void FTextInputLabel::RecalculateCharacterOffsets()
    {
        Ref<FFusionContext> context = GetContext();
        if (!context)
            return;

        FPainter* painter = context->GetPainter();
        if (!painter)
            return;

        textSize = painter->CalculateCharacterOffsets(characterOffsets, Text(), Font());
    }

    void FTextInputLabel::OnTimeOut()
    {
        cursorState = !cursorState;
        MarkDirty();
    }

    void FTextInputLabel::OnMouseClick(Vec2 localPos, bool isDoubleClick)
    {
        if (characterOffsets.IsEmpty())
        {
            cursorState = true;
            cursorTimer->Reset();
            cursorPos = 0;

            if (!IsEditing())
            {
                StartEditing();
            }
            else
            {
                MarkDirty();
            }

	        return;
        }

	    for (int i = 0; i < characterOffsets.GetSize(); ++i)
	    {
            if ((i > 0 && localPos.x < characterOffsets[i].min) || (i < characterOffsets.GetSize() - 1 && localPos.x > characterOffsets[i].max))
                continue;

            f32 halfWayPos = (characterOffsets[i].min + characterOffsets[i].max) / 2.0f;
            if (localPos.x <= halfWayPos)
                cursorPos = i;
            else
                cursorPos = i + 1;

            cursorState = true;
            cursorTimer->Reset();

            if (!IsEditing())
            {
                StartEditing();
            }
            else
            {
                MarkDirty();
            }

            if (isDoubleClick)
            {
                SelectAll();
                ScrollTo(characterOffsets.GetSize());
            }
            else if (IsTextSelected())
            {
                DeselectAll();
            }
            
            break;
	    }
    }

    Range FTextInputLabel::GetCharacterMinMax(int characterIndex)
    {
        if (characterOffsets.IsEmpty())
        {
            return Range(0, 0);
        }

        if (characterIndex < characterOffsets.GetSize())
        {
            return Range(characterOffsets[characterIndex].min, characterOffsets[characterIndex].max);
        }

        f32 lastWidth = characterOffsets.Top().max - characterOffsets.Top().min;
        return Range(characterOffsets.Top().max, characterOffsets.Top().max + lastWidth);
    }

    bool FTextInputLabel::IsEditing()
    {
        return textInput->IsEditing();
    }

    void FTextInputLabel::StartEditing()
    {
        if (!textInput->IsEditing())
        {
            textInput->m_OnBeginEditing(textInput);
        }

        originalText = m_Text;

        cursorTimer->Reset();
        cursorState = true;
        cursorTimer->Start(500);

        textInput->SetEditingInternal(true);
        MarkDirty();
    }

    void FTextInputLabel::StopEditing(bool restoreOriginal)
    {
        if (!textInput->IsEditing())
            return;

        if (restoreOriginal)
        {
            m_Text = originalText;
        }

        textInput->OnFinishEdit();

        cursorTimer->Stop();
        cursorState = false;
        
        textInput->SetEditingInternal(false);
        ScrollTo(0);
        DeselectAll();

        MarkLayoutDirty();

        if (!restoreOriginal && m_TextBinding.write.IsBound())
        {
            m_TextBinding.write.Invoke(m_Text, this);
        }

        textInput->m_OnTextEditingFinished(textInput);
    }

    void FTextInputLabel::SetCursorPos(int newCursorPos)
    {
        cursorPos = newCursorPos;
        cursorPos = Math::Clamp<int>(cursorPos, 0, characterOffsets.GetSize());

        MarkDirty();
    }

    void FTextInputLabel::ScrollTo(int charIndex)
    {
        charIndex = Math::Clamp<int>(charIndex, 0, characterOffsets.GetSize());

        Range charRange = GetCharacterMinMax(charIndex);

        if (charRange.min - textScrollOffset < 0)
        {
            textScrollOffset = charRange.min;

            MarkDirty();
        }
        else if (charRange.max - textScrollOffset > computedSize.width)
        {
            textScrollOffset = charRange.max - computedSize.width;

            MarkDirty();
        }
    }

    void FTextInputLabel::InsertAt(const String& string, int insertPos)
    {
        if (insertPos < 0 || insertPos > m_Text.GetLength())
            return;



        String newText = m_Text;
        newText.Reserve(newText.GetLength() + string.GetLength());
        for (int i = 0; i < string.GetLength(); ++i)
        {
            newText.InsertAt(string[i], insertPos + i);
        }

        m_Text = newText;
        cursorPos = insertPos + string.GetLength();

        MarkLayoutDirty();

        textInput->m_OnTextEdited(textInput);
    }

    void FTextInputLabel::InsertAt(char character, int insertPos)
    {
        if (insertPos < 0 || insertPos > m_Text.GetLength())
            return;

        String newText = m_Text;
        newText.Reserve(newText.GetLength() + 1);
        newText.InsertAt(character, insertPos);

        m_Text = newText;
        cursorPos = insertPos + 1;

        MarkLayoutDirty();

        textInput->m_OnTextEdited(textInput);
    }

    bool FTextInputLabel::CanInsertAt(const String& string, int insertPos)
    {
        if (!textInput->m_Validator.IsValid())
            return true;

        String text = Text();
        for (int i = 0; i < string.GetLength(); ++i)
        {
            text.InsertAt(string[i], insertPos + i);
        }

        return textInput->m_Validator(text);
    }

    void FTextInputLabel::RemoveRange(int startIndex, int count)
    {
        if (startIndex < 0 || startIndex > m_Text.GetLength())
            return;

        count = Math::Min<int>(count, m_Text.GetLength() - startIndex);

        String newText = m_Text;
        newText.Remove(startIndex, count);

        m_Text = newText;

        SetCursorPos(startIndex);

        MarkLayoutDirty();

        textInput->m_OnTextEdited(textInput);
    }

    void FTextInputLabel::RemoveSelectedRange()
    {
        if (IsTextSelected() && selectionStart <= selectionEnd)
        {
            RemoveRange(selectionStart, selectionEnd - selectionStart + 1);
            DeselectAll();

            ScrollTo(selectionStart);
        }
    }

    bool FTextInputLabel::CanRemoveSelectedText()
    {
        if (!textInput->m_Validator.IsValid() || !IsTextSelected())
            return true;

        String text = Text();
        text.Remove(selectionStart, selectionEnd - selectionStart + 1);

        return textInput->m_Validator(text);
    }

    bool FTextInputLabel::CanRemoveRange(int startIndex, int count)
    {
        if (!textInput->m_Validator.IsValid())
            return true;

        String text = Text();
        if (startIndex >= text.GetLength() || (count + startIndex) > text.GetLength())
            return true;

        text.Remove(startIndex, count);

        return textInput->m_Validator(text);
    }

    String FTextInputLabel::GetSelectedText()
    {
        if (!IsTextSelected())
        {
            return "";
        }

        return m_Text.GetSubstring(selectionStart, selectionEnd - selectionStart + 1);
    }

    void FTextInputLabel::SelectAll()
    {
        if (characterOffsets.IsEmpty())
        {
            DeselectAll();
	        return;
        }

        SelectRange(0, characterOffsets.GetSize() - 1);
    }

    void FTextInputLabel::SelectRange(int startIndex, int endIndex)
    {
        if (characterOffsets.IsEmpty())
        {
            DeselectAll();
            return;
        }

        isSelectionActive = true;

        selectionStart = Math::Max(0, startIndex);
        selectionEnd = Math::Min<int>(endIndex, characterOffsets.GetSize() - 1);
        MarkDirty();
    }

    void FTextInputLabel::DeselectAll()
    {
    	isSelectionActive = false;
        MarkDirty();
    }

    FTextInput::FTextInput()
    {
        m_Padding = Vec4(7.5f, 5, 7.5f, 5);
    }

    void FTextInput::StartEditing(bool selectAll, bool cursorAtEnd)
    {
        if (!IsEditing())
        {
            inputLabel->cursorPos = 0;
            if (cursorAtEnd)
            {
                inputLabel->cursorPos = inputLabel->Text().GetLength();
			}
            inputLabel->StartEditing();

            FusionApplication::Get()->GetRootContext()->SetFocusWidget(inputLabel);

            if (selectAll)
            {
                inputLabel->SelectAll();
            }
        }
    }

    void FTextInput::StopEditing(bool restoreOriginal)
    {
        inputLabel->StopEditing(restoreOriginal);
        Unfocus();
    }

    void FTextInput::OnPaintContent(FPainter* painter)
    {
        Super::OnPaintContent(painter);

        m_OnBeforeTextPaint(painter);
    }

    void FTextInput::OnLostFocus()
    {
	    Super::OnLostFocus();

        inputLabel->StopEditing();
    }

    void FTextInput::Construct()
    {
	    Super::Construct();
        
        Child(
            FAssignNew(FHorizontalStack, contentStack)
            .ContentVAlign(VAlign::Center)
            (
                FNew(FStyledWidget)
                .ClipShape(FRectangle())
                .FillRatio(1.0f)
                (
                    FAssignNew(FTextInputLabel, inputLabel)
                    .Text("")
                    .WordWrap(FWordWrap::NoWrap)
                    .FontSize(10)
                    .Foreground(Colors::White)
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Fill)
                )
            )
        );

        inputLabel->textInput = this;
    }

    void FTextInput::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        thread_local const CE::Name validator = "Validator";
        thread_local const CE::Name text = "Text";

        if (propertyName == validator)
        {
            if (m_Validator.IsValid() && !Text().IsEmpty() && !m_Validator(Text()))
            {
                Text("");
            }
        }
        else if (propertyName == text && inputLabel->IsEditing())
        {
            inputLabel->StopEditing();
        }
    }

    void FTextInput::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && event->sender == this && !event->isConsumed)
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);
            auto app = FusionApplication::Get();
            
            if (mouseEvent->type == FEventType::MouseEnter)
            {
                SetHoveredInternal(true);
                app->PushCursor(SystemCursor::IBeam);
                isCursorPushed = true;
            }
            else if (mouseEvent->type == FEventType::MouseLeave)
            {
                isCursorPushed = false;
                SetHoveredInternal(false);
                app->PopCursor();
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left)
            {
                Vec2 localPos = mouseEvent->mousePosition - inputLabel->globalPosition;
                inputLabel->OnMouseClick(localPos, mouseEvent->isDoubleClick);
            }
        }
        else if (event->IsKeyEvent() && event->sender == this && !event->isConsumed)
        {
            // Pass-through keyboard events directly to the child widget (FTextInputLabel)
            inputLabel->HandleEvent(event);
            return;
        }

	    Super::HandleEvent(event);
    }

    void FTextInput::SetHoveredInternal(bool hovered)
    {
	    if (IsHovered() != hovered)
	    {
            if (hovered)
                state |= FTextInputState::Hovered;
            else
                state &= ~FTextInputState::Hovered;

            ApplyStyle();
	    }
    }

    void FTextInput::SetHighlightedInternal(bool highlighted)
    {
    	if (IsHighlighted() != highlighted)
	    {
            if (highlighted)
                state |= FTextInputState::Highlighted;
            else
                state &= ~FTextInputState::Highlighted;

            ApplyStyle();
	    }
    }

    void FTextInput::SetEditingInternal(bool editing)
    {
        if (IsEditing() == editing)
            return;

        if (editing)
        {
            state |= FTextInputState::Editing;
        }
        else
        {
            state &= ~FTextInputState::Editing;
        }

        ApplyStyle();

    }

    FTextInput& FTextInput::LeftSlot(FWidget& content)
    {
        contentStack->InsertChild(0, &content);
        return *this;
    }

} // namespace CE
