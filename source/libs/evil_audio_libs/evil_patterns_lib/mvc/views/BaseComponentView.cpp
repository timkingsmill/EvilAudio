

#include "mvc/views/BaseComponentView.h"

namespace evil::mvc
{
    BaseComponentView::BaseComponentView(juce::JUCEApplication& application,
                                         const juce::String& name,
                                         juce::Colour backgroundColour)
    {
        juce::ignoreUnused(application, backgroundColour);
        setName(name);
    }

    BaseComponentView::~BaseComponentView()
    {
        if (onDestroy)
            (void)onDestroy(*this);
    }

    std::string BaseComponentView::render() const
    {
        return {};
    }

    void BaseComponentView::visibilityChanged()
    {
        juce::Component::visibilityChanged();

        if (isVisible())
        {
            if (!initialized_)
            {
                initialized_ = true;
                if (onInit)
                    (void)onInit(*this);
            }

            if (onShow)
                (void)onShow(*this);
        }
        else
        {
            if (onHide)
                (void)onHide(*this);
        }
    }
}