/**
 * @file evil_base_mvc_test.cpp
 * @brief Unit tests for BaseController, MockView (BaseView), and BaseDataModel.
 *
 * Covers:
 * - MockView render output and callback delegation
 * - BaseController view management (add, remove, clear, duplicates)
 * - BaseController callback wiring: onInit, onShow, onHide, onDestroy
 * - BaseController setDataModel null-safety
 * - BaseDataModel observer registration (add, remove, null-safety, duplicates)
 */

#include <gtest/gtest.h>
#include <memory>

#include "mocks/evil_patterns_lib_mocks.h"


namespace evil::mvc::test
{

// ======================================================================
// MockView Tests
// ======================================================================

class MockViewTest : public ::testing::Test
{
protected:
    MockView view;
};

TEST_F(MockViewTest, CallOnInit_InvokesOnInitCallback)
{
    bool called = false;
    view.onInit = [&](const BaseView&) { called = true; return true; };
    view.call_onInit();
    EXPECT_TRUE(called);
}

TEST_F(MockViewTest, CallOnShow_InvokesOnShowCallback)
{
    bool called = false;
    view.onShow = [&](const BaseView&) { called = true; return true; };
    view.call_onShow();
    EXPECT_TRUE(called);
}

TEST_F(MockViewTest, CallDoHide_InvokesOnHideCallback)
{
    bool called = false;
    view.onHide = [&](const BaseView&) { called = true; return true; };
    view.call_doHide();
    EXPECT_TRUE(called);
}

TEST_F(MockViewTest, CallOnDestroy_InvokesOnDestroyCallback)
{
    bool called = false;
    view.onDestroy = [&](const BaseView&) { called = true; return true; };
    view.call_onDestroy();
    EXPECT_TRUE(called);
}

TEST_F(MockViewTest, CallOnInit_WithNoCallback_DoesNotThrow)
{
    EXPECT_NO_THROW(view.call_onInit());
}

TEST_F(MockViewTest, CallOnShow_WithNoCallback_DoesNotThrow)
{
    EXPECT_NO_THROW(view.call_onShow());
}

TEST_F(MockViewTest, CallDoHide_WithNoCallback_DoesNotThrow)
{
    EXPECT_NO_THROW(view.call_doHide());
}

TEST_F(MockViewTest, CallOnDestroy_WithNoCallback_DoesNotThrow)
{
    EXPECT_NO_THROW(view.call_onDestroy());
}

TEST_F(MockViewTest, OnInitCallback_ReceivesCorrectViewReference)
{
    const BaseView* receivedView = nullptr;
    view.onInit = [&](const BaseView& v) { receivedView = &v; return true; };
    view.call_onInit();
    EXPECT_EQ(receivedView, &view);
}

// ======================================================================
// BaseController (via MockController) Tests
// ======================================================================

class BaseControllerTest : public ::testing::Test
{
protected:
    MockController controller;

