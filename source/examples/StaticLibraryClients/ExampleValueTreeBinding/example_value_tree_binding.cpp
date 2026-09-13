#include <cassert>
#include <iostream>

#include "bindings/edaw_property_bindings.h"

int main()
{
    juce::ValueTree state{"State"};
    state.setProperty("gain", 0.5, nullptr);
    state.setProperty("mix", 0.25, nullptr);

    juce::Value gainPercent;
    juce::Value mixPercent;

    auto oneWayBinding = evil::makeValueTreeToValueBinding(
        state,
        juce::Identifier{"gain"},
        gainPercent,
        [](juce::var value)
        {
            return juce::var{static_cast<double>(value) * 100.0};
        });

    oneWayBinding.sync();
    const auto gainAsPercent = static_cast<double>(gainPercent.getValue());
    std::cout << "One-way sync gain percent: " << gainAsPercent << '\n';
    assert(gainAsPercent == 50.0);

    auto twoWayBinding = evil::makeValueTreePropertyBinding(
        state,
        juce::Identifier{"mix"},
        mixPercent,
        [](juce::var treeValue)
        {
            return juce::var{static_cast<double>(treeValue) * 100.0};
        },
        [](juce::var uiValue)
        {
            return juce::var{static_cast<double>(uiValue) / 100.0};
        });

    twoWayBinding.syncTreeToValue();
    const auto mixAfterTreeSync = static_cast<double>(mixPercent.getValue());
    std::cout << "Two-way tree->value mix percent: " << mixAfterTreeSync << '\n';
    assert(mixAfterTreeSync == 25.0);

    mixPercent = 75.0;
    twoWayBinding.syncValueToTree();
    const auto mixInTree = static_cast<double>(state.getProperty("mix"));
    std::cout << "Two-way value->tree mix norm: " << mixInTree << '\n';
    assert(mixInTree == 0.75);

    std::cout << "ValueTree binding example passed." << '\n';
    return 0;
}
