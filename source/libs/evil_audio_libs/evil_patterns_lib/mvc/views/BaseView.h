/**
 * @file BaseView.h
 * @brief Declares the `BaseView` base class for all MVC views.
 *
 * A `BaseView` represents the visual layer in the Evil MVC framework. It
 * participates in two separate protocols:
 * - **Rendering** — subclasses implement `render()` to produce output.
 * - **Lifecycle callbacks** — four `std::function` members (`onInit`,
 *   `onShow`, `onHide`, `onDestroy`) are wired by a `BaseController` when
 *   the view is registered. Subclasses trigger these callbacks at the
 *   appropriate point in their own lifecycle.
 *
 * `BaseView` also inherits `BaseDataModelObserver`, allowing it to receive
 * data-model change notifications via `handleDataModelEvent()`.
 *
 * @see BaseController
 * @see BaseDataModelObserver
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>
#include <any>

namespace evil::mvc
{
    class BaseCommand;
    class ValidationResult;

    class BaseView 
    {
    public:
        /**
         * @brief Destroys the view through a base pointer.
         *
         * The destructor is virtual to ensure correct destruction of derived
         * class resources when held through a `BaseView*` or
         * `std::shared_ptr<BaseView>`.
         */
        virtual ~BaseView() = default;

        std::function<std::unique_ptr<BaseCommand>(std::type_index, const std::vector<std::any>&)> onCreateCommand;
        std::unique_ptr<BaseCommand> createCommand(std::type_index commandType, const std::vector<std::any>& args) const;
        std::unique_ptr<BaseCommand> createCommand(std::type_index commandType) const;

        bool canExecuteCommand(const BaseCommand& command) const;
        std::function<bool(const BaseCommand& command)> onCanExecuteCommand;

        ValidationResult executeCommand(const BaseCommand& command) const;
        std::function<ValidationResult(const BaseCommand& command)> onExecuteCommand;

        /**
         * @brief Produces a string representation of the view's current state.
         *
         * Derived classes must implement this method to generate whatever output
         * format is appropriate for the target rendering surface (e.g. HTML,
         * plain text, or a serialised property tree).
         *
         * @return A non-empty string describing the rendered view.
         */
        virtual std::string render() const = 0;

        /**
         * @brief Callback invoked when the view is about to be destroyed.
         *
         * Populated by the bound `BaseController` via `BaseController::addView()`.
         * The derived class should invoke this at the start of its own destruction
         * sequence, before releasing any resources, so the controller can perform
         * clean-up (e.g. removing the view from its internal list).
         *
         * Signature: `bool(BaseView& view)`
         * - @p view — the view instance raising the event.
         * - Returns `true` if the controller accepted the transition.
         *
         * @note May be empty (default-initialised). Always guard with a null check.
         */
        std::function<bool(BaseView& view)> onDestroy;

        /**
         * @brief Callback invoked when the view is about to be hidden.
         *
         * Populated by the bound `BaseController`. The derived class should call
         * this when it becomes invisible, allowing the controller to execute any
         * hide-specific logic (e.g. pausing updates).
         *
         * Signature: `bool(BaseView& view)`
         * - @p view — the view instance raising the event.
         * - Returns `true` if the controller accepted the transition.
         *
         * @note May be empty (default-initialised). Always guard with a null check.
         */
        std::function<bool(BaseView& view)> onHide;

        /**
         * @brief Callback invoked when the view has finished initialising.
         *
         * Populated by the bound `BaseController`. The derived class should call
         * this at the end of its initialisation sequence so the controller can
         * perform any post-init wiring (e.g. pushing initial model state to the
         * view).
         *
         * Signature: `bool(BaseView& view)`
         * - @p view — the view instance raising the event.
         * - Returns `true` if the controller accepted the transition.
         *
         * @note May be empty (default-initialised). Always guard with a null check.
         */
        std::function<bool(BaseView& view)> onInit;

        /**
         * @brief Callback invoked when the view is about to become visible.
         *
         * Populated by the bound `BaseController`. The derived class should call
         * this just before it becomes visible so the controller can resume updates
         * or refresh displayed data.
         *
         * Signature: `bool(BaseView& view)`
         * - @p view — the view instance raising the event.
         * - Returns `true` if the controller accepted the transition.
         *
         * @note May be empty (default-initialised). Always guard with a null check.
         */
        std::function<bool(BaseView& view)> onShow;
    };   
} // namespace evil::mvc
