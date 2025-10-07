# Tutorial 20: Timer extension {#TUTORIAL_20_TIMER_EXTENSION}

@tableofcontents

## New tutorial setup {#TUTORIAL_20_TIMER_EXTENSION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/20-timer-extension`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_20_TIMER_EXTENSION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-20.a`
- a library `output/Debug/lib/stdlib-20.a`
- an application `output/Debug/bin/20-timer-extension.elf`
- an image in `deploy/Debug/20-timer-extension-image`

## Extending the timer {#TUTORIAL_20_TIMER_EXTENSION_EXTENDING_THE_TIMER}

In tutorial [09-timer](09-timer.md) we created the `Timer` class. However, the functionality was limited to delays.
We would like to have timers which are updated automatically on interrupt basis.

So we'll be extending the `Timer` class for this.
We will start by first creating an interrupt handler, which counts ticks.
Then we'll introduce `KernelTimer`, which can be used to trigger after a certain amount of time.

## Adding interrupts to the timer - Step 1 {#TUTORIAL_20_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1}

We'll add some functionality to the `Timer` class for enabling, handling and disabling interrupts on the timer.

### Timer.h {#TUTORIAL_20_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TIMERH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
47: namespace baremetal {
48: 
49: /// @brief Number of milliseconds in a second
50: #define MSEC_PER_SEC     1000
51: /// @brief Number of microseconds in a second
52: #define USEC_PER_SEC     1000000
53: /// @brief Number of microseconds in a millisecond
54: #define USEC_PER_MSEC    USEC_PER_SEC / MSEC_PER_SEC
55: /// @brief Number of timer ticks per second
56: #define TICKS_PER_SECOND 100
57: /// @brief Convert milliseconds to timer ticks
58: #define MSEC2TICKS(msec) (((msec) * TICKS_PER_SECOND) / MSEC_PER_SEC)
59: 
60: class InterruptSystem;
61: class IMemoryAccess;
62: 
63: /// @brief Periodic timer tick handler
64: using PeriodicTimerHandler = void(void);
65: 
66: /// @brief Maximum number of periodic tick handlers which can be installed
67: #define TIMER_MAX_PERIODIC_HANDLERS 4
68: 
69: /// <summary>
70: /// Timer class. For now only contains busy waiting methods
71: ///
72: /// Note that this class is created as a singleton, using the GetTimer() function.
73: /// </summary>
74: class Timer
75: {
76:     /// <summary>
77:     /// Retrieves the singleton Timer instance. It is created in the first call to this function. This is a friend function of class Timer
78:     /// </summary>
79:     /// <returns>A reference to the singleton Timer</returns>
80:     friend Timer& GetTimer();
81: 
82: private:
83:     /// @brief True if class is already initialized
84:     bool m_isInitialized;
85:     /// @brief Reference to the singleton InterruptSystem instantiation.
86:     InterruptSystem& m_interruptSystem;
87:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
88:     IMemoryAccess& m_memoryAccess;
89:     /// @brief Clock ticks per timer tick
90:     uint64 m_clockTicksPerSystemTick;
91:     /// @brief Timer tick counter
92:     volatile uint64 m_ticks;
93:     /// @brief Uptime in seconds
94:     volatile uint32 m_upTime;
95:     /// @brief Time in seconds (epoch time)
96:     volatile uint64 m_time;
97:     /// @brief Periodic tick handler functions
98:     PeriodicTimerHandler* m_periodicHandlers[TIMER_MAX_PERIODIC_HANDLERS];
99:     /// @brief Number of periodic tick handler functions installed
100:     volatile unsigned m_numPeriodicHandlers;
101: 
102:     Timer();
103: 
104: public:
105:     Timer(IMemoryAccess& memoryAccess);
106:     ~Timer();
107: 
108:     void Initialize();
109: 
110:     uint64 GetTicks() const;
111: 
112:     uint32 GetUptime() const;
113: 
114:     uint64 GetTime() const;
115: 
116:     void GetTimeString(char* buffer, size_t bufferSize);
117: 
118:     void RegisterPeriodicHandler(PeriodicTimerHandler* handler);
119:     void UnregisterPeriodicHandler(PeriodicTimerHandler* handler);
120: 
121:     static void WaitCycles(uint32 numCycles);
122: 
123:     uint64 GetSystemTimer();
124: 
125:     static void WaitMilliSeconds(uint64 msec);
126:     static void WaitMicroSeconds(uint64 usec);
127: 
128: private:
129:     void InterruptHandler();
130:     static void InterruptHandler(void* param);
131: };
132: 
133: Timer& GetTimer();
134: 
135: } // namespace baremetal
```

- Line 49-54: We move the definitions `MSEC_PER_SEC`, `USEC_PER_SEC` and `USEC_PER_MSEC` from `Timer.cpp` to `Timer.h`
- Line 55-56: We set the timer tick `TICKS_PER_SECOND` to be 100 times per second
- Line 57-58: We add a macro `MSEC2TICKS` to convert from milliseconds to timer ticks
- Line 63-64: We define a new type `PeriodicTimerHandler` which is a callback for every timer tick
- Line 66-67: We define the maximum number of periodic handlers `TIMER_MAX_PERIODIC_HANDLERS` which can be installed
- Line 69-131: We update the class `Timer`
  - Line 83-84: We add a boolean `m_isInitialized` to check if the class is initialized
  - Line 85-86: We add a reference to the singleton `InterruptSystem` instance, as we need to handle interrupts
  - Line 89-90: We add the member variable `m_clockTicksPerSystemTick` to hold the number of clock ticks per timer tick
  - Line 91-92: We add the member variable `m_ticks` to count the timer ticks
  - Line 93-94: We add the member variable `m_upTime` to count the uptime in seconds
  - Line 95-96: We add the member variable `m_time` to hold the time in seconds (epoch time).
For now this will be the same as the uptime, but later we will be retrieving the actual time
  - Line 97-98: We add an array of `PeriodicTimerHandler` functions to hold the periodic tick handlers
  - Line 99-100: We add a variable `m_numPeriodicHandlers` to hold the number of periodic tick handlers installed
  - Line 106: We add a destructor
  - Line 108: We add a method `Initialize()`
  - Line 110: We add a method `GetTicks()` to retrieve the number of timer ticks
  - Line 112: We add a method `GetUptime()` to retrieve the uptime in seconds
  - Line 114: We add a method `GetTime()` to retrieve the epoch time in seconds
  - Line 118: We add a method `RegisterPeriodicHandler()` to register a periodic tick handler
  - Line 119: We add a method `UnregisterPeriodicHandler()` to unregister a periodic tick handler
  - Line 129: We add a private method `InterruptHandler()` to handle the timer interrupt
  - Line 130: We add a static method `InterruptHandler()` to handle the timer interrupt, which calls the private method `InterruptHandler` by using the `param` parameter

### Timer.cpp {#TUTORIAL_20_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TIMERCPP}

Let's implement the newly added methods in the `Timer` class.

Update the file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/Assert.h"
44: #include "baremetal/BCMRegisters.h"
45: #include "baremetal/InterruptHandler.h"
46: #include "baremetal/MemoryAccess.h"
47: #include "stdlib/Util.h"
...
52: using namespace baremetal;
53: 
54: /// <summary>
55: /// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
56: /// </summary>
57: Timer::Timer()
58:     : m_interruptSystem{GetInterruptSystem()}
59:     , m_memoryAccess{GetMemoryAccess()}
60:     , m_clockTicksPerSystemTick{}
61:     , m_ticks{}
62:     , m_upTime{}
63:     , m_time{}
64:     , m_periodicHandlers{}
65:     , m_numPeriodicHandlers{}
66: {
67: }
68: 
69: /// <summary>
70: /// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
71: /// </summary>
72: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
73: Timer::Timer(IMemoryAccess& memoryAccess)
74:     : m_interruptSystem{GetInterruptSystem()}
75:     , m_memoryAccess{memoryAccess}
76:     , m_clockTicksPerSystemTick{}
77:     , m_ticks{}
78:     , m_upTime{}
79:     , m_time{}
80:     , m_periodicHandlers{}
81:     , m_numPeriodicHandlers{}
82: {
83: }
84: 
85: /// <summary>
86: /// Destructor
87: ///
88: /// Disables the timer, as well as the timer interrupt
89: /// </summary>
90: Timer::~Timer()
91: {
92:     SetTimerControl(~CNTP_CTL_EL0_ENABLE);
93: 
94:     m_interruptSystem.UnregisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
95: }
96: 
97: /// <summary>
98: /// Timer initialization
99: ///
100: /// Add a timer interrupt handler, calculate the number of clock ticks per timer tick, set the next timer deadline.
101: /// Then enables the timer
102: /// </summary>
103: void Timer::Initialize()
104: {
105:     if (m_isInitialized)
106:         return;
107: 
108:     memset(m_periodicHandlers, 0, TIMER_MAX_PERIODIC_HANDLERS * sizeof(PeriodicTimerHandler*));
109:     m_interruptSystem.RegisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS, InterruptHandler, this);
110: 
111:     uint64 counterFreq{};
112:     GetTimerFrequency(counterFreq);
113:     assert(counterFreq % TICKS_PER_SECOND == 0);
114:     m_clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
115: 
116:     uint64 counter{};
117:     GetTimerCounter(counter);
118:     SetTimerCompareValue(counter + m_clockTicksPerSystemTick);
119:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
120: 
121:     m_isInitialized = true;
122: }
123: 
124: /// <summary>
125: /// Return the current timer tick cound
126: /// </summary>
127: /// <returns>The current timer tick count</returns>
128: uint64 Timer::GetTicks() const
129: {
130:     return m_ticks;
131: }
132: 
133: /// <summary>
134: /// Return the uptime in seconds
135: /// </summary>
136: /// <returns>Uptime in seconds</returns>
137: uint32 Timer::GetUptime() const
138: {
139:     return m_upTime;
140: }
141: 
142: /// <summary>
143: /// Return the current time in seconds (epoch time)
144: /// </summary>
145: /// <returns>Current time in seconds (epoch time)</returns>
146: uint64 Timer::GetTime() const
147: {
148:     return m_time;
149: }
150: 
151: /// <summary>
152: /// Writes a representation of the current time to a buffer, or of the uptime if the current time is not valid.
153: ///
154: /// For now returns an empty string
155: /// </summary>
156: /// <param name="buffer">Buffer to write the time string to</param>
157: /// <param name="bufferSize">Size of the buffer</param>
158: void Timer::GetTimeString(char* buffer, size_t bufferSize)
159: {
160:     if ((buffer == nullptr) || (bufferSize == 0))
161:     {
162:         return;
163:     }
164:     *buffer = '\0';
165: }
166: 
167: /// <summary>
168: /// Register a periodic timer handler
169: ///
170: /// Registers a periodic timer handler function. The handler function will be called every timer tick.
171: /// </summary>
172: /// <param name="handler">Pointer to periodic timer handler to register</param>
173: void Timer::RegisterPeriodicHandler(PeriodicTimerHandler* handler)
174: {
175:     assert(handler != nullptr);
176:     assert(m_numPeriodicHandlers < TIMER_MAX_PERIODIC_HANDLERS);
177: 
178:     size_t index{};
179:     for (index = 0; index < TIMER_MAX_PERIODIC_HANDLERS; ++index)
180:     {
181:         if (m_periodicHandlers[index] == nullptr)
182:             break;
183:     }
184:     assert(index < TIMER_MAX_PERIODIC_HANDLERS);
185:     m_periodicHandlers[index] = handler;
186: 
187:     DataSyncBarrier();
188: 
189:     m_numPeriodicHandlers++;
190: }
191: 
192: /// <summary>
193: /// Unregister a periodic timer handler
194: ///
195: /// Removes aperiodic timer handler function from the registration. The handler function will no longer be called.
196: /// </summary>
197: /// <param name="handler">Pointer to periodic timer handler to unregister</param>
198: void Timer::UnregisterPeriodicHandler(const PeriodicTimerHandler* handler)
199: {
200:     assert(handler != nullptr);
201:     assert(m_numPeriodicHandlers > 0);
202: 
203:     size_t index{};
204:     for (index = 0; index < TIMER_MAX_PERIODIC_HANDLERS; ++index)
205:     {
206:         if (m_periodicHandlers[index] == handler)
207:             break;
208:     }
209:     assert(index < TIMER_MAX_PERIODIC_HANDLERS);
210:     m_periodicHandlers[index] = nullptr;
211: 
212:     DataSyncBarrier();
213: 
214:     m_numPeriodicHandlers--;
215: }
216: 
...
290: /// <summary>
291: /// Interrupt handler for the timer
292: ///
293: /// Sets the next timer deadline, increments the timer tick count, as well as the time if needed, and calls the periodic handlers.
294: /// </summary>
295: void Timer::InterruptHandler()
296: {
297:     uint64 compareValue;
298:     GetTimerCompareValue(compareValue);
299:     SetTimerCompareValue(compareValue + m_clockTicksPerSystemTick);
300: 
301:     if (++m_ticks % TICKS_PER_SECOND == 0)
302:     {
303:         m_upTime++;
304:         m_time++;
305:     }
306: 
307:     for (unsigned i = 0; i < m_numPeriodicHandlers; i++)
308:     {
309:         if (m_periodicHandlers[i] != nullptr)
310:             (*m_periodicHandlers[i])();
311:     }
312: }
313: 
314: /// <summary>
315: /// Static interrupt handler
316: ///
317: /// Calls the instance interrupt handler
318: /// </summary>
319: /// <param name="param"></param>
320: void Timer::InterruptHandler(void* param)
321: {
322:     Timer* instance = reinterpret_cast<Timer*>(param);
323:     assert(instance != nullptr);
324: 
325:     instance->InterruptHandler();
326: }
327: 
328: /// <summary>
329: /// Retrieves the singleton Timer instance. It is created in the first call to this function.
330: /// </summary>
331: /// <returns>A reference to the singleton Timer</returns>
332: Timer& baremetal::GetTimer()
333: {
334:     static Timer timer;
335:     timer.Initialize();
336:     return timer;
337: }
...
```

