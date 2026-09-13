/**
 * @file evil_patterns_lib_mocks.h
 * @brief Test double implementations for the Evil MVC framework.
 *
 * Provides lightweight mock classes used exclusively by unit and integration
 * tests targeting the `evil_patterns_lib` MVC components. Each mock exposes
 * the minimal interface required to exercise the corresponding production
 * contract while adding test-specific introspection (call counters, captured
 * state, and helper methods).
 *
 * @note These types must never be used in production code.
 */

#pragma once

#include <algorithm>
#include <vector>
#include <mvc/views/BaseView.h>
#include <mvc/controllers/BaseController.h>
#include <mvc/datamodels/BaseDataModel.h>

namespace evil::mvc
{

// ============================================================================

/**
 * @brief Concrete test double for `BaseView`.
 *
 * Provides a minimal, renderable view that does not depend on any UI
 * framework. The four `call_*` helper methods allow tests to fire the
 * lifecycle callbacks (`onInit`, `onShow`, `onHide`, `onDestroy`) that are
 * normally triggered by a bound `BaseController`, enabling isolated
 * callback-wiring verification.
 *
 * ### Typical usage
 * @code
 * MockView view;
 * view.onInit = [](const BaseView&) { return true; };
 * view.call_onInit(); // fires the lambda above
 * @endcode
 */
class MockView : public BaseView
{
public:
    /**
     * @brief Returns a fixed string representing the rendered view.
     *
     * Always returns `"MockView Rendered"`. Satisfies the pure-virtual
     * `BaseView::render()` contract without performing any real rendering.
     *
     * @return A non-empty string identifying this mock view.
     */
    std::string render() const override;

    /**
     * @brief Invokes the `onDestroy` lifecycle callback, if set.
     *
     * Calls `onDestroy(*this)` when the callback is non-null, passing this
     * view as the argument. Safe to call when no callback is registered.
     */
    void call_onDestroy();

    /**
     * @brief Invokes the `onHide` lifecycle callback, if set.
     *
     * Calls `onHide(*this)` when the callback is non-null. Mirrors the
     * action a controller would take when hiding the view.
     */
    void call_doHide();

    /**
     * @brief Invokes the `onInit` lifecycle callback, if set.
     *
     * Calls `onInit(*this)` when the callback is non-null. Mirrors the
     * action a controller would take when initialising the view.
     */
    void call_onInit();

    /**
     * @brief Invokes the `onShow` lifecycle callback, if set.
     *
     * Calls `onShow(*this)` when the callback is non-null. Mirrors the
     * action a controller would take when showing the view.
     */
    void call_onShow();
};

// ============================================================================

/**
 * @brief Concrete test double for `BaseController`.
 *
 * Implements the pure-virtual `handleRequest()` method and overrides all four
 * `onView*` lifecycle hooks to record how many times each was invoked. Tests
 * can verify that callback wiring set up by `BaseController::addView()` routes
 * correctly through to these overrides.
 *
 * ### Typical usage
 * @code
 * MockController controller;
 * auto view = std::make_shared<MockView>();
 * controller.addView(view);
 * view->call_onInit();
 * EXPECT_EQ(controller.onViewInitCalls, 1);
 * @endcode
 */
class MockController : public BaseController
{
public:
    using BaseController::setDataModel;

    /** @brief Number of times `onViewInit()` has been called. */
    int onViewInitCalls    = 0;

    /** @brief Number of times `onViewShow()` has been called. */
    int onViewShowCalls    = 0;

    /** @brief Number of times `onViewHide()` has been called. */
    int onViewHideCalls    = 0;

    /** @brief Number of times `onViewDestroy()` has been called. */
    int onViewDestroyCalls = 0;

protected:
    /**
     * @brief Records the call and delegates to the base implementation.
     * @param view The view that was initialised.
     * @return `true` (base class default).
     */
    bool onViewInit(BaseView& view) override;

