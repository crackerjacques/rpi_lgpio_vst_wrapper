# GPIO Controller VST Plugins for Raspberry Pi >3B

![leds](https://github.com/user-attachments/assets/caa9318c-d068-4812-bda5-a86546036146)


LGPIO ( https://github.com/joan2937/lg ) 

and 

rpi_ws281x ( https://github.com/jgarff/rpi_ws281x ) wrapped to VST with JUCE.
Completed to the extent that it somehow works.

The operation was checked with Reaper 7.20 on Raspberry pi 3B/4B/CM4/5.

Computer musician friendly PWM and LED control is possible, but who is going to compose music with a Linux SBC?
If there is, you are so great.

# Install

download archive from vst2_binary or vst3_binary dir and extract  and put .so or folder to ~/.vst, ~/.vst3
and install dependency if you needed.

```
sudo apt install liblgpio-dev
sudo ln -s /usr/lib/aarch64-linux-gnu/liblgpio.so /usr/lib 

sudo apt install cmake build-essential 
git clone https://github.com/jgarff/rpi_ws281x.git

cd rpi_ws281x
mkdir build
cd build
cmake -D BUILD_SHARED=ON -D BUILD_TEST=ON ..

sudo make install
sudo ldconfig

```

# lgpio_gpio

![gpio](https://github.com/user-attachments/assets/d928cc3b-5a9f-4378-84e6-ba9cef833f44)


Checkboxes arranged like an Idaho potato field. It looks like the rural landscape of Niigata Prefecture where I grew up. 
In other words, it's a bit drab.
If this find /dev/spidev*, the pins for spi are grayed out.

# lgpio_pwm

![pwm](https://github.com/user-attachments/assets/e38ba7ad-cd09-4817-9985-74fe07d27756)


The user interface is as desolate as a desert.
Sooner or later, a camel will emerge from the shade of the faders.

# ws2812_tester


![spi](https://github.com/user-attachments/assets/1873be96-f0e9-471d-b497-e022f95229e5)

first (of all)
https://github.com/jgarff/rpi_ws281x

I would like you to look here and set up your own hardware.

___note : cmdline.txt and config.txt have been moved to /boot/firmware/.___

Connect the signal pin of WS2812 to pin 10, and connect power and GND. Now it's party time.
I don't recommend using the Raspberry Pi built-in power pins. The available current is limited.
(Excessive current flow may cause paperweight)

<img src="https://github.com/user-attachments/assets/250303af-2224-4f60-9968-e0faf7716bed" width="66%" alt="circuit_ex">

Also, a 3.3>5V level shifter would be better.

That said, I experimented with a tape with 7 LEDs mounted on it for testing and had no problem with the built-in power supply for this level.

This one is the only one very much in order. I am sure he is sorry too, so forgive him.

#

Overall, VST3 is more stable than VST2.
When building, use Projucer to open the projucer files in each plug-in directory
