# Tutorial 15: Memory management {#TUTORIAL_15_MEMORY_MANAGEMENT}

@tableofcontents

## New tutorial setup {#TUTORIAL_15_MEMORY_MANAGEMENT_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/14-memory-management`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_15_MEMORY_MANAGEMENT_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-15.a`
- a library `output/Debug/lib/stdlib-15.a`
- an application `output/Debug/bin/15-memory-management.elf`
- an image in `deploy/Debug/15-memory-management-image`

## Setting up memory management - step 1 {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1}

As we would like to start using classes that allocate and de-allocate memory, such as a string class, we need to set up for memory management.
This is going to be both a straightforward and a tricky task, we'll get around to that.

At times it may seem that we're reimplementing functionality that you are already used to from the C and C++ standard libraries.
And of course this is partly true.
However, we are building code from the ground up, so you can expect nothing to be done for you,
but as we'll see later, after some time we can start using functionality that is provided by e.g. the standard C++ library,
as long as the basis is layed out.

We'll start by adding some definitions in both the memory map and the system configuration.
Then we'll add methods to the `MemoryManager` class for allocating and freeing memory.
This will depend on a `HeapAllocator` which performs the actual allocation.

### Main CMake file {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_MAIN_CMAKE_FILE}

First let's add some definitions for debugging memory allocation.

Update the file `CMakeLists.txt`

```cmake
File: CMakeLists.txt
...
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
69: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" ON)
70: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" ON)
...
98: if (BAREMETAL_TRACE_MEMORY)
99:     set(BAREMETAL_MEMORY_TRACING 1)
100: else ()
101:     set(BAREMETAL_MEMORY_TRACING 0)
102: endif()
103: if (BAREMETAL_TRACE_MEMORY_DETAIL)
104:     set(BAREMETAL_MEMORY_TRACING 1)
105:     set(BAREMETAL_MEMORY_TRACING_DETAIL 1)
106: else ()
107:     set(BAREMETAL_MEMORY_TRACING_DETAIL 0)
108: endif()
109: set(BAREMETAL_LOAD_ADDRESS 0x80000)
110: 
111: set(DEFINES_C
112:     PLATFORM_BAREMETAL
113:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
114:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
115:     BAREMETAL_MEMORY_TRACING=${BAREMETAL_MEMORY_TRACING}
116:     BAREMETAL_MEMORY_TRACING_DETAIL=${BAREMETAL_MEMORY_TRACING_DETAIL}
117:     BAREMETAL_MAJOR=${VERSION_MAJOR}
118:     BAREMETAL_MINOR=${VERSION_MINOR}
119:     BAREMETAL_LEVEL=${VERSION_LEVEL}
120:     BAREMETAL_BUILD=${VERSION_BUILD}
121:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
122:     )
...
285: message(STATUS "Baremetal settings:")
286: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
287: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
288: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
289: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
290: message(STATUS "-- Debug output to UART0:           ${BAREMETAL_CONSOLE_UART0}")
291: message(STATUS "-- Debug output to UART1:           ${BAREMETAL_CONSOLE_UART1}")
292: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
293: message(STATUS "-- Memory tracing output:           ${BAREMETAL_TRACE_MEMORY}")
294: message(STATUS "-- Detailed memory tracing output:  ${BAREMETAL_TRACE_MEMORY_DETAIL}")
295: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
296: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
297: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
298: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
299: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
...
```

- Line 69: We add a variable `BAREMETAL_TRACE_MEMORY` that enables tracing memory allocation functions.
We'll set it to `ON` for now
- Line 70: We add a variable `BAREMETAL_TRACE_MEMORY_DETAIL` that enables tracing memory allocation functions in more detail.
We'll set it to `ON` for now
- Line 98-102: We set variable `BAREMETAL_MEMORY_TRACING` to 1 if `BAREMETAL_TRACE_MEMORY` is `ON`, and 0 otherwise
- Line 103-108: We set variable `BAREMETAL_MEMORY_TRACING_DETAIL` to 1 if `BAREMETAL_TRACE_MEMORY_DETAIL` is `ON`, and 0 otherwise.
If `BAREMETAL_TRACE_MEMORY_DETAIL` is `ON`, we also set `BAREMETAL_MEMORY_TRACING` to 1
- Line 115: We set the compiler definition `BAREMETAL_MEMORY_TRACING` to the value of the `BAREMETAL_MEMORY_TRACING` variable
- Line 116: We set the compiler definition `BAREMETAL_MEMORY_TRACING_DETAIL` to the value of the `BAREMETAL_MEMORY_TRACING_DETAIL` variable
- Line 293: We print the value of `BAREMETAL_TRACE_MEMORY`
- Line 294: We print the value of `BAREMETAL_TRACE_MEMORY_DETAIL`

### Synchronization.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_SYNCHRONIZATIONH}

We'll add some definitions need for alignment. These have to do with the length of a cache line.

Create the file `code/libraries/baremetal/include/baremetal/Synchronization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Synchronization.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
42: /// @file
43: /// Synchronization functionality
44: 
45: /// @brief Minimum cache line length (16 x 32 bit word) as specified in CTR_EL0 register, see @ref ARM_REGISTERS
46: #define DATA_CACHE_LINE_LENGTH_MIN 64
47: /// @brief Maximum cache line length (16 x 32 bit word) as specified in CTR_EL0 register, see @ref ARM_REGISTERS
48: #define DATA_CACHE_LINE_LENGTH_MAX 64
```

### SysConfig.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_SYSCONFIGH}

We'll add some definitions for heap types and bucket sizes, which will be explained in a minute.

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

- Line 61-72: We create a definition `HEAP_DEFAULT_NEW` if not already defined. This specifies the heap to use for normal (i.e. non placement) calls to new() operators.
- Line 74-81: We create a definition `HEAP_DEFAULT_MALLOC` if not already defined. This specifies the heap to use for calls to the `malloc()` function.
- Line 83-99: We create a definition `HEAP_BLOCK_BUCKET_SIZES` if not already defined. This specifies the heap buckets sizes as a comma separated list.
If a memory block of size n is needed, the list will be searched to find the first size larger than or equal to n.
So here, the sizes for blocks allocated are:
  - 64 bytes
  - 1 Kbyte
  - 4 Kbyte
  - 16 Kbyte
  - 64 Kbyte
  - 256 Kbyte
  - 512 Kbyte
  - Any size allocated larger than the largest block is allocated differently, but never returned. This is the limitation of the poor man's solution we implement

We will define an enum class in a second named `HeapType`, which specifies the heap to be used for a certain function. We distinguish two heap types:
- Low heap, which is the memory available for allocation below 1 Gb.
As Raspberry Pi 3 only has 1 Gb of RAM, this is the only type for that board
- High heap, which is the memory available for allocation above 1 Gb.
This will reach up to the end of RAM (excluding reserved addresses), to a maximum of 3Gb.
The reason for this limit is that DMA can only address up to that address range.

The reason for the split between low and high heap is that the memory assigned to the VideoCore (GPU) for graphics is at the end of the 1 Gb range.
So we will use the memory range from just above the coherent memory region up to 1 Gb, minus the memory assign to VideoCore to the low heap.
The high heap will range from 1 Gb up to max. 3 Gb, as far as physical memory is available.

### Memory map revisited {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_MEMORY_MAP_REVISITED}

<img src="images/memory-map.png" alt="Memory map" width="800"/>

As you can see the memory in the low heap starts at `MEM_HEAP_START` and ends at `ARM_MEM_SIZE`, which is just below the memory reserved for the VideoCore.
The high heap starts at `MEM_HIGHMEM_START`.

### MemoryMap.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_MEMORYMAPH}

