#pragma once

#include <juce_gui_basics.h>
#include "BaseView.h"

namespace evil::mvc
{
    class BaseComponentView : public juce::Component,
                              public BaseView
    {
    public:
        BaseComponentView(juce::JUCEApplication& application,
            const juce::String& name = {},
            juce::Colour backgroundColour = juce::Colours::darkgrey);

        ~BaseComponentView() override;

        std::string render() const override;

        void visibilityChanged() override;

    private:
        bool initialized_ = false;
    };
}