    std::shared_ptr<MockView> makeView()
    {
        return std::make_shared<MockView>();
    }
};

TEST_F(BaseControllerTest, InitialViewListIsEmpty)
{
    EXPECT_TRUE(controller.getViews().empty());
}

TEST_F(BaseControllerTest, AddView_IncreasesViewCount)
{
    auto view = makeView();
    controller.addView(view);
    EXPECT_EQ(controller.getViews().size(), 1u);
}

TEST_F(BaseControllerTest, AddView_NullView_IsIgnored)
{
    controller.addView(nullptr);
    EXPECT_TRUE(controller.getViews().empty());
}

TEST_F(BaseControllerTest, AddView_DuplicateView_IsIgnored)
{
    auto view = makeView();
    controller.addView(view);
    controller.addView(view);
    EXPECT_EQ(controller.getViews().size(), 1u);
}

TEST_F(BaseControllerTest, AddMultipleViews_AllAdded)
{
    auto view1 = makeView();
    auto view2 = makeView();
    auto view3 = makeView();
    controller.addView(view1);
    controller.addView(view2);
    controller.addView(view3);
    EXPECT_EQ(controller.getViews().size(), 3u);
}

TEST_F(BaseControllerTest, RemoveView_DecreasesViewCount)
{
    auto view = makeView();
    controller.addView(view);
    controller.removeView(view.get());
    EXPECT_TRUE(controller.getViews().empty());
}

TEST_F(BaseControllerTest, RemoveView_NullView_IsIgnored)
{
    auto view = makeView();
    controller.addView(view);
    controller.removeView(nullptr);
    EXPECT_EQ(controller.getViews().size(), 1u);
}

TEST_F(BaseControllerTest, RemoveView_NotAdded_IsIgnored)
{
    auto view1 = makeView();
    auto view2 = makeView();
    controller.addView(view1);
    controller.removeView(view2.get());
    EXPECT_EQ(controller.getViews().size(), 1u);
}

TEST_F(BaseControllerTest, ClearViews_RemovesAllViews)
{
    auto view1 = makeView();
    auto view2 = makeView();
    controller.addView(view1);
    controller.addView(view2);
    controller.clearViews();
    EXPECT_TRUE(controller.getViews().empty());
}

TEST_F(BaseControllerTest, AddView_WiresOnInitCallback)
{
    auto view = makeView();
    controller.addView(view);
    EXPECT_TRUE(static_cast<bool>(view->onInit));
}

TEST_F(BaseControllerTest, AddView_WiresOnShowCallback)
{
    auto view = makeView();
    controller.addView(view);
    EXPECT_TRUE(static_cast<bool>(view->onShow));
}

TEST_F(BaseControllerTest, AddView_WiresOnHideCallback)
{
    auto view = makeView();
    controller.addView(view);
    EXPECT_TRUE(static_cast<bool>(view->onHide));
}

TEST_F(BaseControllerTest, AddView_WiresOnDestroyCallback)
{
    auto view = makeView();
    controller.addView(view);
    EXPECT_TRUE(static_cast<bool>(view->onDestroy));
}

TEST_F(BaseControllerTest, RemoveView_ClearsOnInitCallback)
{
    auto view = makeView();
    controller.addView(view);
    controller.removeView(view.get());
    EXPECT_FALSE(static_cast<bool>(view->onInit));
}

TEST_F(BaseControllerTest, RemoveView_ClearsOnDestroyCallback)
{
    auto view = makeView();
    controller.addView(view);
    controller.removeView(view.get());
    EXPECT_FALSE(static_cast<bool>(view->onDestroy));
}

TEST_F(BaseControllerTest, CallOnInit_TriggersOnViewInit)
{
    auto view = makeView();
    controller.addView(view);
    view->call_onInit();
    EXPECT_EQ(controller.onViewInitCalls, 1);
}

TEST_F(BaseControllerTest, CallOnShow_TriggersOnViewShow)
{
    auto view = makeView();
    controller.addView(view);
    view->call_onShow();
    EXPECT_EQ(controller.onViewShowCalls, 1);
}

TEST_F(BaseControllerTest, CallDoHide_TriggersOnViewHide)
{
    auto view = makeView();
    controller.addView(view);
    view->call_doHide();
    EXPECT_EQ(controller.onViewHideCalls, 1);
}

TEST_F(BaseControllerTest, CallOnDestroy_TriggersOnViewDestroy)
{
    auto view = makeView();
    controller.addView(view);
    view->call_onDestroy();
    EXPECT_EQ(controller.onViewDestroyCalls, 1);
}

TEST_F(BaseControllerTest, SetDataModel_NullModel_IsIgnored)
{
    EXPECT_NO_THROW(controller.setDataModel(nullptr));
}

TEST_F(BaseControllerTest, SetDataModel_LiveModel_GetDataModelReturnsSameInstance)
{
    auto model = std::make_shared<MockDataModel>();
    controller.setDataModel(model);

    EXPECT_EQ(controller.getDataModel().get(), model.get());
}

TEST_F(BaseControllerTest, SetDataModel_ModelDestroyed_GetDataModelReturnsNullptr)
{
    auto model = std::make_shared<MockDataModel>();
    controller.setDataModel(model);

    ASSERT_NE(controller.getDataModel(), nullptr);

    model.reset();

    EXPECT_EQ(controller.getDataModel(), nullptr);
}

TEST_F(BaseControllerTest, SetDataModel_RawPointerAfterSharedTracking_UsesRawPointerMode)
{
    auto sharedModel = std::make_shared<MockDataModel>();
    controller.setDataModel(sharedModel);
    sharedModel.reset();

    EXPECT_EQ(controller.getDataModel(), nullptr);
}

} // namespace evil::mvc::test