- Line 42-47: We need to include the header for `memset()`, `assert()` as well as for the `InterruptSystem` class.
- Line 49-54: We remove the definitions `MSEC_PER_SEC`, `USEC_PER_SEC` and `USEC_PER_MSEC` as they have been moved to `Timer.h`
- Line 54-67: We update the constructor of the `Timer` class to initialize the newly added member variables
- Line 69-83: We update the special constructor of the `Timer` class to initialize the newly added member variables
- Line 85-95: We implement the destructor to disable the timer and the timer interrupt
- Line 97-122: We implement the method `Initialize` to initialize the timer
  - Line 105-106: We check if the timer is already initialized
  - Line 108: We clear the array of periodic handlers
  - Line 109: We register the timer interrupt handler
  - Line 111-114: We calculate the number of clock ticks per timer tick
  - Line 117-118: We set the timer compare value to the current timer counter plus the number of clock ticks per timer tick
  - Line 119: We enable the timer
  - Line 121: We set the timer as initialized
- Line 124-131: We implement the method `GetTicks`
- Line 133-140: We implement the method `GetUptime`
- Line 142-149: We implement the method `GetTime`
- Line 151-165: We make a few small changes to `GetTimeString()`.
If the buffer pointer is null or the size is 0, we return immediately
- Line 167-190: We implement the method `RegisterPeriodicHandler`
  - Line 175: We perform a sanity check if the handler is not null
  - Line 176: We perform a sanity check if the number of registered periodic handlers is not the maximum
  - Line 178-183: We find an empty slot in the array of periodic handlers
  - Line 184: We perform a sanity check whether we found an empty slot
  - Line 185: We store the handler in the array
  - Line 189: We increment the number of periodic handlers