We also need to update the memory map defines, to point to the heap start and end

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
98: #define MEM_HIGHMEM_START GIGABYTE
99: /// @brief High memory region (above 1 Gb) end. Memory above 3 GB is not safe to be used for DMA and is not used.
100: #define MEM_HIGHMEM_END   (3 * GIGABYTE - 1)
101: 
102: #endif
```

- Line 93-94: We define the start of the low heap, `MEM_HEAP_START` to be directly after the end of the coherent memory region
- Line 97-98: For Raspberry Pi 4 or higher, we define the start of the high heap, `MEM_HIGHMEM_START`
- Line 99-100: For Raspberry Pi 4 or higher, we also define the maximum end of the high heap, `MEM_HIGHMEM_END`

### HeapAllocator.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_HEAPALLOCATORH}

We will declare a class to handle allocation and freeing of memory for a specific heap (low or high).

Create the file `code/libraries/baremetal/include/baremetal/HeapAllocator.h`

```cpp
File: code/libraries/baremetal/include/baremetal/HeapAllocator.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
42: #include "baremetal/Synchronization.h"
43: #include "stdlib/Macros.h"
44: #include "stdlib/Types.h"
45: 
46: /// @file
47: /// Heap allocator
48: 
49: namespace baremetal {
50: 
51: /// @brief Block alignment
52: #define HEAP_BLOCK_ALIGN       DATA_CACHE_LINE_LENGTH_MAX
53: /// @brief Block alignment mask
54: #define HEAP_ALIGN_MASK        (HEAP_BLOCK_ALIGN - 1)
55: 
56: /// @brief Maximum number of heap buckets used
57: #define HEAP_BLOCK_MAX_BUCKETS 20
58: 
59: /// <summary>
60: /// Administration on an allocated block of memory
61: /// </summary>
62: struct HeapBlockHeader
63: {
64:     /// @brief Heap block magic number
65:     uint32 magic;
66:     /// @brief Heap block magic number (BLMC)
67: #define HEAP_BLOCK_MAGIC 0x424C4D43
68:     /// @brief Size of allocated block
69:     uint32 size;
70:     /// @brief Pointer to next header
71:     HeapBlockHeader* next;
72:     /// @brief Padding to align to HEAP_BLOCK_ALIGN bytes
73:     uint8 align[HEAP_BLOCK_ALIGN - 16];
74:     /// @brief Start of actual allocated block
75:     uint8 data[0];
76: }
77: /// @brief Just specifies the struct is packed
78: PACKED;
79: 
80: /// <summary>
81: /// Bucket containing administration on allocated blocks of memory
82: /// </summary>
83: struct HeapBlockBucket
84: {
85:     /// @brief Size of bucket (size actual memory allocated, excluding bucket header)
86:     uint32 size;
87: #if BAREMETAL_MEMORY_TRACING
88:     /// @brief Count of blocks allocated in bucket
89:     unsigned count;
90:     /// @brief Maximum count of blocks allocated in bucket over time
91:     unsigned maxCount;
92:     /// @brief Total number of blocks allocated in bucket over time
93:     uint64 totalAllocatedCount;
94:     /// @brief Total number of bytes allocated in bucket over time
95:     uint64 totalAllocated;
96:     /// @brief Total number of blocks freed in bucket over time
97:     uint64 totalFreedCount;
98:     /// @brief Total number of bytes freed in bucket over time
99:     uint64 totalFreed;
100: #endif
101:     /// @brief List of free blocks in bucket to be re-used
102:     HeapBlockHeader* freeList;
103: };
104: 
105: /// <summary>
106: /// Allocates blocks from a flat memory region
107: /// </summary>
108: class HeapAllocator
109: {
110: private:
111:     /// @brief Name of the heap
112:     const char* m_heapName;
113:     /// @brief Next available address
114:     uint8* m_next;
115:     /// @brief End of available address space
116:     uint8* m_limit;
117:     /// @brief Reserved address space
118:     size_t m_reserve;
119:     /// @brief Allocated bucket administration
120:     HeapBlockBucket m_buckets[HEAP_BLOCK_MAX_BUCKETS + 1];
121: 
122:     /// @brief Sizes of allocation buckets
123:     static uint32 s_bucketSizes[];
124: 
125: public:
126:     explicit HeapAllocator(const char* heapName = "heap");
127: 
128:     void Setup(uintptr baseAddress, size_t size, size_t reserve);
129: 
130:     size_t GetFreeSpace() const;
131:     void* Allocate(size_t size);
132:     void* ReAllocate(void* block, size_t size);
133:     void Free(void* block);
134: 
135: #if BAREMETAL_MEMORY_TRACING
136:     void DumpStatus();
137: 
138:     uint64 GetCurrentAllocatedBlockCount();
139:     uint64 GetCurrentAllocationSize();
140:     uint64 GetMaxAllocatedBlockCount();
141:     uint64 GetTotalAllocatedBlockCount();
142:     uint64 GetTotalFreedBlockCount();
143:     uint64 GetTotalAllocationSize();
144:     uint64 GetTotalFreeSize();
145: #endif
146: };
147: 
148: } // namespace baremetal
```

- Line 44: We include `Synchronization.h` for the definition of the cache line length
- Line 51-52: We define `HEAP_BLOCK_ALIGN` which signifies the alignment of allocated memory blocks.
This depends on the cache line length for performance reasons
- Line 53-54: We define `HEAP_ALIGN_MASK` which is used to check whether a memory block is indeed aligned correctly
- Line 56-57: We define `HEAP_BLOCK_MAX_BUCKETS` which signifies the maximum number of buckets used for memory allocation.
This is the maximum length for the list of different bucket sizes.
We select the minimum buckets size which can hold the requested memory block, and allocate within that bucket.
The currently defined bucket sizes are 64, 1K, 4K, 16K, 64K, 256K, 512K, and > 512K bytes.
- Line 59-78: We declare a structure to administer memory block information
  - Line 64-65: The member `magic` is used as a magic number to check against corruption of the memory block information
  - Line 66-67: We define the magic number `HEAP_BLOCK_MAGIC` to be used (the hex version of the string 'BLMC')
  - Line 68-69: The member `size` is the size of the allocated memory
  - Line 70-71: The member `next` is a pointer to the next allocated memory block in a linked list
  - Line 72-73: The member `align` adds padding to make sure that the data that follows is aligned to `HEAP_BLOCK_ALIGN` bytes.
The struct data itself is 4 + 4 + 8 = 16 bytes. The memory block administration therefore uses `HEAP_BLOCK_ALIGN` bytes of memory
  - Line 74-75: The member `data` is a placeholder for the actual allocated memory.
Notice we use PACKED again to make sure the struct is packed
- Line 80-103: We declare a structure to administer bucket information
  - Line 85-86: The member `size` is the size of the bucket (the memory block excluding its administration needs to fit in a bucket).
The amount of memory used is therefore the bucket size plus the size of the memory block administration
  - Line 88-89: The member `count` is only defined when memory tracing is enabled. It holds the current number of allocated blocks in a bucket
  - Line 90-91: The member `maxCount` is only defined when memory tracing is enabled. It holds the maximum of the number of allocated blocks in a bucket that were allocated at any time
  - Line 92-93: The member `totalAllocatedCount` is only defined when memory tracing is enabled. It holds the cumulative number of blocks that were allocated in a bucket over time
  - Line 94-95: The member `totalAllocated` is only defined when memory tracing is enabled. It holds the cumulative amount of memory that was allocated in a bucket over time
  - Line 96-97: The member `totalFreedCount` is only defined when memory tracing is enabled. It holds the cumulative number of blocks that were freed in a bucket over time.
  So the currently allocated blocks in the bucket should be equal to `totalFreedCount` - `totalAllocatedCount`
  - Line 98-99: The member `totalFreed` is only defined when memory tracing is enabled. It holds the cumulative amount of memory that was freed in a bucket over time.
  So the currently allocated memory in the bucket should be equal to `totalFreed` - `totalAllocated`
  - Line 101-102: The member `freeList` points to the first memory block administration of a list of blocks that were freed and not yet re-used
- Line 105-146: We declare the class `HeapAllocator`
  - Line 111-112: The member `m_heapName` holds the name of the heap for debugging purposes. This is passed to the constructor
  - Line 113-114: The member `m_next` holds the pointer to the next address that is still free for allocation
  - Line 115-116: The member `m_limit` holds the pointer to one byte beyond the last address available for the heap
  - Line 117-118: The member `m_reserve` holds an amount of memory reserved for other purposes (e.g. a block allocated that does not fit in any bucket)
  - Line 119-120: The member `m_buckets` holds the pointers to the first memory block information for each bucket.
  The last pointer is always nullptr, for this reason there are `HEAP_BLOCK_MAX_BUCKETS` + 1 elements
  - Line 122-123: The static member `s_bucketSizes` holds the size of each bucket. Any values equal to 0 are unused buckets
  - Line 126: We declare the constructor, which receives the heap name
  - Line 128: We declare the `Setup()` method, which is used to set the start address, size, and reserved space for the heap
  - Line 130: We declare the `GetFreeSpace()` method, which returns the amount of unused memory space in the heap (excluding any unused returned previously allocated blocks)
  - Line 131: We declare the `Allocate()` method, which allocates a block of memory, and returns the pointer to the memory block.
If it returns nullptr this means there is no more free memory available
  - Line 132: We declare the `Rellocate()` method, which reallocates a block of memory (allocates a block of the newly requested size, copies the contents, and frees the original), and returns the pointer to the new memory block.
Again, if it returns nullptr this means there is no more free memory available
  - Line 133: We declare the `Free()` method, which frees an allocated block of memory.
This means the block is added to the free list of the related bucket, the memory is not really returned as this would complicate memory management considerably.
After all this is a poor man's solution
  - Line 136: The method `DumpStatus()` is only defined when memory tracing is enabled.
It logs information on the currently allocated and free memory blocks
  - Line 138: The method `GetCurrentAllocatedBlockCount()` is only defined when memory tracing is enabled.
It returns the total count of currently allocated memory blocks for all buckets
  - Line 138: The method `GetCurrentAllocationSize()` is only defined when memory tracing is enabled.
It returns the total size of allocated memory blocks for all buckets
  - Line 140: The method `GetMaxAllocatedBlockCount()` is only defined when memory tracing is enabled
It returns the maximum count of allocated memory blocks over time for all buckets
  - Line 141: The method `GetTotalAllocatedBlockCount()` is only defined when memory tracing is enabled.
It returns the cumulative count of allocated memory blocks over time for all buckets
  - Line 142: The method `GetTotalFreedBlockCount()` is only defined when memory tracing is enabled.
It returns the cumulative count of freed memory blocks over time for all buckets
  - Line 143: The method `GetTotalAllocationSize()` is only defined when memory tracing is enabled
It returns the cumulative size of allocated memory blocks over time for all buckets
  - Line 144: The method `GetTotalFreeSize()` is only defined when memory tracing is enabled.
It returns the cumulative size of freed memory blocks over time for all buckets

### HeapAllocator.cpp {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_HEAPALLOCATORCPP}

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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
40: #include "baremetal/HeapAllocator.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/SysConfig.h"
45: #include "stdlib/Util.h"
46: 
47: /// @file
48: /// Heap allocator implementation
49: 
50: using namespace baremetal;
51: 
52: /// @brief Define log name
53: LOG_MODULE("HeapAllocator");
54: 
55: uint32 HeapAllocator::s_bucketSizes[] = {HEAP_BLOCK_BUCKET_SIZES};
56: 
57: /// <summary>
58: /// Constructs a heap allocator
59: /// </summary>
60: /// <param name="heapName">Name of the heap for debugging purpose (must be static)</param>
61: HeapAllocator::HeapAllocator(const char* heapName)
62:     : m_heapName{heapName}
63:     , m_next{}
64:     , m_limit{}
65:     , m_reserve{}
66:     , m_buckets{}
67: {
68:     memset(m_buckets, 0, sizeof(m_buckets));
69: 
70:     size_t numBuckets = sizeof(s_bucketSizes) / sizeof(s_bucketSizes[0]);
71:     if (numBuckets > HEAP_BLOCK_MAX_BUCKETS)
72:     {
73:         numBuckets = HEAP_BLOCK_MAX_BUCKETS;
74:     }
75: 
76:     for (size_t i = 0; i < numBuckets; ++i)
77:     {
78:         m_buckets[i].size = s_bucketSizes[i];
79:     }
80: }
81: 
82: /// <summary>
83: /// Sets up the heap allocator
84: /// </summary>
85: /// <param name="baseAddress">Base address of memory region (must be 16 bytes aligned)</param>
86: /// <param name="size">Size of memory region</param>
87: /// <param name="reserve">Free space reserved for handling of "Out of memory" messages.
88: /// (Allocate() returns nullptr, if reserve is 0 and memory region is full)</param>
89: void HeapAllocator::Setup(uintptr baseAddress, size_t size, size_t reserve)
90: {
91:     m_next = reinterpret_cast<uint8*>(baseAddress);
92:     m_limit = reinterpret_cast<uint8*>(baseAddress + size);
93:     m_reserve = reserve;
94: #if BAREMETAL_MEMORY_TRACING
95:     DumpStatus();
96: #endif
97: }
98: 
99: /// <summary>
100: /// Calculate and return the amount of free (unallocated) space in all buckets
101: /// @note Unused blocks on a free list do not count here.
102: /// </summary>
103: /// <returns>Free space of the memory region, which is not allocated by blocks.</returns>
104: size_t HeapAllocator::GetFreeSpace() const
105: {
106:     return m_limit - m_next;
107: }
108: 
109: /// <summary>
110: /// Allocate a block of memory
111: /// \note Resulting block is always 16 bytes aligned
112: /// \note If reserve in Setup() is non-zero, the system panics if heap is full.
113: /// </summary>
114: /// <param name="size">Block size to be allocated</param>
115: /// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
116: void* HeapAllocator::Allocate(size_t size)
117: {
118:     if (m_next == nullptr)
119:     {
120:         return nullptr;
121:     }
122: 
123:     HeapBlockBucket* bucket;
124:     for (bucket = m_buckets; bucket->size > 0; bucket++)
125:     {
126:         if (size <= bucket->size)
127:         {
128:             size = bucket->size;
129: 
130: #if BAREMETAL_MEMORY_TRACING
131:             if (++bucket->count > bucket->maxCount)
132:             {
133:                 bucket->maxCount = bucket->count;
134:             }
135:             ++bucket->totalAllocatedCount;
136:             bucket->totalAllocated += size;
137: 
138: #endif
139:             break;
140:         }
141:     }
142: 
143:     HeapBlockHeader* blockHeader{bucket->freeList};
144:     if ((bucket->size > 0) && (blockHeader != nullptr))
145:     {
146:         assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
147:         bucket->freeList = blockHeader->next;
148: #if BAREMETAL_MEMORY_TRACING_DETAIL
149:         TRACE_DEBUG("Reuse %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
150:         TRACE_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
151: #endif
152:     }
153:     else
154:     {
155:         blockHeader = reinterpret_cast<HeapBlockHeader*>(m_next);
156: 
157:         uint8* nextBlock = m_next;
158:         nextBlock += (sizeof(HeapBlockHeader) + size + HEAP_BLOCK_ALIGN - 1) & ~HEAP_ALIGN_MASK;
159: 
160:         if ((nextBlock <= m_next) || // may have wrapped
161:             (nextBlock > m_limit - m_reserve))
162:         {
163: #if BAREMETAL_MEMORY_TRACING
164:             DumpStatus();
165: #endif
166:             LOG_ERROR("%s: Out of memory", m_heapName);
167:             return nullptr;
168:         }
169: 
170:         m_next = nextBlock;
171: 
172:         blockHeader->magic = HEAP_BLOCK_MAGIC;
173:         blockHeader->size = static_cast<uint32>(size);
174: 
175: #if BAREMETAL_MEMORY_TRACING_DETAIL
176:         TRACE_DEBUG("Allocate %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
177:         TRACE_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
178: #endif
179:     }
180: 
181:     blockHeader->next = nullptr;
182: 
183:     void* result = blockHeader->data;
184:     assert((reinterpret_cast<uintptr>(result) & HEAP_ALIGN_MASK) == 0);
185: 
186:     return result;
187: }
188: 
189: /// <summary>
190: /// Reallocate block of memory
191: /// </summary>
192: /// <param name="block">Block of memory to be reallocated to the new size</param>
193: /// <param name="size">Block size to be allocated</param>
194: /// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
195: void* HeapAllocator::ReAllocate(void* block, size_t size)
196: {
197:     if (block == nullptr)
198:     {
199:         return Allocate(size);
200:     }
201: 
202:     if (size == 0)
203:     {
204:         Free(block);
205: 
206:         return nullptr;
207:     }
208: 
209:     const HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
210:     assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
211:     if (blockHeader->size >= size)
212:     {
213:         return block;
214:     }
215: 
216:     void* newBlock = Allocate(size);
217:     if (newBlock == nullptr)
218:     {
219:         return nullptr;
220:     }
221: 
222:     memcpy(newBlock, block, blockHeader->size);
223: 
224:     Free(block);
225: 
226:     return newBlock;
227: }
228: 
229: /// <summary>
230: /// Free (de-allocate) block of memory.
231: /// \note Memory space of blocks, which are bigger than the largest bucket size, cannot be returned to a free list and is lost.
232: /// </summary>
233: /// <param name="block">Memory block to be freed</param>
234: void HeapAllocator::Free(void* block)
235: {
236:     if (block == nullptr)
237:     {
238:         return;
239:     }
240: 
241:     HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
242:     assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
243: 
244:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; bucket++)
245:     {
246:         if (blockHeader->size == bucket->size)
247:         {
248:             blockHeader->next = bucket->freeList;
249:             bucket->freeList = blockHeader;
250: 
251: #if BAREMETAL_MEMORY_TRACING
252:             bucket->count--;
253:             ++bucket->totalFreedCount;
254:             bucket->totalFreed += blockHeader->size;
255: #if BAREMETAL_MEMORY_TRACING_DETAIL
256:             TRACE_DEBUG("Free %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
257:             TRACE_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
258: #endif
259: #endif
260: 
261:             return;
262:         }
263:     }
264: 
265: #if BAREMETAL_MEMORY_TRACING
266:     LOG_WARNING("%s: Trying to free large block (size %lu)", m_heapName, blockHeader->size);
267: #endif
268: }
269: 
270: #if BAREMETAL_MEMORY_TRACING
271: /// <summary>
272: /// Display the current status of the heap allocator
273: /// </summary>
274: void HeapAllocator::DumpStatus()
275: {
276:     TRACE_DEBUG("Heap allocator info:     %s", m_heapName);
277:     TRACE_DEBUG("Current #allocations:    %llu", GetCurrentAllocatedBlockCount());
278:     TRACE_DEBUG("Max #allocations:        %llu", GetMaxAllocatedBlockCount());
279:     TRACE_DEBUG("Current #allocated bytes:%llu", GetCurrentAllocationSize());
280:     TRACE_DEBUG("Total #allocated blocks: %llu", GetTotalAllocatedBlockCount());
281:     TRACE_DEBUG("Total #allocated bytes:  %llu", GetTotalAllocationSize());
282:     TRACE_DEBUG("Total #freed blocks:     %llu", GetTotalFreedBlockCount());
283:     TRACE_DEBUG("Total #freed bytes:      %llu", GetTotalFreeSize());
284: 
285: #if BAREMETAL_MEMORY_TRACING_DETAIL
286:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
287:     {
288:         TRACE_DEBUG("malloc(%lu): %lu blocks (max %lu) total alloc #blocks = %llu, #bytes = %llu, total free #blocks = %llu, #bytes = %llu", bucket->size, bucket->count, bucket->maxCount,
289:                   bucket->totalAllocatedCount, bucket->totalAllocated, bucket->totalFreedCount, bucket->totalFreed);
290:     }
291: #endif
292: }
293: 
294: /// <summary>
295: /// Returns the number of currently allocated memory blocks for this heap allocator.
296: /// </summary>
297: /// <returns>Number of currently allocated memory blocks for this heap allocator</returns>
298: uint64 HeapAllocator::GetCurrentAllocatedBlockCount()
299: {
300:     uint64 total{};
301:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
302:     {
303:         total += bucket->count;
304:     }
305:     return total;
306: }
307: 
308: /// <summary>
309: /// Returns the total size of currently allocated memory blocks for this heap allocator.
310: /// </summary>
311: /// <returns>Total size of currently allocated memory blocks for this heap allocator</returns>
312: uint64 HeapAllocator::GetCurrentAllocationSize()
313: {
314:     uint64 total{};
315:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
316:     {
317:         total += bucket->count * bucket->size;
318:     }
319:     return total;
320: }
321: 
322: /// <summary>
323: /// Returns the maximum number of currently allocated memory blocks for this heap allocator over time.
324: /// </summary>
325: /// <returns>Maximum number of currently allocated memory blocks for this heap allocator over time</returns>
326: uint64 HeapAllocator::GetMaxAllocatedBlockCount()
327: {
328:     uint64 total{};
329:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
330:     {
331:         total += bucket->maxCount;
332:     }
333:     return total;
334: }
335: 
336: /// <summary>
337: /// Returns the total number of allocated memory blocks for this heap allocator over time.
338: /// </summary>
339: /// <returns>Total number of allocated memory blocks for this heap allocator over time</returns>
340: uint64 HeapAllocator::GetTotalAllocatedBlockCount()
341: {
342:     uint64 total{};
343:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
344:     {
345:         total += bucket->totalAllocatedCount;
346:     }
347:     return total;
348: }
349: 
350: /// <summary>
351: /// Returns the total number of freed memory blocks for this heap allocator over time.
352: /// </summary>
353: /// <returns>Total number of freed memory blocks for this heap allocator over time</returns>
354: uint64 HeapAllocator::GetTotalFreedBlockCount()
355: {
356:     uint64 total{};
357:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
358:     {
359:         total += bucket->totalFreedCount;
360:     }
361:     return total;
362: }
363: 
364: /// <summary>
365: /// Returns the total size of allocated memory blocks for this heap allocator over time.
366: /// </summary>
367: /// <returns>Total size of allocated memory blocks for this heap allocator over time</returns>
368: uint64 HeapAllocator::GetTotalAllocationSize()
369: {
370:     uint64 total{};
371:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
372:     {
373:         total += bucket->totalAllocated;
374:     }
375:     return total;
376: }
377: 
378: /// <summary>
379: /// Returns the total size of freed memory blocks for this heap allocator over time.
380: /// </summary>
381: /// <returns>Total size of freed memory blocks for this heap allocator over time</returns>
382: uint64 HeapAllocator::GetTotalFreeSize()
383: {
384:     uint64 total{};
385:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
386:     {
387:         total += bucket->totalFreed;
388:     }
389:     return total;
390: }
391: 
392: #endif
```

