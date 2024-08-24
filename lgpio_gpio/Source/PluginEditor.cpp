#include "PluginProcessor.h"
#include "PluginEditor.h"

LgpioGpioAudioProcessorEditor::LgpioGpioAudioProcessorEditor (LgpioGpioAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts), processorRef(p)
{
    for (int i = 0; i < LgpioGpioAudioProcessor::gpioPins.size(); ++i) {
        gpioButtons[i] = std::make_unique<juce::ToggleButton>("GPIO " + juce::String(LgpioGpioAudioProcessor::gpioPins[i]));
        addAndMakeVisible(gpioButtons[i].get());
        gpioAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            valueTreeState, "gpio" + juce::String(LgpioGpioAudioProcessor::gpioPins[i]), *gpioButtons[i]);

        gpioModeButtons[i] = std::make_unique<juce::ToggleButton>("OUT/IN");
        addAndMakeVisible(gpioModeButtons[i].get());
        gpioModeAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            valueTreeState, "gpio" + juce::String(LgpioGpioAudioProcessor::gpioPins[i]) + "_mode", *gpioModeButtons[i]);
        
        gpioModeButtons[i]->setToggleState(true, juce::dontSendNotification);
        gpioModeButtons[i]->setButtonText(gpioModeButtons[i]->getToggleState() ? "OUT" : "IN");
        gpioModeButtons[i]->onClick = [this, i]() {
            gpioModeButtons[i]->setButtonText(gpioModeButtons[i]->getToggleState() ? "OUT" : "IN");
        };

        if (processorRef.isGPIOUsedBySPI[i])
        {
            gpioButtons[i]->setEnabled(false);
            gpioButtons[i]->setTooltip("This pin is used by SPI");
            gpioModeButtons[i]->setEnabled(false);
            gpioModeButtons[i]->setTooltip("This pin is used by SPI");
        }
    }

    setSize (350, 620);
}

LgpioGpioAudioProcessorEditor::~LgpioGpioAudioProcessorEditor()
{
}

void LgpioGpioAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("GPIO Control", getLocalBounds(), juce::Justification::centredTop, 1);
}

void LgpioGpioAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(40);

    int buttonWidth = 100;
    int modeButtonWidth = 60;
    int buttonHeight = 30;
    int margin = 10;

    for (int row = 0; row < 14; ++row) {
        auto rowArea = area.removeFromTop(buttonHeight);
        
        // left
        if (row < LgpioGpioAudioProcessor::gpioPins.size()) {
            gpioButtons[row]->setBounds(rowArea.removeFromLeft(buttonWidth));
            gpioModeButtons[row]->setBounds(rowArea.removeFromLeft(modeButtonWidth));
        }
        
        rowArea.removeFromLeft(margin);
        
        // right
        if (row + 14 < LgpioGpioAudioProcessor::gpioPins.size()) {
            gpioButtons[row + 14]->setBounds(rowArea.removeFromLeft(buttonWidth));
            gpioModeButtons[row + 14]->setBounds(rowArea.removeFromLeft(modeButtonWidth));
        }
        
        area.removeFromTop(margin);
    }
}
