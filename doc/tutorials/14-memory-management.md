# Tutorial 14: Memory management {#TUTORIAL_14_MEMORY_MANAGEMENT}

@tableofcontents

## New tutorial setup {#TUTORIAL_14_MEMORY_MANAGEMENT_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/14-memory-management`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_14_MEMORY_MANAGEMENT_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-14.a`
- an application `output/Debug/bin/14-memory-management.elf`
- an image in `deploy/Debug/14-memory-management-image`

## Setting up memory management - step 1 {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1}

As we would like to start using classes that allocate and de-allocate memory, such as a string class. We need to set up for memory management.
This is going to be both a straightforward and a tricky task, we'll get arround to that.

At times it may seem that we're reimplementing functionality that you are already used to from the C and C++ standard libraries.
And of course this is partly true.
However, we are building code from the ground up, so you can expect nothing to be done for you,
but as we'll see later, after some time we can start using functionality that is provided by e.g. the standard C++ library,
as long as the basis is layed out.

We'll start by adding some definitions in both the memory map and the system configuration.
Then we'll add methods to the `MemoryManager` class for allocating and freeing memory.
This will depend on a `HeapAllocator` which performs the actual allocation.

### Main CMake file {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_MAIN_CMAKE_FILE}

First let's add some definitions for debugging memory allocation.
Update the file `CMakeLists.txt`

```cmake
File: CMakeLists.txt
...
63: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
64: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
65: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" ON)
66: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" ON)
...
90: if (BAREMETAL_TRACE_DEBUG)
91:     set(BAREMETAL_DEBUG_TRACING 1)
92: else ()
93:     set(BAREMETAL_DEBUG_TRACING 0)
94: endif()
95: if (BAREMETAL_TRACE_MEMORY)
96:     set(BAREMETAL_MEMORY_TRACING 1)
97: else ()
98:     set(BAREMETAL_MEMORY_TRACING 0)
99: endif()
100: if (BAREMETAL_TRACE_MEMORY_DETAIL)
101:     set(BAREMETAL_MEMORY_TRACING 1)
102:     set(BAREMETAL_MEMORY_TRACING_DETAIL 1)
103: else ()
104:     set(BAREMETAL_MEMORY_TRACING_DETAIL 0)
105: endif()
106: set(BAREMETAL_LOAD_ADDRESS 0x80000)
107: 
108: set(DEFINES_C
109:     PLATFORM_BAREMETAL
110:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
111:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
112:     BAREMETAL_DEBUG_TRACING=${BAREMETAL_DEBUG_TRACING}
113:     BAREMETAL_MEMORY_TRACING=${BAREMETAL_MEMORY_TRACING}
114:     BAREMETAL_MEMORY_TRACING_DETAIL=${BAREMETAL_MEMORY_TRACING_DETAIL}
115:     USE_PHYSICAL_COUNTER
116:     BAREMETAL_MAJOR=${VERSION_MAJOR}
117:     BAREMETAL_MINOR=${VERSION_MINOR}
118:     BAREMETAL_LEVEL=${VERSION_LEVEL}
119:     BAREMETAL_BUILD=${VERSION_BUILD}
120:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
121:     )
122: 
...
278: message(STATUS "Baremetal settings:")
279: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
280: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
281: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
282: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
283: message(STATUS "-- Debug ouput to UART0:            ${BAREMETAL_CONSOLE_UART0}")
284: message(STATUS "-- Debug ouput to UART1:            ${BAREMETAL_CONSOLE_UART1}")
285: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
286: message(STATUS "-- Debug tracing output:            ${BAREMETAL_TRACE_DEBUG}")
287: message(STATUS "-- Memory tracing output:           ${BAREMETAL_TRACE_MEMORY}")
288: message(STATUS "-- Detailed memory tracing output:  ${BAREMETAL_TRACE_MEMORY_DETAIL}")
289: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
290: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
291: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
292: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
293: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
...
```

- Line 65: We add a variable `BAREMETAL_TRACE_MEMORY` that enables tracing memory functions. We'll set it to `ON` for now
- Line 66: We add a variable `BAREMETAL_TRACE_MEMORY_DETAIL_` that enables tracing memory functions at more detail. We'll set it to `ON` for now
- Line 95-99: We set variable `BAREMETAL_MEMORY_TRACING` to 1 if `BAREMETAL_TRACE_MEMORY` is `ON`, and 0 otherwise
- Line 100-105: We set variable `BAREMETAL_MEMORY_TRACING_DETAIL` to 1 if `BAREMETAL_TRACE_MEMORY_DETAIL` is `ON`, and 0 otherwise.
If `BAREMETAL_TRACE_MEMORY_DETAIL` is `ON`, we also set `BAREMETAL_MEMORY_TRACING` to 1
- Line 113: We set the compiler definition `BAREMETAL_MEMORY_TRACING` to the value of the `BAREMETAL_MEMORY_TRACING` variable
- Line 114: We set the compiler definition `BAREMETAL_MEMORY_TRACING_DETAIL` to the value of the `BAREMETAL_MEMORY_TRACING_DETAIL` variable
- Line 287: We print the value of `BAREMETAL_TRACE_MEMORY`
- Line 288: We print the value of `BAREMETAL_TRACE_MEMORY_DETAIL_`

### Synchronization.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_SYNCHRONIZATIONH}

We'll add some definitions need for alignment. These have to do with the length of a cache line.
Create the file `code/libraries/baremetal/include/baremetal/Synchronization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Synchronization.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Synchronization.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Synchronization functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @brief Minimum cache line length (16 x 32 bit word) as specified in CTR_EL0 register, see @ref ARM_REGISTERS
43: #define DATA_CACHE_LINE_LENGTH_MIN  64
44: /// @brief Maximum cache line length (16 x 32 bit word) as specified in CTR_EL0 register, see @ref ARM_REGISTERS
45: #define DATA_CACHE_LINE_LENGTH_MAX  64
```

### SysConfig.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_SYSCONFIGH}

Well add some definitions for heap types and bucket size, which will be explained in a minute.
Update the file `code/libraries/baremetal/include/baremetal/SysConfig.h`

```cpp
File: code/libraries/baremetal/include/baremetal/SysConfig.h
...
57: #ifndef KERNEL_MAX_SIZE
58: #define KERNEL_MAX_SIZE (2 * MEGABYTE)
59: #endif
60: 
61: /// @brief HEAP_DEFAULT_NEW defines the default heap to be used for the "new"
62: /// operator, if a memory type is not explicitly specified. Possible
63: /// values are HEAP_LOW (memory below 1 GByte), HEAP_HIGH (memory above
64: /// 1 GByte) or HEAP_ANY (memory above 1 GB, if available, or memory
65: /// below 1 GB otherwise). This value can be set to HEAP_ANY for
66: /// a virtually unified heap, which uses the whole available memory
67: /// space. Because this may cause problems with some devices, which
68: /// explicitly need low memory for DMA, this value defaults to HEAP_LOW.
69: /// This setting is only of importance for the Raspberry Pi 4.
70: #ifndef HEAP_DEFAULT_NEW
71: #define HEAP_DEFAULT_NEW HeapType::LOW
72: #endif
73: 
74: /// @brief HEAP_DEFAULT_MALLOC defines the heap to be used for malloc() and
75: /// calloc() calls. See the description of HEAP_DEFAULT_NEW for details!
76: /// Modifying this setting is not recommended, because there are device
77: /// drivers, which require to allocate low memory for DMA purpose using
78: /// malloc(). This setting is only of importance for the Raspberry Pi 4.
79: #ifndef HEAP_DEFAULT_MALLOC
80: #define HEAP_DEFAULT_MALLOC HeapType::LOW
81: #endif
82: 
83: /// @brief HEAP_BLOCK_BUCKET_SIZES configures the heap allocator, which is the
84: /// base of dynamic memory management ("new" operator and malloc()). The
85: /// heap allocator manages free memory blocks in a number of free lists
86: /// (buckets). Each free list contains blocks of a specific size. On
87: /// block allocation the requested block size is rounded up to the
88: /// size of next available bucket size. If the requested size is greater
89: /// than the largest available bucket size, the block can be allocated,
90: /// but the memory space is lost, if the block will be freed later.
91: /// Because the block buckets have to be walked through on each allocate
92: /// and free operation, it is preferable to have only a few buckets.
93: /// With this option you can configure the bucket sizes, so that they
94: /// fit best for your application needs. You have to define a comma
95: /// separated list of increasing bucket sizes. All sizes must be a
96: /// multiple of 64. Up to 20 sizes can be defined.
97: #ifndef HEAP_BLOCK_BUCKET_SIZES
98: #define HEAP_BLOCK_BUCKET_SIZES 0x40, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x80000
99: #endif
100: 
101: /// @brief Set part to be used by GPU (normally set in config.txt)
102: #ifndef GPU_MEM_SIZE
103: #define GPU_MEM_SIZE (64 * MEGABYTE)
104: #endif
...
```

- Line 71: We create a definition `HEAP_DEFAULT_NEW` if not already defined. This specifies the heap to use for normal (i.e. non placement) calls to new() operators.
- Line 80: We create a definition `HEAP_DEFAULT_MALLOC` if not already defined. This specifies the heap to use for calls to the malloc() function.
- Line 98: We create a definition `HEAP_BLOCK_BUCKET_SIZES` if not already defined. This specifies the heap buckets sizes as a comma separated list

