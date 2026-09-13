/**
 * @file BaseDataModelObserver.h
 * @brief Declares the `BaseDataModelObserver` interface for model-change notifications.
 */

#pragma once
#include <concepts>
#include <typeindex>

namespace evil::mvc
{
    class BaseDataModel;

    template <typename TDataModel>
        requires std::derived_from<TDataModel, BaseDataModel>
    class BaseDataModelObserver
    {
        public:
            virtual ~BaseDataModelObserver() = default;

        /**
         * @brief Called by the observed data model when its state has changed.
         *
         * In the current `BaseDataModel` implementation this is invoked
         * synchronously on the notifying thread.
         */
        virtual void handleDataModelEvent(const TDataModel& dataModel, std::type_index dataModelType) = 0;
    };

} // namespace evil::mvc