- Line 55: We define the static member `s_bucketSizes` with the initializer `HEAP_BLOCK_BUCKET_SIZES` defined in `SysConfig.h` (unless overridden).
The default bucket sizes are:
  - 64 bytes (128 including memory block administration)
  - 1024 bytes (1088 including memory block administration)
  - 4096 bytes (4160 including memory block administration)
  - 16384 bytes (16448 including memory block administration)
  - 65536 bytes (65600 including memory block administration)
  - 262144 bytes (262208 including memory block administration)
  - 524288 bytes (524352 including memory block administration)
- Line 57-80: We implement the `HeapAllocator` constructor
  - Line 68: We clear the bucket administration array
  - Line 71-79: We set the sizes of buckets from the static member `s_bucketSizes`, making sure we stay within the maximum bucket count
- Line 82-97: We implement the method `Setup()` for this heap
  - Line 91: We set the pointer to the next available space to the start of the region
  - Line 92: We set the pointer to the end of available space to the end of the region
  - Line 93: We save the reserved space
  - Line 95: Only if `BAREMETAL_MEMORY_TRACING` is defined: We trace the current memory management status
- Line 99-107: We implement the method `GetFreeSpace()` which returns the available space left in this heap
- Line 109-187: We implement the method `Allocate()`
  - Line 118-121: We add a sanity check to verify that the memory manager is set up
  - Line 123-141: We find the smallest bucket that supports the requested size.
