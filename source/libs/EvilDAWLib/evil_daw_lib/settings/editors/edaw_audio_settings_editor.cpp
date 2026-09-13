#include "edaw_audio_settings_editor.h"

//==============================================================================

namespace evil
{ 

    EvilDAWAudioSettingsEditor::EvilDAWAudioSettingsEditor(juce::AudioDeviceManager& audioDeviceManager) :
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

    triggerAsyncUpdate();

}

// --------------------------------------------------------------------------------------------

/**
 * @brief Removes listeners and releases editor resources.
 */
EvilDAWAudioSettingsEditor::~EvilDAWAudioSettingsEditor()
{
    _audioDeviceManager.removeChangeListener(this);
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

/**
 * @brief Handles deferred asynchronous updates triggered by the component.
 */
void EvilDAWAudioSettingsEditor::handleAsyncUpdate()
{ 
}

// --------------------------------------------------------------------------------------------

void EvilDAWAudioSettingsEditor::paint(juce::Graphics& graphics)
{
    // (Our component is opaque, so we must completely fill 
    // the background with a solid colour)
    graphics.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

/**
 * @brief Updates child component layout after a resize event.
 */
void EvilDAWAudioSettingsEditor::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto rectangle = getLocalBounds().reduced(4);
    audioSetupComp->setBounds(rectangle.removeFromTop(proportionOfHeight(0.65f)));
    diagnosticsBox.setBounds(rectangle);
}

void EvilDAWAudioSettingsEditor::logMessage(const juce::String& m)
{
    diagnosticsBox.moveCaretToEnd();
    diagnosticsBox.insertTextAtCaret(m + juce::newLine);
}

/**
 * @brief Collects and logs details about the currently selected audio device.
 */
void EvilDAWAudioSettingsEditor::dumpDeviceInfo()
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
}

void EvilDAWAudioSettingsEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    juce::ignoreUnused(source);
    dumpDeviceInfo();
}

/**
 * @brief Reapplies diagnostics text formatting after look-and-feel changes.
 */
void EvilDAWAudioSettingsEditor::lookAndFeelChanged()
{
    diagnosticsBox.applyFontToAllText(diagnosticsBox.getFont());
}

} // namespace evil