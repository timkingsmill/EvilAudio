#pragma once

#include <juce_gui_basics.h>
#include "BaseView.h"

namespace evil::mvc
{
    class BaseWindowView : public juce::DocumentWindow,
                           public BaseView
    {
    public:
        BaseWindowView(juce::JUCEApplication& application,
                       const juce::String& name = {},
                       juce::Colour backgroundColour = juce::Colours::darkgrey,
                       int requiredButtons = juce::DocumentWindow::allButtons);

        ~BaseWindowView() override;

        std::string render() const override;

        void visibilityChanged() override;

    private:
        bool initialized_ = false;
    };
}