- Line 192-216: We implement the method `UnregisterPeriodicHandler`
  - Line 200: We perform a sanity check if the handler is not null
  - Line 201: We perform a sanity check if there are periodic handlers registered
  - Line 203-208: We find the handler in the array of periodic handlers
  - Line 209: We perform a sanity check whether we found the handler
  - Line 210: We remove the handler from the array
  - Line 214: We decrement the number of periodic handlers
- Line 290-312: We implement the instance method `InterruptHandler`
  - Line 298: We retrieve the current timer compare value
  - Line 299: We set the next timer deadline
  - Line 301-305: We increment the timer tick count, we check if a second has passed, if so we increment the uptime and the time
  - Line 307-311: We call the periodic handlers
- Line 314-326: We implement the static method `InterruptHandler`
  - Line 322-323: We cast the `param` to a `Timer` instance and perform a sanity check
  - Line 325: We call the instance method `InterruptHandler`
- Line 335: We call `Initialize()` in `GetTimer()`

### Update application code {#TUTORIAL_20_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_UPDATE_APPLICATION_CODE}

We'll make a few small changes to use the new `InterruptSystem` implementation.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/InterruptHandler.h"
5: #include "baremetal/Interrupts.h"
6: #include "baremetal/Logger.h"
7: #include "baremetal/System.h"
8: #include "baremetal/Timer.h"
9: 
10: LOG_MODULE("main");
11: 
12: using namespace baremetal;
13: 
14: void PeriodicHandler()
15: {
16:     LOG_INFO("Ping");
17: }
18: 
19: int main()
20: {
21:     auto& console = GetConsole();
22: 
23:     auto exceptionLevel = CurrentEL();
24:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
25: 
26:     Timer &timer = GetTimer();
27:     timer.RegisterPeriodicHandler(PeriodicHandler);
28: 
29:     LOG_INFO("Wait 5 seconds");
30:     Timer::WaitMilliSeconds(5000);
31: 
32:     timer.UnregisterPeriodicHandler(PeriodicHandler);
33: 
34:     console.Write("Press r to reboot, h to halt\n");
35:     char ch{};
36:     while ((ch != 'r') && (ch != 'h'))
37:     {
38:         ch = console.ReadChar();
39:         console.WriteChar(ch);
40:     }
41: 
42:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
43: }
```

- Line 14-27: We change the callback function to `PeriodicHandler` which will be called every timer tick.
We longer need to set the timer deadline, as that is done by the Timer class
- Line 26: We retrieve the singleton instance of the `Timer` class
- Line 27: We register the `PeriodicHandler` function as a periodic handler
- Line 32: We unregister the `PeriodicHandler` function as a periodic handler

### Configuring, building and debugging {#TUTORIAL_20_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now show a line for each timer tick, so around 500 in total.

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:92)
Info   Starting up (System:211)
Info   Current EL: 1 (main:24)
Info   Wait 5 seconds (main:29)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
Info   Ping (main:16)
...
Press r to reboot, h to halt...
```

## Updating the time string - Step 2 {#TUTORIAL_20_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2}

Until now, we return an empty string for the time. We'll update this to return the actual time, or the uptime in hours, minutes, seconds and microseconds if we don't have the actual time.

