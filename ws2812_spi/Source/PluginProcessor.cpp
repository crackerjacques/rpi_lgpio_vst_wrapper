#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

#define DEBUG_PRINT(x) std::cout << "[DEBUG] " << __FUNCTION__ << ":" << __LINE__ << " " << x << std::endl
#define MAX_LEDS 300

LEDStripAudioProcessor::LEDStripAudioProcessor()
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                     ),
    parameters (*this, nullptr, juce::Identifier ("LEDStrip"),
    {
        std::make_unique<juce::AudioParameterInt>("red", "Red", 0, 255, 0),
        std::make_unique<juce::AudioParameterInt>("green", "Green", 0, 255, 0),
        std::make_unique<juce::AudioParameterInt>("blue", "Blue", 0, 255, 0),
        std::make_unique<juce::AudioParameterFloat>("brightness", "Brightness", 0.0f, 100.0f, 20.0f),
        std::make_unique<juce::AudioParameterInt>("ledCount", "LED Count", 1, 300, 7),
        std::make_unique<juce::AudioParameterBool>("scrollMode", "Scroll Mode", false),
        std::make_unique<juce::AudioParameterFloat>("scrollSpeed", "Scroll Speed", 0.0f, 10.0f, 0.0f),
        std::make_unique<juce::AudioParameterBool>("scrollDirection", "Scroll Direction", false),
        std::make_unique<juce::AudioParameterChoice>("gradientType", "Gradient Type",
            juce::StringArray("Red", "Green", "Blue", "Rainbow", "Red to Green", "Red to Blue", "Green to Blue"), 0),
        std::make_unique<juce::AudioParameterFloat>("strobeSpeed", "Strobe Speed", 0.0f, 60.0f, 0.0f), 
    }),
    handle(-1), spi_fd(-1)
{
    DEBUG_PRINT("Initializing LEDStripAudioProcessor");

    redParameter = parameters.getRawParameterValue("red");
    greenParameter = parameters.getRawParameterValue("green");
    blueParameter = parameters.getRawParameterValue("blue");
    brightnessParameter = parameters.getRawParameterValue("brightness");
    ledCountParameter = parameters.getRawParameterValue("ledCount");
    scrollModeParameter = parameters.getRawParameterValue("scrollMode");
    scrollSpeedParameter = parameters.getRawParameterValue("scrollSpeed");
    scrollDirectionParameter = parameters.getRawParameterValue("scrollDirection");
    gradientTypeParameter = parameters.getRawParameterValue("gradientType");
    strobeSpeedParameter = parameters.getRawParameterValue("strobeSpeed");

    // Verify
    jassert(redParameter != nullptr);
    jassert(greenParameter != nullptr);
    jassert(blueParameter != nullptr);
    jassert(brightnessParameter != nullptr);
    jassert(ledCountParameter != nullptr);
    jassert(scrollModeParameter != nullptr);
    jassert(scrollSpeedParameter != nullptr);
    jassert(scrollDirectionParameter != nullptr);
    jassert(gradientTypeParameter != nullptr);
    jassert(strobeSpeedParameter != nullptr);

    if (!initializeGPIO()) {
        DEBUG_PRINT("Failed to initialize GPIO");
        return;
    }

    if (!initializeSPI()) {
        DEBUG_PRINT("Failed to initialize SPI");
        return;
    }

    DEBUG_PRINT("Waiting 50ms after GPIO and SPI initialization");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    DEBUG_PRINT("Initializing WS2811");

    ledstring.freq = WS2811_TARGET_FREQ;
    ledstring.dmanum = 10;
    ledstring.channel[0].gpionum = 10;
    ledstring.channel[0].count = 7;
    ledstring.channel[0].invert = 0;
    ledstring.channel[0].brightness = 255;
    ledstring.channel[0].strip_type = WS2811_STRIP_GRB;

    DEBUG_PRINT("WS2811 configuration set, calling ws2811_init");
    ws2811_return_t init_result = ws2811_init(&ledstring);
    if (init_result != WS2811_SUCCESS) {
        DEBUG_PRINT("Failed to initialize WS2811: " << ws2811_get_return_t_str(init_result));
    } else {
        DEBUG_PRINT("WS2811 initialized successfully");
    }

    DEBUG_PRINT("Initialization complete, starting operation");

    juce::Timer::callAfterDelay(100, [this]() {
        startTimerHz(60);
        DEBUG_PRINT("Timer started");
    });
}

