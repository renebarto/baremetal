#include "baremetal/Logger.h"
#include "baremetal/PhysicalGPIOPin.h"
#include "baremetal/System.h"
#include "baremetal/stubs/MemoryAccessStubGPIO.h"

LOG_MODULE("main");

using namespace baremetal;

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Debug);

    MemoryAccessStubGPIO fakeMemoryAccess;

    uint8 CLKPinNumber{11};
    uint8 DTPinNumber{9};
    uint8 SWPinNumber{10};
    PhysicalGPIOPin clkPin(CLKPinNumber, GPIOMode::InputPullUp, fakeMemoryAccess);
    PhysicalGPIOPin dtPin(DTPinNumber, GPIOMode::InputPullUp, fakeMemoryAccess);
    PhysicalGPIOPin swPin(SWPinNumber, GPIOMode::InputPullUp, fakeMemoryAccess);

    LOG_INFO("Rebooting");

    return static_cast<int>(ReturnCode::ExitReboot);
}
