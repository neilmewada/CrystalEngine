#include "EditorCore.h"

namespace CE::Editor
{
    static constexpr f32 MaxScrollDist = 512;

    NumericEditorField::NumericEditorField()
    {
        m_ScrollAmount = 0.5f;
    }

    void NumericEditorField::Construct()
    {
        Super::Construct();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        Child(
			FAssignNew(FTextInput, input)
            .FontSize(fontSize)
            .OnTextEditingFinished(FUNCTION_BINDING(this, OnFinishEdit))
            .OnTextEdited(FUNCTION_BINDING(this, OnTextFieldEdited))
            .OnBeforeTextPaint(FUNCTION_BINDING(this, OnPaintBeforeText))
            .OnBeginEditing(FUNCTION_BINDING(this, OnBeginEdit))
            .Padding(Vec4(11, 4, 8, 4))
        );

        NumericType<f32>();
        input->Text("0");
    }

    EditorField& NumericEditorField::FixedInputWidth(f32 width)
    {
        input->Width(width);
        return *this;
    }

#define FIELD_START_VALUE_IF(type)\
	if (numericType == TYPEID(type))\
	{\
	    type value = 0;\
	    if (String::TryParse(input->Text(), value))\
	    {\
	        startValue = (f64)value;\
	    }\
	}

#define FIELD_DRAG_IF(type)\
	if (numericType == TYPEID(type))\
	{\
	    type value = 0;\
	    if (String::TryParse(input->Text(), value))\
	    {\
	        value = (type)(startValue + amount);\
            if (isRanged) { value = Math::Clamp<type>(value, (type)min, (type)max); }\
	        input->Text(String::Format("{}", value));\
	    }\
	}

    void NumericEditorField::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && event->sender == this)
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (event->type == FEventType::MouseEnter)
            {
                if (!isCursorPushed)
	            {
		            isCursorPushed = true;
	            	FusionApplication::Get()->PushCursor(SystemCursor::SizeHorizontal);
	            }

                input->SetHoveredInternal(true);
                event->Consume(this);
            }
            else if (event->type == FEventType::MouseMove)
            {
                event->Consume(this);
            }
            else if (event->type == FEventType::MouseLeave)
            {
                if (isCursorPushed && !isDragging)
                {
                    isCursorPushed = false;
                    FusionApplication::Get()->PopCursor();
                }

                input->SetHoveredInternal(false);

                event->Consume(this);
            }
            else if (event->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left)
            {
                event->Consume(this);

                if (mouseEvent->isDoubleClick)
                {
                    FIELD_START_VALUE_IF(u8)
					else FIELD_START_VALUE_IF(s8)
					else FIELD_START_VALUE_IF(u16)
					else FIELD_START_VALUE_IF(s16)
                    else FIELD_START_VALUE_IF(u32)
					else FIELD_START_VALUE_IF(s32)
					else FIELD_START_VALUE_IF(u64)
                    else FIELD_START_VALUE_IF(s32)
                    else FIELD_START_VALUE_IF(f32)
                    else FIELD_START_VALUE_IF(f64)

                    input->StartEditing(true);
                }
            }
        }

        if (event->IsDragEvent())
        {
            FDragEvent* drag = static_cast<FDragEvent*>(event);

	        if (event->type == FEventType::DragBegin && drag->buttons == MouseButtonMask::Left)
            {
                if (!input->IsEditing())
                {
                    input->SetHighlightedInternal(true);

                    isDragging = true;
                    startMouseX = drag->mousePosition.x;

                    FIELD_START_VALUE_IF(u8)
					else FIELD_START_VALUE_IF(s8)
					else FIELD_START_VALUE_IF(u16)
					else FIELD_START_VALUE_IF(s16)
                    else FIELD_START_VALUE_IF(u32)
					else FIELD_START_VALUE_IF(s32)
					else FIELD_START_VALUE_IF(u64)
                    else FIELD_START_VALUE_IF(s32)
                    else FIELD_START_VALUE_IF(f32)
                    else FIELD_START_VALUE_IF(f64)

                    OnBeginEdit();

                    drag->draggedWidget = this;
                    drag->Consume(this);
                }
            }
            else if (event->type == FEventType::DragMove && drag->buttons == MouseButtonMask::Left)
            {
                if (isDragging)
                {
                    f64 amount = (drag->mousePosition.x - startMouseX) * m_ScrollAmount;

                    FIELD_DRAG_IF(u8)
	                else FIELD_DRAG_IF(s8)
					else FIELD_DRAG_IF(u16)
					else FIELD_DRAG_IF(s16)
                    else FIELD_DRAG_IF(u32)
					else FIELD_DRAG_IF(s32)
                    else FIELD_DRAG_IF(u64)
                    else FIELD_DRAG_IF(s64)
                    else FIELD_DRAG_IF(f32)
                    else FIELD_DRAG_IF(f64)

                    if (IsBound())
                    {
	                    OnTextFieldEdited(nullptr);
                    }
                    else
                    {
                        m_OnTextEdited(this);
                    }

                    drag->draggedWidget = this;
                    drag->Consume(this);
                }
            }
            else if (event->type == FEventType::DragEnd)
            {
                if (isDragging)
                {
                    input->SetHighlightedInternal(false);

                    if (isCursorPushed && !drag->isInside)
                    {
                        isCursorPushed = false;
                        FusionApplication::Get()->PopCursor();
                    }

                    OnEndEdit();

                    drag->draggedWidget = this;
                    drag->Consume(this);
	                isDragging = false;
                }
            }
        }

	    Super::HandleEvent(event);
    }

    bool NumericEditorField::IsEditing()
    {
        return input->IsEditing();
    }

    FWidget* NumericEditorField::HitTest(Vec2 localMousePos)
    {
	    FWidget* thisHitTest = Super::HitTest(localMousePos);
        if (thisHitTest != nullptr && !input->IsEditing())
        {
            return this;
        }
        return thisHitTest;
    }