See below in [MemoryMap.h](#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_MEMORYMAPH) for explanation of the heap types.

### MemoryMap.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_MEMORYMAPH}

We also need to update the memory map. to point to the heap start and end
Update the file `code/libraries/baremetal/include/baremetal/MemoryMap.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryMap.h
90: /// @brief Region reserved for coherent memory rounded up to 1 Mb with 1 Mb extra space
91: #define MEM_COHERENT_REGION ((MEM_EXCEPTION_STACK_END + 2 * MEGABYTE) & ~(MEGABYTE - 1))
92: 
93: /// @brief Start of region reserved for heap
94: #define MEM_HEAP_START      (MEM_COHERENT_REGION + COHERENT_REGION_SIZE)
95: 
96: #if BAREMETAL_RPI_TARGET >= 4
97: /// @brief High memory region (above 1 Gb) start.
98: #define MEM_HIGHMEM_START             GIGABYTE
99: /// @brief High memory region (above 1 Gb) end. Memory above 3 GB is not safe to be used for DMA and is not used.
100: #define MEM_HIGHMEM_END               (3 * GIGABYTE - 1)
101: 
102: #endif
103: 
```

We will define an enum type in a second named `HeapType`, which specifies the heap to be used for a certain function. We distinguish two heap types:
- Low heap, which is the memory available for allocation below 1 Gb. As Raspberry Pi 3 only has 1 Gb of RAM, this is the only type for that board.
- High heap, which is the memory available for allocation above 1 Gb.
This will reach up to the end of RAM (excluding reserved addresses), to a maximum of 3Gb.
The reasong for this limit is that DMA can only address up to that address range.

The reason for the split between low and high heap is that the memory assign to the VideoCore for graphics is at the end of the 1Gb range.
So we will use the memory range from just above the coherent memory region up to 1Gb, minus the memory assign to VideoCore, minus the memory reserved for paging to the low heap
The high heap will range from 1Gb up to max 3Gb, as far as physical memory is available.

- Line 94: We define the start of the low heap, `MEM_HEAP_START` to be directly after the coherent memory region
- Line 98: For Raspberry Pi 4 or higher, we define the start of the high heap, `MEM_HIGHMEM_START`
- Line 100: For Raspberry Pi 4 or higher, we also define the maximum end of the high heap, `MEM_HIGHMEM_END`

See also the image below.

<img src="images/memory-map.png" alt="Memory map" width="800"/>

### HeapAllocator.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_HEAPALLOCATORH}

We will declare a class to handle allocation and freeing of memory for a specific heap (low or high).
Create the file `code/libraries/baremetal/include/baremetal/HeapAllocator.h`

```cpp
File: code/libraries/baremetal/include/baremetal/HeapAllocator.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : HeapAllocator.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : HeapAllocator
9: //
10: // Description : Heap allocation
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <baremetal/Macros.h>
43: #include <baremetal/Synchronization.h>
44: #include <baremetal/Types.h>
45: 
46: namespace baremetal {
47: 
48: /// @brief Block alignment
49: #define HEAP_BLOCK_ALIGN       DATA_CACHE_LINE_LENGTH_MAX
50: /// @brief Block alignment mask
51: #define HEAP_ALIGN_MASK        (HEAP_BLOCK_ALIGN - 1)
52: 
53: /// @brief Maximum number of heap buckets used
54: #define HEAP_BLOCK_MAX_BUCKETS 20
55: 
56: /// <summary>
57: /// Administration on an allocated block of memory
58: /// </summary>
59: struct HeapBlockHeader
60: {
61:     /// @brief Heap block magic number
62:     uint32 magic;
63:     /// @brief Heap block magic number
64: #define HEAP_BLOCK_MAGIC 0x424C4D43
65:     /// @brief Size of allocated block
66:     uint32           size;
67:     /// @brief Pointer to next header
68:     HeapBlockHeader *next;
69:     /// @brief Padding to align to HEAP_BLOCK_ALIGN bytes
70:     uint8            align[HEAP_BLOCK_ALIGN - 16];
71:     /// @brief Start of actual allocated block
72:     uint8            data[0];
73: } PACKED;
74: 
75: /// <summary>
76: /// Bucket containing administration on allocated blocks of memory
77: /// </summary>
78: struct HeapBlockBucket
79: {
80:     /// @brief Size of bucket (size actual memory allocated, excluding bucket header)
81:     uint32 size;
82: #if BAREMETAL_MEMORY_TRACING
83:     /// @brief Count of blocks allocated in bucket
84:     unsigned count;
85:     /// @brief Maximum count of blocks allocated in bucket over time
86:     unsigned maxCount;
87:     /// @brief Total number of blocks allocated in bucket over time
88:     uint64   totalAllocatedCount;
89:     /// @brief Total number of bytes allocated in bucket over time
90:     uint64   totalAllocated;
91:     /// @brief Total number of blocks freed in bucket over time
92:     uint64   totalFreedCount;
93:     /// @brief Total number of bytes freed in bucket over time
94:     uint64   totalFreed;
95: #endif
96:     /// @brief List of free blocks in bucket to be re-used
97:     HeapBlockHeader *freeList;
98: };
99: 
100: /// <summary>
101: /// Allocates blocks from a flat memory region
102: /// </summary>
103: class HeapAllocator
104: {
105: private:
106:     /// @brief Name of the heap
107:     const char*     m_heapName;
108:     /// @brief Next available address
109:     uint8*          m_next;
110:     /// @brief End of available address space
111:     uint8*          m_limit;
112:     /// @brief Reserved address space
113:     size_t          m_reserve;
114:     /// @brief Allocated bucket administration
115:     HeapBlockBucket m_buckets[HEAP_BLOCK_MAX_BUCKETS + 1];
116: 
117:     /// @brief Sizes of allocation buckets
118:     static uint32   s_bucketSizes[];
119: 
120: public:
121:     /// <summary>
122:     /// Constructs a heap allocator
123:     /// </summary>
124:     /// <param name="heapName">Name of the heap for debugging purpose (must be static)</param>
125:     explicit HeapAllocator(const char *heapName = "heap");
126: 
127:     void Setup(uintptr baseAddress, size_t size, size_t reserve);
128: 
129:     size_t GetFreeSpace() const;
130:     void *Allocate(size_t size);
131:     void *ReAllocate(void *block, size_t size);
132:     void Free(void *block);
133: 
134: #if BAREMETAL_MEMORY_TRACING
135:     void   DumpStatus();
136: 
137:     uint64 GetCurrentAllocatedBlockCount();
138:     uint64 GetCurrentAllocationSize();
139:     uint64 GetMaxAllocatedBlockCount();
140:     uint64 GetTotalAllocatedBlockCount();
141:     uint64 GetTotalFreedBlockCount();
142:     uint64 GetTotalAllocationSize();
143:     uint64 GetTotalFreeSize();
144: #endif
145: };
146: 
147: } // namespace baremetal
```

- Line 43: We include `Synchronization.h` for the definition of the cache line length
- Line 49: We define `HEAP_BLOCK_ALIGN` which signifies the alignment of allocated memory blocks
- Line 51: We define `HEAP_ALIGN_MASK` which is used to check whether a memory block is indeed aligned correctly
- Line 54: We define `HEAP_BLOCK_MAX_BUCKETS` which signifies the maximum number of buckets used for memory allocation.
Think of the buckets as sizes for allocated memory blocks.
We select the minimum buckets size which can hold the requested memory block, and allocate within that bucket.
- Line 59-73: We declare a structure to administer memory block information
  - Line 62: The member `magic` is used as a magic number to check against corruption of the memory block information
  - Line 64: We define the magic number `HEAP_BLOCK_MAGIC` to be used (the hex version of the string 'BLMC')
  - Line 66: The member `size` is the size of the allocated memory
  - Line 68: The member `next` is a pointer to the next allocated memory block in a linked list
  - Line 70: The member `align` makes sure that the data that follows is aligned to `HEAP_BLOCK_ALIGN` bytes.
The memory block administration therefore uses 64 bytes of memory
  - Line 72: The member `data` is a placeholder for the actual allocated memory
- Line 78-98: We declare a structure to administer bucket information
  - Line 81: The member `size` is the size of the bucket (the memory block excluding its administration needs to fit in a bucket).
The amount of memory used is therefore the bucket size plus the size of the memory block administration
  - Line 84: The member `count` is only defined when memory tracing is enabled. It holds the current number of allocated blocks in a bucket
  - Line 86: The member `maxCount` is only defined when memory tracing is enabled. It holds the maximum number of allocated blocks in a bucket at any time
  - Line 88: The member `totalAllocatedCount` is only defined when memory tracing is enabled. It holds the cumulative number of blocks allocated in a bucket over time
  - Line 88: The member `totalAllocated` is only defined when memory tracing is enabled. It holds the cumulative amount of memory allocated in a bucket over time
  - Line 88: The member `totalFreedCount` is only defined when memory tracing is enabled. It holds the cumulative number of blocks freed in a bucket over time
  - Line 88: The member `totalFreed` is only defined when memory tracing is enabled. It holds the cumulative amount of memory freed in a bucket over time
  - Line 97: The member `freeList` points to the first memory block administration of a list of blocks that were freed and not yet re-used
- Line 103-145: We declare the class `HeapAllocator`
  - Line 107: The member `m_heapName` holds the name of the heap for debugging purposes. This is passed to the constructor
  - Line 109: The member `m_next` holds the pointer to the next address that is still free for allocation
  - Line 111: The member `m_limit` holds the pointer to one byte beyond the last address available for the heap
  - Line 113: The member `m_reserve` holds an amount of memory reserved for other purposes (e.g. allocated a block that does not fit in any bucket)
  - Line 115: The member `m_buckets` holds the pointers to the first memory block information for each bucket
  - Line 118: The static member `s_bucketSizes` holds the size of each bucket. Any values equal to 0 are unused buckets
  - Line 125: We declare the constructor, which received the heap name
  - Line 127: We declare the `Setup()` method, which is used to set the start address, size, and reserved space for the heap
  - Line 129: We declare the `GetFreeSpace()` method, which returns the amount of unused memory space in the heap (excluding returned an unused previously allocated blocks)
  - Line 130: We declare the `Allocate()` method, which allocates a block of memory, and returns the pointer to the memory block
  - Line 131: We declare the `Rellocate()` method, which reallocates a block of memory (allocates a block of the newly requested size, copies the contents, and frees the original), and returns the pointer to the new memory block
  - Line 132: We declare the `Free()` method, which frees a block of memory
  - Line 135: The method `DumpStatus()` is only defined when memory tracing is enabled.
It logs information on the currently allocated and free memory blocks
  - Line 137: The method `GetCurrentAllocatedBlockCount()` is only defined when memory tracing is enabled.
It returns the total count of currently allocated memory blocks
  - Line 138: The method `GetCurrentAllocationSize()` is only defined when memory tracing is enabled.
It returns the total size of allocated memory blocks
  - Line 139: The method `GetMaxAllocatedBlockCount()` is only defined when memory tracing is enabled
It returns the maximum count of allocated memory blocks over time
  - Line 140: The method `GetTotalAllocatedBlockCount()` is only defined when memory tracing is enabled.
It returns the cumulative count of allocated memory blocks over time
  - Line 141: The method `GetTotalFreedBlockCount()` is only defined when memory tracing is enabled.
It returns the cumulative size of allocated memory blocks over time
  - Line 142: The method `GetTotalAllocationSize()` is only defined when memory tracing is enabled
It returns the cumulative count of freed memory blocks over time
  - Line 143: The method `GetTotalFreeSize()` is only defined when memory tracing is enabled.
It returns the cumulative size of freed memory blocks over time

### HeapAllocator.cpp {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_HEAPALLOCATORCPP}

We implement the methods of the `HeapAllocator` class.
Create the file `code/libraries/baremetal/src/HeapAllocator.cpp`

```cpp
File: code/libraries/baremetal/src/HeapAllocator.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : HeapAllocator.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : HeapAllocator
9: //
10: // Description : Heap allocation
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include <baremetal/HeapAllocator.h>
41: 
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: #include <baremetal/Util.h>
45: #include <baremetal/SysConfig.h>
46: 
47: using namespace baremetal;
48: 
49: /// @brief Define log name
50: LOG_MODULE("HeapAllocator");
51: 
52: uint32 HeapAllocator::s_bucketSizes[] = { HEAP_BLOCK_BUCKET_SIZES };
53: 
54: HeapAllocator::HeapAllocator(const char* heapName)
55:     : m_heapName{ heapName }
56:     , m_next{}
57:     , m_limit{}
58:     , m_reserve{}
59:     , m_buckets{}
60: {
61:     memset(m_buckets, 0, sizeof(m_buckets));
62: 
63:     size_t numBuckets = sizeof(s_bucketSizes) / sizeof(s_bucketSizes[0]);
64:     if (numBuckets > HEAP_BLOCK_MAX_BUCKETS)
65:     {
66:         numBuckets = HEAP_BLOCK_MAX_BUCKETS;
67:     }
68: 
69:     for (size_t i = 0; i < numBuckets; ++i)
70:     {
71:         m_buckets[i].size = s_bucketSizes[i];
72:     }
73: }
74: 
75: /// <summary>
76: /// Sets up the heap allocator
77: /// </summary>
78: /// <param name="baseAddress">Base address of memory region (must be 16 bytes aligned)</param>
79: /// <param name="size">Size of memory region</param>
80: /// <param name="reserve">Free space reserved for handling of "Out of memory" messages.
81: /// (Allocate() returns nullptr, if reserve is 0 and memory region is full)</param>
82: void HeapAllocator::Setup(uintptr baseAddress, size_t size, size_t reserve)
83: {
84:     m_next = reinterpret_cast<uint8*>(baseAddress);
85:     m_limit = reinterpret_cast<uint8*>(baseAddress + size);
86:     m_reserve = reserve;
87: #if BAREMETAL_MEMORY_TRACING
88:     DumpStatus();
89: #endif
90: }
91: 
92: /// <summary>
93: /// Calculate and return the amount of free (unallocated) space in all buckets
94: /// @note Unused blocks on a free list do not count here.
95: /// </summary>
96: /// <returns>Free space of the memory region, which is not allocated by blocks.</returns>
97: size_t HeapAllocator::GetFreeSpace() const
98: {
99:     return m_limit - m_next;
100: }
101: 
102: /// <summary>
103: /// Allocate a block of memory
104: /// \note Resulting block is always 16 bytes aligned
105: /// \note If reserve in Setup() is non-zero, the system panics if heap is full.
106: /// </summary>
107: /// <param name="size">Block size to be allocated</param>
108: /// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
109: void* HeapAllocator::Allocate(size_t size)
110: {
111:     if (m_next == nullptr)
112:     {
113:         return nullptr;
114:     }
115: 
116:     HeapBlockBucket* bucket;
117:     for (bucket = m_buckets; bucket->size > 0; bucket++)
118:     {
119:         if (size <= bucket->size)
120:         {
121:             size = bucket->size;
122: 
123: #if BAREMETAL_MEMORY_TRACING
124:             if (++bucket->count > bucket->maxCount)
125:             {
126:                 bucket->maxCount = bucket->count;
127:             }
128:             ++bucket->totalAllocatedCount;
129:             bucket->totalAllocated += size;
130: 
131: #endif
132:             break;
133:         }
134:     }
135: 
136:     HeapBlockHeader* blockHeader{ bucket->freeList };
137:     if ((bucket->size > 0) && (blockHeader != nullptr))
138:     {
139:         assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
140:         bucket->freeList = blockHeader->next;
141: #if BAREMETAL_MEMORY_TRACING_DETAIL
142:         LOG_DEBUG("Reuse %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
143:         LOG_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
144: #endif
145:     }
146:     else
147:     {
148:         blockHeader = reinterpret_cast<HeapBlockHeader*>(m_next);
149: 
150:         uint8* nextBlock = m_next;
151:         nextBlock += (sizeof(HeapBlockHeader) + size + HEAP_BLOCK_ALIGN - 1) & ~HEAP_ALIGN_MASK;
152: 
153:         if ((nextBlock <= m_next) ||                    // may have wrapped
154:             (nextBlock > m_limit - m_reserve))
155:         {
File: d:\Projects\baremetal.github\code\libraries\baremetal\src\HeapAllocator.cpp
156: #if BAREMETAL_MEMORY_TRACING
157:             DumpStatus();
158: #endif
159:             LOG_ERROR("%s: Out of memory", m_heapName);
160:             return nullptr;
161:         }
162: 
163:         m_next = nextBlock;
164: 
165:         blockHeader->magic = HEAP_BLOCK_MAGIC;
166:         blockHeader->size = static_cast<uint32>(size);
167: 
168: #if BAREMETAL_MEMORY_TRACING_DETAIL
169:         LOG_DEBUG("Allocate %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
170:         LOG_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
171: #endif
172:     }
173: 
174:     blockHeader->next = nullptr;
175: 
176:     void* result = blockHeader->data;
177:     assert((reinterpret_cast<uintptr>(result) & HEAP_ALIGN_MASK) == 0);
178: 
179:     return result;
180: }
181: 
182: /// <summary>
183: /// Reallocate block of memory
184: /// </summary>
185: /// <param name="block">Block of memory to be reallocated to the new size</param>
186: /// <param name="size">Block size to be allocated</param>
187: /// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
188: void* HeapAllocator::ReAllocate(void* block, size_t size)
189: {
190:     if (block == nullptr)
191:     {
192:         return Allocate(size);
193:     }
194: 
195:     if (size == 0)
196:     {
197:         Free(block);
198: 
199:         return nullptr;
200:     }
201: 
202:     const HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
203:     assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
204:     if (blockHeader->size >= size)
205:     {
206:         return block;
207:     }
208: 
209:     void* newBlock = Allocate(size);
210:     if (newBlock == nullptr)
211:     {
212:         return nullptr;
213:     }
214: 
215:     memcpy(newBlock, block, blockHeader->size);
216: 
217:     Free(block);
218: 
219:     return newBlock;
220: }
221: 
222: /// <summary>
223: /// Free (de-allocate) block of memory.
224: /// \note Memory space of blocks, which are bigger than the largest bucket size, cannot be returned to a free list and is lost.
225: /// </summary>
226: /// <param name="block">Memory block to be freed</param>
227: void HeapAllocator::Free(void* block)
228: {
229:     if (block == nullptr)
230:     {
231:         return;
232:     }
233: 
234:     HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
235:     assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
236: 
237:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; bucket++)
238:     {
239:         if (blockHeader->size == bucket->size)
240:         {
241:             blockHeader->next = bucket->freeList;
242:             bucket->freeList = blockHeader;
243: 
244: #if BAREMETAL_MEMORY_TRACING
245:             bucket->count--;
246:             ++bucket->totalFreedCount;
247:             bucket->totalFreed += blockHeader->size;
248: #if BAREMETAL_MEMORY_TRACING_DETAIL
249:             LOG_DEBUG("Free %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
250:             LOG_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
251: #endif
252: #endif
253: 
254:             return;
255:         }
256:     }
257: 
258: #if BAREMETAL_MEMORY_TRACING
259:     LOG_WARNING("%s: Trying to free large block (size %lu)", m_heapName, blockHeader->size);
260: #endif
261: }
262: 
263: #if BAREMETAL_MEMORY_TRACING
264: /// <summary>
265: /// Display the current status of the heap allocator
266: /// </summary>
267: void HeapAllocator::DumpStatus()
268: {
269:     LOG_DEBUG("Heap allocator info:     %s", m_heapName);
270:     LOG_DEBUG("Current #allocations:    %llu", GetCurrentAllocatedBlockCount());
271:     LOG_DEBUG("Max #allocations:        %llu", GetMaxAllocatedBlockCount());
272:     LOG_DEBUG("Current #allocated bytes:%llu", GetCurrentAllocationSize());
273:     LOG_DEBUG("Total #allocated blocks: %llu", GetTotalAllocatedBlockCount());
274:     LOG_DEBUG("Total #allocated bytes:  %llu", GetTotalAllocationSize());
275:     LOG_DEBUG("Total #freed blocks:     %llu", GetTotalFreedBlockCount());
276:     LOG_DEBUG("Total #freed bytes:      %llu", GetTotalFreeSize());
277: 
278:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
279:     {
280:         LOG_DEBUG("malloc(%lu): %lu blocks (max %lu) total alloc #blocks = %llu, #bytes = %llu, total free #blocks = %llu, #bytes = %llu",
281:             bucket->size, bucket->count, bucket->maxCount, bucket->totalAllocatedCount, bucket->totalAllocated, bucket->totalFreedCount, bucket->totalFreed);
282:     }
283: }
284: 
285: /// <summary>
286: /// Returns the number of currently allocated memory blocks for this heap allocator.
287: /// </summary>
288: /// <returns>Number of currently allocated memory blocks for this heap allocator</returns>
289: uint64 HeapAllocator::GetCurrentAllocatedBlockCount()
290: {
291:     uint64 total{};
292:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
293:     {
294:         total += bucket->count;
295:     }
296:     return total;
297: }
298: 
299: /// <summary>
300: /// Returns the total size of currently allocated memory blocks for this heap allocator.
301: /// </summary>
302: /// <returns>Total size of currently allocated memory blocks for this heap allocator</returns>
303: uint64 HeapAllocator::GetCurrentAllocationSize()
304: {
305:     uint64 total{};
306:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
307:     {
308:         total += bucket->count * bucket->size;
309:     }
310:     return total;
311: }
312: 
313: /// <summary>
314: /// Returns the maximum number of currently allocated memory blocks for this heap allocator over time.
315: /// </summary>
316: /// <returns>Maximum number of currently allocated memory blocks for this heap allocator over time</returns>
317: uint64 HeapAllocator::GetMaxAllocatedBlockCount()
318: {
319:     uint64 total{};
320:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
321:     {
322:         total += bucket->maxCount;
323:     }
324:     return total;
325: }
326: 
327: /// <summary>
328: /// Returns the total number of allocated memory blocks for this heap allocator over time.
329: /// </summary>
330: /// <returns>Total number of allocated memory blocks for this heap allocator over time</returns>
331: uint64 HeapAllocator::GetTotalAllocatedBlockCount()
332: {
333:     uint64 total{};
334:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
335:     {
336:         total += bucket->totalAllocatedCount;
337:     }
338:     return total;
339: }
340: 
341: /// <summary>
342: /// Returns the total number of freed memory blocks for this heap allocator over time.
343: /// </summary>
344: /// <returns>Total number of freed memory blocks for this heap allocator over time</returns>
345: uint64 HeapAllocator::GetTotalFreedBlockCount()
346: {
347:     uint64 total{};
348:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
349:     {
350:         total += bucket->totalFreedCount;
351:     }
352:     return total;
353: }
354: 
355: /// <summary>
356: /// Returns the total size of allocated memory blocks for this heap allocator over time.
357: /// </summary>
358: /// <returns>Total size of allocated memory blocks for this heap allocator over time</returns>
359: uint64 HeapAllocator::GetTotalAllocationSize()
360: {
361:     uint64 total{};
362:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
363:     {
364:         total += bucket->totalAllocated;
365:     }
366:     return total;
367: }
368: 
369: /// <summary>
370: /// Returns the total size of freed memory blocks for this heap allocator over time.
371: /// </summary>
372: /// <returns>Total size of freed memory blocks for this heap allocator over time</returns>
373: uint64 HeapAllocator::GetTotalFreeSize()
374: {
375:     uint64 total{};
376:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
377:     {
378:         total += bucket->totalFreed;
379:     }
380:     return total;
381: }
382: 
383: #endif
```

- Line 52: We define the static member `s_bucketSizes` with the initializer `HEAP_BLOCK_BUCKET_SIZES` defined in `SysConfig.h` (unless overridden).
The default bucket sizes are:
  - 64 bytes (128 including memory block administration)
  - 1024 bytes (1088 including memory block administration)
  - 4096 bytes (4160 including memory block administration)
  - 16384 bytes (16448 including memory block administration)
  - 65536 bytes (65600 including memory block administration)
  - 262144 bytes (262208 including memory block administration)
  - 524288 bytes (524352 including memory block administration)
- Line 54-73: We implement the `HeapAllocator` constructor
  - Line 61: We clear the bucket administration array
  - Line 64-72: We set the sizes of buckets from the static member `s_bucketSizes`, making sure we stay within the maximum bucket count
- Line 82-90: We implement the method `Setup()`
  - Line 84: We set the pointer to the next available space to the start of the region
  - Line 85: We set the pointer to the end of available space to the end of the region
  - Line 86: We save the reserved space
  - Line 88: Only if `BAREMETAL_MEMORY_TRACING` is defined: We dump the current memory management status to the log
- Line 97-100: We implement the method `GetFreeSpace()` which returns the available space left
- Line 109-187: We implement the method `Allocate()`
  - Line 111-114: We add a sanity check to verify that the memory manager is set up
  - Line 116-134: We find the smallest bucket that supports the requested size
  - Line 136-145: We check whether there is a block within the selected bucket that was freed before, so we can re-use it
  - Line 147-179: If no block can be re-used, we allocate a new one
    - Line 148: We use the first available address in free space
    - Line 150-151: We calculate the next available address in free space (we add the size of the administration block and the requested size rounded up to the heap alignment)
    - Line 153-161: If the next available address would no longer fit in the available space, we fail by returning nullptr, after logging an error
    - Line 163: We update the first available address in free space
    - Line 165-166: We fill the memory block administration
  - Line 174: We set the pointer to the next memory block
  - Line 176-177: We get the pointer to the actual memory block, and check for correct alignment
- Line 188-220: We implement the method `ReAllocate()`
  - Line 190-193: If the memory block is actually a nullptr, we simply allocate
  - Line 195-200: If the new size is 0, we free the block and return nullptr
  - Line 202-203: We get hold of the memory block administration and check for a valid magic number
  - Line 204-207: If the new size still fits in the current bucket, we simply return the block
  - Line 209-213: We allocate a new block of the new size
  - Line 215: We copy the contents of the current block into the new one (not that this only happens when we grow the block)
  - Line 217: We free the old memory block
- Line 227-261: We implement the method `Free()`
  - Line 229-231: If the memory block is nullptr, we simply return
  - Line 234-235: We get hold of the memory block administration and check for a valid magic number
  - Line 237-256: We look up the bucket this block block belongs to and add it to the list of free memory blocks for that bucket
  - Line 258-261: Note that when a block was allocated that does not fit in any bucket, that memory is lost
- Line 289-297: We implement the method `GetCurrentAllocatedBlockCount()`
- Line 303-311: We implement the method `GetCurrentAllocationSize()`
- Line 317-325: We implement the method `GetMaxAllocatedBlockCount()`
- Line 331-339: We implement the method `GetTotalAllocatedBlockCount()`
- Line 345-352: We implement the method `GetTotalFreedBlockCount()`
- Line 359-367: We implement the method `GetTotalAllocationSize()`
- Line 373-381: We implement the method `GetTotalFreeSize()`

### MemoryManager.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_MEMORYMANAGERH}

We will extend the `MemoryManager` class with methods to allocate and free memory, retrieve the amount of free heap left, and dump information on allocated and freed memory.
This also requires making the `MemoryManager` class instantiable. We again choose to make `MemoryManager` a singleton.
The `sysinit()` function will create the `MemoryManager` instance, as it will be needed very early in the startup process.
Update the file `code/libraries/baremetal/include/baremetal/MemoryManager.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryManager.h
...
42: #include <baremetal/Types.h>
43: #include <baremetal/HeapAllocator.h>
44: 
...
57: /// <summary>
58: /// Type of heap for requested memory block
59: /// </summary>
60: enum class HeapType
61: {
62:     /// @brief Memory below 1 GB
63:     LOW = 0,
64:     /// @brief Memory above 1 GB
65:     HIGH = 1,
66:     /// @brief High memory (if available) or low memory (otherwise)
67:     ANY = 2,
68:     /// @brief 30-bit DMA-able memory
69:     DMA30 = LOW,
70: };
71: 
72: namespace baremetal {
73: 
74: /// <summary>
75: /// Handles memory allocation, re-allocation, and de-allocation for heap and paging memory, as well as assignment of coherent memory slots.
76: ///
77: /// This is a singleton, in that it is not possible to create a default instance (GetMemoryManager() needs to be used for this).
78: /// </summary>
79: class MemoryManager
80: {
81:     /// <summary>
82:     /// Construct the singleton MemoryManager instance if needed, and return a reference to the instance. This is a friend function of class MemoryManager
83:     /// </summary>
84:     /// <returns>Reference to the singleton MemoryManager instance</returns>
85:     friend MemoryManager& GetMemoryManager();
86: 
87: private:
88:     /// @brief Total memory size below 1Gb
89:     size_t                m_memSize;
90:     /// @brief Total memory size above 1Gb (up to 3 Gb boundary)
91:     size_t                m_memSizeHigh;
92: 
93:     /// @brief Heap allocator for low memory (below 1Gb)
94:     HeapAllocator         m_heapLow;
95: #if BAREMETAL_RPI_TARGET >= 4
96:     /// @brief Heap allocator for low memory (above 1Gb)
97:     HeapAllocator         m_heapHigh;
98: #endif
99:     MemoryManager();
100: 
101: public:
102: 
103:     static uintptr GetCoherentPage(CoherentPageSlot slot);
104: 
105:     static void* HeapAllocate(size_t size, HeapType type);
106:     static void* HeapReAllocate(void* block, size_t size);
107:     static void HeapFree(void* block);
108:     static size_t GetHeapFreeSpace(HeapType type);
109:     static void DumpStatus();
110: };
111: 
112: MemoryManager& GetMemoryManager();
113: 
114: } // namespace baremetal
```

- Line 43: We include `HeapAllocator.h`
- Line 60-70: We declare the enum `HeapType` which signifies the type of heap to be used for allocation
- Line 85: We declare the friend function `GetMemoryManager()` which instantiates and returns the singleton MemoryManager instance (which will be create in the `sysinit()` function in `System.cpp`)
- Line 89: The member `m_memSize` signifies the amount of memory available for the ARM CPU up to 1 Gb in bytes.
This does not take into account the reservations for paging, and the part used for other means before the heap.
- Line 91: The member `m_memSizeHigh` signifies the amount of mmemory above 1 Gb, up the maximum of 3 Gb
- Line 94: The member `m_heapLow` is the low heap
- Line 97: The member `m_heapHigh` is only defined for Raspberry Pi 4 and higher, is the high heap
- Line 99: We declare the private constructor for `MemoryManager`. This can only be called by `GetMemoryManager()`
- Line 105: We declare the method `HeapAllocate()`, which allocates memory of the requested size, in the requested heap
- Line 106: We declare the method `HeapReAllocate()`, which re-allocates a memory block to the requested size
- Line 107: We declare the method `HeapFree()`, which frees a memory block
- Line 108: We declare the method `GetHeapFreeSpace()`, which returned available memory in all heaps
- Line 109: We declare the method `DumpStatus()`, which log information on all heaps concerning allocated and freed memory blocks
- Line 112: We declare the method `GetMemoryManager()`, which initiates the singleton `MemoryManager`

### MemoryManager.cpp {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_MEMORYMANAGERCPP}

We will implement the added methods for `MemoryManager`.
Update the file `code/libraries/baremetal/src/MemoryManager.cpp`

```cpp
File: code/libraries/baremetal/src/MemoryManager.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : MemoryManager.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryManager
9: //
10: // Description : Memory handling
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include <baremetal/MemoryManager.h>
41: 
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: #include <baremetal/MachineInfo.h>
45: #include <baremetal/SysConfig.h>
46: 
47: /// @file
48: /// Memory management implementation
49: 
50: using namespace baremetal;
51: 
52: /// @brief Define log name
53: LOG_MODULE("MemoryManager");
54: 
55: /// <summary>
56: /// Constructs a MemoryManager instance
57: /// 
58: /// Retrieves amount of physical RAM available, and sets up heap managers for low (below 1Gb) and high (above 3 Gb, only Raspberry Pi 4 or higher) memory.
59: /// </summary>
60: MemoryManager::MemoryManager()
61:     : m_memSize{}
62:     , m_memSizeHigh{}
63:     , m_heapLow{ "heaplow" }
64: #if BAREMETAL_RPI_TARGET >= 4
65:     , m_heapHigh{ "heaphigh" }
66: #endif
67: {
68:     MachineInfo& machineInfo = GetMachineInfo();
69:     machineInfo.Initialize();
70:     auto baseAddress = machineInfo.GetARMMemoryBaseAddress();
71:     auto size = machineInfo.GetARMMemorySize();
72:     if (size == 0)
73:     {
74:         baseAddress = 0;
75:         size = ARM_MEM_SIZE;
76:     }
77: 
78:     assert(baseAddress == 0);
79:     m_memSize = size;
80: 
81:     size_t blockReserve = m_memSize - MEM_HEAP_START - PAGE_RESERVE;
82:     m_heapLow.Setup(MEM_HEAP_START, blockReserve, 0x40000);
83: 
84: #if BAREMETAL_RPI_TARGET >= 4
85:     auto ramSize = machineInfo.GetRAMSize();
86:     if (ramSize > 1024)
87:     {
88:         uint64 highSize = (ramSize - 1024) * MEGABYTE;
89:         if (highSize > MEM_HIGHMEM_END + 1 - MEM_HIGHMEM_START)
90:         {
91:             highSize = MEM_HIGHMEM_END + 1 - MEM_HIGHMEM_START;
92:         }
93: 
94:         m_memSizeHigh = static_cast<size_t>(highSize);
95: 
96:         m_heapHigh.Setup(MEM_HIGHMEM_START, m_memSizeHigh, 0);
97:     }
98: #endif
99: }
100: 
101: /// <summary>
102: /// Return the coherent memory page (allocated with the GPU) for the requested page slot
103: /// </summary>
104: /// <param name="slot">Page slot to return the address for</param>
105: /// <returns>Page slot coherent memory address</returns>
106: uintptr MemoryManager::GetCoherentPage(CoherentPageSlot slot)
107: {
108:     uint64 pageAddress = MEM_COHERENT_REGION;
109: 
110:     pageAddress += static_cast<uint32>(slot) * PAGE_SIZE;
111: 
112:     return pageAddress;
113: }
114: 
115: /// <summary>
116: /// Allocate memory from the specified heap
117: /// </summary>
118: /// <param name="size">Size of block to allocate</param>
119: /// <param name="type">Heap type to allocate from</param>
120: /// <returns>Pointer to allocated block of memory, or nullptr on failure</returns>
121: void* MemoryManager::HeapAllocate(size_t size, HeapType type)
122: {
123:     auto& memoryManager = GetMemoryManager();
124: #if BAREMETAL_RPI_TARGET >= 4
125:     void* block;
126: 
127:     switch (type)
128:     {
129:     case HeapType::LOW:	    return memoryManager.m_heapLow.Allocate(size);
130:     case HeapType::HIGH:    return memoryManager.m_heapHigh.Allocate(size);
131:     case HeapType::ANY:     return (block = memoryManager.m_heapHigh.Allocate(size)) != nullptr
132:         ? block
133:         : memoryManager.m_heapLow.Allocate(size);
134:     default:                return nullptr;
135:     }
136: #else
137:     switch (type)
138:     {
139:     case HeapType::LOW:
140:     case HeapType::ANY:     return memoryManager.m_heapLow.Allocate(size);
141:     default:                return nullptr;
142:     }
143: #endif
144: }
145: 
146: /// <summary>
147: /// Reallocate block of memory
148: /// </summary>
149: /// <param name="block">Block of memory to be reallocated to the new size</param>
150: /// <param name="size">Block size to be allocated</param>
151: /// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
152: void* MemoryManager::HeapReAllocate(void* block, size_t size)	// block may be nullptr
153: {
154:     auto& memoryManager = GetMemoryManager();
155: #if BAREMETAL_RPI_TARGET >= 4
156:     if (reinterpret_cast<uintptr>(block) < MEM_HIGHMEM_START)
157:     {
158:         return memoryManager.m_heapLow.ReAllocate(block, size);
159:     }
160:     else
161:     {
162:         return memoryManager.m_heapHigh.ReAllocate(block, size);
163:     }
164: #else
165:     return memoryManager.m_heapLow.ReAllocate(block, size);
166: #endif
167: }
168: 
169: /// <summary>
170: /// Free (de-allocate) block of memory.
171: /// </summary>
172: /// <param name="block">Memory block to be freed</param>
173: void MemoryManager::HeapFree(void* block)
174: {
175:     auto& memoryManager = GetMemoryManager();
176: #if BAREMETAL_RPI_TARGET >= 4
177:     if (reinterpret_cast<uintptr>(block) < MEM_HIGHMEM_START)
178:     {
179:         memoryManager.m_heapLow.Free(block);
180:     }
181:     else
182:     {
183:         memoryManager.m_heapHigh.Free(block);
184:     }
185: #else
186:     memoryManager.m_heapLow.Free(block);
187: #endif
188: }
189: 
190: /// <summary>
191: /// Calculate and return the amount of free (unallocated) space for the specified heap
192: /// </summary>
193: /// <param name="type">Heap to return free space for</param>
194: /// <returns>Free space of the memory region, which is not allocated by blocks.</returns>
195: size_t MemoryManager::GetHeapFreeSpace(HeapType type)
196: {
197:     auto& memoryManager = GetMemoryManager();
198: #if BAREMETAL_RPI_TARGET >= 4
199:     switch (type)
200:     {
201:     case HeapType::LOW:     return memoryManager.m_heapLow.GetFreeSpace();
202:     case HeapType::HIGH:    return memoryManager.m_heapHigh.GetFreeSpace();
203:     case HeapType::ANY:     return memoryManager.m_heapLow.GetFreeSpace() + memoryManager.m_heapHigh.GetFreeSpace();
204:     default:                return 0;
205:     }
206: #else
207:     switch (type)
208:     {
209:     case HeapType::LOW:
210:     case HeapType::ANY:     return memoryManager.m_heapLow.GetFreeSpace();
211:     default:                return 0;
212:     }
213: #endif
214: }
215: 
216: /// <summary>
217: /// Display the current status of all heap allocators
218: /// </summary>
219: void MemoryManager::DumpStatus()
220: {
221:     auto& memoryManager = GetMemoryManager();
222:     LOG_DEBUG("Low heap:");
223:     memoryManager.m_heapLow.DumpStatus();
224: #if BAREMETAL_RPI_TARGET >= 4
225:     LOG_DEBUG("High heap:");
226:     memoryManager.m_heapHigh.DumpState();
227: #endif
228: }
229: 
230: /// <summary>
231: /// Construct the singleton MemoryManager instance if needed, and return a reference to the instance
232: /// </summary>
233: /// <returns>Reference to the singleton MemoryManager</returns>
234: MemoryManager& baremetal::GetMemoryManager()
235: {
236:     static MemoryManager instance;
237:     return instance;
238: }
```

- Line 42-45: We need to include `Assert.h`, `Logger.h` and `MachineInfo.h`
- Line 60-99: We implement the constructor for the `MemoryManager` class
  - Line 68-76: We get the `MachineInfo` instance and retrieve the ARM CPU base memory address and allocated size.
If the returned size is 0, we default to the full memory with 64 Mb allocated to the VideoCore
  - Line 78-79: We perform a sanity check, and assign the low heap memory size
  - Line 81-82: We substract the low heap start as well as the space reserved for paging, and set up the low heap
  - Line 84-98: In case of Raspberry Pi 4 or higher, we set up the high heap
    - Line 85: We retrieve the total RAM size
    - Line 86-97: If larger that 1Gb, we determine the high end of the memory and limit it to 3Gb, we then subtract the first 1 Gb and set up the high heap
- Line 121-144: We implement the method `HeapAllocate()`
  - Line 123: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 127-135: In case of Raspberry Pi 4 or higher, we allow for high heap selection, and allocate accordingly
If any heap was selected, we first attempt to claim from the low heap, if that fails we try the high heap
  - Line 137-142: In case of Raspberry Pi 3, we only allow the low heap
- Line 152-167: We implement the method `HeapReAllocate()`
  - Line 154: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 156-163: In case of Raspberry Pi 4 or higher, we check which heap to access, and call `ReAllocate()` on it
  - Line 165: In case of Raspberry Pi 3, we call `ReAllocate()` on the low heap
- Line 173-188: We implement the method `HeapFree()`
  - Line 175: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 177-184: In case of Raspberry Pi 4 or higher, we check which heap to access, and call `Free()` on it
  - Line 186: In case of Raspberry Pi 3, we call `Free()` on the low heap
- Line 195-214: We implement the method `GetHeapFreeSpace()`
  - Line 197: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 199-205: In case of Raspberry Pi 4 or higher, depending on the selected heap, we call `GetFreeSpace()` on it.
If any heap is selected, the freespace for both heaps is added together
  - Line 207-212: In case of Raspberry Pi 3, we call `GetFreeSpace()` on the low heap
- Line 219-228: We implement the method `DumpStatus()`
  - Line 221: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 222-223: We call `DumpStatus()` on the low heap
  - Line 225-226: In case of Raspberry Pi 4 or higher, we call `DumpStatus()` on the high heap.
- Line 234-238: We implement the function `GetMemoryManager()`

### System.cpp {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_SYSTEMCPP}

Although not urgently needed yet, we'll need singleton the `MemoryManagement` instance in the `sysinit()` function soon, in order to set up memory mapping.
So let's go ahead and instantiate the `MemoryManager` there.
Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
45: #include <baremetal/MemoryAccess.h>
46: #include <baremetal/MemoryManager.h>
47: #include <baremetal/SysConfig.h>
...
180:     // halt, if KERNEL_MAX_SIZE is not properly set
181:     // cannot inform the user here
182:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
183:     {
184:         GetSystem().Halt();
185:     }
186: 
187:     // We need to create a memory system first, as core 0 needs to be the first to enable the MMU
188:     GetMemoryManager();
189: 
190:     // Call constructors of static objects
191:     extern void (*__init_start)(void);
192:     extern void (*__init_end)(void);
193:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
194:     {
195:         (**func)();
196:     }
197: 
...
```

- Line 46: We need to include `MemoryManager.h`
- Line 188: We call `GetMemoryManager()` to instantiate the `MemoryManager` singleton instance.

### Application code {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_APPLICATION_CODE}

Let's use the memory allocation and see how it behaves.
We've set `BAREMETAL_TRACE_MEMORY` and `BAREMETAL_TRACE_MEMORY_DETAIL` to `ON`, so we will get quite a bit of detail on memory allocations.
Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/RPIProperties.h>
9: #include <baremetal/SysConfig.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/System.h>
12: #include <baremetal/Timer.h>
13: 
14: LOG_MODULE("main");
15: 
16: using namespace baremetal;
17: 
18: int main()
19: {
20:     auto& console = GetConsole();
21:     LOG_DEBUG("Hello World!");
22: 
23:     MemoryManager& memoryManager = GetMemoryManager();
24:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
25:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
26:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
27: 
28:     auto ptr = memoryManager.HeapAllocate(4, HeapType::LOW);
29:     LOG_INFO("Allocated block %llx", ptr);
30:     memoryManager.DumpStatus();
31: 
32:     memoryManager.HeapFree(ptr);
33:     memoryManager.DumpStatus();
34: 
35:     LOG_INFO("Wait 5 seconds");
36:     Timer::WaitMilliSeconds(5000);
37: 
38:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
39:     char ch{};
40:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
41:     {
42:         ch = console.ReadChar();
43:         console.WriteChar(ch);
44:     }
45:     if (ch == 'p')
46:         assert(false);
47: 
48:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
49: }
```

- Line 23: We get the singleton MemoryManager instance
- Line 24-26: We log the free space for each heap and both heaps (`HeapType::ANY`)
- Line 28-29: We allocate 4 bytes of memory in the low heap and log the resulting memory address
- Line 30: We log the status of the memory manager
- Line 32-33: We free the memory again, and log the status of the memory manager

### Update project configuration {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
52:     )
53: 
54: set(PROJECT_INCLUDES_PUBLIC
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
86:     )
87: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_14_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will allocate memory, and show that one block of memory is allocated and freed again:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:79)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        0 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 0 (HeapAllocator:273)
Debug  Total #allocated bytes:  0 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Starting up (System:201)
Debug  Hello World! (main:21)
Info   Heap space available: 983564288 bytes (main:24)
Info   High heap space available: 0 bytes (main:25)
Info   DMA heap space available: 983564288 bytes (main:26)
Debug  Allocate 64 bytes at 0000000000600040 (HeapAllocator:169)
Debug  Current #allocations = 1, max #allocations = 1 (HeapAllocator:170)
Info   Allocated block 600040 (main:29)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:64 (HeapAllocator:272)
Debug  Total #allocated blocks: 1 (HeapAllocator:273)
Debug  Total #allocated bytes:  64 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Free 64 bytes at 0000000000600040 (HeapAllocator:249)
Debug  Current #allocations = 0, max #allocations = 1 (HeapAllocator:250)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 1 (HeapAllocator:273)
Debug  Total #allocated bytes:  64 (HeapAllocator:274)
Debug  Total #freed blocks:     1 (HeapAllocator:275)
Debug  Total #freed bytes:      64 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 1, #bytes = 64 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Wait 5 seconds (main:42)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

