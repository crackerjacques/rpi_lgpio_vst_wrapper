#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LEDStripAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LEDStripAudioProcessorEditor (LEDStripAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~LEDStripAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;


private:
    LEDStripAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::Slider redSlider, greenSlider, blueSlider;
    juce::Slider brightnessSlider;
    juce::Slider ledCountSlider;
    juce::ToggleButton scrollModeToggle;
    juce::Slider scrollSpeedSlider;
    juce::ToggleButton scrollDirectionToggle;
    juce::ComboBox gradientTypeComboBox;
    juce::Slider strobeSpeedSlider;

    juce::Label redLabel, greenLabel, blueLabel;
    juce::Label brightnessLabel, ledCountLabel;
    juce::Label scrollSpeedLabel, gradientTypeLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> redAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> greenAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> blueAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ledCountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scrollModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scrollSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scrollDirectionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> gradientTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> strobeSpeedAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDStripAudioProcessorEditor)
};