#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{
    class EvilDAWMainWindowContent final :
        //public juce::ApplicationCommandTarget,
        public juce::Component //,
        //public juce::MenuBarModel
    {
    public:

        EvilDAWMainWindowContent() {}


        void paint(juce::Graphics& graphics) override {
            // (Our component is opaque, so we must completely fill 
            // the background with a solid colour)
            graphics.fillAll(juce::Colours::aquamarine);//getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        };

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWMainWindowContent)
    };
}
