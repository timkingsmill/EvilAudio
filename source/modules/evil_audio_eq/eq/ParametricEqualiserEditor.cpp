#include "ParametricEqualiserEditor.h"

static int clickRadius = 4;
static float maxDB = 24.0f;

ParametricEqualiserEditor::ParametricEqualiserEditor(ParametricEqualiserProcessor& audioProcessor,
                                                     juce::AudioProcessorValueTreeState& vts)
    : juce::AudioProcessorEditor(&audioProcessor),
    _audioProcessor(audioProcessor),
    _audioProcessorState(vts)
{
    _tooltipWindow->setMillisecondsBeforeTipAppears(1000);
    // Create edtor controls for each equalizer band
    for (size_t i = 0; i < _audioProcessor.getNumBands(); ++i)
    {
        auto* bandEditor = _bandEditors.add(new BandEditor(i, _audioProcessor, _audioProcessorState));
        addAndMakeVisible(bandEditor);
    }
    // Create the output frame control.
    _outputGainFrame.setText(TRANS("Output"));
    _outputGainFrame.setTextLabelPosition(juce::Justification::centred);
    addAndMakeVisible(_outputGainFrame);
    
    // Create the output gain slider.   
    addAndMakeVisible(_outputGainSlider);
    _outputGainSlider.setTooltip(TRANS("Overall Gain"));

    // Attach the output gain slider to the corresponding parameter.
    _outputGainSliderAttachment.reset(new SliderAttachment(_audioProcessorState, 
                                                           ParametricEqualiserProcessor::paramOutput, 
                                                           _outputGainSlider));

    // Initialize the size of the equalizer editor.
    auto size = _audioProcessor.getSavedSize(); 
    setResizable(false, false);
    //setResizable(true, true);
    setSize(size.x, size.y);
    //setSize(size.getHeight(), size.getWidth());
    setResizeLimits(800, 450, 2990, 1800);

    updateFrequencyResponses();

#ifdef JUCE_OPENGL
    openGLContext.attachTo(*getTopLevelComponent());
#endif

    _audioProcessor.addChangeListener(this);

    // Refresh the display at 30 Hz.
    startTimerHz(30);
}

ParametricEqualiserEditor::~ParametricEqualiserEditor()
{
    juce::PopupMenu::dismissAllActiveMenus();
    _audioProcessor.removeChangeListener(this);
#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif
}