### Timer.h {#TUTORIAL_20_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_TIMERH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
101:     /// @brief Number of days is each month (0 = January, etc.)
102:     static const unsigned s_daysInMonth[12];
103:     /// @brief Name of each month (0 = January, etc.)
104:     static const char* s_monthName[12];
...
132:     static bool IsLeapYear(unsigned year);
133:     static unsigned GetDaysInMonth(unsigned month, unsigned year);
134: 
```

- Line 101-102: We define an array `s_daysInMonth` to hold the number of days in each month
- Line 103-104: We define an array `s_monthName` to hold the name of each month
- Line 132: We add a static method `IsLeapYear()` to check if a year is a leap year
- Line 133: We add a static method `GetDaysInMonth()` to retrieve the number of days in a month

### Timer.cpp {#TUTORIAL_20_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_TIMERCPP}

Let's implement the newly added methods in the `Timer` class, and update the `GetTimeString()` method to use them.

Update the file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
...
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/Assert.h"
44: #include "baremetal/BCMRegisters.h"
45: #include "baremetal/Format.h"
46: #include "baremetal/InterruptHandler.h"
47: #include "baremetal/MemoryAccess.h"
48: #include "stdlib/Util.h"
...
151: /// <summary>
152: /// Writes a representation of the current time to a buffer, or of the uptime if the current time is not valid.
153: ///
154: /// The current time will be in the format "MMM dd HH:MM:SS.mmm", according to our time zone, if the time is valid.
155: /// If the time is not known yet, it we be the uptime in the format "ddd.HH:MM:SS.mmm".
156: /// If not yet initialized, an empty string is returned
157: /// </summary>
158: /// <param name="buffer">Buffer to write the time string to</param>
159: /// <param name="bufferSize">Size of the buffer</param>
160: void Timer::GetTimeString(char* buffer, size_t bufferSize)
161: {
162:     uint64 time  = m_time;
163:     uint64 ticks = m_ticks;
164:
165:     if (bufferSize == 0)
166:     {
167:         return;
168:     }
169:     if (!m_isInitialized)
170:     {
171:         *buffer = '\0';
172:         return;
173:     }
174:
175:     unsigned second = time % 60;
176:     time /= 60; // Time is now in minute
177:     unsigned minute = time % 60;
178:     time /= 60; // Time is now in hour
179:     unsigned hour       = time % 24;
180:     time /= 24; // Time is now in days
181:     unsigned daysTotal = time;
182:
183:     unsigned year = 1970; // Epoch start
184:     while (true)
185:     {
186:         unsigned daysInYear = IsLeapYear(year) ? 366 : 365;
187:         if (time < daysInYear)
188:         {
189:             break;
190:         }
191:
192:         time -= daysInYear;
193:         year++;
194:     }
195:
196:     unsigned month = 0;
197:     while (1)
198:     {
199:         unsigned daysInMonth = GetDaysInMonth(month, year);
200:         if (time < daysInMonth)
201:         {
202:             break;
203:         }
204:
205:         time -= daysInMonth;
206:         month++;
207:     }
208:
209:     unsigned monthDay = time + 1;
210:
211: #if (TICKS_PER_SECOND != MSEC_PER_SEC)
212:     ticks = ticks * MSEC_PER_SEC / TICKS_PER_SECOND;
213: #endif
214:     auto   milliSeconds = ticks % MSEC_PER_SEC;
215:
216:     if (year > 1975) // Just a sanity check to see if we have an actual time
217:     {
218:         FormatNoAlloc(buffer, bufferSize, "%s %2u, %04u %02u:%02u:%02u.%03u", s_monthName[month], monthDay, year, hour, minute, second, milliSeconds);
219:     }
220:     else
221:     {
222:         FormatNoAlloc(buffer, bufferSize, "%u.%02u:%02u:%02u.%03u", daysTotal, hour, minute, second, milliSeconds);
223:     }
224: }
225:
...
349: /// <summary>
350: /// Determine if the specified year is a leap year
351: /// </summary>
352: /// <param name="year">Year</param>
353: /// <returns>Returns true if year is a leap year, false otherwise</returns>
354: bool Timer::IsLeapYear(unsigned year)
355: {
356:     if (year % 100 == 0)
357:     {
358:         return year % 400 == 0;
359:     }
360:
361:     return year % 4 == 0;
362: }
363:
364: /// <summary>
365: /// Calculates the number days in the specified month of the specified year
366: /// </summary>
367: /// <param name="month">Month, 0=January, 1=February, etc.</param>
368: /// <param name="year">Year</param>
369: /// <returns></returns>
370: unsigned Timer::GetDaysInMonth(unsigned month, unsigned year)
371: {
372:     if (month == 1 && IsLeapYear(year))
373:     {
374:         return 29;
375:     }
376:
377:     return s_daysInMonth[month];
378: }
379:
...
```

- Line 45: We need to include the header for `FormatNoAlloc()`
- Line 55-57: We initialize the constants `s_daysInMonth` and `s_monthName`
- Line 156-229: We update the method `GetTimeString()`
  - Line 167-168: We retrieve the time and the ticks
  - Line 170-173: We check if the buffer can hold any data, if not we simply return
  - Line 174-178: We check if the timer is initialized, if not we return an empty string
  - Line 180-186: We calculate the seconds, minutes, hours and days
  - Line 188-199: We calculate the year
  - Line 201-212: We calculate the month
  - Line 214: We calculate the day of the month
  - Line 216-218: We convert the ticks to milliseconds
  - Line 221: We check whether the year has a plausible value, if not we do not have actual time, but uptime
  - Line 223: We format the time string as "[MMM dd ]HH:MM:SS.mmm" if we have the actual time
  - Line 227: We format the time string as "ddd.HH:MM:SS.mmm" if we don't have the actual time
- Line 354-367: We implement the static method `IsLeapYear` to check if a year is a leap year
- Line 369-383: We implement the static method `GetDaysInMonth` to retrieve the number of days in a month

### Update application code {#TUTORIAL_20_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_UPDATE_APPLICATION_CODE}

We'll leave the application code as is.

### Configuring, building and debugging {#TUTORIAL_20_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now show the "time" in each log line.

```text
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.010 Starting up (System:209)
Info   0.00:00:00.010 Current EL: 1 (main:28)
Info   0.00:00:00.020 Wait 5 seconds (main:33)
Info   0.00:00:00.030 Ping (main:20)
Info   0.00:00:00.040 Ping (main:20)
Info   0.00:00:00.050 Ping (main:20)
Info   0.00:00:00.060 Ping (main:20)
Info   0.00:00:00.070 Ping (main:20)
...
Info   0.00:00:04.960 Ping (main:20)
Info   0.00:00:04.970 Ping (main:20)
Info   0.00:00:04.980 Ping (main:20)
Press r to reboot, h to halt
```

## Adding kernel timers - Step 3 {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3}

Let's add so called kernel timers to the `Timer` class. These timers can be used to trigger a function after a certain amount of time.
They are basically one-shot timers which are automatically updated on each timer tick, and once the time has passed, the function is called.

We'll start with a bit of plumbing however.

We'll first introduce a double linked list for our timers.

### List.h {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_LISTH}

Create a new file `code/libraries/baremetal/include/baremetal/List.h`

