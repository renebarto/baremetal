#include "baremetal/ARMInstructions.h"
#include "baremetal/System.h"
#include "baremetal/UART1.h"

using namespace baremetal;

int main()
{
    GetUART1().WriteString("Hello World!\n");
    return static_cast<int>(ReturnCode::ExitHalt);
}
