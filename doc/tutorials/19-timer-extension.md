# Tutorial 19: Timer extension {#TUTORIAL_19_TIMER_EXTENSION}

@tableofcontents

## New tutorial setup {#TUTORIAL_19_TIMER_EXTENSION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/18-timer-extension`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_19_TIMER_EXTENSION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-19.a`
- a library `output/Debug/lib/stdlib-19.a`
- an application `output/Debug/bin/19-timer-extension.elf`
- an image in `deploy/Debug/19-timer-extension-image`

## Extending the timer {#TUTORIAL_19_TIMER_EXTENSION_EXTENDING_THE_TIMER}

In tutorial [09-timer](09-timer.md) we created the `Timer` class. However, the functionality was limited to delays. 
We would like to have timers which are updated automatically on interrupt basis.

So we'll be extending the `Timer` class for this.
We will start by first creating an interrupt handler, which counts ticks.
Then we'll introduce `KernelTimer`, which can be used to trigger after a certain amount of time.

## Adding interrupts to the timer - Step 1 {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1}

We'll add some functionality to the `Timer` class for enabling, handling and disabling interrupts on the timer.

### Timer.h {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TIMERH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
47: namespace baremetal
48: {
49: 
50: class InterruptSystem;
51: class IMemoryAccess;
52: 
53: /// @brief Periodic timer tick handler
54: using PeriodicTimerHandler = void(void);
55: 
56: /// @brief Maximum number of periodic tick handlers which can be installed
57: #define TIMER_MAX_PERIODIC_HANDLERS 4
58: 
59: /// <summary>
60: /// Timer class. For now only contains busy waiting methods
61: ///
62: /// Note that this class is created as a singleton, using the GetTimer() function.
63: /// </summary>
64: class Timer
65: {
66:     /// <summary>
67:     /// Retrieves the singleton Timer instance. It is created in the first call to this function. This is a friend function of class Timer
68:     /// </summary>
69:     /// <returns>A reference to the singleton Timer</returns>
70:     friend Timer &GetTimer();
71: 
72: private:
73:     /// @brief True if class is already initialized
74:     bool m_isInitialized;
75:     /// @brief Reference to the singleton InterruptSystem instantiation.
76:     InterruptSystem &m_interruptSystem;
77:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
78:     IMemoryAccess &m_memoryAccess;
79:     /// @brief Clock ticks per timer tick
80:     uint64 m_clockTicksPerSystemTick;
81:     /// @brief Timer tick counter
82:     volatile uint64 m_ticks;
83:     /// @brief Uptime in seconds
84:     volatile uint32 m_upTime;
85:     /// @brief Time in seconds (epoch time)
86:     volatile uint64 m_time;
87:     /// @brief Periodic tick handler functions
88:     PeriodicTimerHandler *m_periodicHandlers[TIMER_MAX_PERIODIC_HANDLERS];
89:     /// @brief Number of periodic tick handler functions installed
90:     volatile unsigned     m_numPeriodicHandlers;
91: 
92:     Timer();
93: 
94: public:
95:     Timer(IMemoryAccess &memoryAccess);
96:     ~Timer();
97: 
98:     void Initialize();
99: 
100:     uint64 GetTicks() const;
101: 
102:     uint32 GetUptime() const;
103: 
104:     uint64 GetTime() const;
105: 
106:     void GetTimeString(char* buffer, size_t bufferSize);
107: 
108:     void RegisterPeriodicHandler(PeriodicTimerHandler *handler);
109:     void UnregisterPeriodicHandler(PeriodicTimerHandler *handler);
110: 
111:     static void WaitCycles(uint32 numCycles);
112: 
113:     uint64 GetSystemTimer();
114: 
115:     static void WaitMilliSeconds(uint64 msec);
116:     static void WaitMicroSeconds(uint64 usec);
117: 
118: private:
119:     void InterruptHandler();
120:     static void InterruptHandler(void *param);
121: };
122: 
123: Timer &GetTimer();
124: 
125: } // namespace baremetal
```

- Line 54: We define a new type `PeriodicTimerHandler` which is a callback for every timer tick
- Line 57: We define the maximum number of periodic handlers which can be installed
- Line 64-123: We update the class `Timer`
  - Line 74: We add a boolean to check if the class is initialized
  - Line 76: We add a reference to the singleton `InterruptSystem` instance, as we need to handle interrupts
  - Line 80: We add the member variable `m_clockTicksPerSystemTick` to hold the number of clock ticks per timer tick
  - Line 82: We add the member variable `m_ticks` to count the timer ticks
  - Line 84: We add the member variable `m_upTime` to count the uptime in seconds
  - Line 86: We add the member variable `m_time` to hold the time in seconds (epoch time).
For now this will be the same as the uptime, but later we will be retrieving the actual time
  - Line 88: We add an array of `PeriodicTimerHandler` functions to hold the periodic tick handlers
  - Line 90: We add a variable to hold the number of periodic tick handlers installed
  - Line 96: We add a destructor
  - Line 98: We add a method `Initialize`
  - Line 100: We add a method `GetTicks` to retrieve the number of timer ticks
  - Line 102: We add a method `GetUptime` to retrieve the uptime in seconds
  - Line 104: We add a method `GetTime` to retrieve the epoch time in seconds
  - Line 108: We add a method `RegisterPeriodicHandler` to register a periodic tick handler
  - Line 109: We add a method `UnregisterPeriodicHandler` to unregister a periodic tick handler
  - Line 119: We add a private method `InterruptHandler` to handle the timer interrupt
  - Line 120: We add a static method `InterruptHandler` to handle the timer interrupt, which calls the private method `InterruptHandler` by using the `param` parameter

### Timer.cpp {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TIMERCPP}

Let's implement the newly added methods in the `Timer` class.

Update the file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
42: #include <stdlib/Util.h>
43: #include <baremetal/ARMInstructions.h>
44: #include <baremetal/Assert.h>
45: #include <baremetal/BCMRegisters.h>
46: #include <baremetal/InterruptHandler.h>
47: #include <baremetal/MemoryAccess.h>
...
58: /// @brief Number of timer ticks per second
59: #define TICKS_PER_SECOND 100                               
...
63: /// <summary>
64: /// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
65: /// </summary>
66: Timer::Timer()
67:     : m_interruptSystem{GetInterruptSystem()}
68:     , m_memoryAccess{ GetMemoryAccess() }
69:     , m_clockTicksPerSystemTick{}
70:     , m_ticks{}
71:     , m_upTime{}
72:     , m_time{}
73:     , m_periodicHandlers{}
74:     , m_numPeriodicHandlers{}
75: {
76: }
77: 
78: /// <summary>
79: /// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
80: /// </summary>
81: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
82: Timer::Timer(IMemoryAccess &memoryAccess)
83:     : m_interruptSystem{GetInterruptSystem()}
84:     , m_memoryAccess{ memoryAccess }
85:     , m_clockTicksPerSystemTick{}
86:     , m_ticks{}
87:     , m_upTime{}
88:     , m_time{}
89:     , m_periodicHandlers{}
90:     , m_numPeriodicHandlers{}
91: {
92: }
93: 
94: /// <summary>
95: /// Destructor
96: ///
97: /// Disables the timer, as well as the timer interrupt
98: /// </summary> 
99: Timer::~Timer()
100: {
101:     SetTimerControl(~CNTP_CTL_EL0_ENABLE);
102: 
103:     m_interruptSystem.UnregisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
104: }
105: 
106: /// <summary>
107: /// Timer initialization
108: ///
109: /// Add a timer interrupt handler, calculate the number of clock ticks per timer tick, set the next timer deadline.
110: /// Then enables the timer
111: /// </summary> 
112: void Timer::Initialize()
113: {
114:     if (m_isInitialized)
115:         return;
116: 
117:     memset(m_periodicHandlers, 0, TIMER_MAX_PERIODIC_HANDLERS * sizeof(PeriodicTimerHandler*));
118:     m_interruptSystem.RegisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS, InterruptHandler, this);
119: 
120:     uint64 counterFreq{};
121:     GetTimerFrequency(counterFreq);
122:     assert(counterFreq % TICKS_PER_SECOND == 0);
123:     m_clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
124: 
125:     uint64 counter;
126:     GetTimerCounter(counter);
127:     SetTimerCompareValue(counter + m_clockTicksPerSystemTick);
128:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
129: 
130:     m_isInitialized = true;
131: }
132: 
133: /// <summary>
134: /// Return the current timer tick cound
135: /// </summary>
136: /// <returns>The current timer tick count</returns>
137: uint64 Timer::GetTicks() const
138: {
139:     return m_ticks;
140: }
141: 
142: /// <summary>
143: /// Return the uptime in seconds
144: /// </summary>
145: /// <returns>Uptime in seconds</returns>
146: uint32 Timer::GetUptime() const
147: {
148:     return m_upTime;
149: }
150: 
151: /// <summary>
152: /// Return the current time in seconds (epoch time)
153: /// </summary>
154: /// <returns>Current time in seconds (epoch time)</returns>
155: uint64 Timer::GetTime() const
156: {
157:     return m_time;
158: }
159: 
...
173: /// <summary>
174: /// Register a periodic timer handler
175: /// 
176: /// Registers a periodic timer handler function. The handler function will be called every timer tick.
177: /// </summary>
178: /// <param name="handler">Pointer to periodic timer handler to register</param>
179: void Timer::RegisterPeriodicHandler(PeriodicTimerHandler *handler)
180: {
181:     assert(handler != 0);
182: 
183:     size_t index{};
184:     for (index = 0; index < TIMER_MAX_PERIODIC_HANDLERS; ++index)
185:     {
186:         if (m_periodicHandlers[index] == nullptr)
187:             break;
188:     }
189:     assert(index < TIMER_MAX_PERIODIC_HANDLERS);
190:     m_periodicHandlers[index] = handler;
191: 
192:     DataSyncBarrier();
193: 
194:     m_numPeriodicHandlers++;
195: }
196: 
197: /// <summary>
198: /// Unregister a periodic timer handler
199: /// 
200: /// Removes aperiodic timer handler function from the registration. The handler function will no longer be called.
201: /// </summary>
202: /// <param name="handler">Pointer to periodic timer handler to unregister</param>
203: void Timer::UnregisterPeriodicHandler(const PeriodicTimerHandler *handler)
204: {
205:     assert(handler != 0);
206:     assert(m_numPeriodicHandlers > 0);
207: 
208:     size_t index{};
209:     for (index = 0; index < TIMER_MAX_PERIODIC_HANDLERS; ++index)
210:     {
211:         if (m_periodicHandlers[index] == handler)
212:             break;
213:     }
214:     assert(index < TIMER_MAX_PERIODIC_HANDLERS);
215:     m_periodicHandlers[index] = nullptr;
216: 
217:     DataSyncBarrier();
218: 
219:     m_numPeriodicHandlers--;
220: }
221: 
...
296: /// <summary>
297: /// Interrupt handler for the timer
298: /// 
299: /// Sets the next timer deadline, increments the timer tick count, as well as the time if needed, and calls the periodic handlers.
300: /// </summary>
301: void Timer::InterruptHandler()
302: {
303:     uint64 compareValue;
304:     GetTimerCompareValue(compareValue);
305:     SetTimerCompareValue(compareValue + m_clockTicksPerSystemTick);
306: 
307:     if (++m_ticks % TICKS_PER_SECOND == 0)
308:     {
309:         m_upTime++;
310:         m_time++;
311:     }
312: 
313:     for (unsigned i = 0; i < m_numPeriodicHandlers; i++)
314:     {
315:         if (m_periodicHandlers[i] != nullptr)
316:             (*m_periodicHandlers[i])();
317:     }
318: }
319: 
320: /// <summary>
321: /// Static interrupt handler
322: /// 
323: /// Calls the instance interrupt handler
324: /// </summary>
325: /// <param name="param"></param>
326: void Timer::InterruptHandler(void *param)
327: {
328:     Timer *instance = reinterpret_cast<Timer *>(param);
329:     assert(instance != nullptr);
330: 
331:     instance->InterruptHandler();
332: }
333: 
```

