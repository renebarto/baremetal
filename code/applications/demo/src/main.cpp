#include "baremetal/ARMInstructions.h"
#include "baremetal/System.h"
#include "baremetal/UART1.h"

using namespace baremetal;

int main()
{
    GetUART().WriteString("Hello World!\n");
    return static_cast<int>(ReturnCode::ExitHalt);
}