Only if `BAREMETAL_MEMORY_TRACING` is defined: We update the debug administration for this bucket
  - Line 143-152: We check whether there is a block within the selected bucket that was freed before, so we can re-use it.
Only if `BAREMETAL_MEMORY_TRACING_DETAIL` is defined: We trace re-use of the block
  - Line 154-179: If no block can be re-used, we allocate a new one
    - Line 155: We use the first available address in free space
    - Line 157-158: We calculate the next available address in free space (we add the size of the administration block and the requested size rounded up to the heap alignment)
    - Line 160-168: If the next available address would no longer fit in the available space, we fail by returning nullptr, after logging an error.
Only if `BAREMETAL_MEMORY_TRACING` is defined: We dump the allocation status
    - Line 170: We update the first available address in free space
    - Line 172-173: We fill the memory block administration.
Only if `BAREMETAL_MEMORY_TRACING_DETAIL` is defined: We trace allocation of the block
  - Line 181: We set the pointer to the next memory block
  - Line 183-184: We get the pointer to the actual memory block, and check for correct alignment
- Line 189-227: We implement the method `ReAllocate()`
  - Line 197-200: If the memory block is actually a nullptr, we simply allocate
  - Line 202-207: If the new size is 0, we free the block and return nullptr
  - Line 209-210: We get hold of the memory block administration and check for a valid magic number.
If not we return nullptr
  - Line 211-214: If the new size still fits in the current bucket, we simply return the block
  - Line 216-220: We allocate a new block of the new size (as now we know we need a larger block)
  - Line 222: We copy the contents of the current block into the new one (note that this only happens when we grow the block)
  - Line 224: We free the old memory block
- Line 229-268: We implement the method `Free()`
  - Line 236-239: If the memory block is nullptr, we simply return
  - Line 241-242: We get hold of the memory block administration and check for a valid magic number
  - Line 244-263: We look up the bucket this block belongs to and add it to the start of the list of free memory blocks for that bucket.
Only if `BAREMETAL_MEMORY_TRACING` is defined: We update the administration for this bucket
Only if `BAREMETAL_MEMORY_TRACING_DETAIL` is defined: We trace freeing of the block
  - Line 265-267: Note that when a block was allocated that does not fit in any bucket, that memory is lost.
