#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class AudioSettingsComponent final :
    public juce::Component,
    public juce::ChangeListener
{
public:
    AudioSettingsComponent(juce::AudioDeviceManager& audioDeviceManager) :
        _audioDeviceManager{ audioDeviceManager }
    {
        setOpaque(true);

        audioSetupComp.reset(new juce::AudioDeviceSelectorComponent(_audioDeviceManager,
            1, 256, 1, 256,
            false, //true, // Disable MIDI input options
            false, //true, // Disable MIDI output selector
            true, false));

        addAndMakeVisible(audioSetupComp.get());

        // Setup diagnostics text box
        addAndMakeVisible(diagnosticsBox);
        diagnosticsBox.setMultiLine(true);
        diagnosticsBox.setReturnKeyStartsNewLine(true);
        diagnosticsBox.setReadOnly(true);
        diagnosticsBox.setScrollbarsShown(true);
        diagnosticsBox.setCaretVisible(false);
        diagnosticsBox.setPopupMenuEnabled(true);

        _audioDeviceManager.addChangeListener(this);

        logMessage("Audio device diagnostics:\n");

        setSize(600, 440);
    };

    ~AudioSettingsComponent() override {
        _audioDeviceManager.removeChangeListener(this);
    };

    void paint(juce::Graphics& graphics) override {
        // (Our component is opaque, so we must completely fill 
        // the background with a solid colour)
        graphics.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };

    void resized() override {
        // This is called when the MainComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        auto rectangle = getLocalBounds().reduced(4);
        audioSetupComp->setBounds(rectangle.removeFromTop(proportionOfHeight(0.65f)));
        diagnosticsBox.setBounds(rectangle);
    };

private:
    void logMessage(const juce::String& m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + juce::newLine);
    }

    void dumpDeviceInfo()
    {
        logMessage("--------------------------------------");
        logMessage("Current audio device type: " + (_audioDeviceManager.getCurrentDeviceTypeObject() != nullptr
            ? _audioDeviceManager.getCurrentDeviceTypeObject()->getTypeName()
            : "<none>"));

        if (juce::AudioIODevice* device = _audioDeviceManager.getCurrentAudioDevice())
        {
            logMessage("Current audio device: " + device->getName().quoted());
            logMessage("Sample rate: " + juce::String(device->getCurrentSampleRate()) + " Hz");
            logMessage("Block size: " + juce::String(device->getCurrentBufferSizeSamples()) + " samples");
            logMessage("Output Latency: " + juce::String(device->getOutputLatencyInSamples()) + " samples");
            logMessage("Input Latency: " + juce::String(device->getInputLatencyInSamples()) + " samples");
            logMessage("Bit depth: " + juce::String(device->getCurrentBitDepth()));
            logMessage("Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
            //logMessage("Active input channels: " + getListOfActiveBits(device->getActiveInputChannels()));
            logMessage("Output channel names: " + device->getOutputChannelNames().joinIntoString(", "));
            //logMessage("Active output channels: " + getListOfActiveBits(device->getActiveOutputChannels()));
        }
        else
        {
            logMessage("No audio device open");
        }
    };

    // Inherited via ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        dumpDeviceInfo();
    };


    void lookAndFeelChanged() override {
        diagnosticsBox.applyFontToAllText(diagnosticsBox.getFont());
    };

private:
    //==============================================================================
    // Your private member variables go here...
    juce::AudioDeviceManager& _audioDeviceManager;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;
    juce::TextEditor diagnosticsBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSettingsComponent)
};
