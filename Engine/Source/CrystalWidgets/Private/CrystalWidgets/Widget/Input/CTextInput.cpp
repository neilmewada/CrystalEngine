#include "CrystalWidgets.h"

namespace CE::Widgets
{

    CTextInput::CTextInput()
    {
        receiveMouseEvents = true;
        receiveDragEvents = true;
        receiveKeyEvents = true;
        allowVerticalScroll = allowHorizontalScroll = false;
        interactable = true;

        timer = CreateDefaultSubobject<CTimer>("Timer");
        if (!IsDefaultInstance())
        {
            timer->onTimeOut += FUNCTION_BINDING(this, OnTimerTick);
        }
    }

    CTextInput::~CTextInput()
    {
	    
    }

    Vec2 CTextInput::CalculateIntrinsicSize(f32 width, f32 height)
    {
        String display = GetDisplayText();
        if (display.IsEmpty())
            display = " ";

        Name fontName = computedStyle.properties[CStylePropertyType::FontName].string;

        f32 fontSize = 14;
        if (computedStyle.properties.KeyExists(CStylePropertyType::FontSize))
            fontSize = computedStyle.properties[CStylePropertyType::FontSize].single;

        if (fontSize < 8)
            fontSize = 8;
        return CalculateTextSize(display, fontSize, fontName, 0);
    }

    bool CTextInput::SetText(const String& value)
    {
        if (inputValidator.IsValid() && !inputValidator.Invoke(value))
        {
            return false;
        }

        this->text = value;
        OnValidateText();

        RecalculateOffsets();

        onTextChanged.Broadcast(this);

        SetNeedsLayout();
        SetNeedsPaint();

        return true;
    }

    bool CTextInput::SetTextInternal(const String& value)
    {
        if (inputValidator.IsValid() && !inputValidator.Invoke(value))
        {
            return false;
        }

        this->text = value;

        RecalculateOffsets();

        onTextEdited.Broadcast(this);
        onTextChanged.Broadcast(this);

        SetNeedsLayout();
        SetNeedsPaint();

        return true;
    }

    void CTextInput::SetHint(const String& hint)
    {
        this->hint = hint;

        SetNeedsPaint();
    }

    void CTextInput::SetAsPassword(b8 set)
    {
        if (isPassword == set)
            return;

        isPassword = set;

        SetNeedsLayout();
        SetNeedsPaint();
    }

    void CTextInput::StopEditing(bool restoreOriginal)
    {
        if (restoreOriginal)
        {
            text = originalText;
            isEditing = false;

            stateFlags &= ~CStateFlag::Active;
            SetNeedsStyle();
            SetNeedsPaint();
        }

        Unfocus();
    }

    bool CTextInput::IsTextSelected()
    {
        return isSelectionActive && selectionRange.min >= 0 && selectionRange.max >= 0 && selectionRange.min <= selectionRange.max;
    }

    void CTextInput::OnTimerTick()
    {
        cursorState = !cursorState;
        if (isCursorMoving)
        {
            cursorState = true;
            isCursorMoving = false;
        }
        SetNeedsPaint();
    }

    String CTextInput::GetDisplayText()
    {
        if (!isPassword)
        {
            return text;
        }

        String display = "";

        for (int i = 0; i < text.GetLength(); ++i)
        {
            display.Append('*');
        }

        return display;
    }

    void CTextInput::OnFocusGained()
    {
        Super::OnFocusGained();
    }

    void CTextInput::OnFocusLost()
    {
        Super::OnFocusLost();

        DeselectAll();

        if (isEditing)
        {
	        textScrollOffset = 0;
        	timer->Stop();
        	isEditing = false;
        	cursorState = false;

            stateFlags &= ~CStateFlag::Active;
            SetNeedsStyle();
            SetNeedsPaint();

            onEditingFinished.Broadcast(this);
	        OnValidateText();
        }
    }

    void CTextInput::OnDisabled()
    {
	    Super::OnDisabled();

        DeselectAll();

        if (isEditing)
        {
            textScrollOffset = 0;
            timer->Stop();
            text = originalText;
            isEditing = false;
            cursorState = false;

            stateFlags &= ~CStateFlag::Active;
            SetNeedsStyle();
            SetNeedsPaint();

            onEditingFinished.Broadcast(this);
            OnValidateText();
        }
    }

