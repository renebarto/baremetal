# Tutorial 13: Logger {#TUTORIAL_13_LOGGER}

@tableofcontents

## New tutorial setup {#TUTORIAL_13_LOGGER_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/12-logger`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_13_LOGGER_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-12.a`
- an application `output/Debug/bin/12-logger.elf`
- an image in `deploy/Debug/12-mailbox-image`

## Adding a logger {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER}

We can now write to either UART0 or UART1, and we have a common ancestor, `CharDevice`.
It would be nice if we could write to the console in a more generic way, using a timestamp, a log level, a variable number of arguments, and preferably in color if supported.
A log statement might look like this:

```cpp
LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
```

For this, we will need to be able to set a default logger device, by injecting an instance of a `CharDevice`, and then enable printing to that device, much like `printf()`.
Later on, we can add the screen or something else as a device for logging as well.
In order to enable writing variable argument lists, we will need to use strings, which grow automatically as needed.
This then means we need to be able to allocate heap memory.
Also, we will need a way to retrieve the current time.

So, all in all, quite some work to do.

### Console - Step 1 {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER_CONSOLE__STEP_1}

The first step we need to take is get hold of a console, depending on what is set as the default.
We will add a definition in the root CMake file to select whether this is is UART0 or UART1, and retrieve a console based on this.
The console will have some additional functionality, such as enabling the use of ANSI colors.



We will find

### Assertion - Step 1 {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER_ASSERTION__STEP_1}

We will find

### Heap management - Step 1 {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER_HEAP_MANAGEMENT__STEP_1}

### Printing a formatted string - Step 2 {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER_PRINTING_A_FORMATTED_STRING__STEP_2}

### Time retrieval - Step 2 {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER_TIME_RETRIEVAL__STEP_2}

### Logging - Step 3 {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER_LOGGING__STEP_3}

Next: [13-](13-.md)