Only if `BAREMETAL_MEMORY_TRACING` is defined: This case is also logged as a warning
- Line 271-292: We implement the method `DumpStatus()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 294-306: We implement the method `GetCurrentAllocatedBlockCount()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 308-320: We implement the method `GetCurrentAllocationSize()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 322-334: We implement the method `GetMaxAllocatedBlockCount()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 336-348: We implement the method `GetTotalAllocatedBlockCount()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 350-362: We implement the method `GetTotalFreedBlockCount()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 364-376: We implement the method `GetTotalAllocationSize()` if `BAREMETAL_MEMORY_TRACING` is defined
- Line 378-390: We implement the method `GetTotalFreeSize()` if `BAREMETAL_MEMORY_TRACING` is defined

### MemoryManager.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_MEMORYMANAGERH}

We will extend the `MemoryManager` class with methods to allocate and free memory, retrieve the amount of free heap left, and dump information on allocated and freed memory.
This also requires making the `MemoryManager` class instantiable. We again choose to make `MemoryManager` a singleton.
We will also add the definition for the `HeapType` now.

The `sysinit()` function will create the `MemoryManager` instance, as it will be needed very early in the startup process.

Update the file `code/libraries/baremetal/include/baremetal/MemoryManager.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryManager.h
...
42: #include "baremetal/Types.h"
43: #include "baremetal/HeapAllocator.h"
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
89:     size_t m_memSize;
90:     /// @brief Total memory size above 1Gb (up to 3 Gb boundary)
91:     size_t m_memSizeHigh;
92: 
93:     /// @brief Heap allocator for low memory (below 1Gb)
94:     HeapAllocator m_heapLow;
95: #if BAREMETAL_RPI_TARGET >= 4
96:     /// @brief Heap allocator for low memory (above 1Gb)
97:     HeapAllocator m_heapHigh;
98: #endif
99:     MemoryManager();
100: 
101: public:
102:     static uintptr GetCoherentPage(CoherentPageSlot slot);
103: 
104:     static void* HeapAllocate(size_t size, HeapType type);
105:     static void* HeapReAllocate(void* block, size_t size);
106:     static void HeapFree(void* block);
107:     static size_t GetHeapFreeSpace(HeapType type);
108:     static void DumpStatus();
109: };
110: 
111: MemoryManager& GetMemoryManager();
112: 
113: } // namespace baremetal
```

- Line 43: We include `HeapAllocator.h`
- Line 57-70: We declare the enum `HeapType` which signifies the type of heap to be used for allocation
- Line 81-85: We declare the friend function `GetMemoryManager()` which instantiates and returns the singleton MemoryManager instance (which will be created in the `sysinit()` function in `System.cpp`)
- Line 88-89: The member `m_memSize` signifies the amount of memory available for the ARM CPU up to 1 Gb in bytes.
This holds the total ARM memory available, and therefore does not take into account the reservations we need to make for paging, and the part used for other means before the heap.
- Line 90-91: The member `m_memSizeHigh` signifies the amount of memory above 1 Gb, up the maximum of 3 Gb
- Line 93-94: The member `m_heapLow` is the low heap
- Line 96-97: The member `m_heapHigh` is the high heap, only defined for Raspberry Pi 4 and higher
- Line 99: We declare the private constructor for `MemoryManager`. This can only be called by `GetMemoryManager()`
- Line 104: We declare the method `HeapAllocate()`, which allocates memory of the requested size, in the requested heap
- Line 105: We declare the method `HeapReAllocate()`, which re-allocates a memory block to the requested size
- Line 106: We declare the method `HeapFree()`, which frees a memory block
- Line 107: We declare the method `GetHeapFreeSpace()`, which returns available memory in all heaps
- Line 108: We declare the method `DumpStatus()`, which log information on all heaps concerning allocated and freed memory blocks
- Line 111: We declare the method `GetMemoryManager()`, which initiates the singleton `MemoryManager`

### MemoryManager.cpp {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_MEMORYMANAGERCPP}

We will implement the added methods for `MemoryManager`.

Update the file `code/libraries/baremetal/src/MemoryManager.cpp`

```cpp
File: code/libraries/baremetal/src/MemoryManager.cpp
...
40: #include "baremetal/MemoryManager.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/MachineInfo.h"
45: #include "baremetal/SysConfig.h"
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
63:     , m_heapLow{"heaplow"}
64: #if BAREMETAL_RPI_TARGET >= 4
65:     , m_heapHigh{"heaphigh"}
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
129:     case HeapType::LOW:
130:         return memoryManager.m_heapLow.Allocate(size);
131:     case HeapType::HIGH:
132:         return memoryManager.m_heapHigh.Allocate(size);
133:     case HeapType::ANY:
134:         return (block = memoryManager.m_heapHigh.Allocate(size)) != nullptr ? block : memoryManager.m_heapLow.Allocate(size);
135:     default:
136:         return nullptr;
137:     }
138: #else
139:     switch (type)
140:     {
141:     case HeapType::LOW:
142:     case HeapType::ANY:
143:         return memoryManager.m_heapLow.Allocate(size);
144:     default:
145:         return nullptr;
146:     }
147: #endif
148: }
149: 
150: /// <summary>
151: /// Reallocate block of memory
152: /// </summary>
153: /// <param name="block">Block of memory to be reallocated to the new size</param>
154: /// <param name="size">Block size to be allocated</param>
155: /// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
156: void* MemoryManager::HeapReAllocate(void* block, size_t size) // block may be nullptr
157: {
158:     auto& memoryManager = GetMemoryManager();
159: #if BAREMETAL_RPI_TARGET >= 4
160:     if (reinterpret_cast<uintptr>(block) < MEM_HIGHMEM_START)
161:     {
162:         return memoryManager.m_heapLow.ReAllocate(block, size);
163:     }
164:     return memoryManager.m_heapHigh.ReAllocate(block, size);
165: #else
166:     return memoryManager.m_heapLow.ReAllocate(block, size);
167: #endif
168: }
169: 
170: /// <summary>
171: /// Free (de-allocate) block of memory.
172: /// </summary>
173: /// <param name="block">Memory block to be freed</param>
174: void MemoryManager::HeapFree(void* block)
175: {
176:     auto& memoryManager = GetMemoryManager();
177: #if BAREMETAL_RPI_TARGET >= 4
178:     if (reinterpret_cast<uintptr>(block) < MEM_HIGHMEM_START)
179:     {
180:         memoryManager.m_heapLow.Free(block);
181:     }
182:     else
183:     {
184:         memoryManager.m_heapHigh.Free(block);
185:     }
186: #else
187:     memoryManager.m_heapLow.Free(block);
188: #endif
189: }
190: 
191: /// <summary>
192: /// Calculate and return the amount of free (unallocated) space for the specified heap
193: /// </summary>
194: /// <param name="type">Heap to return free space for</param>
195: /// <returns>Free space of the memory region, which is not allocated by blocks.</returns>
196: size_t MemoryManager::GetHeapFreeSpace(HeapType type)
197: {
198:     auto& memoryManager = GetMemoryManager();
199: #if BAREMETAL_RPI_TARGET >= 4
200:     switch (type)
201:     {
202:     case HeapType::LOW:
203:         return memoryManager.m_heapLow.GetFreeSpace();
204:     case HeapType::HIGH:
205:         return memoryManager.m_heapHigh.GetFreeSpace();
206:     case HeapType::ANY:
207:         return memoryManager.m_heapLow.GetFreeSpace() + memoryManager.m_heapHigh.GetFreeSpace();
208:     default:
209:         return 0;
210:     }
211: #else
212:     switch (type)
213:     {
214:     case HeapType::LOW:
215:     case HeapType::ANY:
216:         return memoryManager.m_heapLow.GetFreeSpace();
217:     default:
218:         return 0;
219:     }
220: #endif
221: }
222: 
223: /// <summary>
224: /// Display the current status of all heap allocators
225: /// </summary>
226: void MemoryManager::DumpStatus()
227: {
228: #if BAREMETAL_MEMORY_TRACING
229:     auto& memoryManager = GetMemoryManager();
230:     LOG_DEBUG("Low heap:");
231:     memoryManager.m_heapLow.DumpStatus();
232: #if BAREMETAL_RPI_TARGET >= 4
233:     LOG_DEBUG("High heap:");
234:     memoryManager.m_heapHigh.DumpStatus();
235: #endif
236: #endif
237: }
238: 
239: /// <summary>
240: /// Construct the singleton MemoryManager instance if needed, and return a reference to the instance
241: /// </summary>
242: /// <returns>Reference to the singleton MemoryManager</returns>
243: MemoryManager& baremetal::GetMemoryManager()
244: {
245:     static MemoryManager instance;
246:     return instance;
247: }
```