LEDStripAudioProcessor::~LEDStripAudioProcessor()
{
    DEBUG_PRINT("Destroying LEDStripAudioProcessor");
    if (handle >= 0) {
        lgGpioFree(handle, 10);
        lgGpiochipClose(handle);
    }
    if (spi_fd >= 0) {
        close(spi_fd);
    }
    ws2811_fini(&ledstring);
}

bool LEDStripAudioProcessor::initializeGPIO()
{
    DEBUG_PRINT("Opening GPIO chip");
    handle = lgGpiochipOpen(0);
    if (handle < 0) {
        DEBUG_PRINT("Failed to open GPIO chip, error: " << handle);
        return false;
    }

    DEBUG_PRINT("Setting GPIO 10 to output mode");
    int ret = lgGpioClaimOutput(handle, 0, 10, 0);  // GPIO 10, initial level LOW
    if (ret < 0) {
        DEBUG_PRINT("Failed to set GPIO 10 to output mode, error: " << ret);
        lgGpiochipClose(handle);
        handle = -1;
        return false;
    }

    DEBUG_PRINT("GPIO 10 set to output mode successfully");
    return true;
}

bool LEDStripAudioProcessor::initializeSPI()
{
    DEBUG_PRINT("Opening SPI device");
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) {
        DEBUG_PRINT("Failed to open SPI device, error: " << errno);
        return false;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 800000;

    DEBUG_PRINT("Setting SPI mode");
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        DEBUG_PRINT("Failed to set SPI mode, error: " << errno);
        close(spi_fd);
        return false;
    }

    DEBUG_PRINT("Setting SPI bits per word");
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        DEBUG_PRINT("Failed to set SPI bits per word, error: " << errno);
        close(spi_fd);
        return false;
    }

    DEBUG_PRINT("Setting SPI max speed");
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        DEBUG_PRINT("Failed to set SPI max speed, error: " << errno);
        close(spi_fd);
        return false;
    }

    DEBUG_PRINT("SPI initialized successfully");
    return true;
}

void LEDStripAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DEBUG_PRINT("prepareToPlay called with sampleRate: " << sampleRate << ", samplesPerBlock: " << samplesPerBlock);
}

void LEDStripAudioProcessor::releaseResources()
{
    DEBUG_PRINT("releaseResources called");
}

bool LEDStripAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LEDStripAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Do nothing.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
    }
}

