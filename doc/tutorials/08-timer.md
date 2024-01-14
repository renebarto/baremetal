# Tutorial 08: Timer {#TUTORIAL_08}

Contents:
- [Tutorial information](##Tutorial-information)
  - [Tutorial results](###Tutorial-results)
- [Using the system timer](##Using-the-system-timer)
  - [Timer.h](###Timer.h)
  - [Timer.cpp](###Timer.cpp)
  - [ArmInstructions.h](###ArmInstructions.h)
  - [BCMRegisters.h](###BCMRegisters.h)
  - [System.cpp](###System.cpp)
  - [Update project configuration - Step 3](###Update-project-configuration-Step-3)
  - [Configuring, building and debugging - Step 3](###Configuring-building-and-debugging-Step-3)

## Tutorial information

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/08-timer`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-08.a`
- an application `output/Debug/bin/08-timer.elf`
- an image in `deploy/Debug/08-timer-image`

## Using the system timer

In the previous tutorial we added some delay before halting or rebooting the system. The delay was specified as the number of NOP instructions to execute.
That is not a very accurate way of specifying a delay. It would be better if we could use a hardware timer.

That is exactly what we will do in this tutorial. There are two kinds of timers:
- A free running timer in the ARM processor
- A more advanced timer in the SoC that can generate interrupts when timed out

We will be implementing both, but using the second.

### Timer.h

First, we will add some methods to the `Timer` class.
Update the file `code/libraries/baremetal/include/baremetal/Timer.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
File: f:\Projects\Private\baremetal.tmp\code\libraries\baremetal\include\baremetal\Timer.h
44: namespace baremetal {
45: 
46: class IMemoryAccess;
47: 
48: // Timer class. For now only contains busy waiting methods
49: // Note that this class is created as a singleton, using the GetTimer function.
50: class Timer
51: {
52:     friend Timer& GetTimer();
53: 
54: private:
55:     IMemoryAccess& m_memoryAccess;
56: 
57:     // Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
58:     Timer();
59: 
60: public:
61:     // Constructs a specialized Timer instance with a custom IMemoryAccess instance. This is intended for testing.
62:     Timer(IMemoryAccess& memoryAccess);
63: 
64:     // Wait for specified number of NOP statements. Busy wait
65:     static void WaitCycles(uint32 numCycles);
66: 
67: #if defined(USE_PHYSICAL_COUNTER)
68:     uint64 GetSystemTimer();
69: #endif
70: 
71:     // Wait for msec milliseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical
72:     // counter). Busy wait
73:     static void WaitMilliSeconds(uint64 msec);
74:     // Wait for usec microseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical
75:     // counter). Busy wait
76:     static void WaitMicroSeconds(uint64 usec);
77: };
78: 
79: // Retrieves the singleton Timer instance. It is created in the first call to this function.
80: Timer& GetTimer();
81: 
82: } // namespace baremetal
```

- Line 46: We forward declare IMemoryAccess, as this class will need it for memory access
- Line 52: We declare the friend function `GetTimer()`, much like `GetUART1()` and `GetSystem()` before. This function will return the singleton instance of the Timer
- Line 55: We add a class variable for the memory access
- Line 58: We declare a private constructor (called by `GetTimer()`)
- Line 62: We declare a public constructor for testing
- Line 67-69: We add a method to request the timer count in case we use the BCM2835 (SoC) timer. As this method needs memory access, we use the IMemoryAccess interface, so this method needs to have an instance
- Line 73: We add a method to delay for a specified number of milliseconds
- Line 76: We add a method to delay for a specified number of microseconds
- Line 80: We declare the getter function `GetTimer()`

### Timer.cpp

Let's implement the new methods.
Update the file `code/libraries/baremetal/src/Timer.cpp`.

```cpp
File: code/libraries/baremetal/src/Timer.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : Timer.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Timer
9: //
10: // Description : Timer class
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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
40: #include <baremetal/Timer.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: 
46: #define MSEC_PER_SEC  1000
47: #define USEC_PER_SEC  1000000
48: #define USEC_PER_MSEC USEC_PER_SEC / MSEC_PER_SEC
49: 
50: using namespace baremetal;
51: 
52: Timer::Timer()
53:     : m_memoryAccess{ GetMemoryAccess() }
54: {
55: }
56: 
57: Timer::Timer(IMemoryAccess& memoryAccess)
58:     : m_memoryAccess{ memoryAccess }
59: {
60: }
61: 
62: void Timer::WaitCycles(uint32 numCycles)
63: {
64:     if (numCycles)
65:     {
66:         while (numCycles--)
67:         {
68:             NOP();
69:         }
70:     }
71: }
72: 
73: void Timer::WaitMilliSeconds(uint64 msec)
74: {
75:     WaitMicroSeconds(msec * USEC_PER_MSEC);
76: }
77: 
78: #if !defined(USE_PHYSICAL_COUNTER)
79: void Timer::WaitMicroSeconds(uint64 usec)
80: {
81:     unsigned long freq{};
82:     unsigned long start{};
83:     unsigned long current{};
84:     // Get the current counter frequency (ticks per second)
85:     GetTimerFrequency(freq);
86:     // Read the current counter
87:     GetTimerCounter(start);
88:     // Calculate required count increase
89:     unsigned long wait = ((freq / USEC_PER_SEC) * usec) / USEC_PER_SEC;
90:     // Loop while counter increase is less than wait
91:     // Careful: busy wait
92:     do
93:     {
94:         GetTimerCounter(current);
95:     } while (current - start < wait);
96: }
97: #else
98: // Get System Timer counter (BCM2835 peripheral)
99: uint64 Timer::GetSystemTimer()
100: {
101:     uint32 highWord = -1;
102:     uint32 lowWord{};
103:     // We must read MMIO area as two separate 32 bit reads
104:     highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
105:     lowWord  = m_memoryAccess.Read32(RPI_SYSTMR_LO);
106:     // We have to repeat it if high word changed during read
107:     if (highWord != m_memoryAccess.Read32(RPI_SYSTMR_HI))
108:     {
109:         highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
110:         lowWord  = m_memoryAccess.Read32(RPI_SYSTMR_LO);
111:     }
112:     // compose long int value
114:     return (static_cast<uint64>(highWord) << 32 | lowWord);
115: }
116: 
117: void Timer::WaitMicroSeconds(uint64 usec)
118: {
119:     auto start = GetTimer().GetSystemTimer();
120:     // We must check if it's non-zero, because QEMU does not emulate
121:     // system timer, and returning constant zero would mean infinite loop
122:     if (start)
123:     {
124:         while (GetTimer().GetSystemTimer() - start < usec)
125:             NOP();
126:     }
127: }
128: #endif
129: 
130: Timer& baremetal::GetTimer()
131: {
132:     static Timer timer;
133:     return timer;
134: }
```

- Line 42: The header `ARMInstructions.h` is also needed now to get some ARM registers (still to be defined)
- Line 43: We need to use some registers for the system timer (still to be defined), so we will include `BCMRegisters.h`
- Line 44: We will also need to include `MemoryAccess.h`
- Line 46-48: We add some convenience definitions to convert between seconds, milliseconds and microseconds
- Line 52-55: We implement the default constructor, using the standard singleton `MemoryAccess` instance
- Line 57-60: We implement the specific test constructor, where the memory access instance is passed as a parameter
- Line 73-76: We implement the `WaitMilliSeconds()` method by simply called `WaitMicroSeconds()`
- Line 79-96: We implement the `WaitMicroSeconds()` method in case we don't use the physical system timer, using ARM registers
  - Line 85: We read the timer frequency. Default, this is 54 MHz
`GetTimerFrequency()` is an ARM instruction to read the counter frequency register `CNTFRQ_EL0`, which returns the counter frequency in ticks per second.
This needs to be added
  - Line 87: We read the current counter value. 
'GetTimerCounter()` is an ARM instruction to read the physical counter register `CNTPCT_EL0`, which returns the current counter value.
This needs to be added
  - Line 89: We calculate the number of counter ticks to wait by first calculating the number of ticks per microsecond, and then multiplying by the number of microseconds to wait.
It would be more accurate to first multiply, however we might get an overflow
  - Line 92-95: We read the current count value, and loop as long as the number of ticks has not passed
  - Line 99-114: We need to implement the `GetSystemTimer()` to retrieve the current system timer count
    - Line 104-105: We read the 64 bit system timer value
    - Line 107-111: The high word might have changed during the read, so in case it is read different, we read again. This prevents strange wrap-around errors
    - Line 113: We combine the two 32 bit words into a 64 bit value
- Line 116-126: We implement the `WaitMicroSeconds()` method in case we use the physical system timer, using BCM registers. The System timer updates every microsecond
  - Line 118: We read the current system timer value
  - Line 121-125: We loop (while executing NOP instructions) while number of microseconds has not elapsed
- Line 129-133: We implement the getter function `GetTimer()`, which returns a reference to the singleton Timer instance.

### ARMInstructions.h

You will have noticed that we use two "function calls" which are actually reading ARM registers. We need to add them.
Update the file `code/libraries/baremetal/include/baremetal/ASMInstructions.h`.

```cpp
...
File: code/libraries/baremetal/include/baremetal/ASMInstructions.h
...
64: // Get counter timer frequency.
65: #define GetTimerFrequency(freq)         asm volatile ("mrs %0, CNTFRQ_EL0" : "=r"(freq))
66: // Get counter timer value.
67: #define GetTimerCounter(count)          asm volatile ("mrs %0, CNTPCT_EL0" : "=r"(count))
68: 
69: // Get Physical counter-timer control register.
70: #define GetTimerControl(value)          asm volatile ("mrs %0, CNTP_CTL_EL0" : "=r" (value))
71: // Set Physical counter-timer control register.
72: #define SetTimerControl(value)          asm volatile ("msr CNTP_CTL_EL0, %0" :: "r" (value))
73: 
74: // IStatus bit, flags if Physical counter-timer condition is met.
75: #define CNTP_CTL_EL0_STATUS BIT(2)
76: // IMask bit, flags if interrupts for Physical counter-timer are masked.
77: #define CNTP_CTL_EL0_IMASK BIT(1)
78: // Enable bit, flags if Physical counter-timer is enabled.
79: #define CNTP_CTL_EL0_ENABLE BIT(0)
80: 
81: // Get Physical counter-timer comparison value.
82: #define GetTimerCompareValue(value)     asm volatile ("mrs %0, CNTP_CVAL_EL0" : "=r" (value))
83: // Set Physical counter-timer comparison value.
84: #define SetTimerCompareValue(value)     asm volatile ("msr CNTP_CVAL_EL0, %0" :: "r" (value))
85: 
```

- Line 65: We define `GetTimerFrequency()`. It reads the Counter-timer Frequency (`CNTFRQ_EL0`) register.
See [ARM architecture registers](../cpu/arm/ARM-architecture-registers.pdf), page 222.
- Line 67: We define `GetTimerCounter()`. It reads the Counter-timer Physical Count (`CNTPCT_EL0`) register.
See [ARM architecture registers](../cpu/arm/ARM-architecture-registers.pdf), page 307.
- Line 70: We define `GetTimerControl()`. It reads the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [ARM architecture registers](../cpu/arm/ARM-architecture-registers.pdf), page 293.
- Line 72: We define `SetTimerControl()`. It writes the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [ARM architecture registers](../cpu/arm/ARM-architecture-registers.pdf), page 293.
- Line 75-79: We define the bits of the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
- Line 82: We define `SetTimerControl()`. It read the Counter-timer Physical Timer CompareValue (`CNTP_CVAL_EL0`) register.
See [ARM architecture registers](../cpu/arm/ARM-architecture-registers.pdf), page 298.
- Line 84: We define `SetTimerControl()`. It writes the Counter-timer Physical Timer CompareValue (`CNTP_CVAL_EL0`) register.
See [ARM architecture registers](../cpu/arm/ARM-architecture-registers.pdf), page 298.

Only the first two functions are currently used.

### BCMRegisters.h

In case we use the System Timer, we need to access some registers in the SoC. We need to add them.
Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
55: //---------------------------------------------
56: // Raspberry Pi System Timer
57: //---------------------------------------------
58: 
59: #define RPI_SYSTMR_BASE                 RPI_BCM_IO_BASE + 0x00003000
60: #define RPI_SYSTMR_CS                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000000)
61: #define RPI_SYSTMR_LO                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000004)
62: #define RPI_SYSTMR_HI                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000008)
63: #define RPI_SYSTMR_CMP0                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x0000000C)
64: #define RPI_SYSTMR_CMP1                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000010)
65: #define RPI_SYSTMR_CMP2                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000014)
66: #define RPI_SYSTMR_CMP3                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000018)
67: 
...
```

More information on the System Timer registers can be found in the 
[Broadcom documentation BCM2835 (Raspberry Pi 1/2)](boards/RaspberryPi/BCM2835-peripherals.pdf) (page 172), 
[Broadcom documentation BCM2837 (Raspberry Pi 3)](boards/RaspberryPi/BCM2835-peripherals.pdf) (page 172), 
[Broadcom documentation BCM2711 (Raspberry Pi 4)](boards/RaspberryPi/bcm2711-peripherals.pdf) (page 142).
It is currently unclear whether the System Timer is present in the same shape in Raspberry Pi 5.

As you can see the System timer register addresses are all prefixed with `RPI_SYSTMR_`.

### System.cpp

Let's change the delays in `Halt()` and `Reboot()` to a 10 milliseconds delay.
Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
...
69: static const uint32 WaitTime = 10; // ms
...
87: void System::Halt()
88: {
89:     GetUART1().WriteString("Halt\n");
90:     Timer::WaitMilliSeconds(WaitTime);
...
107: void System::Reboot()
108: {
109:     GetUART1().WriteString("Reboot\n");
110:     Timer::WaitMilliSeconds(WaitTime);
...
```

- Line 69: We replace the count with a delay in milliseconds
- Line 90: We call `WaitMilliSeconds()` instead of `WaitCycles()`
- Line 110: We call `WaitMilliSeconds()` instead of `WaitCycles()`

### Update application

To show that the timer actually waits correctly, let's add a 5 seconds delay between the "Hello World!" output and the "Press r to reboot, h to halt" output.

Update the file `code/application/demo/src/main.cpp`.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/System.h>
3: #include <baremetal/Timer.h>
4: #include <baremetal/UART1.h>
5: 
6: using namespace baremetal;
7: 
8: int main()
9: {
10:     auto& uart = GetUART1();
11:     uart.WriteString("Hello World!\n");
12: 
13:     uart.WriteString("Wait 5 seconds\n");
14:     Timer::WaitMilliSeconds(5000);
15: 
16:     uart.WriteString("Press r to reboot, h to halt\n");
17:     char ch{};
18:     while ((ch != 'r') && (ch != 'h'))
19:     {
20:         ch = uart.Read();
21:         uart.Write(ch);
22:     }
23: 
24:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
25: }
```

### Update project configuration - Step 3

We did not add any new files, so the project configuration needs no change.

However, we need to update the main CMake file, as we wish to use the Physical Counter. This means we have to add `` to the C and C++ definitions.
Update the file `CMakeLists.txt`.

```cmake
File: CMakeLists.txt
...
64: set(DEFINES_C
65:     PLATFORM_BAREMETAL
66:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
67:     USE_PHYSICAL_COUNTER
68:     )
...
```

### Configuring, building and debugging - Step 3

We can now configure and build our code, and start debugging.

The output should correctly wait 5 seconds in between, and output should be complete.

```text
Starting up
Hello World!
Wait 5 seconds
Press r to reboot, h to halt
hHalt
```

Next: [09-umailbox](09-mailbox.md)

