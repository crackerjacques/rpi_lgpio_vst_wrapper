#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LgpioGpioAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LgpioGpioAudioProcessorEditor (LgpioGpioAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~LgpioGpioAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LgpioGpioAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    std::array<std::unique_ptr<juce::ToggleButton>, 28> gpioButtons;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>, 28> gpioAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LgpioGpioAudioProcessorEditor)
};