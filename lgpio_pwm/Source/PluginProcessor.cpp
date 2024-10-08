#include "PluginProcessor.h"
#include "PluginEditor.h"

PWMControlAudioProcessor::PWMControlAudioProcessor()
    : AudioProcessor (BusesProperties()
                      #if ! JucePlugin_IsMidiEffect
                       #if ! JucePlugin_IsSynth
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       #endif
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      #endif
                    ),
    parameters (*this, nullptr, juce::Identifier ("PWMControl"),
        {
            std::make_unique<juce::AudioParameterChoice>("pwmChannel", "PWM Channel", juce::StringArray("Channel 0 (GPIO 18)", "Channel 1 (GPIO 19)"), 0),
            std::make_unique<juce::AudioParameterFloat>("pwmDuty", "PWM Duty", 0.0f, 100.0f, 50.0f),
            std::make_unique<juce::AudioParameterInt>("pwmFrequency", "PWM Frequency", 1, 10000, 1000)
        })
{
    pwmChannelParameter = parameters.getRawParameterValue("pwmChannel");
    pwmDutyParameter = parameters.getRawParameterValue("pwmDuty");
    pwmFrequencyParameter = parameters.getRawParameterValue("pwmFrequency");

    handle = lgGpiochipOpen(0);
    if (handle < 0) {
        juce::Logger::writeToLog("Failed to open GPIO chip");
    }
    else {
        initializePWM();
    }
}

PWMControlAudioProcessor::~PWMControlAudioProcessor()
{
    if (handle >= 0) {
        if (currentPWMPin != -1) {
            lgTxPwm(handle, currentPWMPin, 0, 0, 0, 0); // Stop PWM
            lgGpioFree(handle, currentPWMPin);
        }
        lgGpiochipClose(handle);
    }
}

const juce::String PWMControlAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PWMControlAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PWMControlAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PWMControlAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PWMControlAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PWMControlAudioProcessor::getNumPrograms()
{
    return 1;
}

int PWMControlAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PWMControlAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PWMControlAudioProcessor::getProgramName (int index)
{
    return {};
}

void PWMControlAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void PWMControlAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PWMControlAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PWMControlAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PWMControlAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // refresh
    static int updateCounter = 0;
    if (++updateCounter >= 100) {
        updatePWMState();
        updateCounter = 0;
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto* inputData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = inputData[sample];
        }
    }
}

bool PWMControlAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PWMControlAudioProcessor::createEditor()
{
    return new PWMControlAudioProcessorEditor (*this, parameters);
}

void PWMControlAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PWMControlAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

bool PWMControlAudioProcessor::initializePWM()
{
    if (handle < 0) return false;

    int pwmPin = static_cast<int>(*pwmChannelParameter) == 0 ? 18 : 19;  // GPIO Number = BCM PINS

    if (currentPWMPin != -1 && currentPWMPin != pwmPin) {
        lgTxPwm(handle, currentPWMPin, 0, 0, 0, 0); // Stop PWM
        lgGpioFree(handle, currentPWMPin);
    }

    int result = lgGpioClaimOutput(handle, 0, pwmPin, 0); // init
    if (result < 0) {
        juce::Logger::writeToLog("Failed to claim PWM pin: " + juce::String(result));
        return false;
    }

    currentPWMPin = pwmPin;
    updatePWMState(); 

    return true;
}

void PWMControlAudioProcessor::updatePWMState()
{
    if (handle >= 0 && currentPWMPin != -1)
    {
        int pwmChannel = static_cast<int>(*pwmChannelParameter);
        float pwmDuty = *pwmDutyParameter;
        float pwmFrequency = static_cast<float>(*pwmFrequencyParameter);

        // when change pin, pwm will be stopped.
        int newPWMPin = pwmChannel == 0 ? 18 : 19;
        if (newPWMPin != currentPWMPin) {
            lgTxPwm(handle, currentPWMPin, 0, 0, 0, 0);
            lgGpioFree(handle, currentPWMPin);
            
            int result = lgGpioClaimOutput(handle, 0, newPWMPin, 0);
            if (result < 0) {
                juce::Logger::writeToLog("Failed to claim new PWM pin: " + juce::String(result));
                return;
            }
            currentPWMPin = newPWMPin;
        }

        // PWM設定
        int result = lgTxPwm(handle, currentPWMPin, pwmFrequency, pwmDuty, 0, 0);
        if (result < 0) {
            juce::Logger::writeToLog("Failed to set PWM: " + juce::String(result));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PWMControlAudioProcessor();
}
