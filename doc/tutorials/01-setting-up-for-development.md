# Tutorial 01: Setting up for development {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT}

@tableofcontents

If we want to do development for an embedded board, a number of things need to be set up:

## Compiler toolchain {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_COMPILER_TOOLCHAIN}

In order to perform a cross-build (building on one machine to create code that runs on a different target machine), we will need a so-called toolchain.
This is the complete set of compilers, assemblers, linkers, etc. to build binaries for the target platform.

When building for a specific OS, we also need a sysroot, simply said a SDK, that contains all header files and libraries to compile and link with for the specific OS installation.
This requires all libraries needed to build the SW to be present in the sysroot, otherwise we clearly cannot build the SW.
However, since we're going build for baremetal, the only libraries we'll need is the compiler's set of libraries. So no sysroot is needed.
If however we would build against the Raspberry Pi OS for example, a sysroot would be needed.

For the toolchain, we're going to choose the GNU toolchain for ARM64. It is possible to use Clang for this as well, but we'll not cover that here.

The current version as of writing this document is 13.3 rel 1.

### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_COMPILER_TOOLCHAIN_WINDOWS}

For building on Windows, download [this version](https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-arm-none-linux-gnueabihf.zip).
Be careful to use the aarch64 version (64 bit), for target none-elf (baremetal). Using the none-linux-gnu version will enable you to build against Linux, however that is not what we need.

The best way to install on Windows is to create a directory with a short path, say `D:\Toolchains`, and place the unzipped files in a folder underneath this directory.
Then make sure to add the path to the compiler, etc. to the environment (PATH) at system level, so in this case `D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin`:

<img src="images/env-before.png" alt="Environment edit" width="500"/>

In the lower pane, select Path and click Edit...

<img src="images/env-add.png" alt="Environment add" width="500"/>

Click new and add the directory of the compilers, etc., in this case `D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin`. Click OK.

Click OK again to finalize changing the environment.

To test, open a command line prompt, and execute:
```bat
aarch64-none-elf-gcc --version
```

The command should execute without error, and print the version of the compiler:

```text
aarch64-none-elf-gcc (Arm GNU Toolchain 13.3.Rel1 (Build arm-13.24)) 13.3.1 20240614
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_COMPILER_TOOLCHAIN_LINUX}

For building on Linux, download [this version](https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf.tar.xz).
Be careful to use the aarch64 version (64 bit), for target none-elf (baremetal). Using the none-linux-gnu version will enable you to build against Linux, however that is not what we need.

The easiest way to install on Linux is as follows:

```bash
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf.tar.xz
sudo mkdir -p /opt/toolchains
tar xvf arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf.tar.xz
sudo mv arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/ /opt/toolchains/
```

In order to add the path to your environment, edit `.bashrc` in your home directory, and add the following line, for example at the end:

```text
export PATH=/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin:$PATH
```

To test, first log out and log in again, open a command line prompt, and execute:
```bat
aarch64-none-elf-gcc --version
```

The command should execute without error, and print the version of the compiler:

```text
aarch64-none-elf-gcc (Arm GNU Toolchain 13.3.Rel1 (Build arm-13.24)) 13.3.1 20240614
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

## Build environment {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT}

In order to build, we will be using CMake, in combination with Ninja.

### CMake {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_CMAKE}

#### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_CMAKE_WINDOWS}

When using Visual Studio 2019 or later, there will be a CMake version installed for you. For VS 2019, the location will normally be `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`.

To test, open a command line prompt, and execute:
```bat
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --version
```

The command should execute without error, and print the version of the CMake:

```text
cmake version 3.20.21032501-MSVC_2

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

When not using Visual Studio, or if you prefer to use the latest and greatest version of `CMake`, install CMake by downloading [here](https://cmake.org/download/#latest), 
by selecting Windows x64 Installer and install it. This will also give you a more recent version of CMake.
The latest stable release at the moment of writing this document is [3.30.1](https://github.com/Kitware/CMake/releases/download/v3.30.1/cmake-3.30.1-windows-x86_64.msi).

<img src="images/cmake-install-1.png" alt="CMake installation step 1" width="400"/>

Click Next

<img src="images/cmake-install-2.png" alt="CMake installation step 2" width="400"/>

Check "I accept the terms in the License Agreement", and click Next

<img src="images/cmake-install-3.png" alt="CMake installation step 3" width="400"/>

Select "Add CMake to the PATH environment variable"

Click Next

<img src="images/cmake-install-4.png" alt="CMake installation step 4" width="400"/>

Leave the installation path as is. Click Next

<img src="images/cmake-install-5.png" alt="CMake installation step 5" width="400"/>

Click Install and approve installation

<img src="images/cmake-install-6.png" alt="CMake installation step 6" width="400"/>

Click Finish

To test, open a command line prompt, and execute:
```bat
"C:\Program Files\CMake\bin\cmake.exe" --version
```

The command should execute without error, and print the version of the CMake:

```text
cmake version 3.30.1

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

#### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_CMAKE_LINUX}

On Linux (Debian), CMake can easily be installed using

```bash
sudo apt update
sudo apt install cmake
```

The latest version will depend on your distribution. In this case (Ubuntu 22.04) it is 3.29.6

```text
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following NEW packages will be installed:
  cmake
0 upgraded, 1 newly installed, 0 to remove and 1 not upgraded.
Need to get 13,9 MB of archives.
After this operation, 46,0 MB of additional disk space will be used.
Get:1 https://apt.kitware.com/ubuntu focal/main amd64 cmake amd64 3.29.6-0kitware1ubuntu20.04.1 [13,9 MB]
Fetched 13,9 MB in 3s (5.167 kB/s)
Selecting previously unselected package cmake.
(Reading database ... 306230 files and directories currently installed.)
Preparing to unpack .../cmake_3.29.6-0kitware1ubuntu20.04.1_amd64.deb ...
Unpacking cmake (3.29.6-0kitware1ubuntu20.04.1) ...
Setting up cmake (3.29.6-0kitware1ubuntu20.04.1) ...
```

To test, open a command line prompt, and execute:
```bash
cmake --version
```

The command should execute without error, and print the version of the CMake:

```text
cmake version 3.29.6

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

### Ninja {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_NINJA}

To speed up building, often the Ninja build tool is used. This tool is also used by default by Visual Studio when performing CMake builds.

#### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_NINJA_WINDOWS}

If Visual Studio 2019 is installed, you can find Ninja in `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja`:

To test, open a command line prompt, and execute:
```bat
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" --version
```

The command should execute without error, and print the version of the Ninja:

```text
1.10.2
```

To install Ninja build on your system separately, download it from [here](https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip), and unpack the ZIP file. It will contain only one file, `ninja.exe`.
For example you can place this file in `C:\Program Files\Ninja`. Make sure to add it to your PATH environment variable.

To test, open a command line prompt, and execute:
```bat
"C:\Program Files\Ninja\ninja.exe" --version
```

The command should execute without error, and print the version of the Ninja:

```text
1.12.1
```

#### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_NINJA_LINUX}

On Linux, your can install ninja by installing the `ninja-build` package:

```bash
sudo apt install ninja-build
```

The version installed will differ depending on your distribution. In this case (Ubuntu 22.04) it is 1.10.1.

```text
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following NEW packages will be installed:
  ninja-build
