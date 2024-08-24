#include "PluginProcessor.h"
#include "PluginEditor.h"

LEDStripAudioProcessorEditor::LEDStripAudioProcessorEditor (LEDStripAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    // RGB Sliders
    addAndMakeVisible(redSlider);
    redSlider.setRange(0, 255, 1);
    redSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    redAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "red", redSlider));

    addAndMakeVisible(greenSlider);
    greenSlider.setRange(0, 255, 1);
    greenSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    greenAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "green", greenSlider));

    addAndMakeVisible(blueSlider);
    blueSlider.setRange(0, 255, 1);
    blueSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    blueAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "blue", blueSlider));

    // Brightness Slider
    addAndMakeVisible(brightnessSlider);
    brightnessSlider.setRange(0, 100, 1);
    brightnessSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    brightnessAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "brightness", brightnessSlider));

    // LED Count Slider
    addAndMakeVisible(ledCountSlider);
    ledCountSlider.setRange(1, 300, 1);
    ledCountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    ledCountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "ledCount", ledCountSlider));

    // Scroll Mode Toggle
    addAndMakeVisible(scrollModeToggle);
    scrollModeToggle.setButtonText("Scroll Mode");
    scrollModeAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "scrollMode", scrollModeToggle));

    // Scroll Speed Slider
    addAndMakeVisible(scrollSpeedSlider);
    scrollSpeedSlider.setRange(0.0, 10.0, 0.1);
    scrollSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    scrollSpeedAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "scrollSpeed", scrollSpeedSlider));

    // Scroll Direction Toggle
    addAndMakeVisible(scrollDirectionToggle);
    scrollDirectionToggle.setButtonText("Reverse Direction");
    scrollDirectionAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "scrollDirection", scrollDirectionToggle));

    // Animation List
    addAndMakeVisible(gradientTypeComboBox);
    gradientTypeComboBox.addItem("Red", 1);
    gradientTypeComboBox.addItem("Green", 2);
    gradientTypeComboBox.addItem("Blue", 3);
    gradientTypeComboBox.addItem("Rainbow", 4);
    gradientTypeComboBox.addItem("Red to Green", 5);
    gradientTypeComboBox.addItem("Red to Blue", 6);
    gradientTypeComboBox.addItem("Green to Blue", 7);
    gradientTypeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "gradientType", gradientTypeComboBox));

    // Strobe
    addAndMakeVisible(strobeSpeedSlider);
    strobeSpeedSlider.setRange(0.0f, 100.0f);
    strobeSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    strobeSpeedAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "strobeSpeed", strobeSpeedSlider));

    // Labels
    addAndMakeVisible(redLabel);
    redLabel.setText("Red", juce::dontSendNotification);
    redLabel.attachToComponent(&redSlider, true);

    addAndMakeVisible(greenLabel);
    greenLabel.setText("Green", juce::dontSendNotification);
    greenLabel.attachToComponent(&greenSlider, true);

    addAndMakeVisible(blueLabel);
    blueLabel.setText("Blue", juce::dontSendNotification);
    blueLabel.attachToComponent(&blueSlider, true);

    addAndMakeVisible(brightnessLabel);
    brightnessLabel.setText("Brightness", juce::dontSendNotification);
    brightnessLabel.attachToComponent(&brightnessSlider, true);

    addAndMakeVisible(ledCountLabel);
    ledCountLabel.setText("LED Count", juce::dontSendNotification);
    ledCountLabel.attachToComponent(&ledCountSlider, true);

    addAndMakeVisible(scrollSpeedLabel);
    scrollSpeedLabel.setText("Scroll Speed", juce::dontSendNotification);
    scrollSpeedLabel.attachToComponent(&scrollSpeedSlider, true);

    addAndMakeVisible(gradientTypeLabel);
    gradientTypeLabel.setText("Gradient Type", juce::dontSendNotification);
    gradientTypeLabel.attachToComponent(&gradientTypeComboBox, true);

    addAndMakeVisible(gradientTypeLabel);
    gradientTypeLabel.setText("strobeSpeed", juce::dontSendNotification);
    gradientTypeLabel.attachToComponent(&strobeSpeedSlider, true);


    setSize (400, 450);
}

LEDStripAudioProcessorEditor::~LEDStripAudioProcessorEditor()
{
}

void LEDStripAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("LED Strip Control (SPI)", getLocalBounds(), juce::Justification::centredTop, 1);
}

void LEDStripAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    const int sliderHeight = 20;
    const int margin = 10;

    area.removeFromTop(40);

    redSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    greenSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    blueSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    brightnessSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    ledCountSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    scrollModeToggle.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(margin);

    scrollSpeedSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    scrollDirectionToggle.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(margin);

    gradientTypeComboBox.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);

    strobeSpeedSlider.setBounds(area.removeFromTop(sliderHeight).withTrimmedLeft(100));
    area.removeFromTop(margin);
}