    bool CTextInput::RecalculateOffsets()
    {
        Renderer2D* renderer = GetRenderer();

        if (renderer)
        {
            Name fontName = computedStyle.properties[CStylePropertyType::FontName].string;

            f32 fontSize = 14;
            if (computedStyle.properties.KeyExists(CStylePropertyType::FontSize))
                fontSize = computedStyle.properties[CStylePropertyType::FontSize].single;

            Vec2 oldTextSize = textSize;
            textSize = renderer->CalculateTextOffsets(characterOffsets, GetDisplayText(), fontSize, fontName);

            return textSize != oldTextSize;
        }

        return false;
    }

    void CTextInput::SetCursorPos(int cursorPos)
    {
        cursorPos = Math::Clamp(cursorPos, 0, (int)characterOffsets.GetSize());

        if (this->cursorPos != cursorPos)
        {
            SetNeedsPaint();
        }

        this->cursorPos = cursorPos;

        Vec4 padding = GetComputedLayoutPadding();

        Rect contentRect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
        contentRect.min += padding.min;
        contentRect.max -= padding.max;

        Rect characterRect = {};
        if (cursorPos < characterOffsets.GetSize())
        {
            characterRect = characterOffsets[cursorPos].Translate(padding.min - Vec2(textScrollOffset, 0));
        }
        else if (!characterOffsets.IsEmpty())
        {
            characterRect = characterOffsets.Top().Translate(padding.min - Vec2(textScrollOffset, 0) + Vec2(characterOffsets.Top().GetSize().width, 0));
        }
        else
        {
            textScrollOffset = 0;
	        return;
        }

        ScrollTo(cursorPos);
    }

    void CTextInput::ScrollTo(int cursorPos)
    {
        Vec4 padding = GetComputedLayoutPadding();
        SetNeedsPaint();

        Rect contentRect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
        contentRect.min += padding.min;
        contentRect.max -= padding.max;

        if (characterOffsets.IsEmpty())
        {
            textScrollOffset = 0;
	        return;
        }

        Rect characterRect = {};
        Rect scrolledPosition = {};
        Rect prevCharacterRect = {};
        Rect prevCharScrolledPos = {};

        if (cursorPos < characterOffsets.GetSize())
        {
            characterRect = characterOffsets[cursorPos].Translate(padding.min);
            scrolledPosition = characterOffsets[cursorPos].Translate(padding.min - Vec2(textScrollOffset, 0));
            prevCharacterRect = cursorPos > 0 ? characterOffsets[cursorPos - 1].Translate(padding.min) : characterRect;
            prevCharScrolledPos = cursorPos > 0
        		? characterOffsets[cursorPos - 1].Translate(padding.min - Vec2(textScrollOffset, 0))
        		: scrolledPosition;
        }
        else
        {
            characterRect = characterOffsets.Top().Translate(padding.min + Vec2(characterOffsets.Top().GetSize().width, 0));
            scrolledPosition = characterOffsets.Top().Translate(padding.min + Vec2(characterOffsets.Top().GetSize().width - textScrollOffset, 0));
            prevCharacterRect = characterRect;
            prevCharScrolledPos = scrolledPosition;
        }

        if (scrolledPosition.min.x > contentRect.GetSize().width)
        {
            textScrollOffset = Math::Clamp(characterRect.min.x - contentRect.GetSize().width,
                0.0f,
                Math::Max(0.0f, textSize.width - contentRect.GetSize().width));
        }
        else if (prevCharScrolledPos.min.x <= 0)
        {
            textScrollOffset = Math::Clamp(prevCharacterRect.min.x - padding.min.x,
                0.0f,
                Math::Max(0.0f, textSize.width - contentRect.GetSize().width));
        }
    }

    void CTextInput::InsertAt(const String& string, int insertPos)
    {
        if (insertPos < 0 || insertPos > this->text.GetLength())
            return;

        String newText = text;
        newText.Reserve(newText.GetLength() + string.GetLength());
        for (int i = 0; i < string.GetLength(); ++i)
        {
            newText.InsertAt(string[i], insertPos + i);
        }

        bool success = SetTextInternal(newText);
        if (!success)
            return;
        
        SetCursorPos(insertPos + string.GetLength());
    }

