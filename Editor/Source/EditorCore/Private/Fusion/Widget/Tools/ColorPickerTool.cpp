#include "EditorCore.h"

namespace CE::Editor
{
    static WeakRef<ColorPickerTool> instance = nullptr;

    static const Color hueValues[] = { Color(1, 0, 0), Color(1, 1, 0), Color(0, 1, 0),
                                        Color(0, 1, 1), Color(0, 0, 1), Color(1, 0, 1),
                                        Color(1, 0, 0)
    };

    static String FormatHex(u32 hex)
    {
        String result = String::Format("{:08X}", hex);

        for (int i = 0; i < 2; ++i)
        {
            int pos = i * 2;
            int otherPos = 6 - i * 2;

            std::swap(result[pos], result[otherPos]);
            std::swap(result[pos + 1], result[otherPos + 1]);
        }

        return result;
    }

    static u8 ParseByte(const char* str)
    {
        char msb = str[0];
        char lsb = str[1];

        u8 value = 0;

        if (lsb >= '0' && lsb <= '9')
        {
            value = (u8)lsb - '0';
        }
        else if (lsb >= 'A' && lsb <= 'F')
        {
            value = 10 + (u8)lsb - 'A';
        }

        if (msb >= '0' && msb <= '9')
        {
            value += ((u8)msb - '0') * 16;
        }
        else if (msb >= 'A' && msb <= 'F')
        {
            value += (10 + (u8)msb - 'A') * 16;
        }

        return value;
    }

    ColorPickerTool::ColorPickerTool()
    {
        
    }