- Line 42-47: We need to include the header for `memset()`, `assert()` as well as for the `InterruptSystem` class.
- Line 59: We define the number of timer ticks per second
- Line 66-76: We update the constructor of the `Timer` class to initialize the newly added member variables
- Line 82-92: We update the special constructor of the `Timer` class to initialize the newly added member variables
- Line 99-104: We add a destructor to disable the timer and the timer interrupt
- Line 112-131: We add the method `Initialize` to initialize the timer
  - Line 114: We check if the timer is already initialized
  - Line 117: We clear the array of periodic handlers
  - Line 118: We register the timer interrupt handler
  - Line 120-123: We calculate the number of clock ticks per timer tick
  - Line 126-127: We set the timer compare value to the current timer counter plus the number of clock ticks per timer tick
  - Line 128: We enable the timer
  - Line 130: We set the timer as initialized
- Line 137-140: We implement the method `GetTicks`
- Line 146-149: We implement the method `GetUptime`
- Line 155-158: We implement the method `GetTime`
- Line 179-195: We implement the method `RegisterPeriodicHandler`
  - Line 181: We perform a sanity check if the handler is not null
  - Line 183-188: We find an empty slot in the array of periodic handlers
  - Line 189: We perform a sanity check whether we found an empty slot
  - Line 190: We store the handler in the array
  - Line 194: We increment the number of periodic handlers