void LEDStripAudioProcessor::updateLEDs()
{
    static bool firstUpdate = true;
    if (firstUpdate) {
        DEBUG_PRINT("First LED update");
        firstUpdate = false;
    }

    DEBUG_PRINT("Updating LEDs");

    int actualLedCount = ledstring.channel[0].count;
    int requestedLedCount = static_cast<int>(*ledCountParameter);
    int ledCount = std::min(requestedLedCount, actualLedCount);
    ledCount = std::min(ledCount, MAX_LEDS); 

    DEBUG_PRINT("Requested LED count: " << requestedLedCount 
              << ", Actual LED count: " << actualLedCount 
              << ", Using: " << ledCount);

    std::vector<ws2811_led_t> ledBuffer(ledCount, 0);


    if (*scrollModeParameter > 0.5f)
    {
        DEBUG_PRINT("Scroll mode active");
        static float offset = 0.0f;
        float speed = *scrollSpeedParameter;
        bool direction = *scrollDirectionParameter > 0.5f;
        int gradientType = static_cast<int>(*gradientTypeParameter);
        if (gradientType < 0 || gradientType > 6) {
            DEBUG_PRINT("Invalid gradient type: " << gradientType << ". Defaulting to 0 (Red).");
            gradientType = 0;
        }

        DEBUG_PRINT("Speed: " << speed << ", Direction: " << (direction ? "Forward" : "Backward") 
                  << ", Gradient Type: " << gradientType);

        for (int i = 0; i < ledCount; ++i)
        {
            float position = static_cast<float>(i) / ledCount;
            position += direction ? offset : -offset;
            position = fmodf(position, 1.0f);
            if (position < 0) position += 1.0f;

            uint8_t r = 0, g = 0, b = 0;
            switch (gradientType)
            {
                case 0: // Red
                    r = static_cast<uint8_t>(position * 255);
                    break;
                case 1: // Green
                    g = static_cast<uint8_t>(position * 255);
                    break;
                case 2: // Blue
                    b = static_cast<uint8_t>(position * 255);
                    break;
                case 3: // Rainbow
                    r = static_cast<uint8_t>((1 - position) * 255);
                    g = static_cast<uint8_t>(std::sin(position * 3.14159f) * 255);
                    b = static_cast<uint8_t>(position * 255);
                    break;
                case 4: // R to G
                    r = static_cast<uint8_t>((1 - position) * 255);
                    g = static_cast<uint8_t>(position * 255);
                    break;
                case 5: // R to B
                    r = static_cast<uint8_t>((1 - position) * 255);
                    b = static_cast<uint8_t>(position * 255);
                    break;
                case 6: // G to B
                    g = static_cast<uint8_t>((1 - position) * 255);
                    b = static_cast<uint8_t>(position * 255);
                    break;
                default:
                    DEBUG_PRINT("Unexpected gradient type: " << gradientType << ". Defaulting to Red.");
                    r = static_cast<uint8_t>(position * 255);
                    break;
            }
            ledBuffer[i] = rgb_to_ws2811(r, g, b);
            DEBUG_PRINT("LED " << i << " set to R:" << (int)r << " G:" << (int)g << " B:" << (int)b << " (scroll mode)");
        }

        if (speed > 0.0f)
        {
            offset += speed * 0.01f;
            if (offset > 1.0f) offset -= 1.0f;
        }
    }
    else
    {
        DEBUG_PRINT("Static color mode active");
        uint8_t r = static_cast<uint8_t>(*redParameter);
        uint8_t g = static_cast<uint8_t>(*greenParameter);
        uint8_t b = static_cast<uint8_t>(*blueParameter);
        ws2811_led_t color = rgb_to_ws2811(r, g, b);
        
        DEBUG_PRINT("Color: R=" << (int)r << ", G=" << (int)g << ", B=" << (int)b);

        std::fill(ledBuffer.begin(), ledBuffer.end(), color);
        for (int i = 0; i < ledCount; ++i)
        {
            DEBUG_PRINT("LED " << i << " set to R:" << (int)r << " G:" << (int)g << " B:" << (int)b << " (static mode)");
        }
    }

    float brightnessPercentage = *brightnessParameter / 100.0f;
    uint8_t brightness = static_cast<uint8_t>(brightnessPercentage * 255);
    DEBUG_PRINT("Brightness set to: " << (int)brightness << " (" << *brightnessParameter << "%)");

    DEBUG_PRINT("Copying buffer to LED string");
    for (int i = 0; i < ledCount && i < MAX_LEDS; ++i)
    {
        uint32_t color = ledBuffer[i];
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        r = (r * brightness) >> 8;
        g = (g * brightness) >> 8;
        b = (b * brightness) >> 8;

        ledstring.channel[0].leds[i] = rgb_to_ws2811(r, g, b);
        DEBUG_PRINT("LED " << i << " final values: R:" << (int)r << " G:" << (int)g << " B:" << (int)b);
    }

    // Strobe effect
    float strobeSpeed = *strobeSpeedParameter;
    static bool strobeState = true;
    static float strobeTimer = 0.0f;

    DEBUG_PRINT("Strobe Speed: " << strobeSpeed << " Hz");

    if (strobeSpeed > 0.0f)
    {
        float strobePeriod = 1.0f / strobeSpeed;
        strobeTimer += 1.0f / 60.0f; 

        if (strobeTimer >= strobePeriod)
        {
            strobeState = !strobeState;
            strobeTimer = 0.0f;
            DEBUG_PRINT("Strobe state changed to: " << (strobeState ? "ON" : "OFF"));
        }

        if (!strobeState)
        {
            std::fill(ledstring.channel[0].leds, ledstring.channel[0].leds + ledCount, 0);
            DEBUG_PRINT("Strobe OFF - All LEDs set to black");
        }
    }

    DEBUG_PRINT("Preparing to render LED strip");
    DEBUG_PRINT("ledstring.freq: " << ledstring.freq);
    DEBUG_PRINT("ledstring.dmanum: " << ledstring.dmanum);
    DEBUG_PRINT("ledstring.channel[0].gpionum: " << ledstring.channel[0].gpionum);
    DEBUG_PRINT("ledstring.channel[0].count: " << ledstring.channel[0].count);
    DEBUG_PRINT("ledstring.channel[0].invert: " << ledstring.channel[0].invert);
    DEBUG_PRINT("ledstring.channel[0].brightness: " << (int)ledstring.channel[0].brightness);
    DEBUG_PRINT("ledstring.channel[0].strip_type: " << ledstring.channel[0].strip_type);

    DEBUG_PRINT("Rendering LED strip");
    ws2811_return_t result = ws2811_render(&ledstring);
    if (result != WS2811_SUCCESS) {
        DEBUG_PRINT("Failed to render LED strip: " << ws2811_get_return_t_str(result));
        DEBUG_PRINT("Error code: " << result);
        // additions
        switch(result) {
            case WS2811_ERROR_GENERIC:
                DEBUG_PRINT("Generic failure");
                break;
            case WS2811_ERROR_OUT_OF_MEMORY:
                DEBUG_PRINT("Out of memory");
                break;
            case WS2811_ERROR_HW_NOT_SUPPORTED:
                DEBUG_PRINT("Hardware revision is not supported");
                break;
            case WS2811_ERROR_MEM_LOCK:
                DEBUG_PRINT("Memory lock failed");
                break;
            case WS2811_ERROR_MMAP:
                DEBUG_PRINT("mmap() failed");
                break;
            case WS2811_ERROR_MAP_REGISTERS:
                DEBUG_PRINT("Unable to map registers into userspace");
                break;
            case WS2811_ERROR_GPIO_INIT:
                DEBUG_PRINT("Unable to initialize GPIO");
                break;
            case WS2811_ERROR_PWM_SETUP:
                DEBUG_PRINT("Unable to initialize PWM");
                break;
            case WS2811_ERROR_MAILBOX_DEVICE:
                DEBUG_PRINT("Unable to open mailbox device");
                break;
            case WS2811_ERROR_DMA:
                DEBUG_PRINT("DMA error");
                break;
            case WS2811_ERROR_ILLEGAL_GPIO:
                DEBUG_PRINT("Selected GPIO not possible");
                break;
            case WS2811_ERROR_PCM_SETUP:
                DEBUG_PRINT("Unable to initialize PCM");
                break;
            case WS2811_ERROR_SPI_SETUP:
                DEBUG_PRINT("Unable to initialize SPI");
                break;
            case WS2811_ERROR_SPI_TRANSFER:
                DEBUG_PRINT("SPI transfer error");
                break;
            default:
                DEBUG_PRINT("Unknown error");
                break;
        }
    } else {
        DEBUG_PRINT("LED strip rendered successfully");
    }

    DEBUG_PRINT("LED update complete");
}

