#include "MainAppViewModel.h"

#include "MainAppDataModel.h"
#include "MainAppWindowView.h"
#include <mvc/datamodels/BaseDataModel.h>
#include <typeindex>
#include <mvc/viewmodels/BaseViewModel.h>

namespace evil::daw
{
    void MainAppViewModel::bindViewToDataModelImpl(const mvc::BaseDataModel& dataModel,
                                                   std::type_index dataModelType)
    {
        if (dataModelType != std::type_index(typeid(MainAppDataModel)))
            return;

        const auto* mainAppDataModel = static_cast<const MainAppDataModel*>(&dataModel);

        if (auto* view = requestView<MainAppWindow>())
        {
            auto n = view->getName();
        }

        (void)mainAppDataModel->getDummyValue();
    }
}