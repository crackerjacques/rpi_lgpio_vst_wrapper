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