    void ColorPickerTool::Construct()
    {
        instance = this;

        Super::Construct();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        const char* colorLabels[4] = { "R", "G", "B", "A" };
        Color basicColors[4] = { Colors::Red, Colors::Green, Colors::Blue, Colors::White };
        Color startColors[4] = { Colors::Black, Colors::Black, Colors::Black, Colors::Clear };
        ColorComponentField* colorFields[4] = { nullptr, nullptr, nullptr, nullptr };

        const char* hsvLabels[3] = { "H", "S", "V" };
        ColorComponentField* hsvFields[3] = { nullptr, nullptr, nullptr };

        FGradient hueGradient = FGradient();
        hueGradient.gradientType = FGradientType::Linear;

        for (int i = 0; i < COUNTOF(hueValues); ++i)
        {
            hueGradient.AddKey((f32)i / (COUNTOF(hueValues) - 1), hueValues[i]);
        }

        WeakRef<Self> self = this;

        (*this)
            .Title("Color Picker")
            .MinimizeEnabled(false)
            .MaximizeEnabled(false)
            .ContentPadding(Vec4(1, 1, 1, 1) * 20)
            .Content(
				FNew(FHorizontalStack)
                .Gap(20)
                .HAlign(HAlign::Fill)
                (
                    FAssignNew(FColorPicker, colorPicker)
                    .OnHSVColorEdited(FUNCTION_BINDING(this, OnHSVColorEdited))
                    .Width(250)
                    .Height(220)
                    .VAlign(VAlign::Top),

                    FNew(FVerticalStack)
                    .Gap(2.0f)
                    .FillRatio(1.0f)
                    .VAlign(VAlign::Top)
                    (
                        FNew(FLabel)
                        .FontSize(fontSize)
                        .Text("Old"),

                        FAssignNew(ColorPickerPreview, previewOld)
                        .PreviewColor(original)
                        .HAlign(HAlign::Fill)
                        .Height(32),

                        FAssignNew(ColorPickerPreview, previewNew)
                        .PreviewColor(value)
                        .HAlign(HAlign::Fill)
                        .Height(32),

                        FNew(FLabel)
                        .FontSize(fontSize)
                        .Text("New"),

                        FNew(FButton)
                        .HAlign(HAlign::Right)
                        (
                            FNew(FImage)
                            .Background(FBrush("/Editor/Assets/Icons/ColorPicker"))
                            .Width(16)
                            .Height(16)
                        )
                    )
                ),

                FNew(FHorizontalStack)
                .Gap(10)
                .HAlign(HAlign::Fill)
                .Margin(Vec4(0, 1, 0, 1) * 20)
                (
                    FNew(FVerticalStack)
                    .Gap(20)
                    .FillRatio(0.5f)
                    .VAlign(VAlign::Top)
                    (
                        FForEach { 4, [this, &colorLabels, &colorFields, &startColors, &basicColors, fontSize]
                        	(int index) -> FWidget&
                            {
                                return FNew(FHorizontalStack)
                                .Gap(5)
                                .ContentVAlign(VAlign::Center)
                                .HAlign(HAlign::Fill)
                                (
                                    FNew(FLabel)
                                    .FontSize(fontSize + 2)
                                    .Text(colorLabels[index]),

                                    FAssignNew(ColorComponentField, colorFields[index])
                                    .Gradient(FGradient().AddKey(0, startColors[index]).AddKey(1, basicColors[index]))
                                    .Transparency(index == 3)
                                    .Text("0")
                                    .NumericType<f32>()
                                    .Range(0, 1)
                                    .OnTextEdited([this, index](NumericEditorField* field)
	                                    {
                                            f32 number = 0;
											if (String::TryParse(field->Text(), number))
											{
                                                number = Math::Clamp01(number);

                                                if (index == 0)
                                                    value.r = number;
                                                else if (index == 1)
                                                    value.g = number;
                                                else if (index == 2)
                                                    value.b = number;
                                                else if (index == 3)
                                                    value.a = number;

                                                hsv = value.ToHSV();

												UpdateFields((ColorComponentField*)field);
											}
	                                    })
                                    .FillRatio(1.0f)
                                );
                            }
                        }
                    ),

                    FNew(FVerticalStack)
                    .Gap(20)
                    .FillRatio(0.5f)
                    .VAlign(VAlign::Top)
                    (
                        FForEach{ 3, [this, &hsvLabels, &hsvFields, &hueGradient, fontSize]
                            (int index) -> FWidget&
                            {
                                return FNew(FHorizontalStack)
                                .Gap(5)
                                .ContentVAlign(VAlign::Center)
                                .HAlign(HAlign::Fill)
                                (
                                    FNew(FLabel)
                                    .FontSize(fontSize + 2)
                                    .Text(hsvLabels[index]),

                                    FAssignNew(ColorComponentField, hsvFields[index])
                                    .Gradient(index == 0 ? hueGradient : FGradient())
                                    .Text("0")
                                    .NumericType<f32>()
                                    .Range(0, index == 0 ? 360 : 1)
                                    .OnTextEdited([this, index](NumericEditorField* field)
                                        {
                                            f32 number = 0;
                                            if (String::TryParse(field->Text(), number))
                                            {
                                                if (index == 0)
                                                    number = Math::Clamp<f32>(number, 0, 360);
                                                else
                                                    number = Math::Clamp01(number);

                                                if (index == 0)
                                                    hsv.x = number;
                                                else if (index == 1)
                                                    hsv.y = number;
                                                else if (index == 2)
                                                    hsv.z = number;

                                                value = Color::HSV(hsv.x, hsv.y, hsv.z);

                                                UpdateFields((ColorComponentField*)field);
                                            }
                                        })
                                    .FillRatio(1.0f)
                                );
                            }
                        },

                        FNew(FHorizontalStack)
                        .Gap(5)
                        .ContentVAlign(VAlign::Center)
                        .HAlign(HAlign::Right)
                        (
                            FNew(FLabel)
                            .FontSize(fontSize)
                            .Text("Hex"),

                            FAssignNew(FTextInput, hexInput)
                            .FontSize(fontSize)
                            .Text(FormatHex(value.ToU32()))
                            .OnTextEdited([this](FTextInput*)
                            {
                                ParseHexInput();
                            })
                            .Width(80)
                        )
                    )
                ),

                FNew(FHorizontalStack)
                .Gap(10)
                .HAlign(HAlign::Right)
                .Margin(Vec4(0, 0, 0, 10))
                (
					FNew(FTextButton)
                    .Text("Ok")
                    .TextHAlign(HAlign::Center)
                    .FontSize(fontSize)
                    .OnClicked(FUNCTION_BINDING(this, OnClickOk))
                    .Width(60),

                    FNew(FTextButton)
                    .Text("Cancel")
                    .TextHAlign(HAlign::Center)
                    .FontSize(fontSize)
                    .OnClicked(FUNCTION_BINDING(this, OnClickCancel))
                    .Width(60)
                )
            )
        ;

        inputR = colorFields[0];
        inputG = colorFields[1];
        inputB = colorFields[2];
        inputA = colorFields[3];

        inputH = hsvFields[0];
        inputS = hsvFields[1];
        inputV = hsvFields[2];
    }