- Line 203-220: We implement the method `UnregisterPeriodicHandler`
  - Line 205: We perform a sanity check if the handler is not null
  - Line 206: We perform a sanity check if there are periodic handlers registered
  - Line 208-213: We find the handler in the array of periodic handlers
  - Line 214: We perform a sanity check whether we found the handler
  - Line 215: We remove the handler from the array
  - Line 219: We decrement the number of periodic handlers
- Line 301-318: We implement the instance method `InterruptHandler`
  - Line 304: We retrieve the current timer compare value
  - Line 305: We set the next timer deadline
  - Line 307-311: We increment the timer tick count, we check if a second has passed, if so we increment the uptime and the time
  - Line 313-317: We call the periodic handlers
- Line 326-332: We implement the static method `InterruptHandler`
  - Line 328-329: We cast the `param` to a `Timer` instance and perform a sanity check
  - Line 331: We call the instance method `InterruptHandler`

### Update CMake file {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_UPDATE_CMAKE_FILE}

As we have not added any source files to the `baremetal` library, we do not need to update its CMake file.

### Update application code {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_UPDATE_APPLICATION_CODE}

We'll make a few small changes to use the new `InterruptSystem` implementation.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/InterruptHandler.h>
5: #include <baremetal/Interrupts.h>
6: #include <baremetal/Logger.h>
7: #include <baremetal/System.h>
8: #include <baremetal/Timer.h>
9: 
10: LOG_MODULE("main");
11: 
12: using namespace baremetal;
13: 
14: #define TICKS_PER_SECOND 2 // Timer ticks per second
15: 
16: void PeriodicHandler()
17: {
18:     LOG_INFO("Ping");
19: }
20: 
21: int main()
22: {
23:     auto& console = GetConsole();
24: 
25:     auto exceptionLevel = CurrentEL();
26:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
27: 
28:     Timer &timer = GetTimer();
29:     timer.RegisterPeriodicHandler(PeriodicHandler);
30: 
31:     LOG_INFO("Wait 5 seconds");
32:     Timer::WaitMilliSeconds(5000);
33: 
34:     timer.UnregisterPeriodicHandler(PeriodicHandler);
35: 
36:     console.Write("Press r to reboot, h to halt\n");
37:     char ch{};
38:     while ((ch != 'r') && (ch != 'h'))
39:     {
40:         ch = console.ReadChar();
41:         console.WriteChar(ch);
42:     }
43: 
44:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
45: }
```

- Line 18-21: We change the callback function to `PeriodicHandler` which will be called every timer tick.
We longer need to set the timer deadline, as that is done by the Timer class
- Line 30: We retrieve the singleton instance of the `Timer` class
- Line 31: We register the `PeriodicHandler` function as a periodic handler
- Line 36: We unregister the `PeriodicHandler` function as a periodic handler

### Configuring, building and debugging {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now show a line for each timer tick, so around 500 in total.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:209)
Info   Current EL: 1 (main:28)
Info   Wait 5 seconds (main:33)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
Info   Ping (main:20)
...
```

