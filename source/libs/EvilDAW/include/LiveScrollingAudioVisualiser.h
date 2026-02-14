#pragma once

#include <JuceHeader.h>

class LiveScrollingAudioVisualiser final : public juce::AudioVisualiserComponent,
    public juce::AudioIODeviceCallback
{

public:
    LiveScrollingAudioVisualiser()
        : juce::AudioVisualiserComponent(1) // Mono display
    {
        ///setNumChannels(2); // Stereo display
        setColours(juce::Colours::black, juce::Colours::green);
        setRepaintRate(60); // 60 Hz repaint rate
    }

    ~LiveScrollingAudioVisualiser() override = default;

    // AudioIODeviceCallback implementation
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override
    {
        if (numInputChannels > 0 && inputChannelData[0] != nullptr)
        {
            pushBuffer(inputChannelData, 1, numSamples);
        }
        // Clear output buffers to avoid noise
        for (int i = 0; i < numOutputChannels; ++i)
        {
            if (outputChannelData[i] != nullptr)
                juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        // Prepare for audio processing if needed
    }
    
    void audioDeviceStopped() override
    {
        // Clean up resources if needed
    }
};