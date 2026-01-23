#include "evil_audio_LookAndFeel.h"



EvilAudio_LookAndFeel::EvilAudio_LookAndFeel()
{

    initialiseColours();
}

void EvilAudio_LookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}


void EvilAudio_LookAndFeel::initialiseColours()
{
}
