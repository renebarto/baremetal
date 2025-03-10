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

int value = 0;

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
    
    LOG_INFO("Wait 20 seconds");
    Timer::WaitMilliSeconds(20000);

    rotarySwitch.UnregisterEventHandler(OnEvent);
    rotarySwitch.Uninitialize();

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