#define FIELD_RANGE_IF(type)\
	if (numericType == TYPEID(type))\
	{\
		type value = 0;\
		if (String::TryParse(text, value))\
		{\
			ratio = Math::Clamp01(((f32)value - min) / (max - min));\
        }\
	}

    void NumericEditorField::OnPaintBeforeText(FPainter* painter)
    {
        if (isRanged && !input->IsEditing())
        {
            Vec2 size = input->GetComputedSize();
            f32 ratio = 0.0f;
            const String& text = input->Text();

            FIELD_RANGE_IF(u8)
			else FIELD_RANGE_IF(s8)
		    else FIELD_RANGE_IF(u16)
		    else FIELD_RANGE_IF(s16)
            else FIELD_RANGE_IF(u32)
            else FIELD_RANGE_IF(s32)
            else FIELD_RANGE_IF(u64)
            else FIELD_RANGE_IF(s64)
            else FIELD_RANGE_IF(f32)
            else FIELD_RANGE_IF(f64)

            if (ratio > 0.01f)
	        {
                Vec4 cornerRadius = input->CornerRadius() * 0.7f;

		        painter->SetBrush(input->BorderColor());
            	painter->SetPen(FPen());

                size.x = (size.x - (rangeSliderPadding.left + rangeSliderPadding.right)) * ratio;
                if (cornerRadius.GetMax() > size.x / 2)
                {
                    cornerRadius = Vec4(1, 1, 1, 1) * size.x / 2;
                }

            	painter->DrawRoundedRect(Rect::FromSize(Vec2(rangeSliderPadding.left, rangeSliderPadding.top),
					size - Vec2(0, rangeSliderPadding.top + rangeSliderPadding.bottom)), cornerRadius);
	        }
        }
    }

#define FIELD_SET_VALUE_IF(type)\
    if (numericType == TYPEID(type))\
    {\
        input->Text(String::Format("{}", (type)value));\
    }

    void NumericEditorField::SetValue(f64 value)
    {
        FIELD_SET_VALUE_IF(u8)
	    else FIELD_SET_VALUE_IF(s8)
		else FIELD_SET_VALUE_IF(u16)
        else FIELD_SET_VALUE_IF(s16)
        else FIELD_SET_VALUE_IF(u32)
        else FIELD_SET_VALUE_IF(s32)
        else FIELD_SET_VALUE_IF(u64)
        else FIELD_SET_VALUE_IF(s64)
        else FIELD_SET_VALUE_IF(f32)
        else FIELD_SET_VALUE_IF(f64)

        OnTextFieldEdited(input);
    }

    
