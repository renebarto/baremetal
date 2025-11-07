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
47: namespace baremetal {
48:
49: class IMemoryAccess;
50:
51: /// <summary>
52: /// Timer class. For now only contains busy waiting methods
53: ///
54: /// Note that this class is created as a singleton, using the GetTimer() function.
55: /// </summary>
56: class Timer
57: {
58:     /// <summary>
59:     /// Retrieves the singleton Timer instance. It is created in the first call to this function. This is a friend function of class Timer
60:     /// </summary>
61:     /// <returns>A reference to the singleton Timer</returns>
62:     friend Timer& GetTimer();
63:
64: private:
65:     /// <summary>
66:     /// Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
67:     /// </summary>
68:     IMemoryAccess& m_memoryAccess;
69:
70:     Timer();
71:
72: public:
73:     Timer(IMemoryAccess& memoryAccess);
74:
75:     static void WaitCycles(uint32 numCycles);
76:
77:     uint64 GetSystemTimer();
78:
79:     static void WaitMilliSeconds(uint64 msec);
80:     static void WaitMicroSeconds(uint64 usec);
81: };
82:
83: Timer& GetTimer();
84:
85: } // namespace baremetal
```

- Line 49: We forward declare IMemoryAccess, as this class will need it for memory access
- Line 58-62: We declare the friend function `GetTimer()`, much like `GetUART1()` and `GetSystem()` before.
This function will return the singleton instance of the Timer
- Line 65-68: We add a member variable for the memory access
- Line 70: We declare a private constructor (called by `GetTimer()`)
- Line 73: We declare a public constructor for testing taking a reference to an `IMemoryAccess` implementation
- Line 75: We keep the already existing `WaitCycles()` method
- Line 77: We add a method `GetSystemTimer()` to request the timer count in case we use the BCM2835 (SoC) timer.
As this method needs memory access, we use the IMemoryAccess interface, so this method needs to have an instance
- Line 79: We add a method `WaitMilliSeconds()` to delay for a specified number of milliseconds using the ARM timer
- Line 80: We add a method `WaitMicroSeconds()` to delay for a specified number of microseconds using the ARM timer
- Line 83: We declare the getter function `GetTimer()`

### Timer.cpp {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_TIMERCPP}

Let's implement the new methods.

Update the file `code/libraries/baremetal/src/Timer.cpp`.

```cpp
File: code/libraries/baremetal/src/Timer.cpp
...
40: #include "baremetal/Timer.h"
41:
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/MemoryAccess.h"
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
62:     : m_memoryAccess{GetMemoryAccess()}
63: {
64: }
65:
66: /// <summary>
67: /// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
68: /// </summary>
69: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
70: Timer::Timer(IMemoryAccess& memoryAccess)
71:     : m_memoryAccess{memoryAccess}
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
93: /// The timer used for the delays is the ARM built-in timer.
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
105: /// The timer used is the ARM built-in timer.
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
118:     unsigned long wait = (freq / USEC_PER_SEC) * usec;
119:     // Loop while counter increase is less than wait
120:     // Careful: busy wait
121:     do
122:     {
123:         GetTimerCounter(current);
124:     } while (current - start < wait);
125: }
126:
127: /// <summary>
128: /// Reads the BCM2835 System Timer counter value. See @ref RASPBERRY_PI_SYSTEM_TIMER
129: /// </summary>
130: /// <returns>System Timer count value</returns>
131: uint64 Timer::GetSystemTimer()
132: {
133:     uint32 highWord = -1;
134:     uint32 lowWord{};
135:     // We must read MMIO area as two separate 32 bit reads
136:     highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
137:     lowWord = m_memoryAccess.Read32(RPI_SYSTMR_LO);
138:     // We have to repeat it if high word changed during read
139:     if (highWord != m_memoryAccess.Read32(RPI_SYSTMR_HI))
140:     {
141:         highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
142:         lowWord = m_memoryAccess.Read32(RPI_SYSTMR_LO);
143:     }
144:     // compose long int value
145:     return (static_cast<uint64>(highWord) << 32 | lowWord);
146: }
147:
148: /// <summary>
149: /// Retrieves the singleton Timer instance. It is created in the first call to this function.
150: /// </summary>
151: /// <returns>A reference to the singleton Timer</returns>
152: Timer& baremetal::GetTimer()
153: {
154:     static Timer timer;
155:     return timer;
156: }
```

- Line 42: The header `ARMInstructions.h` is also needed now to get some ARM registers (still to be defined)
- Line 43: We need to use some registers for the system timer (still to be defined), so we will include `BCMRegisters.h`
- Line 44: We will also need to include `MemoryAccess.h`
- Line 49-54: We add some convenience definitions to convert between seconds, milliseconds and microseconds
- Line 58-64: We implement the private default constructor, using the standard singleton `MemoryAccess` instance
- Line 66-73: We implement the public constructor, where the memory access instance is passed as a parameter
- Line 90-99: We implement the `WaitMilliSeconds()` method by simply calling `WaitMicroSeconds()`
- Line 101-126: We implement the `WaitMicroSeconds()` method using ARM registers
  - Line 114: We read the timer frequency. Default, this is 54 MHz.
`GetTimerFrequency()` is an ARM instruction to read the counter frequency register `CNTFRQ_EL0`, which returns the counter frequency in ticks per second.
This needs to be added
  - Line 116: We read the current counter value.
`GetTimerCounter()` is an ARM instruction to read the physical counter register `CNTPCT_EL0`, which returns the current counter value.
This needs to be added
  - Line 118: We calculate the number of counter ticks to wait by first calculating the number of ticks per microsecond, and then multiplying by the number of microseconds to wait.
It would be more accurate to first multiply, however we might get an overflow
  - Line 121-124: We read the current count value, and loop as long as the number of ticks has not passed
- Line 127-146: We implement the `GetSystemTimer()` method to retrieve the current system timer count
    - Line 136-137: We read the 64 bit system timer value
    - Line 139-143: The high word might have changed during the read, so in case it is read different, we read again. This prevents strange wrap-around errors
    - Line 145: We combine the two 32 bit words into a 64 bit value, and return this as the tick count
- Line 148-156: We implement the getter function `GetTimer()`, which returns a reference to the singleton Timer instance.

### ARMInstructions.h {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_ARMINSTRUCTIONSH}

You will have noticed that we use two "function calls" which are actually reading ARM registers. We need to add them.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`.

