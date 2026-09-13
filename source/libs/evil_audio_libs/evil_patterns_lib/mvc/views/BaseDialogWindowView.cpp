#include <juce_gui_basics.h>
#include "BaseDialogWindowView.h"

namespace evil::mvc
{
    BaseDialogWindowView::BaseDialogWindowView(juce::JUCEApplication& application,
        const juce::String& name,
        juce::Colour backgroundColour,
        int requiredButtons)
        : juce::DialogWindow(name, backgroundColour, requiredButtons)
    {
        juce::ignoreUnused(application);
    }

    BaseDialogWindowView::~BaseDialogWindowView()
    {
        if (onDestroy)
            (void)onDestroy(*this);
    }

    std::string BaseDialogWindowView::render() const
    {
        return {};
    }

    void BaseDialogWindowView::visibilityChanged()
    {
        juce::DialogWindow::visibilityChanged();

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