0 upgraded, 1 newly installed, 0 to remove and 1 not upgraded.
Need to get 111 kB of archives.
After this operation, 358 kB of additional disk space will be used.
Get:1 http://nl.archive.ubuntu.com/ubuntu jammy/universe amd64 ninja-build amd64 1.10.1-1 [111 kB]
Fetched 111 kB in 0s (2.032 kB/s)
Selecting previously unselected package ninja-build.
(Reading database ... 306229 files and directories currently installed.)
Preparing to unpack .../ninja-build_1.10.1-1_amd64.deb ...
Unpacking ninja-build (1.10.1-1) ...
Setting up ninja-build (1.10.1-1) ...
Processing triggers for man-db (2.10.2-1) ...
```

To test, open a command line prompt, and execute:
```bash
ninja --version
```

The command should execute without error, and print the version of the Ninja:

```text
1.10.1
```

## Development environment {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEVELOPMENT_ENVIRONMENT}

The development environment can be as simple as a text editor and running commands on the command prompt.
For Linux, we will use this approach. For Windows, we will be using Visual Studio, however we will also start with the command line, as using Visual Studio requires a bit more configuration. 

### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEVELOPMENT_ENVIRONMENT_WINDOWS}

The current choice for Visual Studio 2019 has to do with the fact that debugging on a remote target (Linux or baremetal) with Visual Studio 2022 does not work from within VS, due to a bug. As the debugging part is partly shared between Visual Studio 2019 and Visual Studio 2022, my advice is to only have Visual Studio 2019 installed.

When installing Visual Studio 2019, at least make sure `Desktop development with C++` is checked as a feature. The feature `Linux development with C++` is not necessary.

Configuration of the project for Visual Studio is described later in [Setting up project structure](03-setting-up-project-structure.md).

Of course, a project can also be set up, and code written, using one of the many text editors available for Windows:

- Notepad
- Notepad++
- UltraEdit
- Sublime
- Emacs
- Visual Studio Code
- ...

### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEVELOPMENT_ENVIRONMENT_LINUX}

For Linux, the easiest way to build is on the command line. It is possible to use e.g. Visual Studio Code for this as well, but configuration for building and debugging is quite cumbersome, so will not be described here.
Make sure that your editor of choice is installed. Some logical choices may be:

- vi
- vim
- nano
- sublime
- geany
- joe
- gedit
- pluma
- gnome-text-editor
- Visual Studio Code
- ...

Configuration of the project is described later in [Setting up a project](02-setting-up-a-project.md).

## Deployment mechanism {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM}

In order to deploy a baremetal application, we will need to follow a couple of steps:

- Compile sources and create an application (just a single one, as this is a baremetal system) as a ELF application (.elf)
- Convert the application into a kernel image (.img)
- Place the image onto the target using one of the following methods:
  - Copy the image onto an SD card and start the system (see [Running from SD card](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_FROM_SD_CARD)).
The SD card will need to contain some other files in order for the system to start.
  - Create a network boot SD card, e.g. using [CircleNetboot](https://github.com/probonopd/CircleNetboot) and start the system.
__This is my personal preference, as we don't need to rewrite the SD card every time__. The next time the system boots, it will start netboot again.
See [Running using Netboot](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_USING_NETBOOT).
  - Run the image within QEMU. QEMU is an emulator that supports multiple platforms, including Raspberry Pi (the default QEMU supports up to rpi4 for now).

QEMU requires quite some options, which are not always straightforward. Also, QEMU can never fully emulate all HW on the board.
For running inside QEMU, see [Running in QEMU](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU).

Converting the application into an image will be described later. It uses `aarch64-none-elf-objcopy` (also part of the toolchain) to convert the .elf file into a .img file.

For testing, I've added a pre-built demo application, which we will slowly try to build over the coming tutorials.
The pre-built image is located in `tutorial/01-demo/windows/kernel8.img` (built from `tutorial/01-demo/windows/demo.elf`) and `tutorial/01-demo/linux/kernel8.img` (built from `tutorial/01-demo/linux/demo.elf`).
The only difference is that one was built on Windows, the other on Linux. The console used in this case is UART0.

### Attaching a serial console {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_ATTACHING_A_SERIAL_CONSOLE}

For the demo you will need to attach a serial console

<u>__Beware of the following!__</u>

- Make __VERY SURE__ to use a 3.3V serial to USB converter. Using the 5V version is likely to damage your board.
For an example of a good adapter see [ADAFruit](https://www.adafruit.com/product/954). See below for the one I prefer, however there are more.
<u>__Be careful about the pinning__</u>.
- Attach the USB to serial device <u>__before__</u> powering on the board
- Make sure to connect the serial to USB device to the <u>__correct pins__</u> (GPIO 14 is used for TxD, GPIO15 for RxD for either UART 0 or UART 1.
In our examples we will be using UART 1 initially, but later move to UART 0, as this is slightly harder to initialize)
- Make sure you have the drivers for the serial to USB device (USB side of course) correctly installed on your host system
  - Most devices use a Prolific 2303 TA chip, support on Windows 11 is a bit tricky, you'll have to replace the driver for it to work well.
Refer to [Fixed: Prolific PL2303TA USB to Serial Windows 11 Problem](https://embetronicx.com/uncategorized/fixed-prolific-pl2303ta-usb-to-serial-and-windows-11/). You'll have to re-install the drivers on Windows 11 after every reboot, which is annoying.
- Make sure you have a terminal application to communicate with the serial to USB device
  - [Putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html) has built in support on Windows, or you can download Teraterm (be careful of the source though)
  - [Teraterm](https://tera-term.en.lo4d.com/download) is my personal favorite, but the download sites seem a bit less trustworthy nowadays
  - One Linux there are quite a few applications available, e.g. screen, minicom, gtkterm
- Make sure you have the correct device assigned (COMx on Windows, /dev/ttyusbx on Linux) and the correct settings (115200N81, or 115200 baud, no stop bits, 8 data bits, 1 start bit)

<img src="images/usb-rs232.jpg" alt="USB-to-serial example" width="400"/>

#### Pinning for serial to USB {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_ATTACHING_A_SERIAL_CONSOLE_PINNING_FOR_SERIAL_TO_USB}

Please refer to the image below (the wiring color is common for most Prolific 2303 devices):

<img src="images/gpio-pinout-diagram-uart1.png" alt="GPIO pinout for UART1" width="400"/>

Do <u>__NOT__</u> connect the red wire (+3.3V). The converter is automatically powered through USB, and connecting this wire will try to power Raspberry Pi through the host PC's USB port, which is <u>__NOT__</u> a wise choice.

### Running from SD card {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_FROM_SD_CARD}

The SD card needs certain other files as well, so it's best to unzip the file `tutorial/01-demo/Netboot.zip`, copy the contents to the SD card, 
then copy `tutorial/01-demo/windows/kernel8.img` or `tutorial/01-demo/linux/kernel8.img` (the functionality is the same) over the existing one.
Please make sure to eject / sync your SD card first before removing it from the reader.

Put the SD card in your board, and attach a serial to USB device to the board. Then power on the board.

### Running using Netboot {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_USING_NETBOOT}


In `tutorial/01-demo` you will find a `Netboot.zip`, which is verified to work on both Raspberry Pi 3 and 4.
There is still one caveat, on Raspberry Pi 4, a reboot does not restart Netboot yet.
On Raspberry Pi 5 this is not tested yet.

_Correctly combining files for Raspberry Pi to boot is a challenge, especially if you don't know or understand how the system starts up.
So it's wise to simply take something that works, and play around with it._
More information can be found in [System startup](#SYSTEM_STARTUP) and online [here](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#raspberry-pi-boot-eeprom)

Unpack it onto the SD card (this must be formatted as FAT), make sure to eject (Windows) or sync (Linux) before taking the SD card out.
Place it into your board, and go to the website created by netboot, select an image and upload it. This will reboot the system with your image.

The SD card will be created by unzipping the file `tutorial/01-demo/Netboot.zip`, and copying the contents to the SD card.
Please make sure to eject / sync your SD card first before removing it from the reader.

Put the SD card in your board, and attach a serial to USB device to the board. Also make sure the board is attached to the network. Then power on the board.
Then find out the IP address of the board, and navigate using a browser to `http://<IP address>:8080/index.html`