```cpp
...
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
65: /// @brief Get counter timer frequency. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTFRQ_EL0_REGISTER
66: #define GetTimerFrequency(freq)     asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq))
67: /// @brief Get counter timer value. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTPCT_EL0_REGISTER
68: #define GetTimerCounter(count)      asm volatile("mrs %0, CNTPCT_EL0" : "=r"(count))
69:
70: /// @brief Get Physical counter-timer control register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
71: #define GetTimerControl(value)      asm volatile("mrs %0, CNTP_CTL_EL0" : "=r"(value))
72: /// @brief Set Physical counter-timer control register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
73: #define SetTimerControl(value)      asm volatile("msr CNTP_CTL_EL0, %0" ::"r"(value))
74:
75: /// @brief IStatus bit, flags if Physical counter-timer condition is met. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
76: #define CNTP_CTL_EL0_STATUS         BIT1(2)
77: /// @brief IMask bit, flags if interrupts for Physical counter-timer are masked. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
78: #define CNTP_CTL_EL0_IMASK          BIT1(1)
79: /// @brief Enable bit, flags if Physical counter-timer is enabled. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
80: #define CNTP_CTL_EL0_ENABLE         BIT1(0)
81:
82: /// @brief Get Physical counter-timer comparison value. See \ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER
83: #define GetTimerCompareValue(value) asm volatile("mrs %0, CNTP_CVAL_EL0" : "=r"(value))
84: /// @brief Set Physical counter-timer comparison value. See \ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER
85: #define SetTimerCompareValue(value) asm volatile("msr CNTP_CVAL_EL0, %0" ::"r"(value))
```

- Line 65-66: We define `GetTimerFrequency()`. It reads the Counter-timer Frequency (`CNTFRQ_EL0`) register.
See [CNTFRQ_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTFRQ_EL0_REGISTER).
- Line 67-68: We define `GetTimerCounter()`. It reads the Counter-timer Physical Count (`CNTPCT_EL0`) register.
See [CNTPCT_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTPCT_EL0_REGISTER).
- Line 70-71: We define `GetTimerControl()`. It reads the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [CNTP_CTL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER).
- Line 72-73: We define `SetTimerControl()`. It writes the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [CNTP_CTL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER).
- Line 75-80: We define the bits of the Counter-timer Physical Timer Control (`CNTP_CTL_EL0`) register.
See [CNTP_CTL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER).
- Line 82-83: We define `GetTimerCompareValue()`. It reads the Counter-timer Physical Timer CompareValue (`CNTP_CVAL_EL0`) register.
See [CNTP_CVAL_EL0 register](#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER).
- Line 84-85: We define `SetTimerCompareValue()`. It writes the Counter-timer Physical Timer CompareValue (`CNTP_CVAL_EL0`) register.
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
- [Broadcom documentation BCM2837 (Raspberry Pi 3)](pdf/bcm2837-peripherals.pdf), section 12 System Timer, page 172
- [Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/bcm2711-peripherals.pdf), Chapter 10. System Timer, page 142
- [Broadcom documentation RP1 (Raspberry Pi 5)](pdf/rp1-peripherals.pdf), section 3.8. TICKS, page 52.
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
81: /// @brief Wait time in milliseconds to ensure that UART info is written before system halt or reboot
82: static const uint32 WaitTime = 10;
...
104: void System::Halt()
105: {
106:     GetUART1().WriteString("Halt\n");
107:     Timer::WaitMilliSeconds(WaitTime);
...
127: void System::Reboot()
128: {
129:     GetUART1().WriteString("Reboot\n");
130:     Timer::WaitMilliSeconds(WaitTime);
...
```

- Line 81-82: We replace the count with a delay in milliseconds
- Line 107: We call `WaitMilliSeconds()` instead of `WaitCycles()`
- Line 130: We call `WaitMilliSeconds()` instead of `WaitCycles()`

### Update application {#TUTORIAL_09_TIMER_USING_THE_SYSTEM_TIMER_UPDATE_APPLICATION}

To show that the timer actually waits correctly, let's add a 5 second delay between the "Hello World!" output and the "Press r to reboot, h to halt" output.

Update the file `code/application/demo/src/main.cpp`.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/System.h"
3: #include "baremetal/Timer.h"
4: #include "baremetal/UART1.h"
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

