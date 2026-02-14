#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{
    class MainWindowMenuBar : public juce::MenuBarComponent
    {

    public:

        MainWindowMenuBar(juce::MenuBarModel* model = nullptr) :
            MenuBarComponent(model)
        {}

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindowMenuBar)

    };
}