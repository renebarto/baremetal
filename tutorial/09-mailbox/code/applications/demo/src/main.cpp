#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/Mailbox.h>
#include <baremetal/MemoryManager.h>
#include <baremetal/RPIProperties.h>
#include <baremetal/SysConfig.h>
#include <baremetal/Serialization.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/UART1.h>

using namespace baremetal;

int main()
{
    auto& uart = GetUART1();
    uart.WriteString("Hello World!\n");

    char buffer[128];
    Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
    RPIProperties properties(mailbox);

    uint64 serial;
    if (properties.GetBoardSerial(serial))
    {
        uart.WriteString("Mailbox call succeeded\n");
        uart.WriteString("Serial: ");
        Serialize(buffer, sizeof(buffer), serial, 8, 16, false, true);
        uart.WriteString(buffer);
        uart.WriteString("\n");
    }
    else
    {
        uart.WriteString("Mailbox call failed\n");
    }

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
