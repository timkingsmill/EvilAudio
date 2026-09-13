#pragma once

#include <memory>
#include <juce_gui_basics.h>
#include <mvc/views/BaseWindowView.h>

namespace evil::daw
{
    class MainAppWindow : public evil::mvc::BaseWindowView,
                          private juce::AsyncUpdater
    {
    public:
        MainAppWindow(juce::JUCEApplication& application,
                      const juce::String& name = {},
                      juce::Colour backgroundColour = juce::Colours::darkgrey,
                      int requiredButtons = juce::DocumentWindow::allButtons);

        ~MainAppWindow() override;

        int getClickCounter() const;
        void setClickCounter(int newValue);

        void someMethod(); // Declaration of someMethod

    private:
        void closeButtonPressed() override;
        void handleAsyncUpdate() override; 
        void resized() override;
        void handleMainButtonClicked();
        void handleTestDatModelCommandButtonClicked();

        int clickCounter_ = 0;

    private:
        /** @brief Application reference used for lifecycle integration. */
        juce::JUCEApplication& application_;

        std::unique_ptr<juce::Component> content_;
        juce::Label statusLabel_; 
        juce::TextButton mainButton_;
        juce::TextButton testDatModelCommandButton_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainAppWindow)
    };
}