<img src="images/netboot.png" alt="Netboot screen" width="500"/>

Click on `Choose File`, select the correct image (either version of kernel8.img), and click on `Boot now!`. The board will now restart and start your application.

### Running in QEMU {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU}

Using QEMU is quite straightforward. You will need to install QEMU however.

#### Installing QEMU {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_INSTALLING_QEMU}

##### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_INSTALLING_QEMU_WINDOWS}

There are specific Windows builds of QEMU provided by Stephan Weil, which can be downloaded [here](https://qemu.weilnetz.de/w64/2024/)
The latest version at the moment of writing this document is [9.0.50](https://qemu.weilnetz.de/w64/2024/qemu-w64-setup-20240720.exe).

Simply run the executable, which will install QEMU.

<img src="images/qemu-install-1.png" alt="QEMU installation 1" width="500"/>

Click OK

<img src="images/qemu-install-2.png" alt="QEMU installation 2" width="500"/>

Click Next

<img src="images/qemu-install-3.png" alt="QEMU installation 3" width="500"/>

Click I Agree

<img src="images/qemu-install-4.png" alt="QEMU installation 4" width="500"/>

Click Next

<img src="images/qemu-install-5.png" alt="QEMU installation 5" width="500"/>

Leave the installation path as is, click Install

<img src="images/qemu-install-6.png" alt="QEMU installation 6" width="500"/>

Click Finish

QEMU is now installed in `C:\Program Files\qemu`

```bat
"c:\Program Files\qemu\qemu-system-aarch64.exe" --version
```

```text
QEMU emulator version 9.0.50 (v9.0.0-14417-ga1d2d66911-dirty)
Copyright (c) 2003-2024 Fabrice Bellard and the QEMU Project developers
```

##### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_INSTALLING_QEMU_LINUX}

On Linux (Debian Bookworm), the 64 bit support is very limited, and only goes up to Raspberry Pi 3B.

To install QEMU:

```bash
sudo apt install qemu-system-arm
```

```text
[sudo] password for rene:
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following additional packages will be installed:
  cpu-checker msr-tools ovmf qemu-system-mips qemu-system-misc qemu-system-ppc qemu-system-s390x qemu-system-sparc qemu-system-x86 seabios
Suggested packages:
  samba vde2
The following NEW packages will be installed:
  cpu-checker msr-tools ovmf qemu-system qemu-system-mips qemu-system-misc qemu-system-ppc qemu-system-s390x qemu-system-sparc qemu-system-x86 seabios
0 upgraded, 11 newly installed, 0 to remove and 1 not upgraded.
Need to get 91,2 MB of archives.
After this operation, 366 MB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://nl.archive.ubuntu.com/ubuntu jammy/main amd64 msr-tools amd64 1.3-4 [10,3 kB]
Get:2 http://nl.archive.ubuntu.com/ubuntu jammy/main amd64 cpu-checker amd64 0.7-1.3build1 [6.800 B]
Get:3 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system-mips amd64 1:6.2+dfsg-2ubuntu6.21 [15,1 MB]
Get:4 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system-ppc amd64 1:6.2+dfsg-2ubuntu6.21 [8.418 kB]
Get:5 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system-sparc amd64 1:6.2+dfsg-2ubuntu6.21 [5.735 kB]
Get:6 http://nl.archive.ubuntu.com/ubuntu jammy/main amd64 seabios all 1.15.0-1 [174 kB]
Get:7 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system-x86 amd64 1:6.2+dfsg-2ubuntu6.21 [10,1 MB]
Get:8 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system-s390x amd64 1:6.2+dfsg-2ubuntu6.21 [2.869 kB]
Get:9 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system-misc amd64 1:6.2+dfsg-2ubuntu6.21 [41,2 MB]
Get:10 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 qemu-system amd64 1:6.2+dfsg-2ubuntu6.21 [12,7 kB]
Get:11 http://nl.archive.ubuntu.com/ubuntu jammy-updates/main amd64 ovmf all 2022.02-3ubuntu0.22.04.2 [7.561 kB]
Fetched 91,2 MB in 6s (14,3 MB/s)
Selecting previously unselected package msr-tools.
(Reading database ... 306240 files and directories currently installed.)
Preparing to unpack .../00-msr-tools_1.3-4_amd64.deb ...
Unpacking msr-tools (1.3-4) ...
Selecting previously unselected package cpu-checker.
Preparing to unpack .../01-cpu-checker_0.7-1.3build1_amd64.deb ...
Unpacking cpu-checker (0.7-1.3build1) ...
Selecting previously unselected package qemu-system-mips.
Preparing to unpack .../02-qemu-system-mips_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system-mips (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package qemu-system-ppc.
Preparing to unpack .../03-qemu-system-ppc_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system-ppc (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package qemu-system-sparc.
Preparing to unpack .../04-qemu-system-sparc_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system-sparc (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package seabios.
Preparing to unpack .../05-seabios_1.15.0-1_all.deb ...
Unpacking seabios (1.15.0-1) ...
Selecting previously unselected package qemu-system-x86.
Preparing to unpack .../06-qemu-system-x86_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system-x86 (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package qemu-system-s390x.
Preparing to unpack .../07-qemu-system-s390x_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system-s390x (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package qemu-system-misc.
Preparing to unpack .../08-qemu-system-misc_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system-misc (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package qemu-system.
Preparing to unpack .../09-qemu-system_1%3a6.2+dfsg-2ubuntu6.21_amd64.deb ...
Unpacking qemu-system (1:6.2+dfsg-2ubuntu6.21) ...
Selecting previously unselected package ovmf.
Preparing to unpack .../10-ovmf_2022.02-3ubuntu0.22.04.2_all.deb ...
Unpacking ovmf (2022.02-3ubuntu0.22.04.2) ...
Setting up qemu-system-mips (1:6.2+dfsg-2ubuntu6.21) ...
Setting up qemu-system-sparc (1:6.2+dfsg-2ubuntu6.21) ...
Setting up msr-tools (1.3-4) ...
Setting up qemu-system-ppc (1:6.2+dfsg-2ubuntu6.21) ...
Setting up qemu-system-misc (1:6.2+dfsg-2ubuntu6.21) ...
Setting up ovmf (2022.02-3ubuntu0.22.04.2) ...
Setting up seabios (1.15.0-1) ...
Setting up cpu-checker (0.7-1.3build1) ...
Setting up qemu-system-s390x (1:6.2+dfsg-2ubuntu6.21) ...
Setting up qemu-system-x86 (1:6.2+dfsg-2ubuntu6.21) ...
Setting up qemu-system (1:6.2+dfsg-2ubuntu6.21) ...
Processing triggers for man-db (2.10.2-1) ...
...
```

```bash
qemu-system-aarch64 --version
```

```text
QEMU emulator version 6.2.0 (Debian 1:6.2+dfsg-2ubuntu6.21)
Copyright (c) 2003-2021 Fabrice Bellard and the QEMU Project developers
```

#### Running the application in QEMU {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_RUNNING_THE_APPLICATION_IN_QEMU}

##### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_RUNNING_THE_APPLICATION_IN_QEMU_WINDOWS}

Copy the demo image (`tutorial/01-demo/windows/kernel8.img`) (see [Deployment mechanism](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM)) to your working location, and in a terminal type:

```bat
"C:\Program Files\qemu\qemu-system-aarch64.exe" -M raspi3b -kernel kernel8.img -serial stdio -s
```

For now, we have left out the `-S` option, which makes QEMU wait for a debugger to attach.

The kernel8.img file needs to be in the current location, otherwise point to the correct path. The output should be like:

```text
(qemu:20420): Gtk-WARNING **: 17:43:56.753: Could not load a pixbuf from icon theme.
This may indicate that pixbuf loaders or the mime database could not be found.
Info   00:00:00.000 Baremetal 1.0.0 started on Raspberry Pi 3 Model B (AArch64) (Logger:74)
Info   00:00:00.000 Raspberry Pi 3 Model B (64 bits) 1024 Mb RAM (Main:59)
Info   00:00:00.000 Serial:              0000000000000000 (Main:60)
Info   00:00:00.000 FW revision:         000548E1 (Main:61)
Info   00:00:00.010 Board MAC address:   52:54:00:12:34:57 (Main:65)
Info   00:00:00.010 ARM memory base:     00000000 (Main:66)
Info   00:00:00.010 ARM memory size:     3C000000 (Main:67)
Info   00:00:00.010 VC memory base:      3C000000 (Main:68)
Info   00:00:00.010 VC memory size:      04000000 (Main:69)
Info   00:00:00.020 SoC temperature:     25.000000 (Main:77)
Info   00:00:00.020 Done (Main:84)
Press r to reboot, h to halt
hHalting...
Debug  00:01:53.000 CPU core 0 halt (System:102)
Debug  00:01:53.000 HaltSystem (System:176)
```

##### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_RUNNING_THE_APPLICATION_IN_QEMU_LINUX}

Copy the demo image (`tutorial/01-demo/linux/kernel8.img`) (see [Deployment mechanism](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM)) to e.g. your home directory, and in a terminal type:

```bash
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -s
```

For now, we have left out the -S option, which makes QEMU wait for a debugger to attach.

The kernel8.img file needs to be in the current location, otherwise point to the correct path. The output should be like:

```
Info   00:00:00.000 Baremetal 1.0.0 started on Raspberry Pi 3 Model B (AArch64) (Logger:74)
Info   00:00:00.000 Raspberry Pi 3 Model B (64 bits) 1024 Mb RAM (Main:59)
Info   00:00:00.010 Serial:              0000000000000000 (Main:60)
Info   00:00:00.010 FW revision:         000548E1 (Main:61)
Info   00:00:00.010 Board MAC address:   52:54:00:12:34:57 (Main:65)
Info   00:00:00.020 ARM memory base:     00000000 (Main:66)
Info   00:00:00.020 ARM memory size:     3C000000 (Main:67)
Info   00:00:00.020 VC memory base:      3C000000 (Main:68)
Info   00:00:00.020 VC memory size:      04000000 (Main:69)
Info   00:00:00.030 SoC temperature:     25.000000 (Main:77)
Info   00:00:00.030 Done (Main:84)
Press r to reboot, h to halt
hHalting...
Debug  00:00:20.050 CPU core 0 halt (System:102)
Debug  00:00:20.050 HaltSystem (System:176)
```

#### Run output {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_IN_QEMU_RUN_OUTPUT}

When starting up with e.g. Putty attached to the correct COM device, this is what you'll see

<img src="images/demo-output-putty.png" alt="Console output for demo application" width="600"/>

__Output for demo application__

The data may be different depending on the board you have, the serial number will always be different, but you can see certain board information is shown, in color.

## Debugging {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING}

Debugging is possible in one of two different ways:

- Using QEMU
- Using a FTDI JTAG adapter

Using the JTAG adapter is quite tricky, and currently only supported on Linux. If I find the time, I will add a description later.

### Debugging in QEMU {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU}

#### Start application in QEMU {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU_START_APPLICATION_IN_QEMU}

##### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU_START_APPLICATION_IN_QEMU_WINDOWS}

Copy the demo image (`tutorial/01-demo/kernel8.img`) (see [Deployment mechanism](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM)) to your working location, and in a terminal type:

```bat
"C:\Program Files\qemu\qemu-system-aarch64.exe" -M raspi3b -kernel kernel8.img -serial stdio -s -S
```

Here, we add the `-S` option, which makes QEMU wait for a debugger to attach.

The kernel8.img file needs to be in the current location, otherwise point to the correct path. The output should be like:

<img src="images/qemu-wait-windows.png" alt="QEMU waiting for debugger" width="500"/>

##### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU_START_APPLICATION_IN_QEMU_LINUX}

Copy the demo image (`tutorial/01-demo/kernel8.img`) (see [Deployment mechanism](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM)) to e.g. your home directory, and in a terminal type:

```bash
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -s -S
```

Here, we add the `-S` option, which makes QEMU wait for a debugger to attach.

The kernel8.img file needs to be in the current location, otherwise point to the correct path. The output should be similar to Windows.

<img src="images/qemu-wait-linux.png" alt="QEMU waiting for debugger" width="500"/>

#### Starting the debugger {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU_STARTING_THE_DEBUGGER}

QEMU qill be waiting for the debugger to attach, with a screen similar to the one below.

##### Windows {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU_STARTING_THE_DEBUGGER_WINDOWS}

Start gdb:

```bat
D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gdb.exe --args demo.elf
```

```text
GNU gdb (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.90.20231008-git
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=i686-w64-mingw32 --target=aarch64-none-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://bugs.linaro.org/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from demo.elf...
```

Connect to QEMU and load the symbols (make sure the path to the symbol file is correct, use forward slashes for path delimiter):

```bat
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
0x0000000000000000 in ?? ()
(gdb) load
Loading section .init, size 0x148 lma 0x80000
Loading section .text, size 0x14728 lma 0x80800
Loading section .rodata, size 0x1dd7 lma 0x94f30
Loading section .init_array, size 0x20 lma 0x96d08
Loading section .data, size 0x294 lma 0x96d28
Start address 0x0000000000080000, load size 92411
Transfer rate: 2911 KB/sec, 1885 bytes/write.
```

Now you can use the normal GDB commands to debug the application:

```bat
(gdb) b main.cpp:55
Breakpoint 1 at 0x8080c: file ../code/applications/demo/src/main.cpp, line 55.
(gdb) l
41      #include <baremetal/MachineInfo.h>
42      #include <baremetal/Random.h>
43      #include <baremetal/Serialization.h>
44      #include <baremetal/Timer.h>
45      #include <baremetal/UART0.h>
46
47      using namespace baremetal;
48
49      static UART0 &uart = GetUART0();
50
(gdb) l
51      LOG_MODULE("Main");
52
53      int main()
54      {
55          uart.Initialize();
56
57          auto &machineInfo = GetMachineInfo();
58
59          LOG_INFO("%s (64 bits) %d Mb RAM", machineInfo.GetName(), machineInfo.GetRAMSize());
60          LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
(gdb) c
Continuing.

Thread 1 hit Breakpoint 1, main () at ../code/applications/demo/src/main.cpp:55
55          uart.Initialize();
(gdb) n
57          auto &machineInfo = GetMachineInfo();
(gdb) n
59          LOG_INFO("%s (64 bits) %d Mb RAM", machineInfo.GetName(), machineInfo.GetRAMSize());
(gdb) n
60          LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
(gdb) kill
Kill the program being debugged? (y or n) y
[Inferior 1 (process 1) killed]
(gdb) quit
```

##### Linux {#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEBUGGING_DEBUGGING_IN_QEMU_STARTING_THE_DEBUGGER_LINUX}

As the standard debugger from ARM does not work well due to a Python dependency, we need to install gdb-multiarch first:

```bash
sudo apt install gdb-multiarch
```

```text
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following NEW packages will be installed:
  gdb-multiarch
0 upgraded, 1 newly installed, 0 to remove and 1 not upgraded.
Need to get 4.591 kB of archives.
After this operation, 18,2 MB of additional disk space will be used.
Get:1 http://nl.archive.ubuntu.com/ubuntu jammy-updates/universe amd64 gdb-multiarch amd64 12.1-0ubuntu1~22.04.2 [4.591 kB]
Fetched 4.591 kB in 0s (13,4 MB/s)
Selecting previously unselected package gdb-multiarch.
(Reading database ... 306401 files and directories currently installed.)
Preparing to unpack .../gdb-multiarch_12.1-0ubuntu1~22.04.2_amd64.deb ...
Unpacking gdb-multiarch (12.1-0ubuntu1~22.04.2) ...
Setting up gdb-multiarch (12.1-0ubuntu1~22.04.2) ...
```

Start gdb:

```bash
gdb-multiarch --xargs demo.elf
```

```text
GNU gdb (Debian 13.1-3) 13.1
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from demo.elf...
```

Connect to QEMU and load the symbols (make sure the path to the symbol file is correct):

```bash
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
0x0000000000000000 in ?? ()
(gdb) load
Loading section .init, size 0x148 lma 0x80000
Loading section .text, size 0x14728 lma 0x80800
Loading section .rodata, size 0x1f47 lma 0x94f30
Loading section .init_array, size 0x20 lma 0x96e78
Loading section .data, size 0x294 lma 0x96e98
Start address 0x0000000000080000, load size 92779
Transfer rate: 6471 KB/sec, 1893 bytes/write.
```

Now you can use the normal GDB commands to debug the application:

```bash
(gdb) b main
Breakpoint 1 at 0x8080c: file /home/rene/repo/baremetal/code/applications/demo/src/main.cpp, line 55.
(gdb) l
41	#include <baremetal/MachineInfo.h>
42	#include <baremetal/Random.h>
43	#include <baremetal/Serialization.h>
44	#include <baremetal/Timer.h>
45	#include <baremetal/UART0.h>
46	
47	using namespace baremetal;
48	
49	static UART0 &uart = GetUART0();
50	
(gdb) l
51	LOG_MODULE("Main");
52	
53	int main()
54	{
55	    uart.Initialize();
56	
57	    auto &machineInfo = GetMachineInfo();
58	
59	    LOG_INFO("%s (64 bits) %d Mb RAM", machineInfo.GetName(), machineInfo.GetRAMSize());
60	    LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
(gdb) c
Continuing.

Thread 1 hit Breakpoint 1, main () at /home/rene/repo/baremetal/code/applications/demo/src/main.cpp:55
55	    uart.Initialize();
(gdb) n
57	    auto &machineInfo = GetMachineInfo();
(gdb) n
59	    LOG_INFO("%s (64 bits) %d Mb RAM", machineInfo.GetName(), machineInfo.GetRAMSize());
(gdb) n
60	    LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
(gdb) c
Continuing.
```

Press h in QEMU

``` text
[Inferior 1 (process 1) exited normally]
(gdb) quit
```

Output in QEMU will be as shown below

<img src="images/qemu-output-linux.png" alt="QEMU waiting for debugger" width="500"/>

Next: [02-setting-up-a-project](02-setting-up-a-project.md)

\todo Possibly add explanation on VSCode. It should be possible to add a kit to .vscode\cmake-kits.json:
[
    {
        "name": "Baremetal",
        "toolchainFile": "${workspacedir}/baremetal.toolchain",
        "isTrusted": true
    }
]
And and the correct definitions for CMake using .vscode/settings.json:
{
    "cmake.configureArgs": [
        "-DVERSION_NUMBER=1.0.0",
        "-DVERBOSE_BUILD=ON",
        "-DBAREMETAL_RPI_TARGET=3",
        "-DBAREMETAL_CONSOLE_UART0=ON"
    ],
    "cmake.buildDirectory": "${workspaceFolder}/cmake-build"
}
And then configure the debugger such that it injects the correct commands using .vscode/launch.json:
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/output/Debug/bin/demo.elf",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${fileDirname}",
            "environment": [],
            "externalConsole": false,
            "miDebuggerPath": "gdb-multiarch",
            "miDebuggerArgs": "${workspaceFolder}/output/Debug/bin/demo.elf",
            "MIMode": "gdb",
            "setupCommands": [
                { "text": "set architecture aarch64", "description": "ARM64 architecture", "ignoreFailures": false },
                { "text": "target remote localhost:1234", "description": "Connect to QEMU", "ignoreFailures": false },
                { "text": "load", "description": "Start executable", "ignoreFailures": false }                
            ],
            "customLaunchSetupCommands": []
        }
    ]
}
