#include "ParametricEqualiserProcessor.h"
#include "ParametricEqualiserEditor.h"

juce::String ParametricEqualiserProcessor::paramOutput("output");
juce::String ParametricEqualiserProcessor::paramType("type");
juce::String ParametricEqualiserProcessor::paramFrequency("frequency");
juce::String ParametricEqualiserProcessor::paramQuality("quality");
juce::String ParametricEqualiserProcessor::paramGain("gain");
juce::String ParametricEqualiserProcessor::paramActive("active");

namespace IDs
{
    juce::String editor{ "editor" };
    juce::String sizeX{ "size-x" };
    juce::String sizeY{ "size-y" };
}

std::vector<ParametricEqualiserProcessor::Band> createDefaultBands()
{
    std::vector<ParametricEqualiserProcessor::Band> defaults;
    defaults.push_back(ParametricEqualiserProcessor::Band(TRANS("Lowest"), juce::Colours::blue, ParametricEqualiserProcessor::HighPass, 20.0f, 0.707f));
    defaults.push_back(ParametricEqualiserProcessor::Band(TRANS("Low"), juce::Colours::brown, ParametricEqualiserProcessor::LowShelf, 250.0f, 0.707f));
    defaults.push_back(ParametricEqualiserProcessor::Band(TRANS("Low Mids"), juce::Colours::green, ParametricEqualiserProcessor::Peak, 500.0f, 0.707f));
    defaults.push_back(ParametricEqualiserProcessor::Band(TRANS("High Mids"), juce::Colours::coral, ParametricEqualiserProcessor::Peak, 1000.0f, 0.707f));
    defaults.push_back(ParametricEqualiserProcessor::Band(TRANS("High"), juce::Colours::orange, ParametricEqualiserProcessor::HighShelf, 5000.0f, 0.707f));
    defaults.push_back(ParametricEqualiserProcessor::Band(TRANS("Highest"), juce::Colours::red, ParametricEqualiserProcessor::LowPass, 12000.0f, 0.707f));
    return defaults;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> params;

    // setting defaults
    const float maxGain = juce::Decibels::decibelsToGain(24.0f);
    auto defaults = createDefaultBands();
    {
        auto param = std::make_unique<juce::AudioParameterFloat>(ParametricEqualiserProcessor::paramOutput, TRANS("Output"),
            juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f,
            TRANS("Output level"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

        auto group = std::make_unique<juce::AudioProcessorParameterGroup>("global", TRANS("Globals"), "|", std::move(param));
        params.push_back(std::move(group));
    }

    for (size_t i = 0; i < defaults.size(); ++i)
    {
        auto prefix = "Q" + juce::String(i + 1) + ": ";

        auto typeParameter = std::make_unique<juce::AudioParameterChoice>(ParametricEqualiserProcessor::getTypeParamName(i),
            prefix + TRANS("Filter Type"),
            ParametricEqualiserProcessor::getFilterTypeNames(),
            defaults[i].type);

        auto freqParameter = std::make_unique<juce::AudioParameterFloat>(ParametricEqualiserProcessor::getFrequencyParamName(i),
            prefix + TRANS("Frequency"),
            juce::NormalisableRange<float> {20.0f, 20000.0f, 1.0f, std::log(0.5f) / std::log(980.0f / 19980.0f)},
            defaults[i].frequency,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return (value < 1000.0f) ?
            juce::String(value, 0) + " Hz" :
            juce::String(value / 1000.0f, 2) + " kHz"; },
            [](juce::String text) { return text.endsWith(" kHz") ?
            text.dropLastCharacters(4).getFloatValue() * 1000.0f :
            text.dropLastCharacters(3).getFloatValue(); });

        auto qltyParameter = std::make_unique<juce::AudioParameterFloat>(ParametricEqualiserProcessor::getQualityParamName(i),
            prefix + TRANS("Quality"),
            juce::NormalisableRange<float> {0.1f, 10.0f, 1.0f, std::log(0.5f) / std::log(0.9f / 9.9f)},
            defaults[i].quality,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1); },
            [](const juce::String& text) { return text.getFloatValue(); });

        auto gainParameter = std::make_unique<juce::AudioParameterFloat>(ParametricEqualiserProcessor::getGainParamName(i),
            prefix + TRANS("Gain"),
            juce::NormalisableRange<float> {1.0f / maxGain, maxGain, 0.001f,
            std::log(0.5f) / std::log((1.0f - (1.0f / maxGain)) / (maxGain - (1.0f / maxGain)))},
            defaults[i].gain,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

        auto actvParameter = std::make_unique<juce::AudioParameterBool>(ParametricEqualiserProcessor::getActiveParamName(i),
            prefix + TRANS("Active"),
            defaults[i].active,
            juce::String(),
            [](float value, int) {return value > 0.5f ? TRANS("active") : TRANS("bypassed"); },
            [](juce::String text) {return text == TRANS("active"); });

        auto group = std::make_unique<juce::AudioProcessorParameterGroup>("band" + juce::String(i), defaults[i].name, "|",
            std::move(typeParameter),
            std::move(freqParameter),
            std::move(qltyParameter),
            std::move(gainParameter),
            std::move(actvParameter));

        params.push_back(std::move(group));
    }
    return { params.begin(), params.end() };
}

