# Tutorial 09: Timer {#TUTORIAL_09_TIMER}

@tableofcontents

## Tutorial information {#TUTORIAL_09_TIMER_TUTORIAL_INFORMATION}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/09-timer`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_09_TIMER_TUTORIAL_INFORMATION_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-09.a`
- a library `output/Debug/lib/stdlib-09.a`
- an application `output/Debug/bin/09-timer.elf`
- an image in `deploy/Debug/09-timer-image`

## Using the system timer {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER}

In the previous tutorial we added some delay before halting or rebooting the system. The delay was specified as the number of NOP instructions to execute.
That is not a very accurate way of specifying a delay. It would be better if we could use a hardware timer.

That is exactly what we will do in this tutorial. There are two kinds of timers:
- A free running timer in the ARM processor
- A more advanced timer in the SoC that can generate interrupts when timed out

We will be using the SoC timer later on for interrupt generating timers, but we'll start off creating more accurate delays using the ARM timer for now.

### Timer.h {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_TIMERH}

First, we will add some methods to the `Timer` class.

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
47: namespace baremetal
48: {
49: 
50: class IMemoryAccess;
51: 
52: /// <summary>
53: /// Timer class. For now only contains busy waiting methods
54: ///
55: /// Note that this class is created as a singleton, using the GetTimer() function.
56: /// </summary>
57: class Timer
58: {
59:     /// <summary>
60:     /// Retrieves the singleton Timer instance. It is created in the first call to this function. This is a friend function of class Timer
61:     /// </summary>
62:     /// <returns>A reference to the singleton Timer</returns>
63:     friend Timer &GetTimer();
64: 
65: private:
66:     /// <summary>
67:     /// Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
68:     /// </summary>
69:     IMemoryAccess &m_memoryAccess;
70: 
71:     Timer();
72: 
73: public:
74:     Timer(IMemoryAccess &memoryAccess);
75: 
76:     static void WaitCycles(uint32 numCycles);
77: 
78:     uint64 GetSystemTimer();
79: 
80:     static void WaitMilliSeconds(uint64 msec);
81:     static void WaitMicroSeconds(uint64 usec);
82: };
83: 
84: Timer &GetTimer();
85: 
86: } // namespace baremetal
```

- Line 50: We forward declare IMemoryAccess, as this class will need it for memory access
- Line 63: We declare the friend function `GetTimer()`, much like `GetUART1()` and `GetSystem()` before.
This function will return the singleton instance of the Timer
- Line 69: We add a member variable for the memory access
- Line 71: We declare a private constructor (called by `GetTimer()`)
- Line 74: We declare a public constructor for testing taking a reference to an `IMemoryAccess` implementation
- Line 76: We keep the already existing `WaitCycles()` method
- Line 78: We add a method `GetSystemTimer()` to request the timer count in case we use the BCM2835 (SoC) timer.
As this method needs memory access, we use the IMemoryAccess interface, so this method needs to have an instance
- Line 80: We add a method `WaitMilliSeconds()` to delay for a specified number of milliseconds using the ARM timer
- Line 81: We add a method `WaitMicroSeconds()` to delay for a specified number of microseconds using the ARM timer
- Line 84: We declare the getter function `GetTimer()`

### Timer.cpp {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_TIMERCPP}

Let's implement the new methods.

Update the file `code/libraries/baremetal/src/Timer.cpp`.

```cpp
File: code/libraries/baremetal/src/Timer.cpp
...
File: d:\Projects\RaspberryPi\baremetal.github.shadow\code\libraries\baremetal\src\Timer.cpp
40: #include <baremetal/Timer.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: 
46: /// @file
47: /// Raspberry Pi Timer implementation
48: 
49: /// @brief Number of milliseconds in a second
50: #define MSEC_PER_SEC  1000
51: /// @brief Number of microseconds in a second
52: #define USEC_PER_SEC  1000000
53: /// @brief Number of microseconds in a millisecond
54: #define USEC_PER_MSEC USEC_PER_SEC / MSEC_PER_SEC
55: 
56: using namespace baremetal;
57: 
58: /// <summary>
59: /// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
60: /// </summary>
61: Timer::Timer()
62:     : m_memoryAccess{ GetMemoryAccess() }
63: {
64: }
65: 
66: /// <summary>
67: /// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
68: /// </summary>
69: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
70: Timer::Timer(IMemoryAccess &memoryAccess)
71:     : m_memoryAccess{ memoryAccess }
72: {
73: }
74: 
75: /// <summary>
76: /// Wait for specified number of NOP statements. Busy wait
77: /// </summary>
78: /// <param name="numCycles">Number of cycles to wait</param>
79: void Timer::WaitCycles(uint32 numCycles)
80: {
81:     if (numCycles)
82:     {
83:         while (numCycles--)
84:         {
85:             NOP();
86:         }
87:     }
88: }
89: 
90: /// <summary>
91: /// Wait for msec milliseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical counter). Busy wait
92: ///
93: /// The timer used for the delays is the ARM builtin timer.
94: /// </summary>
95: /// <param name="msec">Wait time in milliseconds</param>
96: void Timer::WaitMilliSeconds(uint64 msec)
97: {
98:     WaitMicroSeconds(msec * USEC_PER_MSEC);
99: }
100: 
101: /// <summary>
102: /// Wait for usec microseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical
103: /// counter). Busy wait
104: ///
105: /// The timer used is the ARM builtin timer.
106: /// </summary>
107: /// <param name="usec">Wait time in microseconds</param>
108: void Timer::WaitMicroSeconds(uint64 usec)
109: {
110:     unsigned long freq{};
111:     unsigned long start{};
112:     unsigned long current{};
113:     // Get the current counter frequency (ticks per second)
114:     GetTimerFrequency(freq);
115:     // Read the current counter
116:     GetTimerCounter(start);
117:     // Calculate required count increase
118:     unsigned long wait = ((freq / USEC_PER_SEC) * usec) / USEC_PER_SEC;
119:     // Loop while counter increase is less than wait
120:     // Careful: busy wait
121:     do
122:     {
123:         GetTimerCounter(current);
124:     }
125:     while (current - start < wait);
126: }
127: 
128: /// <summary>
129: /// Reads the BCM2835 System Timer counter value. See @ref RASPBERRY_PI_SYSTEM_TIMER
130: /// </summary>
131: /// <returns>System Timer count value</returns>
132: uint64 Timer::GetSystemTimer()
133: {
134:     uint32 highWord = -1;
135:     uint32 lowWord{};
136:     // We must read MMIO area as two separate 32 bit reads
137:     highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
138:     lowWord = m_memoryAccess.Read32(RPI_SYSTMR_LO);
139:     // We have to repeat it if high word changed during read
140:     if (highWord != m_memoryAccess.Read32(RPI_SYSTMR_HI))
141:     {
142:         highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
143:         lowWord = m_memoryAccess.Read32(RPI_SYSTMR_LO);
144:     }
145:     // compose long int value
146:     return (static_cast<uint64>(highWord) << 32 | lowWord);
147: }
148: 
149: /// <summary>
150: /// Retrieves the singleton Timer instance. It is created in the first call to this function.
151: /// </summary>
152: /// <returns>A reference to the singleton Timer</returns>
153: Timer &baremetal::GetTimer()
154: {
155:     static Timer timer;
156:     return timer;
157: }
```

- Line 42: The header `ARMInstructions.h` is also needed now to get some ARM registers (still to be defined)
- Line 43: We need to use some registers for the system timer (still to be defined), so we will include `BCMRegisters.h`
- Line 44: We will also need to include `MemoryAccess.h`
- Line 50-54: We add some convenience definitions to convert between seconds, milliseconds and microseconds
- Line 61-64: We implement the private default constructor, using the standard singleton `MemoryAccess` instance
- Line 70-73: We implement the public constructor, where the memory access instance is passed as a parameter
- Line 96-99: We implement the `WaitMilliSeconds()` method by simply calling `WaitMicroSeconds()`
- Line 108-126: We implement the `WaitMicroSeconds()` method using ARM registers
  - Line 114: We read the timer frequency. Default, this is 54 MHz.
`GetTimerFrequency()` is an ARM instruction to read the counter frequency register `CNTFRQ_EL0`, which returns the counter frequency in ticks per second.
This needs to be added
  - Line 116: We read the current counter value.
`GetTimerCounter()` is an ARM instruction to read the physical counter register `CNTPCT_EL0`, which returns the current counter value.
This needs to be added
  - Line 118: We calculate the number of counter ticks to wait by first calculating the number of ticks per microsecond, and then multiplying by the number of microseconds to wait.
It would be more accurate to first multiply, however we might get an overflow
  - Line 121-125: We read the current count value, and loop as long as the number of ticks has not passed
- Line 132-147: We implement the `GetSystemTimer()` method to retrieve the current system timer count
    - Line 137-138: We read the 64 bit system timer value
    - Line 140-144: The high word might have changed during the read, so in case it is read different, we read again. This prevents strange wrap-around errors
    - Line 146: We combine the two 32 bit words into a 64 bit value, and return this as the tick count
- Line 153-157: We implement the getter function `GetTimer()`, which returns a reference to the singleton Timer instance.

### ARMInstructions.h {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_ARMINSTRUCTIONSH}

You will have noticed that we use two "function calls" which are actually reading ARM registers. We need to add them.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`.

