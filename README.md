# baremetal {#BAREMETAL}
A project to develop baremetal 64-bit applications for Raspberry Pi

This project is based on / inspired by the work of many others:
- [Circle](https://github.com/rsta2/circle) by Rene Stange, a great piece of work covering almost all peripherals and devices on Raspberry Pi , with help of [circle stdlib](https://github.com/smuehlst/circle-stdlib) and [newlib](https://github.com/smuehlst/circle-newlib) by Stephan Muehlstrasser.
- [Documentation](https://github.com/raspberrypi/documentation) from the Raspberry Pi foundation
- Tutorials by:
  - [Raspberry Pi foundation](https://www.raspberrypi.com/documentation/computers/getting-started.html#tutorials)
  - [Zoltan Baldaszti](https://github.com/bztsrc/raspi3-tutorial), 
  - [Low Level Devel](https://www.youtube.com/watch?v=pd9AVmcRc6U&list=PLVxiWMqQvhg9FCteL7I0aohj1_YiUx1x8&pp=iAQB), 
  - [David Welch](https://github.com/dwelch67/raspberrypi),
  - [Sergey Matyukevich](https://github.com/s-matyukevich/raspberry-pi-os)
- The community on the [Raspberry Pi forum on Baremetal development](https://forums.raspberrypi.com/viewforum.php?f=72&sid=d06afe6fe03d990ee719c83bde461beb)

I decided to give it my own turn, for a number of reasons:
- Some tutorials are __OLD__, covering only Raspberry Pi 2, sometimes 3
- Most code is in C, not C++
- Most code is not very well documented, and is missing pointers to the correct information sources
- If the source is written in C++, the code is mostly C-ish
- Most of the projects use Make scripts, or even worse automake, which to me seem archaic, as there are more recent tools available such as CMake for build configuration
- The information is spread into bits and pieces
- Circle is pretty complete, but includes all models (starting from RPI1), and also running 32 bit code.
I prefer to focus on RPI3B and later, and only on 64 bit application development
- There is quite some assembly code needed to run certain functions, or run them efficiently. This code is not well documented
- I prefer my code to be readable. Therefore I strive to use clear class, method and variable names, and lay the code out such that it can be easily understood.

In other words, I'll try to slowly build up a framework, much like Circle, while adding explanation and pointer to information, and do it in a proper C++ way.

The projects will be set up using CMake, and with that will allow working from within IDE's such as Visual Studio and Visual Studio Code.

Code will be C++, but will for now not be using any STL classes, as using the STL library implies using the standard C library, which needs to be ported over, much like what Stephan Muehlstrasser did.
This is not trivial, so will have to wait a bit. I hope I'll get around to it eventually, as the being able to use the full power of C++ on a baremetal board is something I'm striving for.

Next to the code and documentation, I'll be adding tooling to ease development and retrieve feedback on code quality.
For example:
- Development should be possible, as in coding, building and debugging, from Visual Studio on Windows, or Visual Studio Code on its supported platforms. This is not very common for baremetal development and linux related development, but many developers, including myself, prefer Visual Studio as an IDE.
- Builds should be CMake based, not using the old and for many illegible make / auto tools based builds
- Code checking using cppcheck
- Doxygen integration, including the documentation provided here
- Using clang-format for uniform code formatting
- Unit testing, even at baremetal level, using stubs and mocks. This will use an alternative to gtest / gmock, that does not rely on the standard C++ library
- Debugging on platform (with the addition of HW), as well as in QEMU, even directly from Visual Studio

## License {#BAREMETAL_LICENSE}

Code included in this repository, as well as code in the tutorials, is shared under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/). See also [the license deed](LICENSE.md).
This means that code will contain a copyright, provided any copies are attributed, but you are free to use it in whichever way you see fit.

## Contents {#BAREMETAL_CONTENTS}

- @subpage TUTORIALS
- @subpage INFORMATION
- @subpage TODO
