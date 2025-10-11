#include "baremetal/ARMInstructions.h"
#include "baremetal/Assert.h"
#include "baremetal/Console.h"
#include "baremetal/InterruptHandler.h"
#include "baremetal/Interrupts.h"
#include "baremetal/Logger.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"

LOG_MODULE("main");

using namespace baremetal;

void KernelTimerHandler3(KernelTimerHandle /*timerHandle*/, void* /*param*/, void* /*context*/)
{
    LOG_INFO("Timer 3 will never expire in time");
}

void KernelTimerHandler2(KernelTimerHandle /*timerHandle*/, void* /*param*/, void* /*context*/)
{
    LOG_INFO("Timer 2 expired");
}

void KernelTimerHandler1(KernelTimerHandle /*timerHandle*/, void* /*param*/, void* /*context*/)
{
    LOG_INFO("Timer 1 expired");
    LOG_INFO("Starting kernel timer 2 to fire in 2 seconds");
    GetTimer().StartKernelTimer(2 * TICKS_PER_SECOND, KernelTimerHandler2, nullptr, nullptr);
}

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Debug);

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

    Timer& timer = GetTimer();
    LOG_INFO("Starting kernel timer 1 to fire in 1 second");
    auto timer1Handle = timer.StartKernelTimer(1 * TICKS_PER_SECOND, KernelTimerHandler1, nullptr, nullptr);

    LOG_INFO("Starting kernel timer 3 to fire in 10 seconds");
    auto timer3Handle = timer.StartKernelTimer(10 * TICKS_PER_SECOND, KernelTimerHandler3, nullptr, nullptr);

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    LOG_INFO("Cancelling kernel timer 3");
    timer.CancelKernelTimer(timer3Handle);

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