    void ColorPickerTool::SetOriginalColor(const Color& color)
    {
        this->original = color;

        previewOld->PreviewColor(color);
    }

    void ColorPickerTool::SetColor(const Color& color)
    {
        this->value = color;
        this->hsv = color.ToHSV();

        UpdateFields();
    }

    void ColorPickerTool::ParseHexInput()
    {
        String text = hexInput->Text().ToUpper();

        if (text.GetLength() == 6)
        {
            f32 r = ParseByte(text.GetCString());
            f32 g = ParseByte(text.GetCString() + 2);
            f32 b = ParseByte(text.GetCString() + 4);

            value.r = r / 255.0f;
            value.g = g / 255.0f;
            value.b = b / 255.0f;
        }
        else if (text.GetLength() >= 8)
        {
            f32 r = ParseByte(text.GetCString());
            f32 g = ParseByte(text.GetCString() + 2);
            f32 b = ParseByte(text.GetCString() + 4);
            f32 a = ParseByte(text.GetCString() + 6);

            value.r = r / 255.0f;
            value.g = g / 255.0f;
            value.b = b / 255.0f;
            value.a = a / 255.0f;
        }
        else if (text.GetLength() == 1)
        {
            char c = text[0];
            u8 halfByte = 0;

            if (c >= '0' && c <= '9')
                halfByte = c - '0';
            else if (c >= 'A' && c <= 'F')
                halfByte = 10 + c - 'A';

            value.r = value.g = value.b = (f32)halfByte / 15.0f;
        }
        else if (text.GetLength() == 2)
        {
            f32 byte = ParseByte(text.GetCString());

            value.r = value.g = value.b = byte / 255.0f;
        }
        else if (text.GetLength() >= 3 && text.GetLength() <= 5)
        {
            f32 components[3] = { 0, 0, 0 };

            for (int i = 0; i < 3; ++i)
            {
                char c = text[i];
                u8 halfByte = 0;

                if (c >= '0' && c <= '9')
                    halfByte = c - '0';
                else if (c >= 'A' && c <= 'F')
                    halfByte = 10 + c - 'A';

                components[i] = (f32)halfByte / 15.0f;
            }

            value.r = components[0];
            value.g = components[1];
            value.b = components[2];
        }

        this->hsv = value.ToHSV();

        UpdateFields(nullptr, true);
    }

    void ColorPickerTool::OnClickClose()
    {
        Super::OnClickClose();

        OnClickCancel();
    }

    void ColorPickerTool::OnClickOk()
    {
        m_OnColorChanged(this);
        m_OnColorChanged.Unbind();

        m_OnColorAccepted(this);
        m_OnColorAccepted.Unbind();

        HideWindow();
    }

    void ColorPickerTool::OnClickCancel()
    {
        value = original;

        m_OnColorChanged(this);
        m_OnColorChanged.Unbind();

        m_OnColorAccepted.Unbind();

        HideWindow();
    }

    void ColorPickerTool::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        m_OnColorChanged(this);
        m_OnColorChanged.Unbind();

