#pragma once

//#include <functional>

#include <juce_gui_basics.h>
#include <juce_audio_processors.h>

namespace evil
{
    class PluginListWindow final : public juce::DocumentWindow
    {
    public:
        PluginListWindow(juce::AudioPluginFormatManager& pluginFormatManager,
                         juce::KnownPluginList& knownPluginList);
        ~PluginListWindow() override;

        /**
         * @brief Sets a callback invoked when the window requests close.
         */
        void setOnCloseRequested(std::function<void()> onCloseRequested);

    private:

        void closeButtonPressed() override;

    private:
        std::function<void()> onCloseRequested_;
    };

}