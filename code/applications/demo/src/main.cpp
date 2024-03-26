#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryAccess.h>
#include <baremetal/InterruptHandler.h>

LOG_MODULE("main");

using namespace baremetal;

#define TICKS_PER_SECOND 2 // Timer ticks per second

static uint32 clockTicksPerSystemTick;

void IntHandler(void* param)
{
    uint64 counterCompareValue;
    GetTimerCompareValue(counterCompareValue);
    SetTimerCompareValue(counterCompareValue + clockTicksPerSystemTick);

    LOG_INFO("Ping");
}

int main()
{
    auto& console = GetConsole();

    MemoryAccess memoryAccess;

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

    GetInterruptSystem().RegisterIRQHandler(ARM_IRQLOCAL0_CNTPNS, IntHandler, nullptr);

    uint64 counterFreq{};
    GetTimerFrequency(counterFreq);
    assert(counterFreq % TICKS_PER_SECOND == 0);
    clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;

    uint64 counter;
    GetTimerCounter(counter);
    SetTimerCompareValue(counter + clockTicksPerSystemTick);
    SetTimerControl(CNTP_CTL_EL0_ENABLE);

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    SetTimerControl(0);

    GetInterruptSystem().UnregisterIRQHandler(ARM_IRQLOCAL0_CNTPNS);

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
