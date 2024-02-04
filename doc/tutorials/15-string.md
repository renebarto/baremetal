# Tutorial 15: String {#TUTORIAL_15_STRING}

@tableofcontents

## New tutorial setup {#TUTORIAL_15_STRING_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/15-string`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_15_STRING_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-15.a`
- an application `output/Debug/bin/15-string.elf`
- an image in `deploy/Debug/15-string-image`

## Creating a string class {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS}

It would be nice if we could deal with strings in a more flexible way. For that, we need memory allocation, so we had to wait until now with that.

We will introduce a string class, and start using it for e.g. serialization, formatting, etc.

### String.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_STRINGH}

First we'll declare the string class.
Create the file `code/libraries/baremetal/include/baremetal/String.h`

```cpp
```

Next: [15-string](15-string.md)

