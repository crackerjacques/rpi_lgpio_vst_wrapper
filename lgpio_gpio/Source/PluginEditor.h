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
    const LgpioGpioAudioProcessor& processorRef;

    std::array<std::unique_ptr<juce::ToggleButton>, 28> gpioButtons;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>, 28> gpioAttachments;
    std::array<std::unique_ptr<juce::ToggleButton>, 28> gpioModeButtons;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>, 28> gpioModeAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LgpioGpioAudioProcessorEditor)
};
