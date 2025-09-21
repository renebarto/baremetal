# TODO {#TODO}

@todo
- Overview of Raspberry Pi and baremetal development
- Overall project structure
- Find and add Raspberry Pi 5 information, extend samples for this platform as soon as possible (look at circle)
- Add description of how to use JTAG device with Raspberry Pi and possibly Odroid, using e.g. [Macoy Madson's page](https://macoy.me/blog/programming/RaspberryPi5Debugging)
- Explain setting up development environment on Windows for Windows build : "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
- Add git hook for commit

@todo Possibly add explanation on VSCode.
- It should be possible to add a kit to .vscode\cmake-kits.json:
```json
[
    {
        "name": "Baremetal",
        "toolchainFile": "${workspacedir}/baremetal.toolchain",
        "isTrusted": true
    }
]
```
- And and the correct definitions for CMake using .vscode/settings.json:
```json
{
    "cmake.configureArgs": [
        "-DVERSION_NUMBER=1.0.0",
        "-DVERBOSE_BUILD=ON",
        "-DBAREMETAL_RPI_TARGET=3",
        "-DBAREMETAL_CONSOLE_UART0=ON"
    ],
    "cmake.buildDirectory": "${workspaceFolder}/cmake-build"
}
```
- And then configure the debugger such that it injects the correct commands using .vscode/launch.json:
```json
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
```

@todo Explain which timers and mailboxes exist, and distinguish between them. So difference between ARM core time, ARM AP804 timer and BCM timer, same for ARM core mailboxes between cores and the BCM mailbox to the GPU.

@todo Start introducing spinlock

