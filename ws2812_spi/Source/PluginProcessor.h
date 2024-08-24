#pragma once

#include <JuceHeader.h>
#include <ws2811/ws2811.h>
#include <lgpio.h>


class LEDStripAudioProcessor  : public juce::AudioProcessor,
                                private juce::Timer
{
public:
    LEDStripAudioProcessor();
    ~LEDStripAudioProcessor() override;

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
    bool initializeSPI();
    void updateLEDs();

    juce::AudioProcessorValueTreeState parameters;

    static inline uint32_t rgb_to_ws2811(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

private:
    void timerCallback() override;

    std::atomic<float>* redParameter = nullptr;
    std::atomic<float>* greenParameter = nullptr;
    std::atomic<float>* blueParameter = nullptr;
    std::atomic<float>* brightnessParameter = nullptr;
    std::atomic<float>* ledCountParameter = nullptr;
    std::atomic<float>* scrollModeParameter = nullptr;
    std::atomic<float>* scrollSpeedParameter = nullptr;
    std::atomic<float>* scrollDirectionParameter = nullptr;
    std::atomic<float>* gradientTypeParameter = nullptr;
    std::atomic<float>* strobeSpeedParameter = nullptr;

    ws2811_t ledstring;
    int handle;
    int spi_fd;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDStripAudioProcessor)
};