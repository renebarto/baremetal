#include <baremetal/ARMInstructions.h>
#include <baremetal/UART1.h>

/// @file
/// Demo application main code

/// <summary>
/// Demo application main code
/// </summary>
/// <returns>For now always 0</returns>
int main()
{
    baremetal::UART1 uart;
    uart.Initialize();

    uart.WriteString("Hello World!\n");
    for (int i = 0; i < 1000000; ++i)
        NOP();
    return 0;
}
