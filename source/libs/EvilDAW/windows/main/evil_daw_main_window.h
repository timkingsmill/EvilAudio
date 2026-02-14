#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{

    class EvilDAWMainWindow final : public juce::DocumentWindow
    {
    public:
        explicit EvilDAWMainWindow(const juce::String& name,
                                         juce::JUCEApplication& application);
        ~EvilDAWMainWindow() override;
  
        void closeButtonPressed() override;

    private:
        void restoreWindowPosition();
        void storeWindowPosition();
        void visibilityChanged() override;



        juce::JUCEApplication& _application;
        std::unique_ptr<juce::Component> _mainWindowContent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWMainWindow)

    };

}
