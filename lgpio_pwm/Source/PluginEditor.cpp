#include "PluginProcessor.h"
#include "PluginEditor.h"

PWMControlAudioProcessorEditor::PWMControlAudioProcessorEditor (PWMControlAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    // PWM Channel ComboBox
    addAndMakeVisible(pwmChannelComboBox);
    pwmChannelComboBox.addItem("Channel 0 (GPIO 18)", 1);
    pwmChannelComboBox.addItem("Channel 1 (GPIO 19)", 2);
    pwmChannelAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        valueTreeState, "pwmChannel", pwmChannelComboBox));

    addAndMakeVisible(pwmChannelLabel);
    pwmChannelLabel.setText("PWM Channel", juce::dontSendNotification);
    pwmChannelLabel.attachToComponent(&pwmChannelComboBox, true);

    // PWM Duty Slider
    addAndMakeVisible(pwmDutySlider);
    pwmDutySlider.setRange(0.0, 100.0, 0.1);
    pwmDutySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    pwmDutyAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "pwmDuty", pwmDutySlider));

    addAndMakeVisible(pwmDutyLabel);
    pwmDutyLabel.setText("PWM Duty (%)", juce::dontSendNotification);
    pwmDutyLabel.attachToComponent(&pwmDutySlider, true);

    // PWM Frequency Slider
    addAndMakeVisible(pwmFrequencySlider);
    pwmFrequencySlider.setRange(1, 10000, 1);
    pwmFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    pwmFrequencyAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "pwmFrequency", pwmFrequencySlider));

    addAndMakeVisible(pwmFrequencyLabel);
    pwmFrequencyLabel.setText("PWM Frequency (Hz)", juce::dontSendNotification);
    pwmFrequencyLabel.attachToComponent(&pwmFrequencySlider, true);

    setSize (400, 300);
}

PWMControlAudioProcessorEditor::~PWMControlAudioProcessorEditor()
{
}

void PWMControlAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("PWM Control", getLocalBounds(), juce::Justification::centredTop, 1);
}

void PWMControlAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(40); // Space for title

    int componentHeight = 40;
    int margin = 10;

    pwmChannelComboBox.setBounds(area.removeFromTop(componentHeight));

    area.removeFromTop(margin);
    pwmDutySlider.setBounds(area.removeFromTop(componentHeight));

    area.removeFromTop(margin);
    pwmFrequencySlider.setBounds(area.removeFromTop(componentHeight));
}