## Supporting C/C++ memory allocation - step 2 {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2}

In order to use memory allocation in the way we are used to, using either `malloc()`, `calloc()`, `realloc()` and `free()` in C or the `new` and `delete` operators in C++, we need to add them.
So we'll update `New.h` and `New.cpp` to add functions for C++, and `Util.h` and `Util.cpp` for the C variants.

### Util.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_UTILH}

We will declares the memory allocation functions for C.
Update the file `code/libraries/baremetal/include/baremetal/Util.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Util.h
...
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50: 
51: void* memset(void* buffer, int value, size_t length);
52: void* memcpy(void* dest, const void* src, size_t length);
53: 
54: size_t strlen(const char* str);
55: char* strncpy(char* dest, const char* src, size_t maxLen);
56: char* strncat(char* dest, const char* src, size_t maxLen);
57: 
58: void* malloc(size_t size);
59: void* calloc(size_t num, size_t size);
60: void* realloc(void* ptr, size_t new_size);
61: void free(void* ptr);
62: 
63: #ifdef __cplusplus
64: }
65: #endif
66: 
```

Line 58-61: We add the function declarations for `malloc()`, `calloc()`, `realloc()` and `free()`

### Util.cpp {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_UTILCPP}

We will implement the added functions.
Update the file `code/libraries/baremetal/src/Util.cpp`

