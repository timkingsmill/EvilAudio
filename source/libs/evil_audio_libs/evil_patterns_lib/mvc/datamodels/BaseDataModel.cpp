#include <algorithm>
#include <vector>
#include <mutex>
#include <typeindex>

#include "BaseDataModel.h"
#include "BaseDataModelObserver.h"
#include <memory>

namespace evil::mvc
{
    void BaseDataModel::addObserver(BaseDataModelObserver<BaseDataModel>* observer)
    {
        std::lock_guard<std::mutex> lock(_observersMutex);
        if (observer && std::find(_observers.begin(), _observers.end(), observer) == _observers.end())
        {
            _observers.push_back(observer);
        }
    }

    void BaseDataModel::removeObserver(BaseDataModelObserver<BaseDataModel>* observer)
    {
        std::lock_guard<std::mutex> lock(_observersMutex);
        _observers.erase(std::remove(_observers.begin(), _observers.end(), observer), _observers.end());
    }

    void BaseDataModel::notifyObservers()
    {
        std::vector<BaseDataModelObserver<BaseDataModel>*> observersCopy;
        {
            std::lock_guard<std::mutex> lock(_observersMutex);
            observersCopy = _observers;
        }

        // Use typeid(*this) directly to obtain the concrete runtime type.
        // This avoids the fragile asBase() virtual override pattern where
        // derived classes had to remember to override it for correct dispatch.
        const std::type_index concreteType{ typeid(*this) };

        for (auto* observer : observersCopy)
        {
            if (observer)
            {
                observer->handleDataModelEvent(*this, concreteType);
            }
        }
    }
}
