#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Console.h"
#include "baremetal/ExceptionHandler.h"
#include "baremetal/Logger.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"

LOG_MODULE("main");

using namespace baremetal;

static ReturnCode RebootOnException(unsigned /*exceptionID*/, AbortFrame* /*abortFrame*/)
{
    return ReturnCode::ExitReboot;
}

int main()
{
    auto& console = GetConsole();

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

    console.Write("Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation\n");
    char c{};
    while ((c != 'r') && (c != 'h') && (c != 't') && (c != 'm'))
    {
        c = console.ReadChar();
        console.WriteChar(c);
    }
    RegisterExceptionPanicHandler(RebootOnException);

    if (c == 't')
        // Trap
        __builtin_trap();
    else if (c == 'm')
    {
        // Memory failure
        auto r = *((volatile unsigned int*)0xFFFFFFFFFF000000);
        // make gcc happy about unused variables :-)
        r++;
    }

    return static_cast<int>((c == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
