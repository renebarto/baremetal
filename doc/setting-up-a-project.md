# Setting up a project for building and debugging

Configuration for the project is largely similar for Windows and Linux, however the way we build and debug is slightly different.

For now, let's set up a project that simply prints something to the console (UART 1 in this case), and then halts the system.

This will quite an extensive chapter, so bare with me. As soon as we have configured the basic project, adding new code and applications should become easier.
Also, the way we configure the project is very verbose and direct, which we will improve later on.

First, we create a folder for the project. Let's say `D:\Projects\tutorial` on Windows and `~/tutorial` on Linux.

In this directory, we first need to create a CMake file, which is named `CMakeLists.txt`. Be careful about the 's' in Lists, and also make sure you have the correct casing, especially in Linux.
As soon as you add this file in Visual Studio, it may detect this is a CMake project and try to configure it. This will fail as we don't have the correct contents yet. Don't worry about this for now.

In this file we will first create a project:

```cmake
cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

project(demo
    DESCRIPTION "Demo application for tutorial"
    LANGUAGES CXX ASM)

```

We have now defined a first project, which requires CMake version 3.18 or higher, that contains code in C++ and / or assembly, with a simple description.

We now wish to add a source file to the project, so let's create a source file first, and simply call it `main.cpp`. The contents will be:

```cpp
int main()
{
    return 0;
}
```

For now the application does nothing. Notice that we have created a main function that returns and `int` and takes no parameters. As we are running a baremetal application, there is no way to specify parameters, except through the kernel parameters file.


### Windows

### Linux