    /**
     * @brief Records the call and delegates to the base implementation.
     * @param view The view that was shown.
     * @return `true` (base class default).
     */ 
    bool onViewShow(BaseView& view) override;

    /**
     * @brief Records the call and delegates to the base implementation.
     * @param view The view that was hidden.
     * @return `true` (base class default).
     */
    bool onViewHide(BaseView& view) override;

    /**
     * @brief Records the call and delegates to the base implementation.
     * @param view The view that was destroyed.
     * @return `true` (base class default).
     */
    bool onViewDestroy(BaseView& view) override;
};

// ============================================================================

// ============================================================================

/**
 * @brief Concrete test double for `BaseDataModel`.
 *
 * Maintains a parallel, publicly queryable observer list that mirrors the
 * private `_observers` list inside `BaseDataModel`. This allows tests to
 * verify observer registration state (count and membership) without
 * accessing production internals.
 *
 * Both `addObserver()` and `removeObserver()` delegate to the base class
 * after updating the tracked list, so the production duplicate-rejection and
 * erase-remove logic remains under test.
 *
 * ### Typical usage
 * @code
 * MockDataModel model;
 * MockDataModelObserver obs;
 * model.addObserver(&obs);
 * EXPECT_EQ(model.observerCount(), 1u);
 * EXPECT_TRUE(model.hasObserver(&obs));
 * model.removeObserver(&obs);
 * EXPECT_EQ(model.observerCount(), 0u);
 * @endcode
 */
class MockDataModel : public BaseDataModel
{
public:
    /**
     * @brief Registers @p observer and updates the internal tracking list.
     *
     * Adds @p observer to the parallel tracked list if it is non-null and
     * not already present, then delegates to `BaseDataModel::addObserver()`.
     * Null and duplicate observers are silently ignored, matching the
     * production behaviour.
     *
     * @param observer Pointer to the observer to register. May be `nullptr`.
     */
    void addObserver(BaseDataModelObserver<BaseDataModel>* observer) override
    {
        if (observer != nullptr &&
            std::find(_trackedObservers.begin(), _trackedObservers.end(), observer) == _trackedObservers.end())
        {
            _trackedObservers.push_back(observer);
        }
        BaseDataModel::addObserver(observer);
    }

    /**
     * @brief Unregisters @p observer and updates the internal tracking list.
     *
     * Removes @p observer from the parallel tracked list (if present), then
     * delegates to `BaseDataModel::removeObserver()`. Safe to call with an
     * unregistered or null pointer.
     *
     * @param observer Pointer to the observer to unregister. May be `nullptr`.
     */
    void removeObserver(BaseDataModelObserver<BaseDataModel>* observer) override
    {
        _trackedObservers.erase(
            std::remove(_trackedObservers.begin(), _trackedObservers.end(), observer),
            _trackedObservers.end());
        BaseDataModel::removeObserver(observer);
    }

    /**
     * @brief Returns the number of currently registered observers.
     *
     * Reflects the state of the parallel tracking list, which is kept in
     * sync with the base class observer list.
     *
     * @return The number of observers currently registered with this model.
     */
    std::size_t observerCount() const { return _trackedObservers.size(); }

    /**
     * @brief Checks whether a specific observer is currently registered.
     *
     * @param observer Pointer to the observer to query. May be `nullptr`.
     * @return `true` if @p observer is present in the tracking list;
     *         `false` otherwise (including when @p observer is `nullptr`).
     */
    bool hasObserver(BaseDataModelObserver<BaseDataModel>* observer) const
    {
        return std::find(_trackedObservers.begin(), _trackedObservers.end(), observer) != _trackedObservers.end();
    }

private:
    /**
     * @brief Parallel observer list mirroring `BaseDataModel::_observers`.
     *
     * Maintained solely for test introspection; does not participate in
     * any event dispatch.
     */
    std::vector<BaseDataModelObserver<BaseDataModel>*> _trackedObservers;
};

} // namespace evil::mvc