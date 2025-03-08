# Fusion Widgets

Fusion is a declarative C++ widget library that powers the Crystal Editor itself using a builtin 2D renderer. In future, it will be modified to be used for game's GUI.

Fusion lets you create widgets in C++ using declarative syntax and apply flexible styling on them.

## Features

* Declarative UI syntax.
* DPI-aware rendering.
* Texture support: Use textures to draw icons and images with custom scaling and tiling.
* Font glyph caching. Font glyphs are only created on-demand and cached for reuse in a Font Atlas.
* Fast 2D renderer with GPU side 2D clipping, and CPU side 2D draw culling.
* Styling: Flexible styling using `FStyle` and `FStyleSet`.
* Flexible layout: Use layouts like FStackBox, FOverlayStack, FCompoundWidget, FSplitBox, etc to create complex layouts.
* Simple data binding between widget and model classes, using `MODEL_PROPERTY()` and `BIND_PROPERTY_R/RW()` macros.
* Create custom widgets with your own declarative properties with `FUSION_PROPERTY()` and other macros.
* Apply layout-independent transformations with Translation(), Angle() and Scale() to individual widgets and have them still work with mouse input hit-test:
<br><img src="../Screenshots/Widget Transformations.gif" height=300/>


### DPI Aware Rendering (New renderer)

Here is the comparison between old renderer (left) and the new renderer (right) that is DPI-aware.
<br><img src="../Screenshots/FusionSample.png" height=500/> <img src="../Screenshots/FusionSample_DPI.png" height=500/>

## Widget Sample

If you are running the Crystal Editor, you can go to Help menu and click "Fusion Samples" to open this sample:
<br><img src="../Screenshots/FusionSample_DPI.png" height=500/>

You can take a look at the cpp file below to see the full implementation
[/Editor/Source/CrystalEditor/Private/CrystalEditor/Windows/Misc/SampleWidgetWindow.cpp](/Editor/Source/CrystalEditor/Private/CrystalEditor/Windows/Misc/SampleWidgetWindow.cpp)

Here is the Construct() function that builds the widget:

```c++
void SampleWidgetWindow::Construct()
{
    Super::Construct();

    titleBar->Height(30);
    titleBarLabel->FontSize(13);

    constexpr auto splashImage = "/Editor/Assets/UI/Splash";
    constexpr auto gridImage = "/Engine/Resources/Icons/TransparentPattern";

    instance = this;

    FBrush splash = FBrush(splashImage);
    splash.SetBrushTiling(FBrushTiling::TileXY);
    splash.SetImageFit(FImageFit::Contain);
    splash.SetBrushPosition(Vec2(0.5f, 0.5f)); // 50%, 50%
    splash.SetBrushSize(Vec2(256, 144));

    FBrush grid = FBrush(gridImage);
    grid.SetBrushTiling(FBrushTiling::TileXY);
    grid.SetBrushSize(Vec2(16, 16));


    (*this)
        .Title("Fusion Samples")
        .MinimizeEnabled(false)
        .MaximizeEnabled(false)
        .ContentPadding(Vec4(1, 1, 1, 1) * 20)
        .ContentGap(5)
        .Content(
            FNew(FImage)
            .Background(splash)
            .BackgroundShape(FRoundedRectangle(5, 10, 15, 20))
            .HAlign(HAlign::Fill)
            .Height(300)
            .Margin(Vec4(0, 0, 0, 10)),

            FNew(FLabel)
            .FontSize(13)
            .Text("Version: " CE_ENGINE_VERSION_STRING_SHORT " pre-alpha [Build " CE_TOSTRING(CE_VERSION_BUILD) "]")
            .HAlign(HAlign::Left),

            FNew(FSplitBox)
            .ContentVAlign(VAlign::Center)
            .HAlign(HAlign::Fill)
            .Name("DebugSplitBox")
            (
                FNew(FStyledWidget)
                .Background(Color::Yellow())
                .Height(25)
                .FillRatio(0.25f),

                FNew(FStyledWidget)
                .Background(Color::Green())
                .Height(25)
                .FillRatio(0.25f),

                FNew(FStyledWidget)
                .Background(Color::Cyan())
                .Height(25)
                .FillRatio(0.5f)
            ),

            FNew(FOverlayStack)
            .ContentVAlign(VAlign::Center)
            .HAlign(HAlign::Fill)
            (
                FNew(FStyledWidget)
                .Background(grid)
                .Height(40),

                FNew(FLabel)
                .Text("This is overlay text")
                .FontSize(18)
                .Bold(true)
                .Foreground(Color::Black())
                .HAlign(HAlign::Center)
            ),

            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            (
                FNew(FTextButton)
                .FontSize(10)
                .Text("Add Item")
                .OnClicked([this]
                {
                    Array<String> list = model->GetItemList();
                    list.Add(String::Format("Item {}", list.GetSize()));
                    model->SetItemList(list);
                }),

                FNew(FComboBox)
                .Bind_Items(BIND_PROPERTY_R(model, ItemList))
                .MinWidth(60)
            ),

            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            (
                FNew(FTextInput)
                .Bind_Text(BIND_PROPERTY_RW(model, Text))
                .FontSize(10)
                .Width(180)
                .Margin(Vec4(0, 0, 10, 0)),

                FNew(FTextButton)
                .FontSize(10)
                .Text("Randomize")
                .OnClicked([this]
                    {
                        model->ModifyTextInCode();
                    }),

                FNew(FLabel)
                .FontSize(13)
                .Bind_Text(BIND_PROPERTY_R(model, Text))
            ),

            FNew(FScrollBox)
            .VerticalScroll(true)
            .HorizontalScroll(true)
            .FillRatio(1.0f)
            .Angle(5)
            .Scale(Vec2(1, 1) * 0.75f)
            .Name("TestScrollBox")
            (
                FNew(FVerticalStack)
                .ContentHAlign(HAlign::Left)
                .VAlign(VAlign::Top)
                .HAlign(HAlign::Left)
                (
                    FNew(FLabel)
                    //.Text("0. The quick brown fox jumps over the lazy dog, showcasing every letter in the English alphabet.")
                    .Text("0. The quick brown fox jumps.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("1. Beneath the starlit sky, a lively fox darted over fallen leaves, leaving soft echoes in the quiet woods.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("2. The curious red fox dashed through the misty forest, weaving between ancient trees with nimble grace.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("3. With a flick of its tail, the agile fox bounded across the meadow, vanishing into the evening shadows.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("4. The quick brown fox jumps over the lazy dog, showcasing every letter in the English alphabet.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("5. Beneath the starlit sky, a lively fox darted over fallen leaves, leaving soft echoes in the quiet woods.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("6, The curious red fox dashed through the misty forest, weaving between ancient trees with nimble grace.")
                    .FontSize(16),

                    FNew(FLabel)
                    .Text("7. With a flick of its tail, the agile fox bounded across the meadow, vanishing into the evening shadows.")
                    .FontSize(16)
                )
            )
        );
}
```