    void CTextInput::RemoveRange(int startIndex, int count)
    {
        if (count <= 0 || startIndex < 0 || startIndex >= text.GetLength())
            return;

        String newText = text;
        newText.Remove(startIndex, count);

        bool success = SetTextInternal(newText);
        if (!success)
            return;

        SetCursorPos(startIndex);
    }

    void CTextInput::RemoveSelected()
    {
        if (!IsTextSelected())
            return;

        RemoveRange(selectionRange.min, selectionRange.max - selectionRange.min + 1);
        DeselectAll();
    }

    void CTextInput::SelectRange(int startIndex, int endIndex)
    {
        startIndex = Math::Max(0, startIndex);
        endIndex = Math::Max(0, endIndex);

        isSelectionActive = true;
        selectionRange = RangeInt(startIndex, endIndex);
    }

    void CTextInput::SelectAll()
    {
        SelectRange(0, characterOffsets.GetSize() - 1);
        ScrollTo(characterOffsets.GetSize());
        SetNeedsPaint();
    }

    String CTextInput::GetSelectedText()
    {
        if (!IsTextSelected())
            return  "";

        return text.GetSubstring(selectionRange.min, selectionRange.max - selectionRange.min + 1);
    }

    void CTextInput::DeselectAll()
    {
        isSelectionActive = false;
        selectionRange.min = selectionRange.max = -1;
    }

    void CTextInput::StartEditing()
    {
        Focus();

        DeselectAll();
        
        timer->Reset();
        timer->Start(cursorBlinkMillis);
        cursorState = true;
        isEditing = true;
        originalText = text;
        stateFlags |= CStateFlag::Active;

        RecalculateOffsets();

        SetCursorPos(0);

        if (selectAllOnEdit)
        {
            SelectAll();
        }

        SetNeedsStyle();
        SetNeedsPaint();
    }

