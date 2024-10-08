#pragma once

#include <JuceHeader.h>
#include <lgpio.h>

class PWMControlAudioProcessor  : public juce::AudioProcessor
{
public:
    PWMControlAudioProcessor();
    ~PWMControlAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool initializePWM();
    void updatePWMState();

    juce::AudioProcessorValueTreeState parameters;

private:
    std::atomic<float>* pwmChannelParameter = nullptr;
    std::atomic<float>* pwmDutyParameter = nullptr;
    std::atomic<float>* pwmFrequencyParameter = nullptr;
    int handle = -1;
    int currentPWMPin = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PWMControlAudioProcessor)
};
