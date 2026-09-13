#pragma once

#include <juce_core.h>
#include <mvc/datamodels/BaseDataModel.h>

namespace evil::daw
{
    class MainAppDataModel : public mvc::BaseDataModel
    {
    public:
        MainAppDataModel() = default;
        ~MainAppDataModel() override;

        int getClickCounter() const;
        void setClickCounter(int newValue);


        int getDummyValue() const
        {
            return _dummyValue;
        }

        void setDummyValue(int value)
        {
            if (_dummyValue != value)
            {
                _dummyValue = value;
                notifyObservers();
            }
        }

    private:
        int _dummyValue = 0;
        int clickCounter_ = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainAppDataModel);
    };
}