```cpp
File: code/libraries/baremetal/src/Util.cpp
...
165: 
166: /// <summary>
167: /// Allocates a block of memory of the desired size.
168: /// </summary>
169: /// <param name="size">The desired size of the memory block</param>
170: /// <returns></returns>
171: void* malloc(size_t size)
172: {
173:     return baremetal::MemoryManager::HeapAllocate(size, HeapType::ANY);
174: }
175: 
176: /// <summary>
177: /// Allocates a contiguous block of memory for the desired number of cells of the desired size each.
178: /// 
179: /// The memory allocated is num x size bytes
180: /// </summary>
181: /// <param name="num">Number of cells to allocate memory for</param>
182: /// <param name="size">Size of each cell</param>
183: /// <returns></returns>
184: void* calloc(size_t num, size_t size)
185: {
186:     return malloc(num * size);
187: }
188: 
189: /// <summary>
190: /// Re-allocates memory previously allocated with malloc() or calloc() to a new size
191: /// </summary>
192: /// <param name="ptr">Pointer to memory block to be re-allocated</param>
193: /// <param name="new_size">The desired new size of the memory block</param>
194: /// <returns></returns>
195: void* realloc(void* ptr, size_t new_size)
196: {
197:     return baremetal::MemoryManager::HeapReAllocate(ptr, new_size);
198: }
199: 
200: /// <summary>
201: /// Frees memory previously allocated with malloc() or calloc()
202: /// </summary>
203: /// <param name="ptr">Pointer to memory block to be freed</param>
204: void free(void* ptr)
205: {
206:     baremetal::MemoryManager::HeapFree(ptr);
207: }
```

