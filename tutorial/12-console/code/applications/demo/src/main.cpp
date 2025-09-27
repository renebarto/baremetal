#include "baremetal/ARMInstructions.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Console.h"
#include "baremetal/Mailbox.h"
#include "baremetal/MemoryManager.h"
#include "baremetal/RPIProperties.h"
#include "baremetal/Serialization.h"
#include "baremetal/SysConfig.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"

using namespace baremetal;

int main()
{
    auto& console = GetConsole();
    console.Write("Hello World!\n", ConsoleColor::Yellow);

    char buffer[128];
    Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
    RPIProperties properties(mailbox);

    uint64 serial{};
    if (properties.GetBoardSerial(serial))
    {
        console.Write("Mailbox call succeeded\n");
        console.Write("Serial: ");
        Serialize(buffer, sizeof(buffer), serial, 16, 16, false, true);
        console.Write(buffer);
        console.Write("\n");
    }
    else
    {
        console.Write("Mailbox call failed\n", ConsoleColor::Red);
    }

    console.Write("Wait 5 seconds\n");
    Timer::WaitMilliSeconds(5000);

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
