# TEA5767

TEA5767 is an Arduino library for controlling the TEA5767 radio module.

## Behind this project

There are many other TEA5767 libraries out there but most of them had issues with / lacked the implementation of the channel search functionality. As a hobbyist I made this library to resolve those issues for a radio project I was working on.

It must be aknowleged that I'm not a professional Arduino developer and this library surely has a lot of optimizations to be made. However, it got the job done for my project and I hope you find it useful aswell!

## Usage

Examples for using this library are found inside the examples directory.

## Notes

- Due to the nature of the TEA5767 module, some functions introduce a delay which should be noted when using the module. These functions are listed below along with the delay they introduce.

  - setFrequency (20 ms)
  - searchUp / searchDown (50 ms ->)
  - getInfo (30ms)

- In the test environment the maximum search stop level (ADC 10) wasn't high enough to find channels in search mode. As a fix this library doesn't use the internal tuning system but instead implements its own tuning logic.
