#pragma once

#include <any>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "mvc/viewmodels/BaseViewModel.h"

namespace evil::mvc
{
    class ViewModelFactory
    {
    public:
        using ArgList   = std::vector<std::any>;
        using FactoryFn = std::function<std::unique_ptr<BaseViewModel>(const ArgList&)>;

        // ------------------------------------------------------------------
        // Direct construction — bypasses the registry.
        // ------------------------------------------------------------------

        template <typename T, typename... Args>
            requires std::derived_from<T, BaseViewModel>
        static std::unique_ptr<T> create(Args&&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        // ------------------------------------------------------------------
        // Registry — keyed by view-model type.
        // ------------------------------------------------------------------

        template <typename T>
            requires std::derived_from<T, BaseViewModel>
        static void registerType(FactoryFn factory)
        {
            std::lock_guard lock(registryMutex());
            registry()[std::type_index(typeid(T))] = std::move(factory);
        }

        template <typename T>
            requires std::derived_from<T, BaseViewModel>
        static void unregisterType()
        {
            std::lock_guard lock(registryMutex());
            registry().erase(std::type_index(typeid(T)));
        }

        // ------------------------------------------------------------------
        // Registry — keyed by view type.
        // ------------------------------------------------------------------

        template <typename TView>
        static void registerViewType(FactoryFn factory)
        {
            std::lock_guard lock(registryMutex());
            registry()[std::type_index(typeid(TView))] = std::move(factory);
        }

        template <typename TView>
        static void unregisterViewType()
        {
            std::lock_guard lock(registryMutex());
            registry().erase(std::type_index(typeid(TView)));
        }

        // ------------------------------------------------------------------
        // Registry lookup by runtime type_index.
        // ------------------------------------------------------------------

        static std::unique_ptr<BaseViewModel> create(std::type_index viewType, const ArgList& args = {})
        {
            std::lock_guard lock(registryMutex());
            auto& reg = registry();
            auto  it  = reg.find(viewType);
            if (it == reg.end())
                return nullptr;
            // Call the factory function with the provided arguments. 
            // The factory function is expected to know how to handle the arguments and 
            // create the appropriate view model.
            return it->second(args);
        }

        // ------------------------------------------------------------------
        // Removes every registration. Intended for test teardown.
        // ------------------------------------------------------------------

        static void clearAll()
        {
            std::lock_guard lock(registryMutex());
            registry().clear();
        }

        // ------------------------------------------------------------------

    private:
        static std::unordered_map<std::type_index, FactoryFn>& registry()
        {
            static std::unordered_map<std::type_index, FactoryFn> instance;
            return instance;
        }

        static std::mutex& registryMutex()
        {
            static std::mutex m;
            return m;
        }
    };
}