- Line 171-174: We implement `malloc()` by calling `MemoryManager::HeapAlloc()`
We use any heap, so in case the low heap is exhausted, on Raspberry Pi 4 and higher we will use the high heap
- Line 184-187: We implement `calloc()` by calling `malloc()` with num x size
- Line 195-198: We implement `realloc()` by calling `MemoryManager::HeapReAlloc()`
- Line 204-207: We implement `free()` by calling `MemoryManager::HeapFree()`

### New.h {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_NEWH}

We will extend the operators `new` and `delete`. We will also add some placement versions, so we can specify the heap to use when allocating.

Update the file `code/libraries/baremetal/include/baremetal/New.h`

```cpp
File: code/libraries/baremetal/include/baremetal/New.h
...
42: /// @file
43: /// Basic memory allocation functions
44: 
45: #include <baremetal/MemoryManager.h>
46: #include <baremetal/Types.h>
47: 
48: void* operator new (size_t size, HeapType type);
49: void* operator new[](size_t size, HeapType type);
50: void* operator new (size_t size, void* address);
51: void* operator new[](size_t size, void* address);
52: 
```

- Line 48: We declare the new operator for placement using a heap specification.
We can then use e.g. `new (HeapType::LOW) X` to allocate an instance of class X in the low heap.
- Line 49: We declare the new[] operator for placement using a heap specification
- Line 50: We declare the new operator for placement using a pointer. This will place the instance at the specified location
- Line 51: We declare the new[] operator for placement using a pointer. This will place the instance at the specified location

