#include "mvc/viewmodels/BaseViewModel.h"

namespace evil::mvc
{
    BaseView* BaseViewModel::requestViewInternal() const
    {
        if (_requestViewCallback)
            return _requestViewCallback();
        return nullptr;
    }

    void BaseViewModel::setRequestViewCallback(RequestViewCallback callback)
    {
        _requestViewCallback = std::move(callback);
    }

} // namespace evil::mvc