```cpp
...
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
65: /// @brief Get counter timer frequency. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTFRQ_EL0_REGISTER
66: #define GetTimerFrequency(freq)         asm volatile ("mrs %0, CNTFRQ_EL0" : "=r"(freq))
67: /// @brief Get counter timer value. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTPCT_EL0_REGISTER
68: #define GetTimerCounter(count)          asm volatile ("mrs %0, CNTPCT_EL0" : "=r"(count))
69: 
70: /// @brief Get Physical counter-timer control register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
71: #define GetTimerControl(value)          asm volatile ("mrs %0, CNTP_CTL_EL0" : "=r" (value))
72: /// @brief Set Physical counter-timer control register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
73: #define SetTimerControl(value)          asm volatile ("msr CNTP_CTL_EL0, %0" :: "r" (value))
74: 
75: /// @brief IStatus bit, flags if Physical counter-timer condition is met. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
76: #define CNTP_CTL_EL0_STATUS BIT1(2)
77: /// @brief IMask bit, flags if interrupts for Physical counter-timer are masked. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
78: #define CNTP_CTL_EL0_IMASK BIT1(1)
79: /// @brief Enable bit, flags if Physical counter-timer is enabled. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
80: #define CNTP_CTL_EL0_ENABLE BIT1(0)
81: 
82: /// @brief Get Physical counter-timer comparison value. See \ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER
83: #define GetTimerCompareValue(value)     asm volatile ("mrs %0, CNTP_CVAL_EL0" : "=r" (value))
84: /// @brief Set Physical counter-timer comparison value. See \ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER
85: #define SetTimerCompareValue(value)     asm volatile ("msr CNTP_CVAL_EL0, %0" :: "r" (value))
```