## Updating the time string - Step 2 {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2}

Until now, we return an empty string for the time. We'll update this to return the actual time, or the uptime in hours, minutes, seconds and microseconds if we don't have the actual time.

### Timer.h {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_TIMERH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
93:     /// @brief Number of days is each month (0 = January, etc.)
94:     static const unsigned s_daysInMonth[12];
95:     /// @brief Name of each month (0 = January, etc.)
96:     static const char    *s_monthName[12];
...
123: 
124:     static bool IsLeapYear(unsigned year);
125:     static unsigned GetDaysInMonth(unsigned month, unsigned year);
```

- Line 94: We define an array `s_daysInMonth` to hold the number of days in each month
- Line 95: We define an array `s_monthName` to hold the name of each month
- Line 124: We add a static method `IsLeapYear` to check if a year is a leap year
- Line 125: We add a static method `GetDaysInMonth` to retrieve the number of days in a month

### Timer.cpp {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_TIMERCPP}

Let's implement the newly added methods in the `Timer` class.

Update the file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
...
166: /// <summary>
167: /// Writes a representation of the current time to a buffer, or of the uptime if the current time is not valid.
168: ///
169: /// The current time will be in the format "MMM dd HH:MM:SS.mmm", according to our time zone, if the time is valid.
170: /// If the time is not known yet, it we be the uptime in the format "ddd.HH:MM:SS.mmm".
171: /// If not yet initialized, an empty string is returned
172: /// </summary>
173: /// <param name="buffer">Buffer to write the time string to</param>
174: /// <param name="bufferSize">Size of the buffer</param>
175: void Timer::GetTimeString(char* buffer, size_t bufferSize)
176: {
177:     uint64 time  = m_time;
178:     uint64 ticks = m_ticks;
179: 
180:     if (bufferSize == 0)
181:     {
182:         return;
183:     }
184:     if (!m_isInitialized)
185:     {
186:         *buffer = '\0';
187:         return;
188:     }
189: 
190:     unsigned second = time % 60;
191:     time /= 60; // Time is now in minute
192:     unsigned minute = time % 60;
193:     time /= 60; // Time is now in hour
194:     unsigned hour       = time % 24;
195:     time /= 24; // Time is now in days
196:     unsigned daysTotal = time;
197: 
198:     unsigned year = 1970; // Epoch start
199:     while (true)
200:     {
201:         unsigned daysInYear = IsLeapYear(year) ? 366 : 365;
202:         if (time < daysInYear)
203:         {
204:             break;
205:         }
206: 
207:         time -= daysInYear;
208:         year++;
209:     }
210: 
211:     unsigned month = 0;
212:     while (1)
213:     {
214:         unsigned daysInMonth = GetDaysInMonth(month, year);
215:         if (time < daysInMonth)
216:         {
217:             break;
218:         }
219: 
220:         time -= daysInMonth;
221:         month++;
222:     }
223: 
224:     unsigned monthDay = time + 1;
225: 
226: #if (TICKS_PER_SECOND != MSEC_PER_SEC)
227:     ticks = ticks * MSEC_PER_SEC / TICKS_PER_SECOND;
228: #endif
229:     auto   milliSeconds = ticks % MSEC_PER_SEC;
230: 
231:     if (year > 1975) // Just a sanity check to see if we have an actual time
232:     {
233:         FormatNoAlloc(buffer, bufferSize, "%s %2u, %04u %02u:%02u:%02u.%03u", s_monthName[month], monthDay, year, hour, minute, second, milliSeconds);
234:     }
235:     else
236:     {
237:         FormatNoAlloc(buffer, bufferSize, "%u.%02u:%02u:%02u.%03u", daysTotal, hour, minute, second, milliSeconds);
238:     }
239: }
240: 
...
364: /// <summary>
365: /// Determine if the specified year is a leap year
366: /// </summary>
367: /// <param name="year">Year</param>
368: /// <returns>Returns true if year is a leap year, false otherwise</returns>
369: bool Timer::IsLeapYear(unsigned year)
370: {
371:     if (year % 100 == 0)
372:     {
373:         return year % 400 == 0;
374:     }
375: 
376:     return year % 4 == 0;
377: }
378: 
379: /// <summary>
380: /// Calculates the number days in the specified month of the specified year
381: /// </summary>
382: /// <param name="month">Month, 0=January, 1=February, etc.</param>
383: /// <param name="year">Year</param>
384: /// <returns></returns>
385: unsigned Timer::GetDaysInMonth(unsigned month, unsigned year)
386: {
387:     if (month == 1 && IsLeapYear(year))
388:     {
389:         return 29;
390:     }
391: 
392:     return s_daysInMonth[month];
393: }
394: 
```

