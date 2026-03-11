#include "stdlib/Util.h"
#include "baremetal/Logger.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"
#include "device/spi/MCP23008SPI.h"

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

int main()
{
    GetLogger().SetLogLevel(LogSeverity::Debug);

    uint8 deviceIndex = 0;
    SPI_CEIndex ceIndex{SPI_CEIndex::CE0};
    const uint16 DisplayWidth = 128;
    const uint16 DisplayHeight = 64;

    MCP23008SPI mcp;
    if (!mcp.Initialize(deviceIndex, ceIndex))
    {
        LOG_INFO("Cannot initialize device");
    }
    mcp.SetPortDirections(MCP23008PinDirection::Out);
    mcp.SetPortValue(0xFF);

    Timer::WaitMilliSeconds(2000);

    mcp.SetPortValue(0x00);

    LOG_INFO("Rebooting");

    return static_cast<int>(ReturnCode::ExitReboot);
}
