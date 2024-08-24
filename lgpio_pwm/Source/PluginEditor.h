#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PWMControlAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PWMControlAudioProcessorEditor (PWMControlAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~PWMControlAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PWMControlAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::ComboBox pwmChannelComboBox;
    juce::Slider pwmDutySlider;
    juce::Slider pwmFrequencySlider;

    juce::Label pwmChannelLabel;
    juce::Label pwmDutyLabel;
    juce::Label pwmFrequencyLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pwmChannelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwmDutyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwmFrequencyAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PWMControlAudioProcessorEditor)
};