### New.cpp {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_NEWCPP}

We will implement the added `new` and `delete` operators.
Update the file `code/libraries/baremetal/src/New.cpp`

```cpp
File: code/libraries/baremetal/src/New.cpp
...
48: using namespace baremetal;
49: 
50: /// <summary>
51: /// Class specific placement allocation for single value.
52: /// </summary>
53: /// <param name="size">Size of block to allocate in bytes</param>
54: /// <param name="type">Heap type to allocate from</param>
55: /// <returns>Pointer to allocated block of memory or nullptr</returns>
56: void* operator new (size_t size, HeapType type)
57: {
58: 	return MemoryManager::HeapAllocate(size, type);
59: }
60: 
61: /// <summary>
62: /// Class specific placement allocation for array.
63: /// </summary>
64: /// <param name="size">Size of block to allocate in bytes</param>
65: /// <param name="type">Heap type to allocate from</param>
66: /// <returns>Pointer to allocated block of memory or nullptr</returns>
67: void* operator new[](size_t size, HeapType type)
68: {
69: 	return MemoryManager::HeapAllocate(size, type);
70: }
71: 
72: /// <summary>
73: /// Non allocating placement allocation for single value.
74: /// </summary>
75: /// <param name="size">Size of block to allocate in bytes</param>
76: /// <param name="address">Address to be used</param>
77: /// <returns>Pointer to block of memory</returns>
78: void* operator new (size_t size, void* address)
79: {
80: 	return address;
81: }
82: 
83: /// <summary>
84: /// Non allocating placement allocation for array.
85: /// </summary>
86: /// <param name="size">Size of block to allocate in bytes</param>
87: /// <param name="address">Address to be used</param>
88: /// <returns>Pointer to block of memory</returns>
89: void* operator new[](size_t size, void* address)
90: {
91: 	return address;
92: }
93: 
94: /// <summary>
95: /// Standard allocation for single value.
96: ///
97: /// Allocates from default heap.
98: /// </summary>
99: /// <param name="size">Size of block to allocate in bytes</param>
100: /// <returns>Pointer to allocated block of memory or nullptr</returns>
101: void* operator new (size_t size)
102: {
103: 	return MemoryManager::HeapAllocate(size, HEAP_DEFAULT_NEW);
104: }
105: 
106: /// <summary>
107: /// Standard allocation for array.
108: ///
109: /// Allocates from default heap.
110: /// </summary>
111: /// <param name="size">Size of block to allocate in bytes</param>
112: /// <returns>Pointer to allocated block of memory or nullptr</returns>
113: void* operator new[](size_t size)
114: {
115: 	return MemoryManager::HeapAllocate(size, HEAP_DEFAULT_NEW);
116: }
117: 
118: /// <summary>
119: /// Standard de-allocation for single value.
120: /// </summary>
121: /// <param name="address">Block to free</param>
122: void operator delete (void* address) noexcept
123: {
124: 	MemoryManager::HeapFree(address);
125: }
126: 
127: /// <summary>
128: /// Standard de-allocation for array.
129: /// </summary>
130: /// <param name="address">Block to free</param>
131: void operator delete[](void* address) noexcept
132: {
133: 	MemoryManager::HeapFree(address);
134: }
135: 
136: /// <summary>
137: /// Standard de-allocation with size for single value.
138: /// </summary>
139: /// <param name="address">Block to free</param>
140: void operator delete (void* address, size_t /*size*/) noexcept
141: {
142: 	MemoryManager::HeapFree(address);
143: }
144: 
145: /// <summary>
146: /// Standard de-allocation for array.
147: /// </summary>
148: /// <param name="address">Block to free</param>
149: //// <param name="size">Size of block to free</param>
150: void operator delete[](void* address, size_t /*size*/) noexcept
151: {
152: 	MemoryManager::HeapFree(address);
153: }
```

