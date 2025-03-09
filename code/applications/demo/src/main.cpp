#include <baremetal/ARMInstructions.h>
#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <device/gpio/KY-040.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

static int value = 0;
static int holdCounter = 0;
static const int HoldThreshold = 2;
static bool reboot = false;

void OnEvent(KY040::Event event, void *param)
{
    LOG_INFO("Event %s", KY040::EventToString(event));
    switch (event)
    {
        case KY040::Event::SwitchDown:
            LOG_INFO("Value %d", value);
            break;
        case KY040::Event::RotateClockwise:
            value++;
            break;
        case KY040::Event::RotateCounterclockwise:
            value--;
            break;
        case KY040::Event::SwitchHold:
            holdCounter++;
            if (holdCounter >= HoldThreshold)
            {
                reboot = true;
                LOG_INFO("Reboot triggered");
            }
            break;
        default:
            break;
    }
}

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Info);

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

    KY040 rotarySwitch(11, 9, 10);
    rotarySwitch.Initialize();
    rotarySwitch.RegisterEventHandler(OnEvent, nullptr);
    
    LOG_INFO("Hold down switch button for %d seconds to reboot", HoldThreshold);
    while (!reboot)
    {
        WaitForInterrupt();
    }

    rotarySwitch.UnregisterEventHandler(OnEvent);
    rotarySwitch.Uninitialize();

    LOG_INFO("Rebooting");

    return static_cast<int>(ReturnCode::ExitReboot);
}