- Line 175-238: We implement the method `GetTimeString`
  - Line 177-178: We retrieve the time and the ticks
  - Line 184-187: We check if the buffer can hold any data, if not we simply return
  - Line 184-187: We check if the timer is initialized, if not we return an empty string
  - Line 190-196: We calculate the seconds, minutes, hours and days
  - Line 198-209: We calculate the year
  - Line 211-222: We calculate the month
  - Line 224: We calculate the day of the month
  - Line 226-229: We convert the ticks to milliseconds
  - Line 233: We format the time string as "[MMM dd ]HH:MM:SS.mmm" if we have the actual time
  - Line 237: We format the time string as "ddd.HH:MM:SS.mmm" if we don't have the actual time
- Line 369-377: We implement the static method `IsLeapYear` to check if a year is a leap year
- Line 385-393: We implement the static method `GetDaysInMonth` to retrieve the number of days in a month

### Update CMake file {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_UPDATE_CMAKE_FILE}

As we have not added any source files to the `baremetal` library, we do not need to update its CMake file.

### Update application code {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_UPDATE_APPLICATION_CODE}

We'll leave the application code as is.

### Configuring, building and debugging {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Adding kernel timers - Step 3 {#TUTORIAL_19_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3}

Let's add so called kernel timers to the `Timer` class. These timers can be used to trigger a function after a certain amount of time.
They are basically one-shot timers which are automatically updated on each timer tick, and once the time has passed, the function is called.

