#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FToolWindow : public FWindow
    {
        CE_CLASS(FToolWindow, FWindow)
    public:

        FToolWindow();

        FStackBox* GetContentBox() const { return content; }

        void SetMaximizeButton(bool interactable);
        void SetMinimizeButton(bool interactable);

        void SetWindowContent(FWidget& content) override;

    protected:

        void Construct() override;

        void OnMaximized() override;
        void OnMinimized() override;
        void OnRestored() override;

        virtual void OnClickClose() {}

        FStyledWidget* borderWidget = nullptr;
        FVerticalStack* rootBox = nullptr;
        FImage* maximizeIcon = nullptr;
        FImage* minimizeIcon = nullptr;
        FStackBox* content = nullptr;
        FTitleBar* titleBar = nullptr;
        FLabel* titleBarLabel = nullptr;
        FHorizontalStack* controlContainer = nullptr;

        FWindowControlButton* minimizeButton = nullptr;
        FWindowControlButton* maximizeButton = nullptr;
        FWindowControlButton* closeButton = nullptr;

    public:

        FUSION_PROPERTY_WRAPPER(Background, borderWidget);
        FUSION_PROPERTY_WRAPPER(BorderWidth, borderWidget);
        FUSION_PROPERTY_WRAPPER(BorderColor, borderWidget);
        FUSION_PROPERTY_WRAPPER2(Background, titleBar, TitleBarBackground);

        FUSION_PROPERTY_WRAPPER2(Padding, content, ContentPadding);
        FUSION_PROPERTY_WRAPPER2(Gap, content, ContentGap);
        FUSION_PROPERTY_WRAPPER2(FillRatio, content, ContentFillRatio);
        FUSION_PROPERTY_WRAPPER2(ContentVAlign, content, ContentVAlign);
        FUSION_PROPERTY_WRAPPER2(ContentHAlign, content, ContentHAlign);

        FUSION_PROPERTY_WRAPPER2(Text, titleBarLabel, Title);

        Self& MinimizeEnabled(bool enabled);
        Self& MinimizeInteractable(bool interactable);
        Self& MaximizeEnabled(bool enabled);
        Self& MaximizeInteractable(bool interactable);

        FStackBoxDirection ContentDirection()
        {
            return content->Direction();
        }

        Self& ContentDirection(FStackBoxDirection contentDirection)
        {
            content->Direction(contentDirection);
            return *this;
        }

        template<typename... TArgs> requires TValidate_Children<TArgs...>::Value and (sizeof...(TArgs) > 0)
        Self& Content(const TArgs&... childWidget)
        {
            if (content)
            {
                content->operator()(childWidget...);
            }
            return *this;
        }

        FUSION_WIDGET;
    };
    
} // namespace CE

#include "FToolWindow.rtti.h"