- Line 56-59: We implement the `new` operator for placement with heap specification
- Line 67-70: We implement the `new[]` operator for placement with heap specification
- Line 78-81: We implement the `new` placement operator
- Line 89-92: We implement the `new[]` placement operator
- Line 101-104: We implement the normal `new` operator
- Line 113-116: We implement the normal `new[]` operator
- Line 122-125: We re-implement the standard `delete` operator using a call to `MemoryManager::HeapFree()`
- Line 131-134: We implement the standard `delete[]` operator
- Line 141-144: We re-implement the `delete` operator with size using a call to `MemoryManager::HeapFree()`
- Line 151-154: We implement the `delete[]` operator with size

### Application code {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_APPLICATION_CODE}

Let's use the `malloc()` function and `new` operator.
Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/SysConfig.h>
11: #include <baremetal/Serialization.h>
12: #include <baremetal/System.h>
13: #include <baremetal/Timer.h>
14: #include <baremetal/Util.h>
15: 
16: LOG_MODULE("main");
17: 
18: using namespace baremetal;
19: 
20: int main()
21: {
22:     auto& console = GetConsole();
23:     LOG_DEBUG("Hello World!");
24: 
25:     MemoryManager& memoryManager = GetMemoryManager();
26:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
27:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
28:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
29: 
30:     auto ptr = memoryManager.HeapAllocate(4, HeapType::LOW);
31:     LOG_INFO("Allocated block %llx", ptr);
32:     memoryManager.DumpStatus();
33: 
34:     memoryManager.HeapFree(ptr);
35:     memoryManager.DumpStatus();
36: 
37:     class X {};
38: 
39:     X* x = new (HeapType::LOW) X;
40:     LOG_INFO("Allocated block %llx", x);
41:     memoryManager.DumpStatus();
42:     delete x;
43:     memoryManager.DumpStatus();
44: 
45:     void*p = malloc(256);
46:     LOG_INFO("Allocated block %llx", p);
47:     memoryManager.DumpStatus();
48:     free(p);
49:     memoryManager.DumpStatus();
50: 
51:     LOG_INFO("Wait 5 seconds");
52:     Timer::WaitMilliSeconds(5000);
53: 
54:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
55:     char ch{};
56:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
57:     {
58:         ch = console.ReadChar();
59:         console.WriteChar(ch);
60:     }
61:     if (ch == 'p')
62:         assert(false);
63: 
64:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
65: }
```

- Line 8: We include `New.h` to declare the placement `new` operators
- Line 37: We declare a class `X`
- Line 39-40: We allocate a class `X` instance in the low heap and log the resulting memory address
- Line 41: We log the status of the memory manager
- Line 42-43: We delete the object again, and log the status of the memory manager
- Line 45-46: We allocate a memory block using `malloc()` in any heap and log the resulting memory address
- Line 47: We log the status of the memory manager
- Line 48-49: We free the memory again, and log the status of the memory manager

### Update project configuration {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_UPDATE_PROJECT_CONFIGURATION}

As we did not add any files, we don't need to update the CMake file.

### Configuring, building and debugging {#TUTORIAL_14_MEMORY_MANAGEMENT_SUPPORTING_CC_MEMORY_ALLOCATION__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will allocate memory, and show that one block of memory is allocated and freed again:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:79)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        0 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 0 (HeapAllocator:273)
Debug  Total #allocated bytes:  0 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Starting up (System:201)
Debug  Hello World! (main:23)
Info   Heap space available: 983564288 bytes (main:26)
Info   High heap space available: 0 bytes (main:27)
Info   DMA heap space available: 983564288 bytes (main:28)
Debug  Allocate 64 bytes at 0000000000600040 (HeapAllocator:169)
Debug  Current #allocations = 1, max #allocations = 1 (HeapAllocator:170)
Info   Allocated block 600040 (main:31)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:64 (HeapAllocator:272)
Debug  Total #allocated blocks: 1 (HeapAllocator:273)
Debug  Total #allocated bytes:  64 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Free 64 bytes at 0000000000600040 (HeapAllocator:249)
Debug  Current #allocations = 0, max #allocations = 1 (HeapAllocator:250)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 1 (HeapAllocator:273)
Debug  Total #allocated bytes:  64 (HeapAllocator:274)
Debug  Total #freed blocks:     1 (HeapAllocator:275)
Debug  Total #freed bytes:      64 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 1, #bytes = 64 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Reuse 64 bytes at 0000000000600040 (HeapAllocator:142)
Debug  Current #allocations = 1, max #allocations = 1 (HeapAllocator:143)
Info   Allocated block 600040 (main:40)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:64 (HeapAllocator:272)
Debug  Total #allocated blocks: 2 (HeapAllocator:273)
Debug  Total #allocated bytes:  128 (HeapAllocator:274)
Debug  Total #freed blocks:     1 (HeapAllocator:275)
Debug  Total #freed bytes:      64 (HeapAllocator:276)
Debug  malloc(64): 1 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 1, #bytes = 64 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Free 64 bytes at 0000000000600040 (HeapAllocator:249)
Debug  Current #allocations = 0, max #allocations = 1 (HeapAllocator:250)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 2 (HeapAllocator:273)
Debug  Total #allocated bytes:  128 (HeapAllocator:274)
Debug  Total #freed blocks:     2 (HeapAllocator:275)
Debug  Total #freed bytes:      128 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Allocate 1024 bytes at 00000000006000C0 (HeapAllocator:169)
Debug  Current #allocations = 1, max #allocations = 1 (HeapAllocator:170)
Info   Allocated block 6000C0 (main:46)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        2 (HeapAllocator:271)
Debug  Current #allocated bytes:1024 (HeapAllocator:272)
Debug  Total #allocated blocks: 3 (HeapAllocator:273)
Debug  Total #allocated bytes:  1152 (HeapAllocator:274)
Debug  Total #freed blocks:     2 (HeapAllocator:275)
Debug  Total #freed bytes:      128 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128 (HeapAllocator:280)
Debug  malloc(1024): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Free 1024 bytes at 00000000006000C0 (HeapAllocator:249)
Debug  Current #allocations = 0, max #allocations = 1 (HeapAllocator:250)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        2 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 3 (HeapAllocator:273)
Debug  Total #allocated bytes:  1152 (HeapAllocator:274)
Debug  Total #freed blocks:     3 (HeapAllocator:275)
Debug  Total #freed bytes:      1152 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 1, #bytes = 1024 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Wait 5 seconds (main:51)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

You can see that we allocated three blocks of memory and freed them again. Two were of size 64, one was of size 1024.

In order to stop overloading the console with memory debug information, let's set the detail to `OFF`

## Switching off debugging for memory management - step 3 {#TUTORIAL_14_MEMORY_MANAGEMENT_SWITCHING_OFF_DEBUGGING_FOR_MEMORY_MANAGEMENT__STEP_3}

### Main CMake file {#TUTORIAL_14_MEMORY_MANAGEMENT_SWITCHING_OFF_DEBUGGING_FOR_MEMORY_MANAGEMENT__STEP_3_MAIN_CMAKE_FILE}

```cmake
File: CMakeLists.txt
...
65: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" ON)
66: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
...
```

### Configuring, building and debugging {#TUTORIAL_14_MEMORY_MANAGEMENT_SWITCHING_OFF_DEBUGGING_FOR_MEMORY_MANAGEMENT__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will still show quite some information, due to the calls to `DumpStatus()`, but a bit less:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:79)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        0 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 0 (HeapAllocator:273)
Debug  Total #allocated bytes:  0 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Starting up (System:201)
Debug  Hello World! (main:23)
Info   Heap space available: 983564288 bytes (main:26)
Info   High heap space available: 0 bytes (main:27)
Info   DMA heap space available: 983564288 bytes (main:28)
Info   Allocated block 600040 (main:31)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:64 (HeapAllocator:272)
Debug  Total #allocated blocks: 1 (HeapAllocator:273)
Debug  Total #allocated bytes:  64 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 1 (HeapAllocator:273)
Debug  Total #allocated bytes:  64 (HeapAllocator:274)
Debug  Total #freed blocks:     1 (HeapAllocator:275)
Debug  Total #freed bytes:      64 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 1, #bytes = 64 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Allocated block 600040 (main:40)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:64 (HeapAllocator:272)
Debug  Total #allocated blocks: 2 (HeapAllocator:273)
Debug  Total #allocated bytes:  128 (HeapAllocator:274)
Debug  Total #freed blocks:     1 (HeapAllocator:275)
Debug  Total #freed bytes:      64 (HeapAllocator:276)
Debug  malloc(64): 1 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 1, #bytes = 64 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        1 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 2 (HeapAllocator:273)
Debug  Total #allocated bytes:  128 (HeapAllocator:274)
Debug  Total #freed blocks:     2 (HeapAllocator:275)
Debug  Total #freed bytes:      128 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Allocated block 6000C0 (main:46)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    1 (HeapAllocator:270)
Debug  Max #allocations:        2 (HeapAllocator:271)
Debug  Current #allocated bytes:1024 (HeapAllocator:272)
Debug  Total #allocated blocks: 3 (HeapAllocator:273)
Debug  Total #allocated bytes:  1152 (HeapAllocator:274)
Debug  Total #freed blocks:     2 (HeapAllocator:275)
Debug  Total #freed bytes:      128 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128 (HeapAllocator:280)
Debug  malloc(1024): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        2 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 3 (HeapAllocator:273)
Debug  Total #allocated bytes:  1152 (HeapAllocator:274)
Debug  Total #freed blocks:     3 (HeapAllocator:275)
Debug  Total #freed bytes:      1152 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 1, #bytes = 1024 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Wait 5 seconds (main:51)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

Next: [15-string](15-string.md)