- Line 42-45: We need to include `Assert.h`, `Logger.h` and `MachineInfo.h`
- Line 55-99: We implement the constructor for the `MemoryManager` class
  - Line 68-76: We get the `MachineInfo` instance and retrieve the ARM CPU base memory address and allocated size.
If the returned size is 0, we default to the full memory (1 Gb) with 64 Mb allocated to the VideoCore
  - Line 78-79: We perform a sanity check, and assign the low heap memory size
  - Line 81-82: We subtract the low heap start as well as the space reserved for paging, and set up the low heap (we keep a reserve of 256 Kb)
  - Line 84-98: In case of Raspberry Pi 4 or higher, we set up the high heap
    - Line 85: We retrieve the total RAM size (in Mb)
    - Line 86-97: If larger than 1Gb, we determine the high end of the memory and limit it to 3Gb, we then subtract the first 1 Gb and set up the high heap
- Line 115-144: We implement the method `HeapAllocate()`
  - Line 123: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 125-137: In case of Raspberry Pi 4 or higher, we allow for high heap selection, and allocate accordingly
If any heap was selected, we first attempt to claim from the high heap, if that fails we try the low heap
  - Line 139-146: In case of Raspberry Pi 3, we only allow the low heap
- Line 150-168: We implement the method `HeapReAllocate()`
  - Line 158: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 160-164: In case of Raspberry Pi 4 or higher, we check which heap to access, and call `ReAllocate()` on it
  - Line 166: In case of Raspberry Pi 3, we call `ReAllocate()` on the low heap
- Line 170-189: We implement the method `HeapFree()`
  - Line 176: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 178-185: In case of Raspberry Pi 4 or higher, we check which heap to access, and call `Free()` on it
  - Line 187: In case of Raspberry Pi 3, we call `Free()` on the low heap
- Line 191-221: We implement the method `GetHeapFreeSpace()`
  - Line 198: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 200-210: In case of Raspberry Pi 4 or higher, depending on the selected heap, we call `GetFreeSpace()` on it.
If any heap is selected, the freespace for both heaps is added together
  - Line 212-219: In case of Raspberry Pi 3, we call `GetFreeSpace()` on the low heap
- Line 223-237: We implement the method `DumpStatus()`
  - Line 229: As this is a static method, we retrieve the `MemoryManager` instance
  - Line 230-231: We call `DumpStatus()` on the low heap
  - Line 233-234: In case of Raspberry Pi 4 or higher, we call `DumpStatus()` on the high heap.
  - All of this only when `BAREMETAL_MEMORY_TRACING` is defined
- Line 239-247: We implement the function `GetMemoryManager()`

### System.cpp {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_SYSTEMCPP}

Although not urgently needed yet, we'll need the singleton `MemoryManagement` instance in the `sysinit()` function soon, in order to set up memory mapping.
So let's go ahead and instantiate the `MemoryManager` there.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
42: #include "stdlib/Util.h"
43: #include "baremetal/ARMInstructions.h"
44: #include "baremetal/BCMRegisters.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/MemoryAccess.h"
47: #include "baremetal/MemoryManager.h"
48: #include "baremetal/SysConfig.h"
49: #include "baremetal/Timer.h"
50: #include "baremetal/UART0.h"
51: #include "baremetal/UART1.h"
52: #include "baremetal/Version.h"
...
160: void sysinit()
161: {
162:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
163:     EnableIRQs(); // go to TASK_LEVEL
164: 
165:     // clear BSS
166:     extern unsigned char __bss_start;
167:     extern unsigned char __bss_end;
168:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
169: 
170:     // halt, if KERNEL_MAX_SIZE is not properly set
171:     // cannot inform the user here
172:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
173:     {
174:         GetSystem().Halt();
175:     }
176: 
177:     // We need to create a memory system first, as core 0 needs to be the first to enable the MMU.
178:     // Will trace, however, there is no instantiated device connected to console yet.
179:     GetMemoryManager();
180: 
181:     // Call constructors of static objects
182:     extern void (*__init_start)(void);
183:     extern void (*__init_end)(void);
184:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
185:     {
186:         (**func)();
187:     }
188: 
189:     Device* logDevice{};
190: #if defined(BAREMETAL_CONSOLE_UART0)
191:     auto& uart = GetUART0();
192:     uart.Initialize(115200);
193:     uart.WriteString("\nSetting up UART0\n");
194:     logDevice = &uart;
195: #elif defined(BAREMETAL_CONSOLE_UART1)
196:     auto& uart = GetUART1();
197:     uart.Initialize(115200);
198:     uart.WriteString("\nSetting up UART1\n");
199:     logDevice = &uart;
200: #endif
201:     GetConsole().AssignDevice(logDevice);
202:     GetLogger().Initialize();
203: 
204:     LOG_INFO("Starting up");
205: 
206:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
207:     {
208: #if BAREMETAL_MEMORY_TRACING
209:         GetMemoryManager().DumpStatus();
210: #endif
211:         GetSystem().Reboot();
212:     }
213: 
214: #if BAREMETAL_MEMORY_TRACING
215:     GetMemoryManager().DumpStatus();
216: #endif
217:     GetSystem().Halt();
218: }
```

- Line 47: We need to include `MemoryManager.h`
- Line 179: We call `GetMemoryManager()` to instantiate the `MemoryManager` singleton instance.
- Line 208-210: If tracing is requested, we call the `MemoryManager` method `DumpStatus()`
- Line 214-216: If tracing is requested, we call the `MemoryManager` method `DumpStatus()`

### Application code {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_APPLICATION_CODE}

Let's use the memory allocation and see how it behaves.
We've set `BAREMETAL_TRACE_MEMORY` and `BAREMETAL_TRACE_MEMORY_DETAIL` to `ON`, so we will get quite a bit of detail on memory allocations.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/BCMRegisters.h"
4: #include "baremetal/Console.h"
5: #include "baremetal/Logger.h"
6: #include "baremetal/Mailbox.h"
7: #include "baremetal/MemoryManager.h"
8: #include "baremetal/RPIProperties.h"
9: #include "baremetal/SysConfig.h"
10: #include "baremetal/Serialization.h"
11: #include "baremetal/System.h"
12: #include "baremetal/Timer.h"
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
48:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
49:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
50:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
51: 
52:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
53: }
```

- Line 21: We set the log / trace level to debug
- Line 23: We get the singleton MemoryManager instance
- Line 24-26: We trace the free space for each heap and both heaps (`HeapType::ANY`)
- Line 28-29: We allocate 4 bytes of memory in the low heap and log the resulting memory address
- Line 30: We dump the status of the memory manager
- Line 32-33: We free the memory again, and again dump the status of the memory manager
- Line 48-50: We trace the free space again for each heap and both heaps (`HeapType::ANY`)

### Configuring, building and debugging {#TUTORIAL_15_MEMORY_MANAGEMENT_SETTING_UP_MEMORY_MANAGEMENT___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will allocate memory, and show that one block of memory is allocated and freed again:

