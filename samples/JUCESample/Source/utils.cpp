

#include "utils.h"

LambdaAsyncUpdater::LambdaAsyncUpdater(fu2::unique_function<void()> callback)
    : callback_(std::move(callback)) {}

void LambdaAsyncUpdater::handleAsyncUpdate() {
    callback_();
}

LambdaParameterListener::LambdaParameterListener(
    fu2::unique_function<void(const juce::String&, float)> callback)
    : callback_(std::move(callback)) {}

void LambdaParameterListener::parameterChanged(const juce::String& parameterID,
                                               float newValue) {
    callback_(parameterID, newValue);
}
