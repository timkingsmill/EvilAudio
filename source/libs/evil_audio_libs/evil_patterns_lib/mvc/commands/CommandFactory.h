#pragma once

#include <any>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BaseCommand.h"

namespace evil::mvc
{
    class CommandFactory
    {
    public:
        using ArgList   = std::vector<std::any>;
        using FactoryFn = std::function<std::unique_ptr<BaseCommand>(const ArgList&)>;

        // ------------------------------------------------------------------
        // Direct construction — bypasses the registry.
        // ------------------------------------------------------------------

        template <typename T, typename... Args>
            requires std::derived_from<T, BaseCommand>
        static std::unique_ptr<T> create(Args&&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        // ------------------------------------------------------------------
        // Registry.
        // ------------------------------------------------------------------

        template <typename T>
            requires std::derived_from<T, BaseCommand>
        static void registerType(FactoryFn factory)
        {
            std::lock_guard lock(registryMutex());
            registry()[std::type_index(typeid(T))] = std::move(factory);
        }

        template <typename T>
            requires std::derived_from<T, BaseCommand>
        static void unregisterType()
        {
            std::lock_guard lock(registryMutex());
            registry().erase(std::type_index(typeid(T)));
        }

        static std::unique_ptr<BaseCommand> create(std::type_index commandType, const ArgList& args = {})
        {
            std::lock_guard lock(registryMutex());
            auto& reg = registry();
            auto  it  = reg.find(commandType);
            if (it == reg.end())
                return nullptr;
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