We'll start with a bit of plumbing however.

We'll first introduce a double linked list for our timers.

### List.h {#TUTORIAL_19_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_SINGLETONH}

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
42: #include <baremetal/Assert.h>
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

### Timer.h {#TUTORIAL_19_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_SINGLETONH}

Update the new file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
45: #include <stdlib/Types.h>
46: #include <baremetal/List.h>
...
53: struct KernelTimer;
54: 
55: /// @brief Handle to a kernel timer
56: using KernelTimerHandle = uintptr;
57: 
58: /// @brief Kernel timer handler
59: using KernelTimerHandler = void(KernelTimerHandle timerHandle, void *param, void *context);
60: 
...
97:     /// @brief Periodic tick handler functions
98:     PeriodicTimerHandler *m_periodicHandlers[TIMER_MAX_PERIODIC_HANDLERS];
99:     /// @brief Number of periodic tick handler functions installed
100:     volatile unsigned     m_numPeriodicHandlers;
101:     /// @brief Kernel timer list
102:     DoubleLinkedList<KernelTimer *> m_kernelTimerList;
103:     /// @brief Number of days is each month (0 = January, etc.)
104:     static const unsigned s_daysInMonth[12];
105:     /// @brief Name of each month (0 = January, etc.)
106:     static const char    *s_monthName[12];
...
111:     Timer(IMemoryAccess &memoryAccess);
112:     ~Timer();
113: 
114:     void Initialize();
115: 
116:     uint64 GetTicks() const;
117: 
118:     uint32 GetUptime() const;
119: 
120:     uint64 GetTime() const;
121: 
122:     void GetTimeString(char* buffer, size_t bufferSize);
123: 
124:     void RegisterPeriodicHandler(PeriodicTimerHandler *handler);
125:     void UnregisterPeriodicHandler(PeriodicTimerHandler *handler);
126: 
127:     KernelTimerHandle StartKernelTimer(uint32 delayTicks, KernelTimerHandler *handler, void *param = nullptr, void *context = nullptr);
128:     void CancelKernelTimer(KernelTimerHandle handle);
129: 
130:     static void WaitCycles(uint32 numCycles);
131: 
132:     uint64 GetSystemTimer();
133: 
134:     static void WaitMilliSeconds(uint64 msec);
135:     static void WaitMicroSeconds(uint64 usec);
136: 
137:     static bool IsLeapYear(unsigned year);
138:     static unsigned GetDaysInMonth(unsigned month, unsigned year);
139: 
140: private:
141:     void PollKernelTimers();
142:     void InterruptHandler();
143:     static void InterruptHandler(void *param);
```

- Line 46: We include the header for the `DoubleLinkedList` class
- Line 56: We define the type for the kernel timer handle `KernelTimerHandle`
- Line 58: We define the type for the kernel timer handler function `KernelTimerHandler`
- Line 102: We declare a member variable for the list of kernel timers `m_kernelTimerList`
- Line 127: We declare a method `StartKernelTimer()` to start a kernel timer
- Line 128: We declare a method `CancelKernelTimer()` to cancel a kernel timer
- Line 141: We declare a method `PollKernelTimers()` to check all registered kernel timers for expiration, and handle them
 
### Timer.h {#TUTORIAL_19_TIMER_EXTENSION_ADDING_KERNEL_TIMERS___STEP_3_SINGLETONH}

Update the new file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
..
69: /// @brief Define log name
70: LOG_MODULE("Timer");
71: 
72: /// @brief Magic number for kernel timer (KTMC)
73: #define KERNEL_TIMER_MAGIC 0x4B544D43
74: 
75: /// <summary>
76: /// Kernel timer administration
77: /// </summary>
78: /// <typeparam name="Pointer"></typeparam>
79: struct KernelTimer
80: {
81: #ifndef NDEBUG
82:     /// @brief Magic number to check if element is valid
83:     unsigned m_magic;
84: #endif
85:     /// @brief Kernel timer deadline in timer ticks
86:     unsigned            m_elapsesAtTicks;
87:     /// @brief Pointer to kernel timer handler
88:     KernelTimerHandler *m_handler;
89:     /// @brief Kernel timer handler parameter
90:     void               *m_param;
91:     /// @brief Kernel timer handler context
92:     void               *m_context;
93: 
94:     /// <summary>
95:     /// Construct a kernel timer administration element
96:     /// </summary>
97:     /// <param name="elapseTimeTicks">Timer deadline in timer ticks</param>
98:     /// <param name="handler">Kernel timer handler pointer</param>
99:     /// <param name="param">Kernel timer handler parameter</param>
100:     /// <param name="context">Kernerl timer handler context</param>
101:     KernelTimer(unsigned elapseTimeTicks, KernelTimerHandler *handler, void *param, void *context)
102:         :
103: #ifndef NDEBUG
104:           m_magic{KERNEL_TIMER_MAGIC}
105:         ,
106: #endif
107:           m_elapsesAtTicks{elapseTimeTicks}
108:         , m_handler{handler}
109:         , m_param{param}
110:         , m_context{context}
111: 
112:     {
113:     }
114:     /// <summary>
115:     /// Verify magic number
116:     /// </summary>
117:     /// <returns>True if the magic number is correct, false otherwise</returns>
118:     bool CheckMagic() const
119:     {
120:         return m_magic == KERNEL_TIMER_MAGIC;
121:     }
122: };
123: /// @brief Kernel timer element, element which is stored in the kernel time list
124: using KernelTimerElement = DoubleLinkedList<KernelTimer *>::Element;
125: 
...
361: /// <summary>
362: /// Starts a kernel timer. After delayTicks timer ticks, it elapses and call the kernel timer handler.
363: /// </summary>
364: /// <param name="delayTicks">Delay time for timer in timer ticks</param>
365: /// <param name="handler">Kernel timer handler to call when time elapses</param>
366: /// <param name="param">Parameter to pass to kernel timer handler</param>
367: /// <param name="context">Kernel timer handler context</param>
368: /// <returns>Handle to kernel timer</returns>
369: KernelTimerHandle Timer::StartKernelTimer(unsigned delayTicks, KernelTimerHandler *handler, void *param, void *context)
370: {
371:     unsigned elapseTimeTicks = m_ticks + delayTicks;
372:     assert(handler != nullptr);
373: 
374:     KernelTimer *timer = new KernelTimer(elapseTimeTicks, handler, param, context);
375:     assert(timer != nullptr);
376:     LOG_DEBUG("Create new timer to expire at %d ticks, handle %p", elapseTimeTicks, timer);
377: 
378:     KernelTimerElement *prevElement{};
379:     KernelTimerElement *element = m_kernelTimerList.GetFirst();
380:     while (element != nullptr)
381:     {
382:         const KernelTimer *timer2 = m_kernelTimerList.GetPointer(element);
383:         assert(timer2 != nullptr);
384:         assert(timer2->m_magic == KERNEL_TIMER_MAGIC);
385: 
386:         if (static_cast<int>(timer2->m_elapsesAtTicks - elapseTimeTicks) > 0)
387:         {
388:             break;
389:         }
390: 
391:         prevElement = element;
392:         element     = m_kernelTimerList.GetNext(element);
393:     }
394: 
395:     if (element != nullptr)
396:     {
397:         m_kernelTimerList.InsertBefore(element, timer);
398:     }
399:     else
400:     {
401:         m_kernelTimerList.InsertAfter(prevElement, timer);
402:     }
403: 
404:     return reinterpret_cast<KernelTimerHandle>(timer);
405: }
406: 
407: /// <summary>
408: /// Cancels and removes a kernel timer.
409: /// </summary>
410: /// <param name="handle">Handle to kernel timer to cancel</param>
411: void Timer::CancelKernelTimer(KernelTimerHandle handle)
412: {
413:     KernelTimer *timer = reinterpret_cast<KernelTimer *>(handle);
414:     assert(timer != 0);
415:     LOG_DEBUG("Cancel timer, expire time %d ticks, handle %p", timer->m_elapsesAtTicks, timer);
416: 
417:     KernelTimerElement *element = m_kernelTimerList.Find(timer);
418:     if (element != nullptr)
419:     {
420:         assert(timer->m_magic == KERNEL_TIMER_MAGIC);
421: 
422:         m_kernelTimerList.Remove(element);
423: 
424: #ifndef NDEBUG
425:         timer->m_magic = 0;
426: #endif
427:         delete timer;
428:     }
429: }
430: 
431: /// <summary>
432: /// Update all registered kernel timers, and handle expiration of timers
433: /// </summary>
434: void Timer::PollKernelTimers()
435: {
436:     auto element = m_kernelTimerList.GetFirst();
437:     while (element != nullptr)
438:     {
439:         KernelTimer *timer = m_kernelTimerList.GetPointer(element);
440:         assert(timer != nullptr);
441:         assert(timer->m_magic == KERNEL_TIMER_MAGIC);
442: 
443:         if (static_cast<int>(timer->m_elapsesAtTicks - m_ticks) > 0)
444:         {
445:             break;
446:         }
447: 
448:         LOG_DEBUG("Expire timer, expire time %d ticks, handle %p", timer->m_elapsesAtTicks, timer);
449: 
450:         m_kernelTimerList.Remove(element);
451: 
452:         KernelTimerHandler *handler = timer->m_handler;
453:         assert(handler != nullptr);
454:         (*handler)(reinterpret_cast<KernelTimerHandle>(timer), timer->m_param, timer->m_context);
455: 
456: #ifndef NDEBUG
457:         timer->m_magic = 0;
458: #endif
459:         delete timer;
460: 
461:         // The list may have changed due to the handler callback, so re-initialize
462:         element = m_kernelTimerList.GetFirst();
463:     }
464: }
465: 
...
576: void Timer::InterruptHandler()
577: {
578:     uint64 compareValue;
579:     GetTimerCompareValue(compareValue);
580:     SetTimerCompareValue(compareValue + m_clockTicksPerSystemTick);
581: 
582:     if (++m_ticks % TICKS_PER_SECOND == 0)
583:     {
584:         m_upTime++;
585:         m_time++;
586:     }
587: 
588:     PollKernelTimers();
589: 
590:     for (unsigned i = 0; i < m_numPeriodicHandlers; i++)
591:     {
592:         if (m_periodicHandlers[i] != nullptr)
593:             (*m_periodicHandlers[i])();
594:     }
595: }
596: 
```