    void CTextInput::HandleEvent(CEvent* event)
    {
        Name fontName = computedStyle.properties[CStylePropertyType::FontName].string;

        f32 fontSize = 14;
        if (computedStyle.properties.KeyExists(CStylePropertyType::FontSize))
            fontSize = computedStyle.properties[CStylePropertyType::FontSize].single;

        if (event->IsMouseEvent() && !event->isConsumed)
        {
            CMouseEvent* mouseEvent = (CMouseEvent*)event;
            Vec2 screenMousePos = mouseEvent->mousePos;
            Vec2 mouseDelta = mouseEvent->mousePos - mouseEvent->prevMousePos;
            Vec2 localMousePos = ScreenToLocalSpacePoint(screenMousePos);
            Vec4 padding = GetComputedLayoutPadding();

            Renderer2D* renderer = GetRenderer();
            String display = GetDisplayText();

            if (renderer && mouseEvent->type == CEventType::MouseEnter)
            {
                
            }
            else if (renderer && mouseEvent->type == CEventType::MouseLeave)
            {
                
            }
            else if (renderer && mouseEvent->type == CEventType::MousePress && mouseEvent->button == MouseButton::Left)
            {
                int selectedIdx = -1;

                event->Consume(this);

                DeselectAll();

                if (mouseEvent->isDoubleClick && !characterOffsets.IsEmpty() && IsEditing())
                {
                    SelectRange(0, characterOffsets.GetSize() - 1);
                    ScrollTo(characterOffsets.GetSize());
                    return;
                }

                for (int i = 0; i < characterOffsets.GetSize(); ++i)
                {
                    Rect localPos = characterOffsets[i].Translate(padding.min - Vec2(textScrollOffset, 0));

                    if (localMousePos.x < localPos.left)
                    {
                        selectedIdx = i;
	                    break;
                    }
                }

                if (selectedIdx == -1 && characterOffsets.NonEmpty())
                {
                    Rect localPos = characterOffsets.Top().Translate(padding.min - Vec2(textScrollOffset, 0));

                    if (localMousePos.x > localPos.max.x)
                    {
                        selectedIdx = characterOffsets.GetSize();
                    }
                }
                else if (selectedIdx == -1)
                {
                    selectedIdx = 0;
                }

                if (selectedIdx != -1)
                {
                    timer->Reset();
                    timer->Start(cursorBlinkMillis);
                    cursorState = true;
                    isEditing = true;
                    originalText = text;
                    stateFlags |= CStateFlag::Active;

                    RecalculateOffsets();

                    SetCursorPos(selectedIdx);

                    if (selectAllOnEdit)
                    {
                        SelectAll();
                    }

                    SetNeedsStyle();
                    SetNeedsPaint();
                }
            }
            else if (renderer && mouseEvent->type == CEventType::DragBegin && mouseEvent->button == MouseButton::Left && IsEditable() &&
                cursorPos >= 0 && cursorPos < display.GetLength())
            {
                CDragEvent* dragEvent = static_cast<CDragEvent*>(mouseEvent);

                dragEvent->Consume(this);

                dragEvent->draggedWidget = this;

                selectionDistance = 0;
            }
            else if (renderer && mouseEvent->type == CEventType::DragMove && mouseEvent->button == MouseButton::Left && IsEditable())
            {
                CDragEvent* dragEvent = static_cast<CDragEvent*>(mouseEvent);

                dragEvent->Consume(this);

                Rect contentRect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
                contentRect.min += padding.min;
                contentRect.max -= padding.max;

                selectionDistance += mouseDelta.x;
                int selectionIndex = -1;

                RangeInt range = RangeInt(-1, -1);
                f32 prevCenterPoint = -NumericLimits<f32>::Infinity();

                for (int i = 0; i < characterOffsets.GetSize(); ++i)
                {
                    Rect characterRect = characterOffsets[i].Translate(padding.min - Vec2(textScrollOffset, 0));
                    Rect cursorCharacterRect = characterOffsets[cursorPos].Translate(padding.min - Vec2(textScrollOffset, 0));
                    f32 centerPoint = (characterRect.min.x + characterRect.max.x) / 2.0f;

                    if (selectionDistance > 0)
                    {
                        if (cursorCharacterRect.min.x + selectionDistance > (characterRect.min.x + characterRect.max.x) * 0.5f)
                        {
                            selectionIndex = i;
                            range.max = i;
                        }
                    }
                    else if (selectionDistance < 0)
                    {
	                    if (cursorCharacterRect.min.x + selectionDistance > prevCenterPoint)
	                    {
                            selectionIndex = i;
                            range.min = i;
	                    }
                    }

                    prevCenterPoint = centerPoint;
                }

                if (localMousePos.x > contentRect.max.x + padding.left + padding.right)
                {
                    SelectRange(cursorPos, characterOffsets.GetSize() - 1);
                    ScrollTo(characterOffsets.GetSize());
                }
                else if (localMousePos.x < contentRect.min.x + padding.left + padding.right)
                {
                    SelectRange(0, cursorPos);
                    ScrollTo(0);
                }
                else if (selectionDistance > 0)
                {
                    SelectRange(cursorPos, selectionIndex);
                }
                else if (selectionDistance < 0)
                {
                    SelectRange(selectionIndex, cursorPos - 1);
                }

                SetNeedsPaint();
            }
        }
        else if (!event->isConsumed && (event->type == CEventType::KeyHeld || event->type == CEventType::KeyPress))
        {
            Renderer2D* renderer = GetRenderer();
            
            CKeyEvent* keyEvent = static_cast<CKeyEvent*>(event);
            event->Consume(this);

        	bool shiftPressed = EnumHasAnyFlags(keyEvent->modifier, KeyModifier::LShift | KeyModifier::RShift);
            bool capslock = EnumHasAnyFlags(keyEvent->modifier, KeyModifier::Caps);
            bool ctrl = EnumHasFlag(keyEvent->modifier, KeyModifier::Ctrl);

            bool isUpperCase = shiftPressed != capslock;

            char c = 0;
            Vec4 padding = GetComputedLayoutPadding();

            Rect contentRect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
            contentRect.min += padding.min;
            contentRect.max -= padding.max;
            String display = GetDisplayText();

            // A valid text character
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
                        RemoveSelected();

                        isCommand = true;
	                }
                    else if (keyEvent->key == KeyCode::C && IsTextSelected()) // Copy
                    {
                        String selected = GetSelectedText();
                        PlatformApplication::Get()->SetClipboardText(selected);

                        isCommand = true;
                    }
                    else if (keyEvent->key == KeyCode::V)
                    {
                        String textToPaste = "";
                        if (PlatformApplication::Get()->HasClipboardText())
                        {
	                        textToPaste = PlatformApplication::Get()->GetClipboardText();
                            if (IsTextSelected())
                        		RemoveSelected();
                            InsertAt(textToPaste, cursorPos);
                        }

                        isCommand = true;
                    }
                    else if (keyEvent->key == KeyCode::A)
                    {
                        SelectAll();

                        isCommand = true;
                    }
                    else if (IsTextSelected())
                    {
                        RemoveSelected();
                    }
                }
                else if (IsTextSelected())
                {
                    RemoveSelected();
                }