- Line 66: We define `GetTimerFrequency()`. It reads the Counter-timer Frequency (`CNTFRQ_EL0`) register.
See [CNTFRQ_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTFRQ_EL0_REGISTER).
- Line 68: We define `GetTimerCounter()`. It reads the Counter-timer Physical Count (`CNTPCT_EL0`) register.
See [CNTPCT_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTPCT_EL0_REGISTER).
- Line 71: We define `GetTimerControl()`. It reads the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [CNTP_CTL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER).
- Line 73: We define `SetTimerControl()`. It writes the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [CNTP_CTL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER).
- Line 76-80: We define the bits of the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [CNTP_CTL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER).
- Line 83: We define `SetTimerControl()`. It read the Counter-timer Physical Timer CompareValue (`CNTP_CVAL_EL0`) register.
See [CNTP_CVAL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER).
- Line 85: We define `SetTimerControl()`. It writes the Counter-timer Physical Timer CompareValue (`CNTP_CVAL_EL0`) register.
See [CNTP_CVAL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER).

Only the first two functions are currently used.

### BCMRegisters.h {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_BCMREGISTERSH}

In case we use the System Timer, we need to access some registers in the SoC. We need to add them.

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
70: //---------------------------------------------
71: // Raspberry Pi System Timer
72: //---------------------------------------------
73: 
74: /// @brief Raspberry Pi System Timer Registers base address. See @ref RASPBERRY_PI_SYSTEM_TIMER
75: #define RPI_SYSTMR_BASE                 RPI_BCM_IO_BASE + 0x00003000
76: /// @brief System Timer Control / Status register. See @ref RASPBERRY_PI_SYSTEM_TIMER
77: #define RPI_SYSTMR_CS                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000000)
78: /// @brief System Timer Counter Lower 32 bits register. See @ref RASPBERRY_PI_SYSTEM_TIMER
79: #define RPI_SYSTMR_LO                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000004)
80: /// @brief System Timer Counter Higher 32 bits register. See @ref RASPBERRY_PI_SYSTEM_TIMER
81: #define RPI_SYSTMR_HI                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000008)
82: /// @brief System Timer Compare 0 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
83: #define RPI_SYSTMR_CMP0                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x0000000C)
84: /// @brief System Timer Compare 1 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
85: #define RPI_SYSTMR_CMP1                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000010)
86: /// @brief System Timer Compare 2 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
87: #define RPI_SYSTMR_CMP2                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000014)
88: /// @brief System Timer Compare 3 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
89: #define RPI_SYSTMR_CMP3                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000018)
90: 
...
```

More information on the System Timer registers can be found in:
- [Raspberry Pi System timer](#RASPBERRY_PI_SYSTEM_TIMER)
- [Broadcom documentation BCM2837 (Raspberry Pi 3)](pdf/bcm2837-peripherals.pdf) (section `12 System Timer`)
- [Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/bcm2711-peripherals.pdf) (`Chapter 10. System Timer`)
- [Broadcom documentation RP1 (Raspberry Pi 5)](pdf/rp1-peripherals.pdf) (section `3.8. TICKS`).
For Raspberry Pi 5, there is a TICKS device, that also generates ticks for a clock.
It behaves differently however, and is still to be described / implemented.

\todo Describe and implement TICKS device for Raspberry Pi 5.

As you can see the System timer register addresses are all prefixed with `RPI_SYSTMR_`.

### System.cpp {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_SYSTEMCPP}

Let's change the delays in `Halt()` and `Reboot()` to a 10 millisecond delay.

Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
...
82: /// @brief Wait time in milliseconds to ensure that UART info is written before system halt or reboot
83: static const uint32 WaitTime = 10;
...
115: void System::Halt()
116: {
117:     GetUART1().WriteString("Halt\n");
118:     Timer::WaitMilliSeconds(WaitTime);
...
138: void System::Reboot()
139: {
140:     GetUART1().WriteString("Reboot\n");
141:     Timer::WaitMilliSeconds(WaitTime);
...
```