\todo

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:91)
Info   Starting up (System:204)
Debug  main (main.cpp:24) Heap space available: 967835648 bytes
Debug  main (main.cpp:25) High heap space available: 2147483648 bytes
Debug  main (main.cpp:26) DMA heap space available: 3115319296 bytes
Debug  Allocate (HeapAllocator.cpp:176) Allocate 64 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:177) Current #allocations = 1, max #allocations = 1
Info   Allocated block 900040 (main:29)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:64
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     1
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 1, #bytes = 64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Info   Wait 5 seconds (main:35)
Press r to reboot, h to halt, p to fail assertion and panic
rInfo   Heap space available: 967835520 bytes (main:48)
Info   High heap space available: 2147483648 bytes (main:49)
Info   DMA heap space available: 3115319168 bytes (main:50)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     1
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 1, #bytes = 64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Info   Reboot (System:136)
```

We see that even though we only allocate 4 bytes, as the minimum bucket size is 64 bytes, that amount of memory is allocated.
Actually, due to the administration block, 64 + 64 bytes is used.
We can see that when we compare the free memory left before and after allocation and freeing.
The block that was allocated, will be in the freed list, but the `GetHeapFreeSpace()` call does not show this as it is still reserved in the bucket.

Before:
```text
Debug  main (main.cpp:24) Heap space available: 967835648 bytes
Debug  main (main.cpp:25) High heap space available: 2147483648 bytes
Debug  main (main.cpp:26) DMA heap space available: 3115319296 bytes
```

After:
```text
Info   Heap space available: 967835520 bytes (main:48)
Info   High heap space available: 2147483648 bytes (main:49)
Info   DMA heap space available: 3115319168 bytes (main:50)
```

So we see that 967835648 - 967835520 = 128 bytes was still reserved in the bucket.

## Supporting C/C++ memory allocation - step 2 {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2}

In order to use memory allocation in the way we are used to, using either `malloc()`, `calloc()`, `realloc()` and `free()` in C or the `new` and `delete` operators in C++, we need to add them.
So we'll add `New.h` and `New.cpp` to add functions for C++, and add `Malloc.h` and `Malloc.cpp` for the C variants.
We need to add this code to the `baremetal` library, as it used the `MemoryManager` class.

### Malloc.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2_MALLOCH}

We will declares the memory allocation functions for C.

Create the file `code/libraries/baremetal/include/baremetal/Malloc.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Malloc.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Malloc.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Memory allocation functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
42: #include "stdlib/Types.h"
43:
44: /// @file
45: /// Standard C library memory allocation functions
46:
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50:
51: void* malloc(size_t size);
52: void* calloc(size_t num, size_t size);
53: void* realloc(void* ptr, size_t new_size);
54: void free(void* ptr);
55:
56: #ifdef __cplusplus
57: }
58: #endif
```

Line 51-54: We add the function declarations for `malloc()`, `calloc()`, `realloc()` and `free()`

### Malloc.cpp {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2_MALLOCCPP}

We will implement the added functions.

Create the file `code/libraries/baremetal/src/Malloc.cpp`

```cpp
File: code/libraries/baremetal/src/Malloc.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Malloc.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Memory allocation functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
40: #include "baremetal/Malloc.h"
41:
42: #include "baremetal/MemoryManager.h"
43: #include "baremetal/SysConfig.h"
44:
45: /// @file
46: /// Standard C memory allocation functions implementation
47:
48: /// <summary>
49: /// Allocates a block of memory of the desired size.
50: /// </summary>
51: /// <param name="size">The desired size of the memory block</param>
52: /// <returns></returns>
53: void* malloc(size_t size)
54: {
55:     return baremetal::MemoryManager::HeapAllocate(size, HEAP_DEFAULT_MALLOC);
56: }
57:
58: /// <summary>
59: /// Allocates a contiguous block of memory for the desired number of cells of the desired size each.
60: ///
61: /// The memory allocated is num x size bytes
62: /// </summary>
63: /// <param name="num">Number of cells to allocate memory for</param>
64: /// <param name="size">Size of each cell</param>
65: /// <returns></returns>
66: void* calloc(size_t num, size_t size)
67: {
68:     return malloc(num * size);
69: }
70:
71: /// <summary>
72: /// Re-allocates memory previously allocated with malloc() or calloc() to a new size
73: /// </summary>
74: /// <param name="ptr">Pointer to memory block to be re-allocated</param>
75: /// <param name="new_size">The desired new size of the memory block</param>
76: /// <returns></returns>
77: void* realloc(void* ptr, size_t new_size)
78: {
79:     return baremetal::MemoryManager::HeapReAllocate(ptr, new_size);
80: }
81:
82: /// <summary>
83: /// Frees memory previously allocated with malloc() or calloc()
84: /// </summary>
85: /// <param name="ptr">Pointer to memory block to be freed</param>
86: void free(void* ptr)
87: {
88:     baremetal::MemoryManager::HeapFree(ptr);
89: }
```

- Line 42: We need to include `MemoryManager.h` for declaration of the `MemoryManager` class
- Line 43: We need to include `SysConfig.h` for declaration of the default heap type
- Line 48-56: We implement `malloc()` by calling `MemoryManager::HeapAlloc()`
We use any heap, so on Raspberry Pi 4 and higher in case the high heap is exhausted, we will use the low heap
- Line 58-69: We implement `calloc()` by calling `malloc()` with num x size
- Line 71-80: We implement `realloc()` by calling `MemoryManager::HeapReAlloc()`
- Line 82-89: We implement `free()` by calling `MemoryManager::HeapFree()`

### New.h {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2_NEWH}

We will add the operators `new` and `delete`. We will also add some placement versions, so we can specify the heap to use when allocating.
This file will replace the `New.h` in `stdlib`, as we need access to the heap, which is defined in `baremetal`.

Move the file `code/libraries/stdlib/include/stdlib/New.h` to `code/libraries/baremetal/include/baremetal/New.h`.
Update the file `code/libraries/baremetal/include/baremetal/New.h`

```cpp
File: code/libraries/baremetal/include/baremetal/New.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : New.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Generic memory allocation functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
42: /// @file
43: /// Basic memory allocation functions
44: 
45: #include "baremetal/MemoryManager.h"
46: #include "stdlib/Types.h"
47: 
48: void* operator new(size_t size, HeapType type);
49: void* operator new[](size_t size, HeapType type);
50: void* operator new(size_t size, void* address);
51: void* operator new[](size_t size, void* address);
52: void* operator new(size_t size);
53: void* operator new[](size_t size);
54: 
55: void operator delete(void* block) noexcept;
56: void operator delete[](void* address) noexcept;
57: void operator delete(void* block, size_t size) noexcept;
58: void operator delete[](void* address, size_t size) noexcept;
```

- Line 48: We declare the new operator using a heap specification.
We can then use e.g. `new (HeapType::LOW) X` to allocate an instance of class X in the low heap.
- Line 49: We declare the array new[] operator using a heap specification
- Line 50: We declare the new operator for placement using a pointer. This will place the instance at the specified location
- Line 51: We declare the array new[] operator for placement using a pointer. This will place the instance at the specified location
- Line 52: We declare the standard new operator
- Line 53: We declare the standard new[] operator
- Line 56: We declare a array delete[] operator
- Line 58: We declare a array delete[] operator with specified size

### New.cpp {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2_NEWCPP}

We will implement the `new` and `delete` operators.
This file will replace the `New.cpp` in `stdlib`, as we need access to the heap, which is defined in `baremetal`.

Move the file `code/libraries/stdlib/src/New.cpp` to `code/libraries/baremetal/src/New.cpp`.
Update the file `code/libraries/baremetal/src/New.cpp`

```cpp
File: code/libraries/baremetal/src/New.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : New.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Generic memory allocation functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
40: #include "baremetal/New.h"
41: 
42: #include "baremetal/SysConfig.h"
43: 
44: /// @file
45: /// Basic memory allocation functions implementation
46: 
47: using namespace baremetal;
48: 
49: /// <summary>
50: /// Class specific placement allocation for single value.
51: /// </summary>
52: /// <param name="size">Size of block to allocate in bytes</param>
53: /// <param name="type">Heap type to allocate from</param>
54: /// <returns>Pointer to allocated block of memory or nullptr</returns>
55: void* operator new(size_t size, HeapType type)
56: {
57:     return MemoryManager::HeapAllocate(size, type);
58: }
59: 
60: /// <summary>
61: /// Class specific placement allocation for array.
62: /// </summary>
63: /// <param name="size">Size of block to allocate in bytes</param>
64: /// <param name="type">Heap type to allocate from</param>
65: /// <returns>Pointer to allocated block of memory or nullptr</returns>
66: void* operator new[](size_t size, HeapType type)
67: {
68:     return MemoryManager::HeapAllocate(size, type);
69: }
70: 
71: /// <summary>
72: /// Non allocating placement allocation for single value.
73: /// </summary>
74: /// <param name="size">Size of block to allocate in bytes</param>
75: /// <param name="address">Address to be used</param>
76: /// <returns>Pointer to block of memory</returns>
77: void* operator new(size_t size, void* address)
78: {
79:     return address;
80: }
81: 
82: /// <summary>
83: /// Non allocating placement allocation for array.
84: /// </summary>
85: /// <param name="size">Size of block to allocate in bytes</param>
86: /// <param name="address">Address to be used</param>
87: /// <returns>Pointer to block of memory</returns>
88: void* operator new[](size_t size, void* address)
89: {
90:     return address;
91: }
92: 
93: /// <summary>
94: /// Standard allocation for single value.
95: ///
96: /// Allocates from default heap.
97: /// </summary>
98: /// <param name="size">Size of block to allocate in bytes</param>
99: /// <returns>Pointer to allocated block of memory or nullptr</returns>
100: void* operator new(size_t size)
101: {
102:     return MemoryManager::HeapAllocate(size, HEAP_DEFAULT_NEW);
103: }
104: 
105: /// <summary>
106: /// Standard allocation for array.
107: ///
108: /// Allocates from default heap.
109: /// </summary>
110: /// <param name="size">Size of block to allocate in bytes</param>
111: /// <returns>Pointer to allocated block of memory or nullptr</returns>
112: void* operator new[](size_t size)
113: {
114:     return MemoryManager::HeapAllocate(size, HEAP_DEFAULT_NEW);
115: }
116: 
117: /// <summary>
118: /// Standard de-allocation for single value.
119: /// </summary>
120: /// <param name="address">Block to free</param>
121: void operator delete(void* address) noexcept
122: {
123:     MemoryManager::HeapFree(address);
124: }
125: 
126: /// <summary>
127: /// Standard de-allocation for array.
128: /// </summary>
129: /// <param name="address">Block to free</param>
130: void operator delete[](void* address) noexcept
131: {
132:     MemoryManager::HeapFree(address);
133: }
134: 
135: /// <summary>
136: /// Standard de-allocation with size for single value.
137: /// </summary>
138: /// <param name="address">Block to free</param>
139: /// <param name="size">Size of block to free</param>
140: void operator delete(void* address, size_t /*size*/) noexcept
141: {
142:     MemoryManager::HeapFree(address);
143: }
144: 
145: /// <summary>
146: /// Standard de-allocation for array.
147: /// </summary>
148: /// <param name="address">Block to free</param>
149: /// <param name="size">Size of block to free</param>
150: void operator delete[](void* address, size_t /*size*/) noexcept
151: {
152:     MemoryManager::HeapFree(address);
153: }
```

- Line 42: We need to include `SysConfig.h` for the definitions on the heap types
- Line 49-58: We implement the `new` operator with heap specification
- Line 60-69: We implement the `new[]` operator with heap specification
- Line 71-80: We implement the placement `new` operator.
As you can see we simply return the specified address
- Line 82-91: We implement the placement `new[]` operator.
As you can see we simply return the specified address
- Line 93-103: We implement the standard `new` operator.
We use the heap specification `HEAP_DEFAULT_NEW`, which by default is specified as `HeapType::LOW`
- Line 105-115: We implement the standard `new[]` operator.
We use the heap specification `HEAP_DEFAULT_NEW`, which by default is specified as `HeapType::LOW`
- Line 117-124: We re-implement the standard `delete` operator using a call to `MemoryManager::HeapFree()`
- Line 126-133: We implement the standard `delete[]` operator
- Line 135-143: We re-implement the `delete` operator with size using a call to `MemoryManager::HeapFree()`
- Line 144-153: We implement the `delete[]` operator with size

### Application code {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2_APPLICATION_CODE}

Let's use the `malloc()` function and `new` operator.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/BCMRegisters.h"
4: #include "baremetal/Console.h"
5: #include "baremetal/Logger.h"
6: #include "baremetal/Mailbox.h"
7: #include "baremetal/Malloc.h"
8: #include "baremetal/MemoryManager.h"
9: #include "baremetal/New.h"
10: #include "baremetal/RPIProperties.h"
11: #include "baremetal/SysConfig.h"
12: #include "baremetal/Serialization.h"
13: #include "baremetal/System.h"
14: #include "baremetal/Timer.h"
15: 
16: LOG_MODULE("main");
17: 
18: using namespace baremetal;
19: 
20: int main()
21: {
22:     auto& console = GetConsole();
23:     GetLogger().SetLogLevel(LogSeverity::Debug);
24:     LOG_DEBUG("Hello World!");
25: 
26:     MemoryManager& memoryManager = GetMemoryManager();
27:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
28:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
29:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
30: 
31:     auto ptr = memoryManager.HeapAllocate(4, HeapType::LOW);
32:     LOG_INFO("Allocated block %p", ptr);
33:     memoryManager.DumpStatus();
34: 
35:     memoryManager.HeapFree(ptr);
36:     memoryManager.DumpStatus();
37: 
38:     class X {};
39: 
40:     X* x = new (HeapType::LOW) X;
41:     LOG_INFO("Allocated block %p", x);
42:     memoryManager.DumpStatus();
43:     delete x;
44:     memoryManager.DumpStatus();
45: 
46:     void* p = malloc(256);
47:     LOG_INFO("Allocated block %p", p);
48:     memoryManager.DumpStatus();
49:     free(p);
50:     memoryManager.DumpStatus();
51: 
52:     LOG_INFO("Wait 5 seconds");
53:     Timer::WaitMilliSeconds(5000);
54: 
55:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
56:     char ch{};
57:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
58:     {
59:         ch = console.ReadChar();
60:         console.WriteChar(ch);
61:     }
62:     if (ch == 'p')
63:         assert(false);
64: 
65:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
66: }
```

