#include <baremetal/ARMInstructions.h>
#include <baremetal/UART1.h>

int main()
{
    baremetal::UART1 uart;
    uart.Initialize();

    uart.WriteString("Hello World!\n");
    return 0;
}
