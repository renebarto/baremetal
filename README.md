# baremetal {#BAREMETAL}
A project to develop baremetal applications for Raspberry Pi

This project is based on / inspired by the work of many others:
- Circle by Rene Stange, a great piece of work covering almost all peripherals and devices on Raspberry Pi ([link](https://github.com/rsta2/circle)), with help of circle stdlib ([link](https://github.com/smuehlst/circle-stdlib)) and newlib ([link](https://github.com/smuehlst/circle-newlib)) by Stephan Muehlstrasser.
When starting to write this tutorial, Circle did not cover Raspberry Pi 5 yet, but Rene Stange is well underway to also cover that.
- Documentation from the Raspberry Pi foundation ([link](https://github.com/raspberrypi/documentation))
- Tutorials by:
  - Raspberry Pi foundation ([link](https://www.raspberrypi.com/documentation/computers/getting-started.html#tutorials))
  - Zoltan Baldaszti ([link](https://github.com/bztsrc/raspi3-tutorial))
  - Low Level Devel ([link](https://www.youtube.com/watch?v=pd9AVmcRc6U&list=PLVxiWMqQvhg9FCteL7I0aohj1_YiUx1x8&pp=iAQB))
  - David Welch ([link](https://github.com/dwelch67/raspberrypi))
  - Sergey Matyukevich ([link](https://github.com/s-matyukevich/raspberry-pi-os))
- The community on the Raspberry Pi forum on Baremetal development ([link](https://forums.raspberrypi.com/viewforum.php?f=72&sid=d06afe6fe03d990ee719c83bde461beb))

I decided to give it my own turn, for a number of reasons:
- Some tutorials are __OLD__, covering only Raspberry Pi 2, sometimes 3
- Most code is in C, not C++
- Most code is not very well documented, and is missing pointers to the correct information resources
- Even if the source is written in C++, the code is mostly C-ish
- Most of the projects use Make scripts, or even worse automake, which to me seems archaic, as there are more recent tools available such as CMake for build configuration, which also integrate better with state of the art IDEs
- The information is spread into bits and pieces
- Circle is pretty complete, and includes all models of Raspberry Pi (starting from RPI1), and also running 32 bit code.
I prefer to focus on RPI3B and later, and only on 64 bit application development
- There is quite some assembly code needed to run certain functions, or run them efficiently. This code is often not well documented
- I prefer my code to be readable.
Therefore I strive to use clear class, method and variable names, and lay the code out such that it can be easily understood

In other words, I'll try to slowly build up a framework, much like Circle, while adding explanation and documentation, as well as pointers to information, and do it in a proper C++ way.

The tutorial projects will be set up using CMake, and with that will allow working from within IDEs such as Visual Studio (or e.g. CLion).

Code will be C++, but will for now not be using any STL classes, as using the STL library implies using the standard C library, which needs to be ported over, much like what Stephan Muehlstrasser did.
This is not trivial, so will have to wait a bit.
I hope I'll get around to it eventually, as the being able to use the full power of C++ on a baremetal board is something I'm striving for.

Next to the code and documentation, I'll be adding tooling to ease development and retrieve feedback on code quality.
For example:
- Development should be possible, as in coding, building and debugging, from Visual Studio on Windows, or Visual Studio Code on its supported platforms. This is not very common for baremetal development and linux related development, but many developers, including myself, prefer Visual Studio as an IDE
- Builds should be CMake based, not using the old and for many illegible make / auto tools based builds
- Scripting will be available for both Windows as well as Linux host systems
- Code checking is done using cppcheck ([link](http://cppcheck.net/))
- Doxygen ([link](https://www.doxygen.nl/)) integration is offered, including the documentation provided here
- Pre-commit hooks using clang-format ([link](https://clang.llvm.org/docs/ClangFormat.html)) for uniform code formatting
- Unit testing, even at baremetal level, using stubs and mocks.
This will use an alternative to gtest / gmock, as Google Test / Google Mock ([link](https://github.com/google/googletest)) rely on the standard C++ library
- Debugging on platform (with the addition of HW), as well as in QEMU ([link](https://www.qemu.org/)), even directly from Visual Studio

## License {#BAREMETAL_LICENSE}

Code included in this repository, as well as code in the tutorials, is shared under the Creative Commons 1.0 license (see [here](LICENSE.md)).
This means that code will contain a copyright, but you are free to use it in whichever way you see fit. I would appreciate at least a reference to this GitHub project though.

## Contents {#BAREMETAL_CONTENTS}

- @subpage ABBREVIATIONS_AND_TERMS
- @subpage INTRODUCTION
- @subpage TUTORIALS
- @subpage INFORMATION

@todo
- Check if bug in Visual Studio 2022 is fixed
- Build demo for RPI 4
- Try out Raspberry Pi 4 emulation in QEMU
- Find and add Raspberry Pi 5 information, extend samples for this platform as soon as possible (look at circle)
- Add description of how to use JTAG device with Raspberry Pi, using e.g. [Macoy Madson's page](https://macoy.me/blog/programming/RaspberryPi5Debugging)
- Explain setting up development environment on Windows for Windows build : "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
- Add logger, inject character device
- Add retrieval of system information
- Add clang format and git hook for commit
