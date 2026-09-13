#pragma once

#include <juce_gui_basics.h>
#include <juce_audio_processors.h>


namespace evil
{
    class EvilDAWPluginsListComponent final : public juce::PluginListComponent
    {
    public:
        EvilDAWPluginsListComponent(juce::AudioPluginFormatManager& manager,
                                    juce::KnownPluginList& listToRepresent,
                                    const juce::File& pedal,
                                    juce::PropertiesFile* props,
                                    bool async);

        ~EvilDAWPluginsListComponent() override;

        void paint(juce::Graphics& graphics) override;

    private:
        void handleResize();
        void resized() override;

        /// Label describing the plugin scan / validation mode selection.
        juce::Label _validationModeLabel{ {}, "Scan mode" };

        /// Combo box used to choose the scan / validation mode (e.g. in-process,
        /// out-of-process, or other custom modes supported by the scanner).
        juce::ComboBox _validationModeBox;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWPluginsListComponent)
    };
}