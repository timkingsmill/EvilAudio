#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Analyser.h"

class ParametricEqualiserProcessor : 
    public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener,
    public juce::ChangeBroadcaster
{
public:
    enum FilterType
    {
        NoFilter = 0,
        HighPass,
        HighPass1st,
        LowShelf,
        BandPass,
        AllPass,
        AllPass1st,
        Notch,
        Peak,
        HighShelf,
        LowPass1st,
        LowPass,
        LastFilterID
    };

    static juce::String paramOutput;
    static juce::String paramType;
    static juce::String paramFrequency;
    static juce::String paramQuality;
    static juce::String paramGain;
    static juce::String paramActive;

    static juce::String getBandID(size_t index);
    static juce::String getTypeParamName(size_t index);
    static juce::String getFrequencyParamName(size_t index);
    static juce::String getQualityParamName(size_t index);
    static juce::String getGainParamName(size_t index);
    static juce::String getActiveParamName(size_t index);

    static juce::StringArray getFilterTypeNames();

    struct Band {
        Band(const juce::String& nameToUse, juce::Colour colourToUse, FilterType typeToUse,
            float frequencyToUse, float qualityToUse, float gainToUse = 1.0f, bool shouldBeActive = true)
            : name(nameToUse),
            colour(colourToUse),
            type(typeToUse),
            frequency(frequencyToUse),
            quality(qualityToUse),
            gain(gainToUse),
            active(shouldBeActive)
        {
        }

        juce::String name;
        juce::Colour colour;
        FilterType   type = BandPass;
        float        frequency = 1000.0f;
        float        quality = 1.0f;
        float        gain = 1.0f;
        bool         active = true;
        std::vector<double> magnitudes;
    };

public:
    ParametricEqualiserProcessor();
    ~ParametricEqualiserProcessor() override;

    bool checkForNewAnalyserData();
    void createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble);
    void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq, bool input);

    Band* getBand(size_t index);
    bool getBandSolo(int index) const;
    juce::String getBandName(size_t index) const;
    juce::Colour getBandColour(size_t index) const;
    int getBandIndexFromID(juce::String paramID);
    size_t getNumBands() const;
    const std::vector<double>& getMagnitudes();

    void setBandSolo(int index);

    // Implement all pure virtual methods from juce::AudioProcessor
    const juce::String getName() const override;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;
    
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::Point<int> getSavedSize() const;
    void setSavedSize(const juce::Point<int>& size);

    // AudioProcessorValueTreeState::Listener method override.
    void parameterChanged(const juce::String& parameter, float newValue) override;

private:
    void updateBand(const size_t index);
    void updateBypassedStates();
    void updatePlots();

    juce::AudioProcessorValueTreeState _parameters;
    juce::UndoManager _undo;

    std::vector<Band> _bands;
    std::vector<double> _frequencies;
    std::vector<double> _magnitudes;

    double _sampleRate = 0;
    int _soloedBand = -1;
    bool _wasBypassed = true;

    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using Gain = juce::dsp::Gain<float>;
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, Gain> _filterChain;

    Analyser<float> _inputAnalyser;
    Analyser<float> _outputAnalyser;

    juce::Point<int> _editorSize = { 900, 500 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEqualiserProcessor)

};
