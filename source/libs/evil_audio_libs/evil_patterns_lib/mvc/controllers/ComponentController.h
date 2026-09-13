#pragma once

#include <juce_core.h>
#include "BaseController.h"
#include <system/juce_PlatformDefs.h>

namespace evil::mvc
{ 
    class BaseDataModel;

    class ComponentController final : public BaseController
    {
    public:
        ComponentController();
        ~ComponentController() override;

        using BaseController::setDataModel;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentController);
    };
}