//==============================================================================

ParametricEqualiserProcessor::ParametricEqualiserProcessor() :
    AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ),
    _parameters(*this, &_undo, "PARAMS", createParameterLayout())
{
    _frequencies.resize(300);
    for (size_t i = 0; i < _frequencies.size(); ++i) {
        _frequencies[i] = 20.0 * std::pow(2.0, i / 30.0);
    }
    _magnitudes.resize(_frequencies.size());

    _bands = createDefaultBands();

    for (size_t i = 0; i < _bands.size(); ++i)
    {
        _bands[i].magnitudes.resize(_frequencies.size(), 1.0);

        _parameters.addParameterListener(getTypeParamName(i), this);
        _parameters.addParameterListener(getFrequencyParamName(i), this);
        _parameters.addParameterListener(getQualityParamName(i), this);
        _parameters.addParameterListener(getGainParamName(i), this);
        _parameters.addParameterListener(getActiveParamName(i), this);
    }
    _parameters.addParameterListener(paramOutput, this);
    _parameters.state = juce::ValueTree("PROGRAM_Name");
}

ParametricEqualiserProcessor::~ParametricEqualiserProcessor() {
};

//==============================================================================

bool ParametricEqualiserProcessor::checkForNewAnalyserData()
{
    return _inputAnalyser.checkForNewData() || _outputAnalyser.checkForNewData();
}

void ParametricEqualiserProcessor::createFrequencyPlot(juce::Path& p, 
                                                       const std::vector<double>& mags, 
                                                       const juce::Rectangle<int> bounds, 
                                                       float pixelsPerDouble) {
    p.startNewSubPath(float(bounds.getX()), mags[0] > 0 ? float(bounds.getCentreY() - pixelsPerDouble * std::log(mags[0]) / std::log(2.0)) : bounds.getBottom());
    const auto xFactor = static_cast<double> (bounds.getWidth()) / _frequencies.size();
    for (size_t i = 1; i < _frequencies.size(); ++i)
    {
        p.lineTo(float(bounds.getX() + i * xFactor),
            float(mags[i] > 0 ? bounds.getCentreY() - pixelsPerDouble * std::log(mags[i]) / std::log(2.0) : bounds.getBottom()));
    }
};

void ParametricEqualiserProcessor::createAnalyserPlot(juce::Path& p, 
                                                      const juce::Rectangle<int> bounds, 
                                                      float minFreq, 
                                                      bool input) {
    if (input)
        _inputAnalyser.createPath(p, bounds.toFloat(), minFreq);
    else
        _outputAnalyser.createPath(p, bounds.toFloat(), minFreq);
};  

ParametricEqualiserProcessor::Band* ParametricEqualiserProcessor::getBand(size_t index)
{
    if (juce::isPositiveAndBelow(index, _bands.size()))
        return &_bands[index];
    return nullptr;
};

