# baremetal
A project to develop baremetal applications for Raspberry Pi and Odroid

This project is based on the work of many others:
- Circle by Rene Stange, a great piece of work covering almost all peripherals and devices on Raspberry Pi ([link](https://github.com/rsta2/circle)), with help of circle stdlib ([link](https://github.com/smuehlst/circle-stdlib)) and newlib ([link](https://github.com/smuehlst/circle-newlib)) by Stephan Muhlstrasser.
- Tutotials by Zoltan Baldaszti ([link](https://github.com/bztsrc/raspi3-tutorial)), Daniels Umanovskis ([link](https://github.com/umanovskis/baremetal-arm)), Leon de Boer ([link](https://github.com/LdB-ECM/Raspberry-Pi)), azo ([link](https://github.com/AZO234/RaspberryPi_BareMetal))

I decided to give it my own turn, for a number of reasons:
- Most code is in C, not C++
- Most code is not very well documented, and is missing pointers to the correct information sources
- If the source is written in C++, the code is mostly C-ish
- The information is spread into bits and pieces
- Circle is pretty complete, but is covering only Raspberry Pi, and includes all models (starting from RPI1), and this also running 32 bit code. I prefer to foces on RPI3B and later, include Odroid boards, and focus only on 64 bit application
- There is quite some assembly code needed to run certain functions, or run them efficiently. This code is not well documented

In other words, I'll try to slowly build up a framework, much like Circle, while adding explanation and pointer to information, and do it in a proper C++ way.

Code will be C++, but will for now not be using any STL classes, as using the STL library implies using the standard C library, which needs to be ported over, much link what Stephan Muhlstrasser did. This is not trivial, so will have to wait a bit.

Next to the code and documentation, I'll be adding tooling to ease development and retrieve feedback on code quality.
For example:
- Development should be possible, as in coding, building and debugging, from Visual Studio on Windows, or Visual Studio Code on its supported platforms. This is not very common for baremetal development and linux related development, but many developers, including myself, prefer Visual Studio as an IDE.
- Builds should be CMake based, not using the old and for many illegible make / auto tools based builds
- Code checking using cppcheck
- Doxygen integration, including the documentation provided here
- Pre-commit hooks using clang-format
- Unit testing, even at baremetal level, using stubs and mocks. This will use an alternative to gtest / gmock, as this relies on the STD C++ library
- Debugging on platform (with the addition of HW), as well as in QEMU, even directly from Visual Studio

