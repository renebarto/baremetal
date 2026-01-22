#include <baremetal/ARMInstructions.h>
#include <baremetal/Format.h>
#include <baremetal/I2CMaster.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <device/gpio/KY-040.h>
#include <device/i2c/HD44780DisplayI2C.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

static int option{};
static bool select{};
static HD44780Display* lcdDevice{};

static void ShowOption();

static void OnEvent(KY040::Event event, void *param)
{
    LOG_INFO("Event %s", KY040::EventToString(event));
    switch (event)
    {
        case KY040::Event::SwitchDown:
            LOG_INFO("Value selected");
            select = true;
            break;
        case KY040::Event::RotateClockwise:
            option++;
            ShowOption();
            break;
        case KY040::Event::RotateCounterclockwise:
            option--;
            ShowOption();
            break;
        default:
            break;
    }
}

static bool ShouldReboot()
{
    return (option % 2 == 0);
}

static void ShowOption()
{
    lcdDevice->Write(0, 1, "      ");
    if (ShouldReboot())
    {
        LOG_INFO("Select Reboot");
        lcdDevice->Write(0, 1, "Reboot");
    }
    else
    {
        LOG_INFO("Select Halt");
        lcdDevice->Write(0, 1, "Halt");
    }
}

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Info);

    KY040 rotarySwitch(11, 9, 10);
    rotarySwitch.Initialize();
    rotarySwitch.RegisterEventHandler(OnEvent, nullptr);

    uint8 busIndex = 1;
    uint8 address{ 0x27 };
    const uint8 NumRows    = 2;
    const uint8 NumColumns = 16;
    I2CMaster i2cMaster;
    i2cMaster.Initialize(busIndex);

    HD44780DisplayI2C device(i2cMaster, address, NumColumns, NumRows);
    device.Initialize();
    lcdDevice = &device;

    console.Write("Select from menu by turning rotary switch and pressing to select option\n");

    device.SetBacklight(true);
    device.SetDisplayEnabled(true);
    device.SetCursorMode(HD44780Display::CursorMode::Hide);

    device.SetCursorPosition(0, 0);
    device.Write("Menu", 4);
    ShowOption();

    while (!select)
    {
        WaitForInterrupt();
    }

    device.SetBacklight(false);
    device.SetDisplayEnabled(false);

    if (ShouldReboot())
        LOG_INFO("Rebooting");
    else
        LOG_INFO("Halting");

    return static_cast<int>(ShouldReboot() ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