juce::String ParametricEqualiserProcessor::getBandID(size_t index)
{
    switch (index)
    {
    case 0: return "Lowest";
    case 1: return "Low";
    case 2: return "Low Mids";
    case 3: return "High Mids";
    case 4: return "High";
    case 5: return "Highest";
    default: break;
    }
    return "unknown";
}

juce::String ParametricEqualiserProcessor::getBandName(size_t index) const {
    if (juce::isPositiveAndBelow(index, _bands.size()))
        return _bands[index].name;
    return TRANS ("unknown");
}

juce::Colour ParametricEqualiserProcessor::getBandColour(size_t index) const {
    if (juce::isPositiveAndBelow(index, _bands.size()))
        return _bands[index].colour;
    return juce::Colours::silver;
}

int ParametricEqualiserProcessor::getBandIndexFromID(juce::String paramID)
{
    for (size_t i = 0; i < 6; ++i)
        if (paramID.startsWith(getBandID(i) + "-"))
            return int(i);
    return -1;
}

void ParametricEqualiserProcessor::setBandSolo(int index)
{
    _soloedBand = index;
    updateBypassedStates();
}

bool ParametricEqualiserProcessor::getBandSolo(int index) const {
    return index == _soloedBand;
};

size_t ParametricEqualiserProcessor::getNumBands() const
{
    return _bands.size();
}

const std::vector<double>& ParametricEqualiserProcessor::getMagnitudes() {
    return _magnitudes;
}

juce::String ParametricEqualiserProcessor::getTypeParamName(size_t index)
{
    return getBandID(index) + "-" + paramType;
}

juce::String ParametricEqualiserProcessor::getFrequencyParamName(size_t index)
{
    return getBandID(index) + "-" + paramFrequency;
}

juce::String ParametricEqualiserProcessor::getQualityParamName(size_t index)
{
    return getBandID(index) + "-" + paramQuality;
}

juce::String ParametricEqualiserProcessor::getGainParamName(size_t index)
{
    return getBandID(index) + "-" + paramGain;
}

juce::String ParametricEqualiserProcessor::getActiveParamName(size_t index)
{
    return getBandID(index) + "-" + paramActive;
}

juce::StringArray ParametricEqualiserProcessor::getFilterTypeNames()
{
    return {
        TRANS("No Filter"),
        TRANS("High Pass"),
        TRANS("1st High Pass"),
        TRANS("Low Shelf"),
        TRANS("Band Pass"),
        TRANS("All Pass"),
        TRANS("1st All Pass"),
        TRANS("Notch"),
        TRANS("Peak"),
        TRANS("High Shelf"),
        TRANS("1st Low Pass"),
        TRANS("Low Pass")
    };
}

void ParametricEqualiserProcessor::parameterChanged(const juce::String& parameter, float newValue) {
    if (parameter == paramOutput) {
        _filterChain.get<6>().setGainLinear(newValue);
        updatePlots();
        return;
    }
    int index = getBandIndexFromID(parameter);
    if (juce::isPositiveAndBelow(index, _bands.size()))
    {
        auto* band = getBand(size_t(index));
        if (parameter.endsWith(paramType)) {
            band->type = static_cast<FilterType> (static_cast<int> (newValue));
        }
        else if (parameter.endsWith(paramFrequency)) {
            band->frequency = newValue;
        }
        else if (parameter.endsWith(paramQuality)) {
            band->quality = newValue;
        }
        else if (parameter.endsWith(paramGain)) {
            band->gain = newValue;
        }
        else if (parameter.endsWith(paramActive)) {
            band->active = newValue >= 0.5f;
        }
        updateBand(size_t(index));
    }
};

