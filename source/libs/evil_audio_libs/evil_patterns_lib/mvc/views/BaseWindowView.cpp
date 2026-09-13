#include "BaseWindowView.h"

namespace evil::mvc
{
    BaseWindowView::BaseWindowView(juce::JUCEApplication& application,
                                   const juce::String& name,
                                   juce::Colour backgroundColour,
                                   int requiredButtons)
        : juce::DocumentWindow(name, backgroundColour, requiredButtons)
    {
        juce::ignoreUnused(application);
    }

    BaseWindowView::~BaseWindowView()
    {
        if (onDestroy)
            (void)onDestroy(*this);
    }

    std::string BaseWindowView::render() const
    {
        return {};
    }

    void BaseWindowView::visibilityChanged()
    {
        juce::DocumentWindow::visibilityChanged();

        if (isVisible())
        {
            if (!initialized_)
            {
                initialized_ = true;
                if (onInit)
                {
                    (void)onInit(*this);
                }   
            }
            if (onShow) 
            {
                (void)onShow(*this);
            }
        }
        else
        {
            if (onHide)
                (void)onHide(*this);
        }
    }
}