```cpp
File: code/libraries/baremetal/include/baremetal/List.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : PointerList.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : DoubleLinkedList
9: //
10: // Description : Template for pointer list class
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
42: #include "baremetal/Assert.h"
43:
44: namespace baremetal {
45:
46: // Magic number for list (PLMC)
47: #define PTR_LIST_MAGIC 0x504C4D43
48:
49: /// <summary>
50: /// Doubly linked list template of pointers
51: /// </summary>
52: /// <typeparam name="Pointer"></typeparam>
53: template<class Pointer>
54: class DoubleLinkedList
55: {
56: public:
57:     /// <summary>
58:     /// Element in double linked pointer list
59:     /// </summary>
60:     struct Element
61:     {
62: #ifndef NDEBUG
63:         /// @brief Magic number to check if element is valid
64:         unsigned m_magic;
65: #endif
66:         /// @brief Actual pointer
67:         Pointer  m_ptr;
68:
69:         /// @brief Pointer to previous element
70:         Element *m_prev;
71:         /// @brief Pointer to next element
72:         Element *m_next;
73:
74:         explicit Element(Pointer ptr);
75:
76:         bool CheckMagic() const;
77:     };
78:
79: private:
80:     /// @brief Pointer to first element in list
81:     DoubleLinkedList<Pointer>::Element *m_head;
82:
83: public:
84:     DoubleLinkedList();
85:     ~DoubleLinkedList();
86:
87:     DoubleLinkedList<Pointer>::Element *GetFirst(); // Returns nullptr if list is empty
88:     DoubleLinkedList<Pointer>::Element *GetNext(const DoubleLinkedList<Pointer>::Element *element);
89:     // Returns nullptr if nothing follows
90:
91:     Pointer                             GetPointer(const DoubleLinkedList<Pointer>::Element *element); // get pointer for element
92:
93:     void                                InsertBefore(DoubleLinkedList<Pointer>::Element *before, Pointer pointer); // after must be != nullptr
94:     void                                InsertAfter(DoubleLinkedList<Pointer>::Element *after, Pointer pointer);   // before == nullptr to set first element
95:
96:     void                                Remove(DoubleLinkedList<Pointer>::Element *element); // remove this element
97:
98:     DoubleLinkedList<Pointer>::Element *Find(Pointer pointer); // find element using pointer
99: };
100:
101: /// <summary>
102: /// Construct element for pointer
103: /// </summary>
104: /// <typeparam name="Pointer"></typeparam>
105: /// <param name="ptr">ptr Pointer to store in element</param>
106: template<class Pointer>
107: DoubleLinkedList<Pointer>::Element::Element(Pointer ptr)
108:     : m_magic{PTR_LIST_MAGIC}
109:     , m_ptr{ptr}
110:     , m_prev{}
111:     , m_next{}
112: {
113: }
114:
115: /// <summary>
116: /// Verify magic number
117: /// </summary>
118: /// <typeparam name="Pointer"></typeparam>
119: /// <returns>True if the magic number is correct, false otherwise</returns>
120: template<class Pointer>
121: bool DoubleLinkedList<Pointer>::Element::CheckMagic() const
122: {
123: #ifndef NDEBUG
124:     return (m_magic == PTR_LIST_MAGIC);
125: #else
126:     return true;
127: #endif
128: }
129:
130: /// <summary>
131: /// Construct a default double linked list
132: /// </summary>
133: /// <typeparam name="Pointer"></typeparam>
134: template<class Pointer>
135: DoubleLinkedList<Pointer>::DoubleLinkedList()
136:     : m_head{}
137: {
138: }
139:
140: /// <summary>
141: /// Destruct a double linked list
142: /// </summary>
143: /// <typeparam name="Pointer"></typeparam>
144: template<class Pointer>
145: DoubleLinkedList<Pointer>::~DoubleLinkedList()
146: {
147:     assert(m_head == nullptr);
148: }
149:
150: /// <summary>
151: /// Get the first element in the list
152: /// </summary>
153: /// <typeparam name="Pointer"></typeparam>
154: /// <returns>Pointer to first element in the list, or nullptr if none exists</returns>
155: template<class Pointer>
156: typename DoubleLinkedList<Pointer>::Element *DoubleLinkedList<Pointer>::GetFirst()
157: {
158:     return m_head;
159: }
160:
161: /// <summary>
162: /// Get the next element in the list
163: /// </summary>
164: /// <typeparam name="Pointer"></typeparam>
165: /// <param name="element">Current element</param>
166: /// <returns>Pointer to next element, or nullptr if none exists</returns>
167: template<class Pointer>
168: typename DoubleLinkedList<Pointer>::Element *DoubleLinkedList<Pointer>::GetNext(const DoubleLinkedList<Pointer>::Element *element)
169: {
170:     assert(element != nullptr);
171:     assert(element->CheckMagic());
172:
173:     return element->m_next;
174: }
175:
176: /// <summary>
177: /// Extract pointer from element
178: /// </summary>
179: /// <typeparam name="Pointer"></typeparam>
180: /// <param name="element">Current element</param>
181: /// <returns>Pointer stored inside element</returns>
182: template<class Pointer>
183: Pointer DoubleLinkedList<Pointer>::GetPointer(const typename DoubleLinkedList<Pointer>::Element *element)
184: {
185:     assert(element != nullptr);
186:     assert(element->CheckMagic());
187:
188:     return element->m_ptr;
189: }
190:
191: /// <summary>
192: /// Insert a pointer before a given element
193: /// </summary>
194: /// <typeparam name="Pointer"></typeparam>
195: /// <param name="before">Pointer to element before which to store e new element for the pointer</param>
196: /// <param name="pointer">Pointer to store in new element</param>
197: template<class Pointer>
198: void DoubleLinkedList<Pointer>::InsertBefore(typename DoubleLinkedList<Pointer>::Element *before, Pointer pointer)
199: {
200:     assert(m_head != nullptr);
201:     assert(before != nullptr);
202:     assert(before->CheckMagic());
203:
204:     Element *element = new Element(pointer);
205:     assert(element != nullptr);
206:
207:     if (before == m_head)
208:     {
209:         element->m_prev = nullptr;
210:         element->m_next = before;
211:
212:         m_head->m_prev = element;
213:
214:         m_head = element;
215:     }
216:     else
217:     {
218:         element->m_prev = before->m_prev;
219:         element->m_next = before;
220:
221:         if (before->m_prev != nullptr)
222:         {
223:             assert(before->m_prev->CheckMagic());
224:             before->m_prev->m_next = element;
225:         }
226:
227:         before->m_prev = element;
228:     }
229: }
230:
231: /// <summary>
232: /// Insert a pointer after a given element
233: /// </summary>
234: /// <typeparam name="Pointer"></typeparam>
235: /// <param name="after">Pointer to element after which to store e new element for the pointer</param>
236: /// <param name="pointer">Pointer to store in new element</param>
237: template<class Pointer>
238: void DoubleLinkedList<Pointer>::InsertAfter(typename DoubleLinkedList<Pointer>::Element *after, Pointer pointer)
239: {
240:     Element *element = new Element(pointer);
241:     assert(element != nullptr);
242:
243:     if (after == nullptr)
244:     {
245:         assert(m_head == nullptr);
246:
247:         element->m_prev = nullptr;
248:         element->m_next = nullptr;
249:
250:         m_head = element;
251:     }
252:     else
253:     {
254:         assert(m_head != nullptr);
255:         assert(after->CheckMagic());
256:
257:         element->m_prev = after;
258:         element->m_next = after->m_next;
259:
260:         if (after->m_next != nullptr)
261:         {
262:             assert(after->m_next->CheckMagic());
263:             after->m_next->m_prev = element;
264:         }
265:
266:         after->m_next = element;
267:     }
268: }
269:
270: /// <summary>
271: /// Remove an element
272: /// </summary>
273: /// <typeparam name="Pointer"></typeparam>
274: /// <param name="element">Pointer to element to remove</param>
275: template<class Pointer>
276: void DoubleLinkedList<Pointer>::Remove(typename DoubleLinkedList<Pointer>::Element *element)
277: {
278:     assert(element != nullptr);
279:     assert(element->CheckMagic());
280:
281:     if (element == m_head)
282:     {
283:         m_head = element->m_next;
284:
285:         if (element->m_next != nullptr)
286:         {
287:             assert(element->m_next->CheckMagic());
288:             element->m_next->m_prev = nullptr;
289:         }
290:     }
291:     else
292:     {
293:         assert(element->m_prev != nullptr);
294:         assert(element->m_prev->CheckMagic());
295:         element->m_prev->m_next = element->m_next;
296:
297:         if (element->m_next != nullptr)
298:         {
299:             assert(element->m_next->CheckMagic());
300:             element->m_next->m_prev = element->m_prev;
301:         }
302:     }
303:
304: #ifndef NDEBUG
305:     element->m_magic = 0;
306: #endif
307:     delete element;
308: }
309:
310: /// <summary>
311: /// Find the element containing a pointer
312: /// </summary>
313: /// <typeparam name="Pointer"></typeparam>
314: /// <param name="pointer">Pointer to search for</param>
315: /// <returns>Pointer stored inside element</returns>
316: template<class Pointer>
317: typename DoubleLinkedList<Pointer>::Element *DoubleLinkedList<Pointer>::Find(Pointer pointer)
318: {
319:     for (Element *element = m_head; element != nullptr; element = element->m_next)
320:     {
321:         assert(element->CheckMagic());
322:
323:         if (element->m_ptr == pointer)
324:         {
325:             return element;
326:         }
327:     }
328:
329:     return nullptr;
330: }
331:
332: } // namespace baremetal
```

