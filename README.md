# baremetal
A project to develop baremetal applications for Raspberry Pi and Odroid

This project is based on / inspired by the work of many others:
- Circle by Rene Stange, a great piece of work covering almost all peripherals and devices on Raspberry Pi ([link](https://github.com/rsta2/circle)), with help of circle stdlib ([link](https://github.com/smuehlst/circle-stdlib)) and newlib ([link](https://github.com/smuehlst/circle-newlib)) by Stephan Muehlstrasser.
- Documentation from the Raspberry Pi foundation ([link](https://github.com/raspberrypi/documentation))
- Tutorials by:
  - Raspberry Pi foundation ([link](https://www.raspberrypi.com/documentation/computers/getting-started.html#tutorials))
  - Zoltan Baldaszti ([link](https://github.com/bztsrc/raspi3-tutorial)), 
  - Low Level Devel ([link](https://www.youtube.com/watch?v=pd9AVmcRc6U&list=PLVxiWMqQvhg9FCteL7I0aohj1_YiUx1x8&pp=iAQB)), 
  - David Welch ([link](https://github.com/dwelch67/raspberrypi))
  - Sergey Matyukevich ([link](https://github.com/s-matyukevich/raspberry-pi-os))
- The community on the Raspberry Pi forum on Baremetal development ([link](https://forums.raspberrypi.com/viewforum.php?f=72&sid=d06afe6fe03d990ee719c83bde461beb))

I decided to give it my own turn, for a number of reasons:
- Some tutorials are __OLD__, covering only Raspberry Pi 2, sometimes 3
- Most code is in C, not C++
- Most code is not very well documented, and is missing pointers to the correct information sources
- If the source is written in C++, the code is mostly C-ish
- Most of the projects use Make scripts, or even worse automake, which to me seem archaic, as there are more recent tools available such as CMake for build configuration
- The information is spread into bits and pieces
- Circle is pretty complete, but is covering only Raspberry Pi, and includes all models (starting from RPI1), and also running 32 bit code.
I prefer to focus on RPI3B and later, including Odroid boards, and only on 64 bit application development
- There is quite some assembly code needed to run certain functions, or run them efficiently. This code is not well documented
- I prefer my code to be readable. Therefore I strive to use clear class, method and variable names, and lay the code out such that it can be easily understood.

In other words, I'll try to slowly build up a framework, much like Circle, while adding explanation and pointer to information, and do it in a proper C++ way.

The projects will be set up using CMake, and with that will allow working from within IDE's such as Visual Studio.

Code will be C++, but will for now not be using any STL classes, as using the STL library implies using the standard C library, which needs to be ported over, much like what Stephan Muehlstrasser did.
This is not trivial, so will have to wait a bit. I hope I'll get around to it eventually, as the being able to use the full power of C++ on a baremetal board is something I'm striving for.

Next to the code and documentation, I'll be adding tooling to ease development and retrieve feedback on code quality.
For example:
- Development should be possible, as in coding, building and debugging, from Visual Studio on Windows, or Visual Studio Code on its supported platforms. This is not very common for baremetal development and linux related development, but many developers, including myself, prefer Visual Studio as an IDE.
- Builds should be CMake based, not using the old and for many illegible make / auto tools based builds
- Code checking using cppcheck
- Doxygen integration, including the documentation provided here
- Pre-commit hooks using clang-format for uniform code formatting
- Unit testing, even at baremetal level, using stubs and mocks. This will use an alternative to gtest / gmock, as this relies on the standard C++ library
- Debugging on platform (with the addition of HW), as well as in QEMU, even directly from Visual Studio

## License

Code included in this repository, as well as code in the tutorials, is shared under the Creative Commons 1.0 license (see [here](LICENSE.md)).
This means that code will contain a copyright, but you are free to use it in whichever way you see fit. I would appreciate at least a reference to this GitHub project though.

For now, we will focus on Raspberry Pi, as the information is more readily available than for Odroid.

## Contents

- \subpage TUTORIALS 
- [Setting up for development](doc/01-setting-up-for-development.md)
- [Setting up a first project](doc/02-setting-up-a-project.md)
- [Setting up project structure](doc/03-setting-up-project-structure.md)
- [Setting up common options](doc/04-setting-up-common-options.md)
- [First application - Using the console - UART1](doc/05-console-uart1.md)
- [Improving startup and static initialization](doc/06-improving-startup-static-initialization.md)
- [Generalization](doc/07-generalization.md)
- [Timer](doc/08-timer.md)
- [Mailbox](doc/09-mailbox.md)
- [UART0](doc/10-uart0.md)
- Add doxygen
- Add logger, inject character device
- Add retrieval of system information
- Add clang format and git hook for commit

## Additional information

- \subpage INFORMATION
- [Hardware overview](doc/hardware-overview.md)
- [System startup](doc/system-startup.md)

## Todo

@todo
- Check if bug in Visual Studio 2022 is fixed
- Build demo for RPI 4
- Try out Raspberry Pi 4 emulation in QEMU
- Find and add Raspberry Pi 5 information, extend samples for this platform as soon as possible (look at circle)
- Add boot process for Odroid system
- Add description of Odroid platform support in QEMU
- Add description of how to use JTAG device with Raspberry Pi and possibly Odroid
- Explain setting up development environment on Windows for Windows build : "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"