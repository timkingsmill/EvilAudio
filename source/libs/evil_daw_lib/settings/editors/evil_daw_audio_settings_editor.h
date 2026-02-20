#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================

class EvilDAWAudioSettingsEditor final :
    public juce::Component,
    public juce::ChangeListener
{
public:
    EvilDAWAudioSettingsEditor(juce::AudioDeviceManager& audioDeviceManager);
    ~EvilDAWAudioSettingsEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void logMessage(const juce::String& m);
    void dumpDeviceInfo();
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void lookAndFeelChanged() override;

private:
    //==============================================================================
    juce::AudioDeviceManager& _audioDeviceManager;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;
    juce::TextEditor diagnosticsBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWAudioSettingsEditor)
};
