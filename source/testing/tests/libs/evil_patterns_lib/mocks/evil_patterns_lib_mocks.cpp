#include "evil_patterns_lib_mocks.h"

namespace evil::mvc
{
    // ------------------------------------------------------------------
    // MockView
    // ------------------------------------------------------------------

    std::string MockView::render() const
    {
        return "MockView Rendered";
    }
    
    void MockView::call_onDestroy()
    {
        if (onDestroy)
            (void)onDestroy(*this);
    }
    
    void MockView::call_doHide()
    {
        if (onHide)
            (void)onHide(*this);
    }
    
    void MockView::call_onInit()
    {
        if (onInit)
            (void)onInit(*this);
    }

    void MockView::call_onShow()
    {
        if (onShow)
            (void)onShow(*this);
    }

    // ------------------------------------------------------------------
    // MockController
    // ------------------------------------------------------------------

    bool MockController::onViewInit(BaseView& view)
    {
        ++onViewInitCalls;
        return BaseController::onViewInit(view);
    }

    bool MockController::onViewShow(BaseView& view)
    {
        ++onViewShowCalls;
        return BaseController::onViewShow(view);
    }

    bool MockController::onViewHide(BaseView& view)
    {
        ++onViewHideCalls;
        return BaseController::onViewHide(view);
    }

    bool MockController::onViewDestroy(BaseView& view)
    {
        ++onViewDestroyCalls;
        return BaseController::onViewDestroy(view);
    }

}