# Sangster's AVR Library

This library provides helpful code for developing toy projects with the
[ATmega328P AVR
microcontroller](https://www.microchip.com/wwwproducts/en/ATmega328p). You
likely don't want to use this for anything critical, as it was developed mainly
as a learning tool to teach myself embedded development.

## Getting Started

After installing this library (see below) you can include it in your project in
the typical way. ex:

```sh
avr-gcc -mmcu=atmega328p -lsangster_atmega328p ...
```

### Installing

You can install this library on the local system with:

```sh
git clone "https://github.com/sangster/embedded-sangster_atmega328p.git"
cd embedded-sangster_atmega328p
./setup_buld.sh && make && sudo make install
```

Alternatively, on [Arch Linux](https://www.archlinux.org/), you can use the
provided [PKGBUILD](./PKGBUILD) file to install this library with the package
manager:

```sh
makepkg -s
pacman -U libsangster_atmega328p-*.pkg.tar.xz
```

## Functionality

This library provides code for the following:

 - **LCD**: The super-common HD44780 2x16 LCD screen
 - **MCU Pins**: Encapsulates the code for managing individual pins
 - **Ring Buffer**: A simple [Ring
   Buffer](https://en.wikipedia.org/wiki/Ring_buffer) implementation
 - **Realtime Clock (DS1307)**: Read and write the current date/time from a RTC
 - **FAT32 filesystem on an SD Card**: A C transcription of William Greiman's
   SD C++ library
 - **Sonar (OSEPP HC-SR04)**
 - **Timer**
 - **TWI/I2C**
 - **USART**
 - **Misc. utility functions**

## License

This project is licensed under the GPL License - see the [LICENSE](./LICENSE)
file for details