void ParametricEqualiserProcessor::updateBand(const size_t index) {
    if (_sampleRate > 0) {
        juce::dsp::IIR::Coefficients<float>::Ptr newCoefficients;
        switch (_bands[index].type) {
            case NoFilter:
                newCoefficients = new juce::dsp::IIR::Coefficients<float>(1, 0, 1, 0);
                break;
            case LowPass:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(_sampleRate, _bands[index].frequency, _bands[index].quality);
                break;
            case LowPass1st:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(_sampleRate, _bands[index].frequency);
                break;
            case LowShelf:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(_sampleRate, _bands[index].frequency, _bands[index].quality, _bands[index].gain);
                break;
            case BandPass:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(_sampleRate, _bands[index].frequency, _bands[index].quality);
                break;
            case AllPass:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(_sampleRate, _bands[index].frequency, _bands[index].quality);
                break;
            case AllPass1st:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderAllPass(_sampleRate, _bands[index].frequency);
                break;
            case Notch:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(_sampleRate, _bands[index].frequency, _bands[index].quality);
                break;
            case Peak:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(_sampleRate, _bands[index].frequency, _bands[index].quality, _bands[index].gain);
                break;
            case HighShelf:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(_sampleRate, _bands[index].frequency, _bands[index].quality, _bands[index].gain);
                break;
            case HighPass1st:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(_sampleRate, _bands[index].frequency);
                break;
            case HighPass:
                newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(_sampleRate, _bands[index].frequency, _bands[index].quality);
                break;
            case LastFilterID:
            default:
                break;
        }

        if (newCoefficients)
        {
            {
                // minimise lock scope, get<0>() needs to be a  compile time constant
                juce::ScopedLock processLock(getCallbackLock());
                if (index == 0)
                    *_filterChain.get<0>().state = *newCoefficients;
                else if (index == 1)
                    *_filterChain.get<1>().state = *newCoefficients;
                else if (index == 2)
                    *_filterChain.get<2>().state = *newCoefficients;
                else if (index == 3)
                    *_filterChain.get<3>().state = *newCoefficients;
                else if (index == 4)
                    *_filterChain.get<4>().state = *newCoefficients;
                else if (index == 5)
                    *_filterChain.get<5>().state = *newCoefficients;
            }
            newCoefficients->getMagnitudeForFrequencyArray(_frequencies.data(),
                _bands[index].magnitudes.data(),
                _frequencies.size(), _sampleRate);
        }
        updateBypassedStates();
        updatePlots();
    }
};  
    
void ParametricEqualiserProcessor::updateBypassedStates() {
    if (juce::isPositiveAndBelow(_soloedBand, _bands.size())) {
        _filterChain.setBypassed<0>(_soloedBand != 0);
        _filterChain.setBypassed<1>(_soloedBand != 1);
        _filterChain.setBypassed<2>(_soloedBand != 2);
        _filterChain.setBypassed<3>(_soloedBand != 3);
        _filterChain.setBypassed<4>(_soloedBand != 4);
        _filterChain.setBypassed<5>(_soloedBand != 5);
    }
    else {
        _filterChain.setBypassed<0>(!_bands[0].active);
        _filterChain.setBypassed<1>(!_bands[1].active);
        _filterChain.setBypassed<2>(!_bands[2].active);
        _filterChain.setBypassed<3>(!_bands[3].active);
        _filterChain.setBypassed<4>(!_bands[4].active);
        _filterChain.setBypassed<5>(!_bands[5].active);
    }
    updatePlots();
};

void ParametricEqualiserProcessor::updatePlots() {
    auto gain = _filterChain.get<6>().getGainLinear();
    std::fill(_magnitudes.begin(), _magnitudes.end(), gain);

    if (juce::isPositiveAndBelow(_soloedBand, _bands.size())) {
        juce::FloatVectorOperations::multiply(_magnitudes.data(), _bands[size_t(_soloedBand)].magnitudes.data(), static_cast<int> (_magnitudes.size()));
    }
    else
    {
        for (size_t i = 0; i < _bands.size(); ++i)
            if (_bands[i].active)
                juce::FloatVectorOperations::multiply(_magnitudes.data(), _bands[i].magnitudes.data(), static_cast<int>(_magnitudes.size()));
    }
    sendChangeMessage();
};


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Implement all pure virtual methods from juce::AudioProcessor

