#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <settings/evil_daw_appearance_settings.h>

namespace evil
{
    class EvilDawApplicationSettings final : private juce::ValueTree::Listener
    {
    public:
        EvilDawApplicationSettings();
        ~EvilDawApplicationSettings() override;
    
        juce::PropertiesFile& getGlobalProperties();
        juce::PropertiesFile& getProjectProperties(const juce::String& projectUID);

        void flush();
        void reload();

    private:
        std::unique_ptr<EvilDawAppearanceSettings> _appearanceSettings;

        juce::OwnedArray<juce::PropertiesFile> _propertyFiles;


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDawApplicationSettings)
    };

    // ------------------------------------------------------------------------------

    EvilDawApplicationSettings& getAppSettings();
    juce::PropertiesFile& getGlobalProperties();

    // ------------------------------------------------------------------------------
}