- Line 47: We define a magic number for the list
- Line 53-99: We declare a template class `DoubleLinkedList` to hold a list of pointers
  - Line 60-77: We declare a struct `Element` to be an element in the list
    - Line 62-72: We declare the magic number, the pointer, and the previous and next element
    - Line 74: We declare a constructor for the element
    - Line 76: We declare a method `CheckMagic()` to check the magic number
  - Line 80-81: We declare the head of the list
  - Line 84: We declare a constructor for the list
  - Line 85: We declare a destructor for the list
  - Line 87-88-97: We declare methods to get the first `GetFirst()` and next element `GetNext()`
  - Line 91: We declare a method `GetPointer()` to extract the pointer of an element
  - Line 93-94: We declare methods `InsertBefore()` and `InsertAfter()`, to insert a new element before or after an element
  - Line 96: We declare a method `Remove()` to remove an element
  - Line 98: We declare a method `Find()` to find an element by the pointer
  - Line 106-113: We implement the `Element` constructor for the element
  - Line 120-128: We implement the `Element` method `CheckMagic()`
  - Line 134-138: We implement the `DoubleLinkedList` constructor
  - Line 144-148: We implement the `DoubleLinkedList` destructor
  - Line 155-159: We implement the `DoubleLinkedList` method `GetFirst()`
  - Line 167-174: We implement the `DoubleLinkedList` method `GetNext()`
  - Line 182-189: We implement the `DoubleLinkedList` method `GetPointer()`
  - Line 197-229: We implement the `DoubleLinkedList` method `InsertBefore()`
  - Line 237-268: We implement the `DoubleLinkedList` method `InsertAfter()`
  - Line 275-308: We implement the `DoubleLinkedList` method `Remove()`
  - Line 316-330: We implement the `DoubleLinkedList` method `Find()`

### Timer.h {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_TIMERH}

