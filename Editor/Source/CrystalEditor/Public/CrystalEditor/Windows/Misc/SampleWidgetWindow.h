#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API SampleWidgetModel : public FDataModel
    {
        CE_CLASS(SampleWidgetModel, FDataModel)
    public:

        SampleWidgetModel();

        void OnModelPropertyEdited(const CE::Name& propertyName, Object* modifyingObject) override;

        FUNCTION()
        void ModifyTextInCode();

    public: // Model properties

        // For the ComboBox
        MODEL_PROPERTY(Array<String>, ItemList);

        // For the TextInput
        MODEL_PROPERTY(String, Text);

    };

    CLASS()
    class CRYSTALEDITOR_API SampleWidgetWindow : public FToolWindow
    {
        CE_CLASS(SampleWidgetWindow, FToolWindow)
    protected:

        SampleWidgetWindow();

        void Construct() override;

        void OnBeginDestroy() override;

    public: // - Public API -

        static SampleWidgetWindow* Show();

    protected: // - Internal -

        SampleWidgetModel* model = nullptr;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "SampleWidgetWindow.rtti.h"
