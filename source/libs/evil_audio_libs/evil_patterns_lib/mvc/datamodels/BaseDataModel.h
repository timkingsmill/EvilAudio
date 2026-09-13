/**
 * @file BaseDataModel.h
 * @brief Declares the `BaseDataModel` base class for all MVC data models.
 *
 * A `BaseDataModel` holds application state and maintains a list of
 * `BaseDataModelObserver` subscribers that are notified whenever state changes.
 */

#pragma once

#include <mutex>
#include <vector>
#include <concepts>

namespace evil::mvc
{
    class BaseDataModel;

    template <typename TDataModel>
        requires std::derived_from<TDataModel, BaseDataModel>
    class BaseDataModelObserver;

    class BaseDataModel
    {
    public:
        virtual ~BaseDataModel() = default;

        virtual void addObserver(BaseDataModelObserver<BaseDataModel>* observer);
        virtual void removeObserver(BaseDataModelObserver<BaseDataModel>* observer);

    protected:
        /**
         * @brief Notifies all registered observers of a state change.
         *
         * Passes `typeid(*this)` as the concrete type index so that observers
         * receive the most-derived runtime type without requiring a virtual
         * `asBase()` override in every subclass.
         *
         * Observers are snapshotted under the lock before dispatch to avoid
         * holding `_observersMutex` during callbacks.
         */
        void notifyObservers();

    private:
        std::vector<BaseDataModelObserver<BaseDataModel>*> _observers;
        mutable std::mutex _observersMutex;
    };

} // namespace evil::mvc
