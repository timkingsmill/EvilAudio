#pragma once

#include "ParametricEqualiserProcessor.h"

/*
Pseudocode plan (detailed step-by-step):
- Add comprehensive Doxygen-style documentation comments for the public API and important private members
  of the ParametricEqualiserEditor and its nested BandEditor class.
- For the top-level class:
  - Document purpose, lifecycle (constructor/destructor), and responsibilities.
  - Document each overridden virtual method (paint, resized, changeListenerCallback, timerCallback)
    including parameter intent and expected behaviour.
  - Document mouse event handlers and frequency/gain mapping helper functions with parameter and return details.
  - Document important private members (processor references, attachments, UI components, state flags).
- For the nested BandEditor:
  - Document purpose and relationship to the parent editor.
  - Document constructor and public API (resized, updateControls, updateSoloState, setFrequency, setGain, setType).
  - Document buttonClicked and the Path member used for per-band frequency response rendering.
  - Document key private UI components and attachments.
- Ensure all comments are concise, consistent, and use Doxygen-style tags (@param, @return, @note).
- Do not change any code semantics or signatures—only add comments.
*/

/// ParametricEqualiserEditor
/**
 *  UI editor for the parametric equaliser plugin.
 *
 *  Responsibilities:
 *   - Render global frequency response and per-band UI elements.
 *   - Manage UI attachments to the AudioProcessorValueTreeState.
 *   - Forward user interactions (mouse/controls) to the processor via VTS attachments.
 *   - Provide frequency/gain mapping utilities used by mouse interactions and painting.
 *
 *  Lifecycle:
 *   - Constructed with references to the processor and its AudioProcessorValueTreeState.
 *   - Registers change listeners and timers as needed in implementation.
 */
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

    /**
     * Constructor.
     *
     * @param audioProcessor Reference to the audio processor owning this editor.
     * @param vts            Reference to the processor's AudioProcessorValueTreeState used for attachments.
     */
    ParametricEqualiserEditor(ParametricEqualiserProcessor& audioProcessor,
                              juce::AudioProcessorValueTreeState& vts);
    /**
     * Destructor.
     *
     * Cleans up attachments and any owned resources. Because attachments are managed by OwnedArray
     * and smart pointers, the destructor primarily ensures proper JUCE component shutdown.
     */
    ~ParametricEqualiserEditor() override;

    /**
     * Paint the component.
     *
     * Responsible for drawing the frequency response overlays, analyser path, branding and any
     * static UI chrome that is not implemented as child components.
     *
     * @param g Graphics context to draw with.
     */
    void paint(juce::Graphics& g) override;
    /**
     * Called when the component is resized.
     *
     * Should lay out child components (BandEditor instances, frames, sliders) and update
     * rectangles used for plotting.
     */
    void resized() override;
    /**
     * Callback for change messages from ChangeBroadcaster(s) this editor listens to.
     *
     * @param sender The broadcaster that sent the change message (may be nullptr in some cases).
     */
    void changeListenerCallback(juce::ChangeBroadcaster* sender) override;
    /**
     * Timer callback invoked periodically when this object is started as a Timer.
     *
     * Typical use: update analyser data, refresh display paths, or throttle UI updates.
     */
    void timerCallback() override;
    /**
     * Recompute the frequency response Paths used for global and per-band rendering.
     *
     * This function should sample the filter responses and construct juce::Path objects
     * stored in the editor for painting.
     */
    void updateFrequencyResponses();

    /**
     * Mouse event handlers to support direct-manipulation of bands on the response plot.
     *
     * The implementation maps pointer coordinates into frequency/gain space and updates
     * filter parameters accordingly through the VTS attachments.
     */
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
private:
    /**
     * Convert a frequency in Hz to a normalized horizontal position in the plot.
     *
     * Uses the same log mapping used by the UI for consistent placement of bands.
     *
     * @param freq Frequency in Hz.
     * @return Normalized position [0, 1] corresponding to left..right of the plot.
     */
    static float getPositionForFrequency(float freq);
    /**
     * Convert a normalized horizontal position in the plot back to frequency in Hz.
     *
     * Inverse of getPositionForFrequency.
     *
     * @param pos Normalized position [0, 1].
     * @return Frequency in Hz corresponding to the position.
     */
    static float getFrequencyForPosition(float pos);
    /**
     * Convert a gain value (in dB) to a vertical position within the plot rectangle.
     *
     * @param gain   Gain in dB.
     * @param top    The top Y coordinate of the plot (usually smaller value).
     * @param bottom The bottom Y coordinate of the plot (usually larger value).
     * @return Y-position in pixels within the plot rectangle corresponding to the gain.
     */
    static float getPositionForGain(float gain, float top, float bottom);
    /**
     * Convert a vertical position within the plot rectangle back to a gain value (dB).
     *
     * Inverse of getPositionForGain.
     *
     * @param pos    Y-position in pixels within the plot rectangle.
     * @param top    The top Y coordinate of the plot.
     * @param bottom The bottom Y coordinate of the plot.
     * @return Gain in dB corresponding to the Y-position.
     */
    static float getGainForPosition(float pos, float top, float bottom);

    /**
     * Per-band embedded editor component.
     *
     * Each BandEditor contains controls (frequency/quality/gain) and buttons (solo/activate)
     * for a single EQ band and maintains a Path representing the per-band frequency response
     * for overlay rendering.
     */
    class BandEditor : public juce::Component,
                       public juce::Button::Listener
    {
    public:
        /**
         * Construct a BandEditor for a given filter band index.
         *
         * @param index          Zero-based index of the band this editor controls.
         * @param audioProcessor Reference to the parent audio processor used to query band state.
         */
        BandEditor(size_t index, ParametricEqualiserProcessor& audioProcessor, 
                                 juce::AudioProcessorValueTreeState& audioProcessorState);
        /**
         * Layout band control widgets when resized.
         *
         * Lays out the rotary sliders, combo box and buttons inside the band frame.
         */
        void resized() override;
        /**
         * Update UI controls to reflect a given filter type.
         *
         * Some filter types may enable/disable or alter available control ranges.
         *
         * @param type FilterType enum value from ParametricEqualiserProcessor.
         */
        void updateControls(ParametricEqualiserProcessor::FilterType type);
        /**
         * Update the visual solo state (e.g., highlight) for this band.
         *
         * @param isSolo True if the band is currently soloed.
         */
        void updateSoloState(bool isSolo);
        /**
         * Set the frequency slider/programmatic control to a specified frequency.
         *
         * @param frequency Frequency in Hz to set the control to.
         */
        void setFrequency(float frequency);
        /**
         * Set the gain slider/programmatic control to a specified gain (dB).
         *
         * @param gain Gain in dB to set the control to.
         */
        void setGain(float gain);
        /**
         * Set the selected filter type index in the combo box.
         *
         * @param type Integer index corresponding to the combo box entry.
         */
        void setType(int type);
        /**
         * Button::Listener implementation to handle button clicks from _solo/_activate.
         *
         * @param b Pointer to the button that was clicked.
         */
        void buttonClicked(juce::Button* b) override;

        /** Path representing this band's frequency response for overlay drawing. */
        juce::Path frequencyResponse;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandEditor)

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