#pragma once

#include <juce_gui_basics.h>
#include "BaseView.h"

namespace evil::mvc
{
    class BaseDialogWindowView : public juce::DialogWindow,
        public BaseView
    {
    public:
        BaseDialogWindowView(juce::JUCEApplication& application,
            const juce::String& name = {},
            juce::Colour backgroundColour = juce::Colours::darkgrey,
            int requiredButtons = juce::DocumentWindow::allButtons);

        ~BaseDialogWindowView() override;

        std::string render() const override;

        void visibilityChanged() override;

    private:
        bool initialized_ = false;
    };
}