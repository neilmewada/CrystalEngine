#pragma once

namespace CE::Editor
{
    class ColorPickerTool;
    DECLARE_SCRIPT_DELEGATE(ColorPickerDelegate, void, ColorPickerTool*);

    CLASS()
    class EDITORCORE_API ColorPickerTool : public FToolWindow
    {
        CE_CLASS(ColorPickerTool, FToolWindow)
    protected:

        ColorPickerTool();

        void Construct() override;

    public: // - Public API -

        static Ref<ColorPickerTool> Open(const Ref<EditorHistory>& history = nullptr);

        void SetOriginalColor(const Color& color);
        void SetColor(const Color& color);

        const Color& GetOriginalColor() const { return original; }
        const Color& GetColor() const { return value; }

    protected: // - Internal -

        void ParseHexInput();

        void OnClickClose() override;

        FUNCTION()
        void OnClickOk();

        FUNCTION()
        void OnClickCancel();

        void OnBeginDestroy() override;

        FUNCTION()
        void OnHSVColorEdited(f32 h, f32 s, f32 v);

        void UpdateFields(ColorComponentField* excludeField = nullptr, bool excludeHexField = false, bool excludeColorPicker = false);

        FIELD()
        Color original;

        FIELD()
        Color value;

        FIELD()
        Vec3 hsv;

        Ref<FColorPicker> colorPicker;
        Ref<ColorPickerPreview> previewOld;
        Ref<ColorPickerPreview> previewNew;

        Ref<ColorComponentField> inputR;
        Ref<ColorComponentField> inputG;
        Ref<ColorComponentField> inputB;
        Ref<ColorComponentField> inputA;

        Ref<ColorComponentField> inputH;
        Ref<ColorComponentField> inputS;
        Ref<ColorComponentField> inputV;

        Ref<FTextInput> hexInput;

        WeakRef<EditorHistory> history;

    public: // - Fusion Properties - 

        FUSION_EVENT(ColorPickerDelegate, OnColorChanged);
        FUSION_EVENT(ColorPickerDelegate, OnColorAccepted);

        FUSION_WIDGET;
    };
    
}

#include "ColorPickerTool.rtti.h"
