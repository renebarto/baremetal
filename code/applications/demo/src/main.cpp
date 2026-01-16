#include "baremetal/Console.h"
#include "baremetal/I2CMaster.h"
#include "baremetal/Logger.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"
#include "device/i2c/HD44780DisplayI2C.h"

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

int main()
{
    auto& console = GetConsole();

    uint8 busIndex = 1;
    uint8 address{0x27};
    I2CMaster i2cMaster;
    i2cMaster.Initialize(busIndex);
    HD44780DisplayI2C display(i2cMaster, address, 16, 2);
    display.SetBacklight(true);

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    display.SetBacklight(false);

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
