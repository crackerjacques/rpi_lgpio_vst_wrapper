#include "PluginProcessor.h"
#include "PluginEditor.h"

LgpioGpioAudioProcessor::LgpioGpioAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    parameters (*this, nullptr, juce::Identifier ("LgpioGpio"),
    {
        std::make_unique<juce::AudioParameterBool>("gpio2", "GPIO 2", false),
        std::make_unique<juce::AudioParameterBool>("gpio3", "GPIO 3", false),
        std::make_unique<juce::AudioParameterBool>("gpio4", "GPIO 4", false),
        std::make_unique<juce::AudioParameterBool>("gpio17", "GPIO 17", false),
        std::make_unique<juce::AudioParameterBool>("gpio27", "GPIO 27", false),
        std::make_unique<juce::AudioParameterBool>("gpio22", "GPIO 22", false),
        std::make_unique<juce::AudioParameterBool>("gpio10", "GPIO 10", false),
        std::make_unique<juce::AudioParameterBool>("gpio9", "GPIO 9", false),
        std::make_unique<juce::AudioParameterBool>("gpio11", "GPIO 11", false),
        std::make_unique<juce::AudioParameterBool>("gpio5", "GPIO 5", false),
        std::make_unique<juce::AudioParameterBool>("gpio6", "GPIO 6", false),
        std::make_unique<juce::AudioParameterBool>("gpio13", "GPIO 13", false),
        std::make_unique<juce::AudioParameterBool>("gpio19", "GPIO 19", false),
        std::make_unique<juce::AudioParameterBool>("gpio26", "GPIO 26", false),
        std::make_unique<juce::AudioParameterBool>("gpio14", "GPIO 14", false),
        std::make_unique<juce::AudioParameterBool>("gpio15", "GPIO 15", false),
        std::make_unique<juce::AudioParameterBool>("gpio18", "GPIO 18", false),
        std::make_unique<juce::AudioParameterBool>("gpio23", "GPIO 23", false),
        std::make_unique<juce::AudioParameterBool>("gpio24", "GPIO 24", false),
        std::make_unique<juce::AudioParameterBool>("gpio25", "GPIO 25", false),
        std::make_unique<juce::AudioParameterBool>("gpio8", "GPIO 8", false),
        std::make_unique<juce::AudioParameterBool>("gpio7", "GPIO 7", false),
        std::make_unique<juce::AudioParameterBool>("gpio12", "GPIO 12", false),
        std::make_unique<juce::AudioParameterBool>("gpio16", "GPIO 16", false),
        std::make_unique<juce::AudioParameterBool>("gpio20", "GPIO 20", false),
        std::make_unique<juce::AudioParameterBool>("gpio21", "GPIO 21", false),
        std::make_unique<juce::AudioParameterBool>("gpio0", "GPIO 0", false),
        std::make_unique<juce::AudioParameterBool>("gpio1", "GPIO 1", false)
    })
{
    for (int i = 0; i < gpioPins.size(); ++i) {
        gpioStates[i] = parameters.getRawParameterValue("gpio" + juce::String(gpioPins[i]));
    }

    handle = lgGpiochipOpen(0);
    if (handle < 0) {
        juce::Logger::writeToLog("Failed to open GPIO chip");
    }
    else {
        initializeGPIO();
    }
}

LgpioGpioAudioProcessor::~LgpioGpioAudioProcessor()
{
    if (handle >= 0) {
        for (int pin : gpioPins) {
            lgGpioFree(handle, pin);
        }
        lgGpiochipClose(handle);
    }
}

const juce::String LgpioGpioAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LgpioGpioAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LgpioGpioAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LgpioGpioAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LgpioGpioAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LgpioGpioAudioProcessor::getNumPrograms()
{
    return 1;
}

int LgpioGpioAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LgpioGpioAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LgpioGpioAudioProcessor::getProgramName (int index)
{
    return {};
}

void LgpioGpioAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void LgpioGpioAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Nothing to do
}

void LgpioGpioAudioProcessor::releaseResources()
{
    // Nothing to do
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LgpioGpioAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LgpioGpioAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int i = 0; i < gpioPins.size(); ++i) {
        bool state = *gpioStates[i] > 0.5f;
        updateGPIOState(gpioPins[i], state);
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = channelData[sample];
        }
    }
}

bool LgpioGpioAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LgpioGpioAudioProcessor::createEditor()
{
    return new LgpioGpioAudioProcessorEditor (*this, parameters);
}

void LgpioGpioAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LgpioGpioAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

bool LgpioGpioAudioProcessor::initializeGPIO()
{
    if (handle < 0) return false;

    for (int pin : gpioPins) {
        int result = lgGpioClaimOutput(handle, 0, pin, 0);  // INIT
        if (result < 0) {
            juce::Logger::writeToLog("Failed to claim GPIO pin " + juce::String(pin) + ": " + juce::String(result));
            return false;
        }
    }

    return true;
}

void LgpioGpioAudioProcessor::updateGPIOState(int pin, bool state)
{
    if (handle >= 0) {
        lgGpioWrite(handle, pin, state ? 1 : 0);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LgpioGpioAudioProcessor();
}