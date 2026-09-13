#pragma once

#include <juce_core.h>
#include "BaseController.h"
#include <system/juce_PlatformDefs.h>

namespace evil::mvc
{
    class WindowController : public BaseController
    {
    public:
        WindowController(); 
        ~WindowController() override;

        using BaseController::setDataModel;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowController)
    };
}

