# Tutorial 12: Logger {#TUTORIAL_12_LOGGER}

@tableofcontents

## New tutorial setup {#TUTORIAL_12_LOGGER_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/12-logger`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_12_LOGGER_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-10.a`
- an application `output/Debug/bin/10-mailbox.elf`
- an image in `deploy/Debug/10-mailbox-image`

## Adding a logger {#TUTORIAL_12_LOGGER_ADDING_A_LOGGER}

We can now write to either UART0 or UART1, and we have a common ancestor, `CharDevice`.
It would be nice if we could write to the console in a more generic way, using a timestamp, a log level, a variable number of arguments, and preferably in color if supported.
A log statement might look like this:

```cpp
LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
```

For this, we will need to be able to set a default looger device, by injecting an instance of a `CharDevice`, and then enable printing to that device, much like `printf()`.
In order to enable writing variable argument lists, we will need to use strings, which grow automatically as needed.
This then means we need to be able to allocate heap memory.

So, all in all, quite some work to do.

### Heap management - Step 1 {#TUTORIAL_12_LOGGER_ADDING_A_LOGGER_HEAP_MANAGEMENT__STEP_1}

### Printing a formatted string with variable arguments - Step 2 {#TUTORIAL_12_LOGGER_ADDING_A_LOGGER_PRINTING_A_FORMATTED_STRING_WITH_VARIABLE_ARGUMENTS__STEP_2}

### Logging - Step 3 {#TUTORIAL_12_LOGGER_ADDING_A_LOGGER_LOGGING__STEP_3}

Next: [13-](13-.md)