- Line 83: We replace the count with a delay in milliseconds
- Line 118: We call `WaitMilliSeconds()` instead of `WaitCycles()`
- Line 141: We call `WaitMilliSeconds()` instead of `WaitCycles()`

### Update application {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_UPDATE_APPLICATION}

To show that the timer actually waits correctly, let's add a 5 second delay between the "Hello World!" output and the "Press r to reboot, h to halt" output.

Update the file `code/application/demo/src/main.cpp`.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/System.h>
3: #include <baremetal/Timer.h>
4: #include <baremetal/UART1.h>
5: 
6: /// @file
7: /// Demo application main code
8: 
9: using namespace baremetal;
10: 
11: /// <summary>
12: /// Demo application main code
13: /// </summary>
14: /// <returns>For now always 0</returns>
15: int main()
16: {
17:     auto& uart = GetUART1();
18:     uart.WriteString("Hello World!\n");
19: 
20:     uart.WriteString("Wait 5 seconds\n");
21:     Timer::WaitMilliSeconds(5000);
22: 
23:     uart.WriteString("Press r to reboot, h to halt\n");
24:     char ch{};
25:     while ((ch != 'r') && (ch != 'h'))
26:     {
27:         ch = uart.Read();
28:         uart.Write(ch);
29:     }
30: 
31:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
32: }
```

### Update project configuration {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_UPDATE_PROJECT_CONFIGURATION}

We did not add any new files, so the project configuration needs no change.

### Configuring, building and debugging {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The output should correctly wait 5 seconds in between, and output should be complete.

```text
Starting up
Hello World!
Wait 5 seconds
Press r to reboot, h to halt
hHalt
```

Next: [10-mailbox](10-mailbox.md)