#define FIELD_FINISH_IF(type)\
    if (numericType == TYPEID(type))\
    {\
        type value = 0;\
        if (String::TryParse(text, value))\
        {\
            if (isRanged) { value = Math::Clamp<type>(value, (type)min, (type)max); }\
            input->Text(String::Format("{}", value));\
        }\
    }

    void NumericEditorField::OnFinishEdit(FTextInput*)
    {
        const String& text = input->Text();
        String lowerText = text.ToLower();

        if (!text.IsEmpty())
        {
            FIELD_FINISH_IF(u8)
            else FIELD_FINISH_IF(s8)
            else FIELD_FINISH_IF(u16)
            else FIELD_FINISH_IF(s16)
            else FIELD_FINISH_IF(u32)
            else FIELD_FINISH_IF(s32)
            else FIELD_FINISH_IF(u64)
            else FIELD_FINISH_IF(s64)
            else FIELD_FINISH_IF(f32)
            else FIELD_FINISH_IF(f64)
            else
            {
                int value = 0;
                if (isRanged) { value = Math::Clamp(value, (int)min, (int)max); }
                input->Text(String::Format("{}", value));
            }
        }
        else
        {
            int value = 0;
            if (isRanged) { value = Math::Clamp(value, (int)min, (int)max); }
            input->Text(String::Format("{}", value));
        }

        OnEndEdit();

        m_OnTextEditingFinished(this);
    }

#define FIELD_APPLY_OPERATION_IF(type)\
    if (numericType == CE::GetTypeId<type>())\
    {\
        type value = 0;\
        if (String::TryParse(text, value))\
        {\
            if (isRanged) { value = Math::Clamp<type>(value, min, max); }\
            type originalValue = static_cast<type>(startValue);\
            if (auto history = m_History.Lock())\
            {\
                history->PerformOperation<type>("Edit Numeric Field", targetObject, relativeFieldPath,\
                    originalValue, value);\
            }\
            else\
            {\
                SetValueDirect<type>(value);\
            }\
        }\
    }

    void NumericEditorField::OnBeginEdit()
    {
        m_OnBeginEditing(this);
    }

    void NumericEditorField::OnEndEdit()
    {
        m_OnEndEditing(this);

        if (!IsBound())
            return;

        const String& text = input->Text();

        {
            f64 curValue = 0;
            if (String::TryParse(text, curValue))
            {
                if (isRanged) { curValue = Math::Clamp<f64>(curValue, min, max); }

                if (Math::ApproxEquals(curValue, startValue))
                {
                    return; // No change in value
                }
            }
        }

        WeakRef<Object> target = targets[0].Lock();
        Ref<Object> targetObject = target.Lock();

        if (targetObject.IsNull())
            return;

        FIELD_APPLY_OPERATION_IF(u8)
        else FIELD_APPLY_OPERATION_IF(s8)
		else FIELD_APPLY_OPERATION_IF(u16)
        else FIELD_APPLY_OPERATION_IF(s16)
        else FIELD_APPLY_OPERATION_IF(u32)
        else FIELD_APPLY_OPERATION_IF(s32)
        else FIELD_APPLY_OPERATION_IF(u64)
        else FIELD_APPLY_OPERATION_IF(s64)
        else FIELD_APPLY_OPERATION_IF(f32)
        else FIELD_APPLY_OPERATION_IF(f64)
        
    }