        m_OnColorAccepted.Unbind();
    }

    void ColorPickerTool::OnHSVColorEdited(f32 h, f32 s, f32 v)
    {
        hsv = Vec3(h, s, v);

        f32 a = value.a;
        this->value = Color::HSV(h, s, v);
        this->value.a = a;

        UpdateFields(nullptr, false, true);
    }

    void ColorPickerTool::UpdateFields(ColorComponentField* excludeField, bool excludeHexField, bool excludeColorPicker)
    {
        constexpr const char* format = "{:.6f}";

        if (inputH != excludeField)
            inputH->Text(String::Format(format, hsv.x));
        if (inputS != excludeField)
            inputS->Text(String::Format(format, hsv.y));
        if (inputV != excludeField)
            inputV->Text(String::Format(format, hsv.z));

        if (inputR != excludeField)
            inputR->Text(String::Format(format, value.r));
        if (inputG != excludeField)
            inputG->Text(String::Format(format, value.g));
        if (inputB != excludeField)
            inputB->Text(String::Format(format, value.b));
        if (inputA != excludeField)
            inputA->Text(String::Format(format, value.a));

        if (!excludeHexField)
        {
            hexInput->Text(FormatHex(value.ToU32()));
        }

        if (!excludeColorPicker)
        {
            f32 s = hsv.y;
            f32 v = hsv.z;

            f32 x = hsv.x / 360;
            f32 y = s * 0.5f + (1 - v) * 0.5f;

            x = Math::Clamp01(x);
            y = Math::Clamp01(y);
            //colorPicker->NormalizedColorPosition(Vec2(x, y));
        }

        previewNew->PreviewColor(value);

        inputR->Gradient(FGradient()
            .AddKey(0, Color(0, value.g, value.b, 1))
            .AddKey(1, Color(1, value.g, value.b, 1))
        );

        inputG->Gradient(FGradient()
            .AddKey(0, Color(value.r, 0, value.b, 1))
            .AddKey(1, Color(value.r, 1, value.b, 1))
        );

        inputB->Gradient(FGradient()
            .AddKey(0, Color(value.r, value.g, 0, 1))
            .AddKey(1, Color(value.r, value.g, 1, 1))
        );

        inputA->Gradient(FGradient()
            .AddKey(0, Color(value.r, value.g, value.b, 0))
            .AddKey(1, Color(value.r, value.g, value.b, 1))
        );

        inputS->Gradient(FGradient()
            .AddKey(0, Color::HSV(hsv.x, 0, hsv.z))
            .AddKey(1, Color::HSV(hsv.x, 1, hsv.z))
        );

        inputV->Gradient(FGradient()
            .AddKey(0, Color::HSV(hsv.x, hsv.y, 0))
            .AddKey(1, Color::HSV(hsv.x, hsv.y, 1))
        );

        if (excludeField != nullptr || excludeColorPicker || excludeHexField)
        {
            m_OnColorChanged(this);
        }
    }

    Ref<ColorPickerTool> ColorPickerTool::Open(const Ref<EditorHistory>& history)
    {
        if (auto lock = instance.Lock())
        {
            lock->history = history;

            Ref<FNativeContext> nativeContext = CastTo<FNativeContext>(lock->GetContext());
            PlatformWindow* window = nativeContext->GetPlatformWindow();
            window->SetAlwaysOnTop(true);
            window->Show();
            return lock;
        }

        PlatformWindowInfo info{
            .maximised = false,
            .fullscreen = false,
            .resizable = false,
            .hidden = true,
            .windowFlags = PlatformWindowFlags::Utility | PlatformWindowFlags::DestroyOnClose | PlatformWindowFlags::SkipTaskbar
        };

        Ref<ColorPickerTool> colorPickerTool = (Ref<ColorPickerTool>)FusionApplication::Get()->CreateNativeWindow(
            "ColorPickerTool", "Color Picker Tool",
            450, 535,
            Self::StaticClass(), info);

        colorPickerTool->history = history;

        PlatformWindow* platformWindow = CastTo<FNativeContext>(colorPickerTool->GetContext())->GetPlatformWindow();
        platformWindow->SetAlwaysOnTop(true);

        platformWindow->Show();

        colorPickerTool->SetColor(Colors::White);
        return colorPickerTool;
    }
}