Update the new file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
45: #include "stdlib/Types.h"
46: #include "baremetal/List.h"
...
71: struct KernelTimer;
72:
73: /// @brief Handle to a kernel timer
74: using KernelTimerHandle = uintptr;
75:
76: /// @brief Kernel timer handler
77: using KernelTimerHandler = void(KernelTimerHandle timerHandle, void *param, void *context);
78:
...
107:     /// @brief Periodic tick handler functions
108:     PeriodicTimerHandler *m_periodicHandlers[TIMER_MAX_PERIODIC_HANDLERS];
109:     /// @brief Number of periodic tick handler functions installed
110:     volatile unsigned     m_numPeriodicHandlers;
111:     /// @brief Kernel timer list
112:     DoubleLinkedList<KernelTimer *> m_kernelTimerList;
113:     /// @brief Number of days is each month (0 = January, etc.)
114:     static const unsigned s_daysInMonth[12];
115:     /// @brief Name of each month (0 = January, etc.)
116:     static const char    *s_monthName[12];
...
121:     Timer(IMemoryAccess &memoryAccess);
122:     ~Timer();
123:
124:     void Initialize();
125:
126:     uint64 GetTicks() const;
127:
128:     uint32 GetUptime() const;
129:
130:     uint64 GetTime() const;
131:
132:     void GetTimeString(char* buffer, size_t bufferSize);
133:
134:     void RegisterPeriodicHandler(PeriodicTimerHandler *handler);
135:     void UnregisterPeriodicHandler(PeriodicTimerHandler *handler);
136:
137:     KernelTimerHandle StartKernelTimer(uint32 delayTicks, KernelTimerHandler *handler, void *param = nullptr, void *context = nullptr);
138:     void CancelKernelTimer(KernelTimerHandle handle);
139:
140:     static void WaitCycles(uint32 numCycles);
141:
142:     uint64 GetSystemTimer();
143:
144:     static void WaitMilliSeconds(uint64 msec);
145:     static void WaitMicroSeconds(uint64 usec);
146:
147:     static bool IsLeapYear(unsigned year);
148:     static unsigned GetDaysInMonth(unsigned month, unsigned year);
149:
150: private:
151:     void PollKernelTimers();
152:     void InterruptHandler();
153:     static void InterruptHandler(void *param);
...
```

- Line 46: We include the header for the `DoubleLinkedList` class
- Line 74: We define the type for the kernel timer handle `KernelTimerHandle`
- Line 77: We define the type for the kernel timer handler function `KernelTimerHandler`
- Line 112: We declare a member variable for the list of kernel timers `m_kernelTimerList`
- Line 137: We declare a method `StartKernelTimer()` to start a kernel timer
- Line 138: We declare a method `CancelKernelTimer()` to cancel a kernel timer
- Line 151: We declare a method `PollKernelTimers()` to check all registered kernel timers for expiration, and handle them

### Timer.cpp {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_TIMERCPP}

Update the new file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
..
54: namespace baremetal {
55:
56: /// @brief Define log name
57: LOG_MODULE("Timer");
58:
59: /// @brief Magic number for kernel timer (KTMC)
60: #define KERNEL_TIMER_MAGIC 0x4B544D43
61:
62: /// <summary>
63: /// Kernel timer administration
64: /// </summary>
65: /// <typeparam name="Pointer"></typeparam>
66: struct KernelTimer
67: {
68: #ifndef NDEBUG
69:     /// @brief Magic number to check if element is valid
70:     unsigned m_magic;
71: #endif
72:     /// @brief Kernel timer deadline in timer ticks
73:     unsigned            m_elapsesAtTicks;
74:     /// @brief Pointer to kernel timer handler
75:     KernelTimerHandler *m_handler;
76:     /// @brief Kernel timer handler parameter
77:     void               *m_param;
78:     /// @brief Kernel timer handler context
79:     void               *m_context;
80:
81:     /// <summary>
82:     /// Construct a kernel timer administration element
83:     /// </summary>
84:     /// <param name="elapseTimeTicks">Timer deadline in timer ticks</param>
85:     /// <param name="handler">Kernel timer handler pointer</param>
86:     /// <param name="param">Kernel timer handler parameter</param>
87:     /// <param name="context">Kernerl timer handler context</param>
88:     KernelTimer(unsigned elapseTimeTicks, KernelTimerHandler *handler, void *param, void *context)
89:         :
90: #ifndef NDEBUG
91:           m_magic{KERNEL_TIMER_MAGIC}
92:         ,
93: #endif
94:           m_elapsesAtTicks{elapseTimeTicks}
95:         , m_handler{handler}
96:         , m_param{param}
97:         , m_context{context}
98:
99:     {
100:     }
101:     /// <summary>
102:     /// Verify magic number
103:     /// </summary>
104:     /// <returns>True if the magic number is correct, false otherwise</returns>
105:     bool CheckMagic() const
106:     {
107:         return m_magic == KERNEL_TIMER_MAGIC;
108:     }
109: };
110: /// @brief Kernel timer element, element which is stored in the kernel time list
111: using KernelTimerElement = DoubleLinkedList<KernelTimer *>::Element;
112:
113: const unsigned Timer::s_daysInMonth[12]{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
114:
115: const char    *Timer::s_monthName[12]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
116:
117: /// <summary>
118: /// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
119: /// </summary>
120: Timer::Timer()
121:     : m_interruptSystem{GetInterruptSystem()}
122:     , m_memoryAccess{ GetMemoryAccess() }
123:     , m_clockTicksPerSystemTick{}
124:     , m_ticks{}
125:     , m_upTime{}
126:     , m_time{}
127:     , m_periodicHandlers{}
128:     , m_numPeriodicHandlers{}
129:     , m_kernelTimerList{}
130: {
131: }
132:
133: /// <summary>
134: /// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
135: /// </summary>
136: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
137: Timer::Timer(IMemoryAccess &memoryAccess)
138:     : m_interruptSystem{GetInterruptSystem()}
139:     , m_memoryAccess{ memoryAccess }
140:     , m_clockTicksPerSystemTick{}
141:     , m_ticks{}
142:     , m_upTime{}
143:     , m_time{}
144:     , m_periodicHandlers{}
145:     , m_numPeriodicHandlers{}
146:     , m_kernelTimerList{}
147: {
148: }
149:
150: /// <summary>
151: /// Destructor
152: ///
153: /// Disables the timer, as well as the timer interrupt
154: /// </summary>
155: Timer::~Timer()
156: {
157:     SetTimerControl(~CNTP_CTL_EL0_ENABLE);
158:
159:     m_interruptSystem.UnregisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
160:
161:     KernelTimerElement *element;
162:     while ((element = m_kernelTimerList.GetFirst()) != 0)
163:     {
164:         CancelKernelTimer(reinterpret_cast<KernelTimerHandle>(m_kernelTimerList.GetPointer(element)));
165:     }
166: }
167:
...
346: /// <summary>
347: /// Starts a kernel timer. After delayTicks timer ticks, it elapses and call the kernel timer handler.
348: /// </summary>
349: /// <param name="delayTicks">Delay time for timer in timer ticks</param>
350: /// <param name="handler">Kernel timer handler to call when time elapses</param>
351: /// <param name="param">Parameter to pass to kernel timer handler</param>
352: /// <param name="context">Kernel timer handler context</param>
353: /// <returns>Handle to kernel timer</returns>
354: KernelTimerHandle Timer::StartKernelTimer(unsigned delayTicks, KernelTimerHandler *handler, void *param, void *context)
355: {
356:     unsigned elapseTimeTicks = m_ticks + delayTicks;
357:     assert(handler != nullptr);
358:
359:     KernelTimer *timer = new KernelTimer(elapseTimeTicks, handler, param, context);
360:     assert(timer != nullptr);
361:     LOG_DEBUG("Create new timer to expire at %d ticks, handle %p", elapseTimeTicks, timer);
362:
363:     KernelTimerElement *prevElement{};
364:     KernelTimerElement *element = m_kernelTimerList.GetFirst();
365:     while (element != nullptr)
366:     {
367:         const KernelTimer *timer2 = m_kernelTimerList.GetPointer(element);
368:         assert(timer2 != nullptr);
369:         assert(timer2->m_magic == KERNEL_TIMER_MAGIC);
370:
371:         if (static_cast<int>(timer2->m_elapsesAtTicks - elapseTimeTicks) > 0)
372:         {
373:             break;
374:         }
375:
376:         prevElement = element;
377:         element     = m_kernelTimerList.GetNext(element);
378:     }
379:
380:     if (element != nullptr)
381:     {
382:         m_kernelTimerList.InsertBefore(element, timer);
383:     }
384:     else
385:     {
386:         m_kernelTimerList.InsertAfter(prevElement, timer);
387:     }
388:
389:     return reinterpret_cast<KernelTimerHandle>(timer);
390: }
391:
392: /// <summary>
393: /// Cancels and removes a kernel timer.
394: /// </summary>
395: /// <param name="handle">Handle to kernel timer to cancel</param>
396: void Timer::CancelKernelTimer(KernelTimerHandle handle)
397: {
398:     KernelTimer *timer = reinterpret_cast<KernelTimer *>(handle);
399:     assert(timer != 0);
400:     LOG_DEBUG("Cancel timer, expire time %d ticks, handle %p", timer->m_elapsesAtTicks, timer);
401:
402:     KernelTimerElement *element = m_kernelTimerList.Find(timer);
403:     if (element != nullptr)
404:     {
405:         assert(timer->m_magic == KERNEL_TIMER_MAGIC);
406:
407:         m_kernelTimerList.Remove(element);
408:
409: #ifndef NDEBUG
410:         timer->m_magic = 0;
411: #endif
412:         delete timer;
413:     }
414: }
415:
416: /// <summary>
417: /// Update all registered kernel timers, and handle expiration of timers
418: /// </summary>
419: void Timer::PollKernelTimers()
420: {
421:     auto element = m_kernelTimerList.GetFirst();
422:     while (element != nullptr)
423:     {
424:         KernelTimer *timer = m_kernelTimerList.GetPointer(element);
425:         assert(timer != nullptr);
426:         assert(timer->m_magic == KERNEL_TIMER_MAGIC);
427:
428:         if (static_cast<int>(timer->m_elapsesAtTicks - m_ticks) > 0)
429:         {
430:             break;
431:         }
432:
433:         LOG_DEBUG("Expire timer, expire time %d ticks, handle %p", timer->m_elapsesAtTicks, timer);
434:
435:         m_kernelTimerList.Remove(element);
436:
437:         KernelTimerHandler *handler = timer->m_handler;
438:         assert(handler != nullptr);
439:         (*handler)(reinterpret_cast<KernelTimerHandle>(timer), timer->m_param, timer->m_context);
440:
441: #ifndef NDEBUG
442:         timer->m_magic = 0;
443: #endif
444:         delete timer;
445:
446:         // The list may have changed due to the handler callback, so re-initialize
447:         element = m_kernelTimerList.GetFirst();
448:     }
449: }
450:
...
561: void Timer::InterruptHandler()
562: {
563:     uint64 compareValue;
564:     GetTimerCompareValue(compareValue);
565:     SetTimerCompareValue(compareValue + m_clockTicksPerSystemTick);
566:
567:     if (++m_ticks % TICKS_PER_SECOND == 0)
568:     {
569:         m_upTime++;
570:         m_time++;
571:     }
572:
573:     PollKernelTimers();
574:
575:     for (unsigned i = 0; i < m_numPeriodicHandlers; i++)
576:     {
577:         if (m_periodicHandlers[i] != nullptr)
578:             (*m_periodicHandlers[i])();
579:     }
580: }
```

- Line 54: We place the implementation inside the `baremetal` namespace
- Line 57: We define the log module name
- Line 60: We define a magic number for the kernel timer administration
- Line 66-109: We declare a struct `KernelTimer` to hold the kernel timer administration
  - Line 68-79: We declare the magic number, the timer deadline in ticks, the handler function pointer, the handler parameter, and the handler context
  - Line 88-100: We declare and implement the constructor for `KernelTimer`
  - Line 105-108: We declare and implement a method `CheckMagic()` to check the magic number
