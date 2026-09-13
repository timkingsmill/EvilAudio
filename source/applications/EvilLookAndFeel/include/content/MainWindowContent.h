#pragma once

#include <JuceHeader.h>

class LookAndFeelMainWindowContent final : public juce::Component
{
public:
    LookAndFeelMainWindowContent()
    {
        _lookAndFeel.reset(new EvilAudio_LookAndFeel());
        
        addAndMakeVisible(_rotarySlider);
        _rotarySlider.setValue(2.5);

        addAndMakeVisible(_verticalSlider);
        _verticalSlider.setValue(6.2);

        addAndMakeVisible(_barSlider);
        _barSlider.setValue(4.5);

        addAndMakeVisible(_incDecSlider);
        _incDecSlider.setRange(0.0, 10.0, 1.0);
        _incDecSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Horizontal);

        // Move this line to the constructor of your main application class
        // Set our custom look-and-feel as the default for all components
        // in this application.
        juce::LookAndFeel::setDefaultLookAndFeel(_lookAndFeel.get());
        //setLookAndFeel(_lookAndFeel.get());

        //auto& laf = getLookAndFeel();
        //laf.setColour(Slider::thumbColourId, juce::Colours::hotpink);
        //laf.setColour(Slider::trackColourId, juce::Colours::lightblue);
            //Colour::greyLevel(0.95f));

    }

    ~LookAndFeelMainWindowContent()
    {
        // Cleanup all references to our look-and-feel object before it's deleted
        setLookAndFeel(nullptr);

    }

    void paint(juce::Graphics& graphics) {
        // (Our component is opaque, so we must completely fill 
        // the background with a solid colour)
        graphics.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        auto row = area.removeFromTop(100);

        _rotarySlider.setBounds(row.removeFromLeft(100).reduced(10));
        _verticalSlider.setBounds(row.removeFromLeft(100).reduced(10));
        _barSlider.setBounds(row.removeFromLeft(200).reduced(10));
        _incDecSlider.setBounds(row.removeFromLeft(100).reduced(10));
    }

private:
    std::unique_ptr<juce::LookAndFeel> _lookAndFeel;

    using Slider = juce::Slider;
        Slider _rotarySlider { Slider::RotaryHorizontalVerticalDrag, Slider::NoTextBox },
               _verticalSlider { Slider::LinearVertical, Slider::NoTextBox },
               _barSlider { Slider::LinearBar, Slider::NoTextBox },
               _incDecSlider { Slider::IncDecButtons, Slider::TextBoxBelow };
};