                if (!isCommand)
                {
                    String str = "";
                    str.Append(c);

                    InsertAt(str, cursorPos);
                }
            }
            else if ((int)keyEvent->key >= (int)KeyCode::KeypadDivide && (int)keyEvent->key <= (int)KeyCode::KeypadPeriod &&
                keyEvent->key != KeyCode::KeypadEnter
                && IsEditing() &&
                EnumHasFlag(keyEvent->modifier, KeyModifier::Num))
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
                    if (IsTextSelected())
                    {
                        RemoveSelected();
                    }

                    String str = "";
                    str.Append(c);

                    InsertAt(str, cursorPos);
                }
            }
            else if (keyEvent->key == KeyCode::Backspace && cursorPos > 0 && !IsTextSelected())
            {
                RemoveRange(cursorPos - 1, 1);
            }
            else if (keyEvent->key == KeyCode::Delete && cursorPos < display.GetLength() && !IsTextSelected())
            {
                RemoveRange(cursorPos, 1);
            }
            else if ((keyEvent->key == KeyCode::Backspace || keyEvent->key == KeyCode::Delete) && IsTextSelected()) // A text selection got removed
            {
                RemoveSelected();
            }
            else if (keyEvent->key == KeyCode::Left)
            {
                isCursorMoving = true;
                int newCursorPos = cursorPos - 1;

                if (shiftPressed)
                {
                    int startIdx = selectionRange.min;
                    int endIdx = selectionRange.max;
                    if (!IsTextSelected())
                    {
                        startIdx = cursorPos;
                        endIdx = cursorPos - 1;
                    }
                    SelectRange(startIdx - 1, endIdx);
                }
                else
                {
					if (IsTextSelected())
					{
                        newCursorPos = selectionRange.min;
					}
                    DeselectAll();
                }

                SetCursorPos(newCursorPos);
            }
            else if (keyEvent->key == KeyCode::Right)
            {
                isCursorMoving = true;
                int newCursorPos = cursorPos + 1;

                if (shiftPressed)
                {
                    int startIdx = selectionRange.min;
                    int endIdx = selectionRange.max;
                    if (!IsTextSelected())
                    {
                        startIdx = cursorPos;
                        endIdx = cursorPos - 1;
                    }
                    SelectRange(startIdx, endIdx + 1);
                }
                else
                {
                    if (IsTextSelected())
                    {
                        newCursorPos = selectionRange.max + 1;
                    }
                    DeselectAll();
                }

                SetCursorPos(newCursorPos);
            }
            else if (keyEvent->key == KeyCode::Return || keyEvent->key == KeyCode::KeypadEnter)
            {
                Unfocus();
            }
            else if (keyEvent->key == KeyCode::Escape)
            {
                text = originalText;
                isEditing = false;

                stateFlags &= ~CStateFlag::Active;
                SetNeedsStyle();
                SetNeedsPaint();

                Unfocus();
            }
        }

        Super::HandleEvent(event);
    }

    Vec2 CTextInput::GetComputedLayoutSize()
    {
        Vec2 size = Super::GetComputedLayoutSize();
        return size;
    }

    void CTextInput::OnPaint(CPaintEvent* paintEvent)
    {
        Super::OnPaint(paintEvent);

        String display = GetDisplayText();
        
        CPainter* painter = paintEvent->painter;

        Name fontName = computedStyle.properties[CStylePropertyType::FontName].string;

        f32 fontSize = 14;
        if (computedStyle.properties.KeyExists(CStylePropertyType::FontSize))
            fontSize = computedStyle.properties[CStylePropertyType::FontSize].single;

        Color color = Colors::White;
        Color textSelectionColor = Color::RGBA8(0, 112, 224);

        if (computedStyle.properties.KeyExists(CStylePropertyType::Foreground))
            color = computedStyle.properties[CStylePropertyType::Foreground].color;

        CFont font = CFont(fontName, (u32)fontSize, false);
        CPen pen = CPen(color);
        CBrush brush = CBrush();
        
        painter->SetFont(font);
        painter->SetPen(pen);

        Rect rect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
        Vec4 padding = GetComputedLayoutPadding();
        Vec2 usableSize = rect.GetSize();
        usableSize.y -= padding.top + padding.bottom;
        usableSize.x -= padding.left + padding.right;

        Rect contentRect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
        contentRect.min += padding.min;
        contentRect.max -= padding.max;

        if (characterOffsets.IsEmpty())
        {
            textSize = painter->CalculateTextSize(" ");
        }

        Rect textRect = rect.Translate(Vec2(padding.left - textScrollOffset, padding.top + usableSize.height / 2 - textSize.height / 2));
        textRect.max -= Vec2(padding.left + padding.right, usableSize.height / 2 - textSize.height / 2);

        cursorPos = Math::Clamp(cursorPos, 0, (int)text.GetLength());

        selectionRange.min = Math::Max(0, selectionRange.min);
        selectionRange.max = Math::Min(selectionRange.max, (int)characterOffsets.GetSize() - 1);

        //bool pushClipRect = characterOffsets.NonEmpty();

        // Add text scroll offset to negate its effect from Clip Rect
        
		painter->PushClipRect(textRect.Translate(Vec2(textScrollOffset, 0)));
        painter->PushChildCoordinateSpace(rect.min);
        {
            if (characterOffsets.IsEmpty() && hint.NonEmpty())
            {
                painter->SetPen(CPen(Color::RGBA(255, 255, 255, 60)));
                painter->DrawText(hint, textRect.min - rect.min);
            }

            painter->SetPen(pen);

            if (isEditing) // Edit mode
            {
                // Draw text selection background
                if (IsTextSelected())
                {
                    Vec2 selectionStart = characterOffsets[selectionRange.min].Translate(padding.min - Vec2(textScrollOffset, 0)).min;
                    Vec2 selectionEnd = characterOffsets[selectionRange.max].Translate(padding.min - Vec2(textScrollOffset, 0)).min +
                        Vec2(characterOffsets[selectionRange.max].GetSize().width, textSize.height);

                    brush.SetColor(textSelectionColor);
                    painter->SetBrush(brush);

                    painter->DrawRect(Rect(selectionStart, selectionEnd));
                }

                // Draw text

                painter->DrawText(display, textRect.min - rect.min);

                // Draw cursor

                constexpr f32 cursorWidth = 2.0f;
                Rect cursorRect = Rect();
                if (cursorPos >= 0 && cursorPos < characterOffsets.GetSize())
                {
	                cursorRect = characterOffsets[cursorPos].Translate(padding.min - Vec2(textScrollOffset, 0) - Vec2(cursorWidth / 2.0f, 0));
                }
                else if (cursorPos == characterOffsets.GetSize() && characterOffsets.NonEmpty())
                {
	                cursorRect = characterOffsets.Top().Translate(padding.min - Vec2(textScrollOffset, 0) - Vec2(cursorWidth / 2.0f, 0) +
					   Vec2(characterOffsets.Top().GetSize().width, 0));
                }
                else if (characterOffsets.IsEmpty())
                {
                    cursorPos = 0;
                    cursorRect.min = (padding.min - Vec2(textScrollOffset, 0) - Vec2(cursorWidth / 2.0f, 0));
                }
                cursorRect.max.x = cursorRect.min.x + cursorWidth;
                cursorRect.max.y = cursorRect.min.y + textSize.height;
                if (characterOffsets.IsEmpty())
                {
                    Vec2 size = painter->CalculateTextSize(" ");
                    cursorRect.max.y = cursorRect.min.y + size.height;
                }
                
                if (cursorState)
                {
                    brush.SetColor(Colors::White);
                    painter->SetBrush(brush);

                    painter->DrawRect(cursorRect);
                }
            }
            else
            {
                painter->DrawText(display, textRect.min - rect.min);
            }
        }

        painter->PopChildCoordinateSpace();
		painter->PopClipRect();
    }

} // namespace CE::Widgets