- Line 70: We define the log module name
- Line 73: We define a magic number for the kernel timer administration
- Line 79-122: We declare a struct `KernelTimer` to hold the kernel timer administration
  - Line 81-92: We declare the magic number, the timer deadline in ticks, the handler function pointer, the handler parameter, and the handler context
  - Line 101-113: We declare and implement the constructor for `KernelTimer`
  - Line 118-121: We declare and implement a method `CheckMagic()` to check the magic number
- Line 369-405: We implement the method `StartKernelTimer()` to start a kernel timer
  - Line 371: We calculate the time the timer should expire
  - Line 372: We perform a sanity check that the handler is valid
  - Line 374-375: We create a new kernel timer, and verify it was created
  - Line 378-393: We first find the correct position in the list to insert the new timer (the list is order by expiration time)
  - Line 395-402: We then insert the timer in the list at the correct position
- Line 411-429: We implement the method `CancelKernelTimer()` to cancel a kernel timer
  - Line 413-414: We convert the handle back to a pointer, and verify it is valid
  - Line 417-428: We find the element in the list, and remove it
- Line 434-464: We implement the method `PollKernelTimers()` to check all registered kernel timers for expiration, and handle them
  - Line 436: We get the first element in the list
  - Line 437-463: We loop through all elements in the list
     - Line 439: We get the timer from the element, and verify it is valid, and has the correct magic number
     - Line 443: We check if the timer has expired
     - Line 450: If it did expire we remove the timer from the list
     - Line 452-454: We extract the handler, verify it is valid, and call it
     - Line 459: We delete the timer
     - Line 462: We re-initialize the element, as the list may have changed
- Line 588: We call `PollKernelTimers()` from the interrupt handler, to update the kernel timers

### Update CMake file {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_UPDATE_APPLICATION_CODE}

We'll updat the application code to user kernel timers.

Update the file `code/applications/baremetal/src/main.cpp`

```cpp
File: code/applications/baremetal/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/InterruptHandler.h>
5: #include <baremetal/Interrupts.h>
6: #include <baremetal/Logger.h>
7: #include <baremetal/System.h>
8: #include <baremetal/Timer.h>
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

### Configuring, building and debugging {#TUTORIAL_19_TIMER_EXTENSION_UPDATING_THE_TIME_STRING___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

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

Next: [19-timer-extension](19-timer-extension.md)