const juce::String  ParametricEqualiserProcessor::getName() const { 
    return "Parametric Equalizer"; 
}

void  ParametricEqualiserProcessor::prepareToPlay(double newSampleRate, int newSamplesPerBlock) {
    _sampleRate = newSampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = newSampleRate;
    spec.maximumBlockSize = juce::uint32(newSamplesPerBlock);
    spec.numChannels = juce::uint32(getTotalNumOutputChannels());

    for (size_t i = 0; i < _bands.size(); ++i) {
        updateBand(i);
    }
    _filterChain.get<6>().setGainLinear(*_parameters.getRawParameterValue(paramOutput));

    updatePlots();

    _filterChain.prepare(spec);

    _inputAnalyser.setupAnalyser(int(_sampleRate), float(_sampleRate));
    _outputAnalyser.setupAnalyser(int(_sampleRate), float(_sampleRate));
}

void  ParametricEqualiserProcessor::releaseResources() {
    _inputAnalyser.stopThread(1000);
    _outputAnalyser.stopThread(1000);
}

void ParametricEqualiserProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                                juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    if (getActiveEditor() != nullptr) {
        _inputAnalyser.addAudioData(buffer, 0, getTotalNumInputChannels());
    }

    if (_wasBypassed) {
        _filterChain.reset();
        _wasBypassed = false;
    }
    
    juce::dsp::AudioBlock<float> ioBuffer(buffer);
    juce::dsp::ProcessContextReplacing<float> context(ioBuffer);
    _filterChain.process(context);

    if (getActiveEditor() != nullptr) {
        _outputAnalyser.addAudioData(buffer, 0, getTotalNumOutputChannels());
    }
}

bool ParametricEqualiserProcessor::isBusesLayoutSupported(const BusesLayout& busesLayout) const { 
    
    // If the processor is running as a plugin it should probably return true????
    return false;
///////    return true;
}

juce::AudioProcessorEditor* ParametricEqualiserProcessor::createEditor() { 
    return new ParametricEqualiserEditor(*this, this->_parameters);
}

bool ParametricEqualiserProcessor::hasEditor() const {
    return true; 
}

bool  ParametricEqualiserProcessor::acceptsMidi() const {
    return false; 
}

bool  ParametricEqualiserProcessor::producesMidi() const { 
    return false; 
}

bool  ParametricEqualiserProcessor::isMidiEffect() const {
    return false; 
}

double  ParametricEqualiserProcessor::getTailLengthSeconds() const { 
    return 0.0; 
}

int  ParametricEqualiserProcessor::getNumPrograms() {
    return 1; 
}

int  ParametricEqualiserProcessor::getCurrentProgram() { 
   return 0; 
}

void  ParametricEqualiserProcessor::setCurrentProgram(int) {
}

const juce::String  ParametricEqualiserProcessor::getProgramName(int) { 
    return {}; 
}

void  ParametricEqualiserProcessor::changeProgramName(int, const juce::String&) {
}

void  ParametricEqualiserProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = _parameters.copyState();

    auto editorProperties = state.getOrCreateChildWithName(IDs::editor, nullptr);
    editorProperties.setProperty(IDs::sizeX, _editorSize.x, nullptr);
    editorProperties.setProperty(IDs::sizeY, _editorSize.y, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ParametricEqualiserProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr)
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid()) {
            _parameters.state = tree;
            auto editor = _parameters.state.getChildWithName(IDs::editor);
            if (editor.isValid())
            {
                _editorSize.setX(editor.getProperty(IDs::sizeX, 900));
                _editorSize.setY(editor.getProperty(IDs::sizeY, 500));
                if (auto* thisEditor = getActiveEditor())
                    thisEditor->setSize(_editorSize.x, _editorSize.y);
            }
        }
    }
}

juce::Point<int> ParametricEqualiserProcessor::getSavedSize() const {
    return _editorSize;
}

void ParametricEqualiserProcessor::setSavedSize(const juce::Point<int>& size) { 
    _editorSize = size; 
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