- Line 113: We define the values for the days per month
- Line 115: We define the values for the month names
- Line 129: We update the constructor to initialize the kernel timer list
- Line 146: We update the special constructor to initialize the kernel timer list
- Line 161-164: We update the destructor to clean up the timers
- Line 354-390: We implement the method `StartKernelTimer()` to start a kernel timer
  - Line 356: We calculate the time the timer should expire
  - Line 357: We perform a sanity check that the handler is valid
  - Line 359-360: We create a new kernel timer, and verify it was created
  - Line 363-378: We first find the correct position in the list to insert the new timer (the list is order by expiration time)
  - Line 380-387: We then insert the timer in the list at the correct position
- Line 396-414: We implement the method `CancelKernelTimer()` to cancel a kernel timer
  - Line 398-399: We convert the handle back to a pointer, and verify it is valid
  - Line 402-413: We find the element in the list, and remove it
- Line 419-449: We implement the method `PollKernelTimers()` to check all registered kernel timers for expiration, and handle them
  - Line 421: We get the first element in the list
  - Line 422-448: We loop through all elements in the list
     - Line 424: We get the timer from the element, and verify it is valid, and has the correct magic number
     - Line 428-431: We check if the timer has expired, if not we skip the timer
     - Line 435: If it did expire we remove the timer from the list
     - Line 437-439: We extract the handler, verify it is valid, and call it
     - Line 444: We delete the timer
     - Line 447: We re-initialize the element, as the list may have changed
- Line 573: We call `PollKernelTimers()` from the interrupt handler, to update the kernel timers

### Update CMake file {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_UPDATE_CMAKE_FILE}

As we have added some source files to the `baremetal` library, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`
```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionHandler.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionStub.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/InterruptHandler.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Interrupts.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Malloc.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
53:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
54:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
55:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
56:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
57:     )
58:
59: set(PROJECT_INCLUDES_PUBLIC
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMRegisters.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Exception.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ExceptionHandler.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/InterruptHandler.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Interrupts.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Iterator.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/List.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Malloc.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
88:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
89:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
90:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Synchronization.h
91:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
92:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
93:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
94:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
95:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
96:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
97:     )
98: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_UPDATE_APPLICATION_CODE}

We'll updat the application code to user kernel timers.

Update the file `code/applications/baremetal/src/main.cpp`

```cpp
File: code/applications/baremetal/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/InterruptHandler.h"
5: #include "baremetal/Interrupts.h"
6: #include "baremetal/Logger.h"
7: #include "baremetal/System.h"
8: #include "baremetal/Timer.h"
9:
10: LOG_MODULE("main");
11:
12: using namespace baremetal;
13:
14: #define TICKS_PER_SECOND 2 // Timer ticks per second
15:
16: void KernelTimerHandler3(KernelTimerHandle /*timerHandle*/, void */*param*/, void */*context*/)
17: {
18:     LOG_INFO("Timer 3 will never expire in time");
19: }
20:
21: void KernelTimerHandler2(KernelTimerHandle /*timerHandle*/, void */*param*/, void */*context*/)
22: {
23:     LOG_INFO("Timer 2 expired");
24: }
25:
26: void KernelTimerHandler1(KernelTimerHandle /*timerHandle*/, void */*param*/, void */*context*/)
27: {
28:     LOG_INFO("Timer 1 expired");
29:     LOG_INFO("Starting kernel timer 2 to fire in 2 seconds");
30:     GetTimer().StartKernelTimer(200, KernelTimerHandler2, nullptr, nullptr);
31: }
32:
33: int main()
34: {
35:     auto& console = GetConsole();
36:
37:     auto exceptionLevel = CurrentEL();
38:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
39:
40:     Timer &timer = GetTimer();
41:     LOG_INFO("Starting kernel timer 1 to fire in 1 second");
42:     auto timer1Handle = timer.StartKernelTimer(100, KernelTimerHandler1, nullptr, nullptr);
43:
44:     LOG_INFO("Starting kernel timer 3 to fire in 10 seconds");
45:     auto timer3Handle = timer.StartKernelTimer(1000, KernelTimerHandler3, nullptr, nullptr);
46:
47:     LOG_INFO("Wait 5 seconds");
48:     Timer::WaitMilliSeconds(5000);
49:
50:     LOG_INFO("Cancelling kernel timer 3");
51:     timer.CancelKernelTimer(timer3Handle);
52:
53:     console.Write("Press r to reboot, h to halt\n");
54:     char ch{};
55:     while ((ch != 'r') && (ch != 'h'))
56:     {
57:         ch = console.ReadChar();
58:         console.WriteChar(ch);
59:     }
60:
61:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
62: }
63:
```

We create three kernel timers, one which will never expire in time, one which will expire in 1 second and starts a third, to expire after 2 seconds.

We expect the first one (timer 1) to expire, and start the second one (timer 2) after 1 second. We expect the second one to expire after 2 seconds.
We expect the third one (timer 3) to never expire in time, and be cancelled after 5 seconds.

### Configuring, building and debugging {#TUTORIAL_20_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now show the "time" in each log line.

```text
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.010 Starting up (System:209)
Info   0.00:00:00.010 Current EL: 1 (main:38)
Info   0.00:00:00.020 Starting kernel timer 1 to fire in 1 second (main:41)
Debug  0.00:00:00.030 Create new timer to expire at 103 ticks, handle 600D00 (Timer:376)
Info   0.00:00:00.040 Starting kernel timer 3 to fire in 10 seconds (main:44)
Debug  0.00:00:00.040 Create new timer to expire at 1004 ticks, handle 600E00 (Timer:376)
Info   0.00:00:00.050 Wait 5 seconds (main:47)
Debug  0.00:00:01.030 Expire timer, expire time 103 ticks, handle 600D00 (Timer:448)
Info   0.00:00:01.030 Timer 1 expired (main:28)
Info   0.00:00:01.030 Starting kernel timer 2 to fire in 2 seconds (main:29)
Debug  0.00:00:01.030 Create new timer to expire at 303 ticks, handle 600D80 (Timer:376)
Debug  0.00:00:03.030 Expire timer, expire time 303 ticks, handle 600D80 (Timer:448)
Info   0.00:00:03.030 Timer 2 expired (main:23)
Info   0.00:00:05.010 Cancelling kernel timer 3 (main:50)
Debug  0.00:00:05.010 Cancel timer, expire time 1004 ticks, handle 600E00 (Timer:415)
Press r to reboot, h to halt
```

You can see that at 0.02 seconds (tick 2), timer 1 is started, which is actually created at 0.03 seconds (tick 3) to expire at 1.03 seconds (tick 103).
At 0.04 seconds (tick 4), the timer 3 is started, to expire at 10.04 seconds (tick 1004).
At 1.03 seconds (tick 103), the timer 1 expires, and timer 2 is started, to expire at 3.03 seconds (tick 303). Timer 2 expires at 3.03 seconds (tick 303).
At 5.01 seconds (tick 501), timer 3 is cancelled.

Next: [21-gpio](21-gpio.md)
