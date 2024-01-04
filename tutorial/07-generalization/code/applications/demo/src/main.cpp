#include "baremetal/ARMInstructions.h"
#include "baremetal/System.h"
#include "baremetal/UART1.h"

using namespace baremetal;

int main()
{
    auto& uart = GetUART1();
    uart.WriteString("Hello World!\n");

    uart.WriteString("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = uart.Read();
        uart.Write(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