#define FIELD_SET_IF(type)\
    if (fieldDeclId == CE::GetTypeId<type>())\
    {\
        type value = 0;\
        if (String::TryParse(text, value))\
        {\
            if (isRanged) { value = Math::Clamp<type>(value, (type)min, (type)max); }\
            field->SetFieldValue<type>(instance, value);\
            target->OnFieldEdited(field->GetName());\
        }\
    }

    void NumericEditorField::OnTextFieldEdited(FTextInput*)
    {
        m_OnTextEdited(this);

        if (!IsBound())
            return;

        const String& text = input->Text();

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        TypeId fieldDeclId = numericType;

        Ptr<FieldType> field;
        void* instance = nullptr;
        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
            field, instance);
        if (!success)
        {
            return;
        }

        FIELD_SET_IF(u8)
        else FIELD_SET_IF(s8)
        else FIELD_SET_IF(u16)
        else FIELD_SET_IF(s16)
        else FIELD_SET_IF(u32)
        else FIELD_SET_IF(s32)
        else FIELD_SET_IF(u64)
        else FIELD_SET_IF(s64)
        else FIELD_SET_IF(f32)
        else FIELD_SET_IF(f64)
    }

    void NumericEditorField::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        if (!m_ColorTagVisible || m_ColorTag.a < 0.01f)
            return;

        constexpr f32 padding = 5.0f;
        constexpr f32 width = 2.0f;
        Vec2 pos = Vec2(padding, padding);
        Vec2 size = Vec2(width, computedSize.height - padding * 2);

        painter->SetBrush(m_ColorTag);
        painter->SetPen(FPen());
        painter->DrawRoundedRect(Rect::FromSize(pos, size), Vec4(2, 0, 0, 2));
    }

    bool NumericEditorField::CanBind(FieldType* field)
    {
        return field->IsNumericField();
    }

    bool NumericEditorField::CanBind(TypeId boundTypeId, TypeId underlyingTypeId)
    {
        static HashSet<TypeId> numericTypes = {
            TYPEID(s8),  TYPEID(u8),
            TYPEID(s16), TYPEID(u16),
            TYPEID(s32), TYPEID(u32),
            TYPEID(s64), TYPEID(u64),
            TYPEID(f32), TYPEID(f64),
        };

        return numericTypes.Exists(boundTypeId);
    }

    void NumericEditorField::OnBind()
    {
	    Super::OnBind();

        if (!IsBound())
            return;

        isRanged = false;

        if (Ref<Object> target = targets[0].Lock())
        {
            Ptr<FieldType> field = nullptr;
            Ref<Object> outOwner;
            void* outInstance = nullptr;

            bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                field, outOwner, outInstance);

            if (!success)
            {
	            return;
            }

            if (field->HasAttribute("RangeMin") && field->HasAttribute("RangeMax"))
            {
                Attribute rangeMin = field->GetAttribute("RangeMin");
                Attribute rangeMax = field->GetAttribute("RangeMax");

                if (String::TryParse(rangeMin.GetStringValue(), min) &&
                    String::TryParse(rangeMax.GetStringValue(), max))
                {
                    isRanged = true;
                }
            }
        }
    }

    void NumericEditorField::UpdateValue()
    {
        if (!IsBound())
            return;

        TypeId fieldDeclId = 0;

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field = nullptr;
        Ref<Object> outOwner;
        void* outInstance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
            field, outOwner, outInstance);

        if (!success)
            return;

        fieldDeclId = field->GetDeclarationTypeId();

        NumericType(fieldDeclId);

        if (fieldDeclId == TYPEID(u8))
        {
            u8 value = field->GetFieldValue<u8>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(s8))
        {
            s8 value = field->GetFieldValue<s8>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(u16))
        {
            u16 value = field->GetFieldValue<u16>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(s16))
        {
            s16 value = field->GetFieldValue<s16>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(u32))
        {
            u32 value = field->GetFieldValue<u32>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(s32))
        {
            s32 value = field->GetFieldValue<s32>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(u64))
        {
            u64 value = field->GetFieldValue<u64>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(s64))
        {
            s64 value = field->GetFieldValue<s64>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(f32))
        {
            f32 value = field->GetFieldValue<f32>(outInstance);
            Text(String::Format("{}", value));
        }
        else if (fieldDeclId == TYPEID(f64))
        {
            f64 value = field->GetFieldValue<f64>(outInstance);
            Text(String::Format("{}", value));
        }
    }

    NumericEditorField::Self& NumericEditorField::Range(f32 min, f32 max)
    {
        isRanged = true;
        this->min = min;
        this->max = max;
        m_ScrollAmount = (max - min) / MaxScrollDist;

        return *this;
    }

    NumericEditorField::Self& NumericEditorField::NumericType(TypeId type)
    {
        if (numericType == type)
            return *this;

        numericType = type;

        if (type == TYPEID(f32))
        {
            input->Validator(FNumericInputValidator<f32>);
        }
        else if (type == TYPEID(f64))
        {
            input->Validator(FNumericInputValidator<f64>);
        }
        else if (type == TYPEID(u8))
        {
            input->Validator(FNumericInputValidator<u8>);
        }
        else if (type == TYPEID(s8))
        {
            input->Validator(FNumericInputValidator<s8>);
        }
        else if (type == TYPEID(u16))
        {
            input->Validator(FNumericInputValidator<u16>);
        }
        else if (type == TYPEID(s16))
        {
            input->Validator(FNumericInputValidator<s16>);
        }
        else if (type == TYPEID(u32))
        {
            input->Validator(FNumericInputValidator<u32>);
        }
        else if (type == TYPEID(s32))
        {
            input->Validator(FNumericInputValidator<s32>);
        }
        else if (type == TYPEID(u64))
        {
            input->Validator(FNumericInputValidator<u64>);
        }
        else if (type == TYPEID(s64))
        {
            input->Validator(FNumericInputValidator<s64>);
        }

        return *this;
    }
}

