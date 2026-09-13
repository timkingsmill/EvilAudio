#pragma once

#include <juce_core.h>
#include <mvc/viewmodels/BaseViewModel.h>
#include <mvc/datamodels/BaseDataModel.h>
#include <typeindex>
#include "MainAppWindowView.h"
#include <mvc/views/BaseView.h>

namespace evil::daw
{
    class MainAppViewModel final : public mvc::BaseViewModel
    {
    public:
        MainAppViewModel(MainAppWindow* view)
            : BaseViewModel()

        {
            setRequestViewCallback([view]() -> mvc::BaseView*
            {
                return view;
            });
        }
    protected:
        void bindViewToDataModelImpl(const mvc::BaseDataModel& dataModel,
                                     std::type_index dataModelType) override;
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainAppViewModel);
    };
}