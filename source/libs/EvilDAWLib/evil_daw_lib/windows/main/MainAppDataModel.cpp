#include "MainAppDataModel.h"

namespace evil::daw
{
    MainAppDataModel::~MainAppDataModel() = default;
    int MainAppDataModel::getClickCounter() const
    {
        return clickCounter_;
    }
    void MainAppDataModel::setClickCounter(int newValue)
    {
        if (clickCounter_ != newValue)
        {
            clickCounter_ = newValue;
            notifyObservers();
        }
    }
}
