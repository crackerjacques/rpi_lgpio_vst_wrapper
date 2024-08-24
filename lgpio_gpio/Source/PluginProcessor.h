#pragma once

#include <JuceHeader.h>
#include <lgpio.h>

class LgpioGpioAudioProcessor  : public juce::AudioProcessor
{
public:
    LgpioGpioAudioProcessor();
    ~LgpioGpioAudioProcessor() override;

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

    bool initializeGPIO();
    void updateGPIOState(int pin, bool state);

    juce::AudioProcessorValueTreeState parameters;

    static constexpr std::array<int, 28> gpioPins = {
        2, 3, 4, 17, 27, 22, 10, 9, 11, 5, 6, 13, 19, 26,
        14, 15, 18, 23, 24, 25, 8, 7, 12, 16, 20, 21, 0, 1
    };

private:
    std::array<std::atomic<float>*, 28> gpioStates;
    int handle = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LgpioGpioAudioProcessor)
};