- Line 7: We include `Malloc.h` to declare the placement standard C memory allocation functions
- Line 9: We include `New.h` to declare the placement `new` operators
- Line 37: We declare a class `X`
- Line 39-40: We allocate a class `X` instance in the low heap and log the resulting memory address
- Line 41: We log the status of the memory manager
- Line 42-43: We delete the object again, and log the status of the memory manager
- Line 45-46: We allocate a memory block using `malloc()` in any heap and log the resulting memory address
- Line 47: We log the status of the memory manager
- Line 48-49: We free the memory again, and log the status of the memory manager

### Configuring, building and debugging {#TUTORIAL_15_MEMORY_MANAGEMENT_SUPPORTING_CC___MEMORY_ALLOCATION___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will allocate memory, and show that one block of memory is allocated and freed again:

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:91)
Info   Starting up (System:204)
Debug  Hello World! (main:24)
Info   Heap space available: 967835648 bytes (main:27)
Info   High heap space available: 2147483648 bytes (main:28)
Info   DMA heap space available: 3115319296 bytes (main:29)
Debug  Allocate (HeapAllocator.cpp:176) Allocate 64 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:177) Current #allocations = 1, max #allocations = 1
Info   Allocated block 900040 (main:32)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:64
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     1
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 64, total free #blocks = 1, #bytes = 64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Info   Allocated block 900040 (main:41)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:64
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 2
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  128
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     1
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 1 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 1, #bytes = 64
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 2
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  128
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     2
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      128
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  Allocate (HeapAllocator.cpp:176) Allocate 1024 bytes at 00000000009000C0
Debug  Allocate (HeapAllocator.cpp:177) Current #allocations = 1, max #allocations = 1
Info   Allocated block 9000C0 (main:47)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        2
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:1024
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 3
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  1152
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     2
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      128
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 1 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009000C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        2
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 3
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  1152
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     3
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      1152
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 1, #bytes = 1024
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Info   Wait 5 seconds (main:52)
Press r to reboot, h to halt, p to fail assertion and panic
rDebug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        2
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 3
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  1152
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     3
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      1152
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 1) total alloc #blocks = 2, #bytes = 128, total free #blocks = 2, #bytes = 128
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 1) total alloc #blocks = 1, #bytes = 1024, total free #blocks = 1, #bytes = 1024
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:287) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Info   Reboot (System:136)
```

You can see that we allocated three blocks of memory and freed them again. Two were of size 64, one was of size 1024, due to bucket sizes.
Notice also that the second allocation re-uses the memory allocated in the first allocation, as it was placed in the bucket's free list.

In order to stop overloading the console with memory debug information, let's set the detail to `OFF`

## Switching off debugging for memory management - step 3 {#TUTORIAL_15_MEMORY_MANAGEMENT_SWITCHING_OFF_DEBUGGING_FOR_MEMORY_MANAGEMENT___STEP_3}

### Main CMake file {#TUTORIAL_15_MEMORY_MANAGEMENT_SWITCHING_OFF_DEBUGGING_FOR_MEMORY_MANAGEMENT___STEP_3_MAIN_CMAKE_FILE}

```cmake
File: CMakeLists.txt
...
69: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" ON)
70: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
...
```

### Configuring, building and debugging {#TUTORIAL_15_MEMORY_MANAGEMENT_SWITCHING_OFF_DEBUGGING_FOR_MEMORY_MANAGEMENT___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will still show quite some information, due to the calls to `DumpStatus()`, but a bit less.
Normally we will not be calling `DumpStatus()` on the heap for every allocation.

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:91)
Info   Starting up (System:204)
Debug  Hello World! (main:24)
Info   Heap space available: 967835648 bytes (main:27)
Info   High heap space available: 2147483648 bytes (main:28)
Info   DMA heap space available: 3115319296 bytes (main:29)
Info   Allocated block 900040 (main:32)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:64
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 1
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  64
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     1
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      64
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Info   Allocated block 900040 (main:41)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:64
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 2
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  128
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     1
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      64
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        1
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 2
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  128
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     2
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      128
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Info   Allocated block 9000C0 (main:47)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    1
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        2
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:1024
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 3
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  1152
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     2
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      128
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        2
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 3
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  1152
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     3
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      1152
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Info   Wait 5 seconds (main:52)
Press r to reboot, h to halt, p to fail assertion and panic
rDebug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        2
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 3
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  1152
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     3
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      1152
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Info   Reboot (System:136)
```

Next: [16-string](16-string.md)
