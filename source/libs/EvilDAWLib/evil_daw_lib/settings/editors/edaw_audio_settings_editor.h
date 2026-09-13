#pragma once

#include <juce_audio_utils.h>

//==============================================================================

namespace evil
{
    class EvilDAWAudioSettingsEditor final : public juce::Component,
                                             public juce::ChangeListener,
                                             private juce::AsyncUpdater

    {
    public:
        /**
         * @brief Constructs the audio settings editor.
         * @param audioDeviceManager Shared device manager used to configure audio I/O.
         */
        EvilDAWAudioSettingsEditor(juce::AudioDeviceManager& audioDeviceManager);

        /**
         * @brief Destroys the audio settings editor.
         */
        ~EvilDAWAudioSettingsEditor() override;

        /**
         * @brief Paints the editor background and static UI elements.
         * @param graphics Graphics context used for drawing.
         */
        void paint(juce::Graphics& graphics) override;

        /**
         * @brief Lays out child components when the editor size changes.
         */
        void resized() override;

    private:
        /**
         * @brief Appends a diagnostic message to the diagnostics text area.
         * @param m Message text to append.
         */
        void logMessage(const juce::String& m);

        /**
         * @brief Captures and logs current audio device information.
         */
        void dumpDeviceInfo();

        /**
         * @brief Handles audio device manager change notifications.
         * @param source Broadcaster that emitted the change event.
         */
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        /**
         * @brief Updates component appearance when look-and-feel changes.
         */
        void lookAndFeelChanged() override;

        /**
         * @brief Processes deferred updates requested via AsyncUpdater.
         */
        void handleAsyncUpdate() override;

    private:
        //==============================================================================
        /** @brief Reference to the shared JUCE audio device manager. */
        juce::AudioDeviceManager& _audioDeviceManager;

        /** @brief Audio I/O configuration UI component. */
        std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;

        /** @brief Read-only diagnostics output text box. */
        juce::TextEditor diagnosticsBox;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWAudioSettingsEditor)
    };
}