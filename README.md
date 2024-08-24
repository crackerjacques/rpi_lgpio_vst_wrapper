# GPIO Controller VST Plugins for Raspberry Pi >3B



LGPIO and rpi_ws281x wrapped to VST.
Completed to the extent that it somehow works.

The operation was checked with Reaper 7.20.

Computer musician friendly PWM and LED control is possible, but who is going to compose music with a Linux SBC?
If there is, you are so great.

# Install

extract zip and put .so or folder to ~/.vst, ~/.vst3
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

Checkboxes and toggles arranged like an Idaho potato field. It looks like the rural landscape of Niigata Prefecture where I grew up. 
In other words, it's a bit drab.
If this find /dev/spidev*, the pins for spi are grayed out.

# lgpio_pwm

The user interface is as desolate as a desert.
Sooner or later, a camel will emerge from the shade of the faders.

# ws2812_spi

first (of all)
https://github.com/jgarff/rpi_ws281x
I would like you to look here and set up your own hardware.
Don't panic, because cmdline.txt and config.txt have been moved to /boot/firmware/.

Connect the signal pin of WS2812 to pin 10, and connect power and GND. Now it's party time.
I don't recommend using the Raspberry Pi built-in power pins. The available current is limited.
(Excessive current flow may cause paperweight)

Also, a 3.3>5V level shifter would be better.

That said, I experimented with a tape with 7 LEDs mounted on it for testing and had no problem with the built-in power supply for this level.

This one is the only one very much in order. I am sure he is sorry too, so forgive him.
