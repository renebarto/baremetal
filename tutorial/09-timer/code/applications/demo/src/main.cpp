#include <baremetal/ARMInstructions.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/UART1.h>

/// @file
/// Demo application main code

using namespace baremetal;

/// <summary>
/// Demo application main code
/// </summary>
/// <returns>For now always 0</returns>
int main()
{
    auto& uart = GetUART1();
    uart.WriteString("Hello World!\n");

    uart.WriteString("Wait 5 seconds\n");
    Timer::WaitMilliSeconds(5000);

    uart.WriteString("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = uart.Read();
        uart.Write(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
