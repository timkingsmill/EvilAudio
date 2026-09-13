#pragma once

#include "ParametricEqualiserProcessor.h"
class ParametricEqualiserEditor :
    public juce::AudioProcessorEditor,
    public juce::ChangeListener,
    public juce::Timer
{
public:
    /// Attachment type used for sliders (alias).
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    /// Attachment type used for buttons (alias).
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

    ParametricEqualiserEditor(ParametricEqualiserProcessor& audioProcessor,
                              juce::AudioProcessorValueTreeState& vts);
    ~ParametricEqualiserEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* sender) override;
    void timerCallback() override;
    void updateFrequencyResponses();
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
private:
    static float getPositionForFrequency(float freq);
    static float getFrequencyForPosition(float pos);
    static float getPositionForGain(float gain, float top, float bottom);
    static float getGainForPosition(float pos, float top, float bottom);

    class BandEditor : public juce::Component,
                       public juce::Button::Listener
    {
    public:
        BandEditor(size_t index, ParametricEqualiserProcessor& audioProcessor, 
                                 juce::AudioProcessorValueTreeState& audioProcessorState);
        void resized() override;
        void updateControls(ParametricEqualiserProcessor::FilterType type);
        void updateSoloState(bool isSolo);
        void setFrequency(float frequency);
        void setGain(float gain);
        void setType(int type);
        void buttonClicked(juce::Button* b) override;

        juce::Path frequencyResponse;

    private:

        /** Reference to the parent audio processor for parameter/state queries and actions. */
        ParametricEqualiserProcessor& _audioProcessor;
        
        /** Reference to the AudioProcessorValueTreeState for parameter attachments. */
        juce::AudioProcessorValueTreeState& _audioProcessorState;
        
        /** The index of the band this editor represents. */
        size_t _index;

        /** Frame around the band controls. */
        juce::GroupComponent _frame;
        /** Combo box that selects the filter type for this band. */
        juce::ComboBox _filterTypeComboBox;

        /** Rotary slider controlling the filter frequency. */
        juce::Slider _frequencySlider{
            juce::Slider::RotaryHorizontalVerticalDrag,
            juce::Slider::TextBoxBelow
        };
        /** Rotary slider controlling the filter quality (Q). */
        juce::Slider _quality{
            juce::Slider::RotaryHorizontalVerticalDrag,
            juce::Slider::TextBoxBelow
        };
        /** Rotary slider controlling the filter gain (dB). */
        juce::Slider _gain{
            juce::Slider::RotaryHorizontalVerticalDrag,
            juce::Slider::TextBoxBelow
        };
        /** Solo toggle button for this band (labelled "S"). */
        juce::TextButton _solo{
            TRANS("S")
        };
        /** Activate/enable toggle button for this band (labelled "A"). */
        juce::TextButton _activate{
            TRANS("A")
        };
        /** ComboBox attachments used to connect the UI to the VTS. */
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> _boxAttachments;
        /** Slider attachments used to connect the UI sliders to the VTS. */
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> _attachments;
        /** Button attachments used to connect the UI buttons to the VTS. */
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> _buttonAttachments;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandEditor)

    };

private:
    /** Reference to the processor that owns this editor. */
    ParametricEqualiserProcessor& _audioProcessor;


    /** Reference to the AudioProcessorValueTreeState used for all parameter attachments. */
    juce::AudioProcessorValueTreeState& _audioProcessorState;

    /** OwnedArray that stores attachments for any top-level sliders. */
    juce::OwnedArray<SliderAttachment> _sliderAttachments;

    /** Collection of per-band BandEditor child components. */
    juce::OwnedArray<BandEditor> _bandEditors;
    /** Index of the band currently being dragged by the user, or -1 if none. */
    int _draggingBand = -1;
    /** True while the user is dragging the gain axis specifically. */
    bool _draggingGain = false;

    /** Frame grouping for the output gain control. */
    juce::GroupComponent _outputGainFrame;
    /** Rotary slider used for the global/output gain. */
    juce::Slider _outputGainSlider{
        juce::Slider::RotaryHorizontalVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    /** Attachment that binds the output gain slider to the VTS. */
    std::unique_ptr<SliderAttachment> _outputGainSliderAttachment;


    /** Rectangle describing the plotting area for frequency response rendering. */
    juce::Rectangle<int> _plotFrame;
    /** Rectangle reserved for branding/logo area. */
    juce::Rectangle<int> _brandingFrame;
    /** Shared tooltip window used for contextual hints on controls. */
    juce::SharedResourcePointer<juce::TooltipWindow> _tooltipWindow;
    /** Popup menu used for context-sensitive options (right-click menu). */
    juce::PopupMenu _contextMenu;
    /** Cached full-band frequency response path used for painting. */
    juce::Path _frequencyResponsePath;
    /** Cached analyser path used when visualising audio in real-time. */
    juce::Path _analyserPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEqualiserEditor)

};