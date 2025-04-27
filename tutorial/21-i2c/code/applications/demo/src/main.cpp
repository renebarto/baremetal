#include <baremetal/Format.h>
#include <baremetal/I2CMaster.h>
#include <baremetal/Logger.h>
#include <baremetal/PhysicalGPIOPin.h>
#include <baremetal/String.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <device/i2c/MCP23017.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Debug);

    uint8 busIndex = 1;
    uint8 address{ 0x20 };
    MCP23017 expander;

    if (!expander.Initialize(busIndex, address))
    {
        LOG_INFO("Cannot initialize expander");
    }

    expander.GetPortAValue();
    expander.SetPortADirections(PinDirection::Out);
    expander.SetPortBDirections(PinDirection::Out);
    expander.SetPortAValue(0x55);
    expander.SetPortBValue(0xAA);
    Timer::WaitMilliSeconds(500);
    expander.SetPortAValue(0xAA);
    expander.SetPortBValue(0x55);
    Timer::WaitMilliSeconds(500);
    expander.SetPortAValue(0x00);
    expander.SetPortBValue(0x00);

    LOG_INFO("Rebooting");

    return static_cast<int>(ReturnCode::ExitReboot);
}