bool LEDStripAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LEDStripAudioProcessor::createEditor()
{
    return new LEDStripAudioProcessorEditor (*this, parameters);
}

void LEDStripAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    DEBUG_PRINT("Saving plugin state");
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LEDStripAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    DEBUG_PRINT("Loading plugin state");
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            DEBUG_PRINT("Plugin state loaded successfully");
        }
        else
        {
            DEBUG_PRINT("Failed to load plugin state: invalid XML");
        }
    }
    else
    {
        DEBUG_PRINT("Failed to load plugin state: null XML");
    }
}

void LEDStripAudioProcessor::timerCallback()
{
    DEBUG_PRINT("Timer callback triggered");
    updateLEDs();
}

const juce::String LEDStripAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LEDStripAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LEDStripAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LEDStripAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LEDStripAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LEDStripAudioProcessor::getNumPrograms()
{
    return 1;
                
}

int LEDStripAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LEDStripAudioProcessor::setCurrentProgram (int index)
{
    DEBUG_PRINT("setCurrentProgram called with index: " << index);
}

const juce::String LEDStripAudioProcessor::getProgramName (int index)
{
    DEBUG_PRINT("getProgramName called with index: " << index);
    return {};
}

void LEDStripAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    DEBUG_PRINT("changeProgramName called with index: " << index << " and newName: " << newName);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LEDStripAudioProcessor();
}