void ParametricEqualiserEditor::paint(juce::Graphics& g) {
    juce::Graphics::ScopedSaveState state(g);

    const auto inputColour = juce::Colours::greenyellow;
    const auto outputColour = juce::Colours::indianred;

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(12.0f);
    g.setColour(juce::Colours::silver);
    g.drawRoundedRectangle(_plotFrame.toFloat(), 5, 2);

    for (int i = 0; i < 10; ++i) {
        g.setColour(juce::Colours::silver.withAlpha(0.3f));
        auto x = _plotFrame.getX() + _plotFrame.getWidth() * i * 0.1f;
        if (i > 0) g.drawVerticalLine(juce::roundToInt(x), float(_plotFrame.getY()), float(_plotFrame.getBottom()));

        g.setColour(juce::Colours::silver);
        auto freq = getFrequencyForPosition(i * 0.1f);
        g.drawFittedText((freq < 1000) ? juce::String(freq) + " Hz" :
            juce::String(freq / 1000, 1) + " kHz",
            juce::roundToInt(x + 3), _plotFrame.getBottom() - 18, 50, 15, juce::Justification::left, 1);
    }

    g.setColour(juce::Colours::silver.withAlpha(0.3f));
    g.drawHorizontalLine(juce::roundToInt(_plotFrame.getY() + 0.25 * _plotFrame.getHeight()), float(_plotFrame.getX()), float(_plotFrame.getRight()));
    g.drawHorizontalLine(juce::roundToInt(_plotFrame.getY() + 0.75 * _plotFrame.getHeight()), float(_plotFrame.getX()), float(_plotFrame.getRight()));

    g.setColour(juce::Colours::silver);
    g.drawFittedText(juce::String(maxDB) + " dB", _plotFrame.getX() + 3, _plotFrame.getY() + 2, 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(maxDB / 2) + " dB", _plotFrame.getX() + 3, juce::roundToInt(_plotFrame.getY() + 2 + 0.25 * _plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    g.drawFittedText(" 0 dB", _plotFrame.getX() + 3, juce::roundToInt(_plotFrame.getY() + 2 + 0.5 * _plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(-maxDB / 2) + " dB", _plotFrame.getX() + 3, juce::roundToInt(_plotFrame.getY() + 2 + 0.75 * _plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);

    g.reduceClipRegion(_plotFrame);

    //g.setFont(16.0f);

    // Draw the input analyser plot.
    _audioProcessor.createAnalyserPlot(_analyserPath, _plotFrame, 20.0f, true);
    g.setColour(inputColour);
    g.drawFittedText("Input", _plotFrame.reduced(8), juce::Justification::topRight, 1);
    g.strokePath(_analyserPath, juce::PathStrokeType(1.0));

    // Draw the output analyser plot.
    _audioProcessor.createAnalyserPlot(_analyserPath, _plotFrame, 20.0f, false);
    g.setColour(outputColour);
    g.drawFittedText("Output", _plotFrame.reduced(8, 28), juce::Justification::topRight, 1);
    g.strokePath(_analyserPath, juce::PathStrokeType(1.0));
            
    // Draw the frequency response for each band.
    for (size_t i = 0; i < _audioProcessor.getNumBands(); ++i) {
        auto* bandEditor = _bandEditors.getUnchecked(int(i));
        auto* band = _audioProcessor.getBand(i);

        g.setColour(band->active ? band->colour : band->colour.withAlpha(0.3f));
        g.strokePath(bandEditor->frequencyResponse, juce::PathStrokeType(1.0));
        
        g.setColour(_draggingBand == int(i) ? band->colour : band->colour.withAlpha(0.3f));
        auto x = juce::roundToInt(_plotFrame.getX() + _plotFrame.getWidth() * getPositionForFrequency(float(band->frequency)));
        auto y = juce::roundToInt(getPositionForGain(float(band->gain), float(_plotFrame.getY()), float(_plotFrame.getBottom())));
        g.drawVerticalLine(x, float(_plotFrame.getY()), float(y - 5));
        g.drawVerticalLine(x, float(y + 5), float(_plotFrame.getBottom()));
        g.fillEllipse(float(x - 3), float(y - 3), 6.0f, 6.0f);
    }
    
    g.setColour(juce::Colours::silver);
    g.strokePath(_frequencyResponsePath, juce::PathStrokeType(1.0f));
}

void ParametricEqualiserEditor::resized() {
    _audioProcessor.setSavedSize({ getWidth(), getHeight() });
    _plotFrame = getLocalBounds().reduced(3, 3);

    // Resize the band editor controls.
    auto bandSpace = _plotFrame.removeFromBottom(getHeight() / 2);
    auto bandEditorWidth = juce::roundToInt(bandSpace.getWidth()) / (_bandEditors.size() + 1);
    for (auto* bandEditor : _bandEditors) {
        bandEditor->setBounds(bandSpace.removeFromLeft(bandEditorWidth));
    }

    // Resize the output level control frame.
    _outputGainFrame.setBounds(bandSpace.removeFromTop(bandSpace.getHeight() / 2));
    _outputGainSlider.setBounds(_outputGainFrame.getBounds().reduced(8));

    _plotFrame.reduce(3, 3);
    _brandingFrame = bandSpace.reduced(5);

    updateFrequencyResponses();
}

void ParametricEqualiserEditor::updateFrequencyResponses() {
    auto pixelsPerDouble = 2.0f * _plotFrame.getHeight() / juce::Decibels::decibelsToGain(maxDB);

    for (int i = 0; i < _bandEditors.size(); ++i)
    {
        auto* bandEditor = _bandEditors.getUnchecked(i);

        if (auto* band = _audioProcessor.getBand(size_t(i)))
        {
            bandEditor->updateControls(band->type);
            bandEditor->frequencyResponse.clear();
            _audioProcessor.createFrequencyPlot(bandEditor->frequencyResponse, 
                                                band->magnitudes, _plotFrame.withX(_plotFrame.getX() + 1), pixelsPerDouble);
        }
        bandEditor->updateSoloState(_audioProcessor.getBandSolo(i));
    }
    _frequencyResponsePath.clear();
    _audioProcessor.createFrequencyPlot(_frequencyResponsePath, 
                                        _audioProcessor.getMagnitudes(), _plotFrame, pixelsPerDouble);
};

float ParametricEqualiserEditor::getPositionForFrequency(float freq)
{
    return (std::log(freq / 20.0f) / std::log(2.0f)) / 10.0f;
}

float ParametricEqualiserEditor::getFrequencyForPosition(float pos)
{
    return 20.0f * std::pow(2.0f, pos * 10.0f);
}

float ParametricEqualiserEditor::getPositionForGain(float gain, float top, float bottom)
{
    return juce::jmap(juce::Decibels::gainToDecibels(gain, -maxDB), -maxDB, maxDB, bottom, top);
}

float ParametricEqualiserEditor::getGainForPosition(float pos, float top, float bottom)
{
    return juce::Decibels::decibelsToGain(juce::jmap(pos, bottom, top, -maxDB, maxDB), -maxDB);
}

void ParametricEqualiserEditor::changeListenerCallback(juce::ChangeBroadcaster* sender)
{
    juce::ignoreUnused(sender);
    updateFrequencyResponses();
    repaint();
}

void ParametricEqualiserEditor::timerCallback()
{
    if (_audioProcessor.checkForNewAnalyserData())
        repaint(_plotFrame);
}

void ParametricEqualiserEditor::mouseDown(const juce::MouseEvent& e) {
    if (!e.mods.isPopupMenu() || !_plotFrame.contains(e.x, e.y))
        return;

    for (int i = 0; i < _bandEditors.size(); ++i)
    {
        if (auto* band = _audioProcessor.getBand(size_t(i)))
        {
            if (std::abs(_plotFrame.getX() + getPositionForFrequency(float(int(band->frequency)) * _plotFrame.getWidth())
                - e.position.getX()) < clickRadius)
            {
                _contextMenu.clear();
                const auto& names = ParametricEqualiserProcessor::getFilterTypeNames();
                for (int t = 0; t < names.size(); ++t)
                    _contextMenu.addItem(t + 1, names[t], true, band->type == t);

                _contextMenu.showMenuAsync(juce::PopupMenu::Options()
                    .withTargetComponent(this)
                    .withTargetScreenArea({ e.getScreenX(), e.getScreenY(), 1, 1 })
                    , [this, i](int selected)
                    {
                        if (selected > 0)
                            _bandEditors.getUnchecked(i)->setType(selected - 1);
                    });
                return;
            }
        }
    }

};

void ParametricEqualiserEditor::mouseMove(const juce::MouseEvent& e) {
    if (_plotFrame.contains(e.x, e.y))
    {
        for (int i = 0; i < _bandEditors.size(); ++i)
        {
            if (auto* band = _audioProcessor.getBand(size_t(i)))
            {
                auto pos = _plotFrame.getX() + getPositionForFrequency(float(band->frequency)) * _plotFrame.getWidth();

                if (std::abs(pos - e.position.getX()) < clickRadius)
                {
                    if (std::abs(getPositionForGain(float(band->gain), float(_plotFrame.getY()), float(_plotFrame.getBottom()))
                        - e.position.getY()) < clickRadius)
                    {
                        _draggingGain = _audioProcessorState.getParameter(_audioProcessor.getGainParamName(size_t(i)));
                        setMouseCursor(juce::MouseCursor(juce::MouseCursor::UpDownLeftRightResizeCursor));
                    }
                    else
                    {
                        setMouseCursor(juce::MouseCursor(juce::MouseCursor::LeftRightResizeCursor));
                    }

                    if (i != _draggingBand)
                    {
                        _draggingBand = i;
                        repaint(_plotFrame);
                    }
                    return;
                }
            }
        }
    };
}

void ParametricEqualiserEditor::mouseDrag(const juce::MouseEvent& e) {
    if (juce::isPositiveAndBelow(_draggingBand, _bandEditors.size()))
    {
        auto pos = (e.position.getX() - _plotFrame.getX()) / _plotFrame.getWidth();
        _bandEditors[_draggingBand]->setFrequency(getFrequencyForPosition(pos));
        if (_draggingGain)
            _bandEditors[_draggingBand]->setGain(getGainForPosition(e.position.getY(), float(_plotFrame.getY()), float(_plotFrame.getBottom())));
    }
};

void ParametricEqualiserEditor::mouseDoubleClick(const juce::MouseEvent& e) {
    if (_plotFrame.contains(e.x, e.y))
    {
        for (size_t i = 0; i < size_t(_bandEditors.size()); ++i)
        {
            if (auto* band = _audioProcessor.getBand(i))
            {
                if (std::abs(_plotFrame.getX() + getPositionForFrequency(float(band->frequency)) * _plotFrame.getWidth()
                    - e.position.getX()) < clickRadius)
                {
                    if (auto* param = _audioProcessorState.getParameter(_audioProcessor.getActiveParamName(i)))
                        param->setValueNotifyingHost(param->getValue() < 0.5f ? 1.0f : 0.0f);
                }
            }
        }
    }
};

//==========================================================

ParametricEqualiserEditor::BandEditor::BandEditor(size_t index, 
                                                  ParametricEqualiserProcessor& processor,
                                                  juce::AudioProcessorValueTreeState& audioProcessorState) :
    _index(index),
    _audioProcessor(processor),
    _audioProcessorState(audioProcessorState)
{
    _frame.setText(_audioProcessor.getBandName(_index));
    _frame.setTextLabelPosition(juce::Justification::centred);
    _frame.setColour(juce::GroupComponent::textColourId, _audioProcessor.getBandColour(_index));
    _frame.setColour(juce::GroupComponent::outlineColourId, _audioProcessor.getBandColour(_index));
    addAndMakeVisible(_frame);

    if (auto* choiceParameter = dynamic_cast<juce::AudioParameterChoice*>(_audioProcessorState.getParameter(_audioProcessor.getTypeParamName(index))))
        _filterTypeComboBox.addItemList(choiceParameter->choices, 1);
    addAndMakeVisible(_filterTypeComboBox);
    _boxAttachments.add(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(_audioProcessorState, _audioProcessor.getTypeParamName(index), _filterTypeComboBox));

    addAndMakeVisible(_frequencySlider);
    _attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(_audioProcessorState, _audioProcessor.getFrequencyParamName(index), _frequencySlider));
    _frequencySlider.setTooltip(TRANS("Filter's frequency"));

    addAndMakeVisible(_quality);
    _attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(_audioProcessorState, _audioProcessor.getQualityParamName(index), _quality));
    _quality.setTooltip(TRANS("Filter's steepness (Quality)"));

    addAndMakeVisible(_gain);
    _attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(_audioProcessorState, _audioProcessor.getGainParamName(index), _gain));
    _gain.setTooltip(TRANS("Filter's gain"));

    _solo.setClickingTogglesState(true);
    _solo.addListener(this);
    _solo.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    addAndMakeVisible(_solo);
    _solo.setTooltip(TRANS("Listen only through this filter (solo)"));

    _activate.setClickingTogglesState(true);
    _activate.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    _buttonAttachments.add(new juce::AudioProcessorValueTreeState::ButtonAttachment(_audioProcessorState, _audioProcessor.getActiveParamName(index), _activate));
    
    addAndMakeVisible(_activate);
    _activate.setTooltip(TRANS("Activate or deactivate this filter"));

};

void ParametricEqualiserEditor::BandEditor::resized() {
    auto localBounds = getLocalBounds();
    _frame.setBounds(localBounds);

    // Add some space around the controls.
    localBounds.reduce(8, 16);

    // Position the filter type selection combobox control
    // at the top of the frame.
    _filterTypeComboBox.setBounds(localBounds.removeFromTop(20));

    // Position the frequency slider.
    auto freqSliderBounds = localBounds.removeFromBottom(localBounds.getHeight() * 2 / 3);
    _frequencySlider.setBounds(freqSliderBounds.withTop(freqSliderBounds.getY() + 10));

    // Position the solo and activate buttons.
    auto buttonBounds = freqSliderBounds.reduced(5).withHeight(20);
    _solo.setBounds(buttonBounds.removeFromLeft(20));
    _activate.setBounds(buttonBounds.removeFromRight(20));

    // Position the quality and gain sliders within the remaining localBounds rectangle.
    _quality.setBounds(localBounds.removeFromLeft(localBounds.getWidth() / 2));
    _gain.setBounds(localBounds);
    
};

void ParametricEqualiserEditor::BandEditor::updateControls(ParametricEqualiserProcessor::FilterType type)
{
    switch (type) {
        case ParametricEqualiserProcessor::LowPass:
            _frequencySlider.setEnabled(true); _quality.setEnabled(true); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::LowPass1st:
            _frequencySlider.setEnabled(true); _quality.setEnabled(false); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::LowShelf:
            _frequencySlider.setEnabled(true); _quality.setEnabled(false); _gain.setEnabled(true);
            break;
        case ParametricEqualiserProcessor::BandPass:
            _frequencySlider.setEnabled(true); _quality.setEnabled(true); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::AllPass:
            _frequencySlider.setEnabled(true); _quality.setEnabled(false); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::AllPass1st:
            _frequencySlider.setEnabled(true); _quality.setEnabled(false); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::Notch:
            _frequencySlider.setEnabled(true); _quality.setEnabled(true); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::Peak:
            _frequencySlider.setEnabled(true); _quality.setEnabled(true); _gain.setEnabled(true);
            break;
        case ParametricEqualiserProcessor::HighShelf:
            _frequencySlider.setEnabled(true); _quality.setEnabled(true); _gain.setEnabled(true);
            break;
        case ParametricEqualiserProcessor::HighPass1st:
            _frequencySlider.setEnabled(true); _quality.setEnabled(false); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::HighPass:
            _frequencySlider.setEnabled(true); _quality.setEnabled(true); _gain.setEnabled(false);
            break;
        case ParametricEqualiserProcessor::LastFilterID:
        case ParametricEqualiserProcessor::NoFilter:
        default:
            _frequencySlider.setEnabled(true);
            _quality.setEnabled(true);
            _gain.setEnabled(true);
            break;
    }
}

void ParametricEqualiserEditor::BandEditor::updateSoloState(bool isSolo)
{
    _solo.setToggleState(isSolo, juce::dontSendNotification);
}

void ParametricEqualiserEditor::BandEditor::setFrequency(float freq)
{
    _frequencySlider.setValue(freq, juce::sendNotification);
}

void ParametricEqualiserEditor::BandEditor::setGain(float gainToUse)
{
    _gain.setValue(gainToUse, juce::sendNotification);
}

void ParametricEqualiserEditor::BandEditor::setType(int type)
{
    _filterTypeComboBox.setSelectedId(type + 1, juce::sendNotification);
}

void ParametricEqualiserEditor::BandEditor::buttonClicked(juce::Button* b)
{
    if (b == &_solo) {
        _audioProcessor.setBandSolo(_solo.getToggleState() ? int(_index) : -1);
    }
}

//==========================================================
