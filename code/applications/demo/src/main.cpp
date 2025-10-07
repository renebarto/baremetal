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

void PeriodicHandler()
{
    LOG_INFO("Ping");
}

int main()
{
    auto& console = GetConsole();

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

    Timer& timer = GetTimer();
    timer.RegisterPeriodicHandler(PeriodicHandler);

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    timer.UnregisterPeriodicHandler(PeriodicHandler);

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
