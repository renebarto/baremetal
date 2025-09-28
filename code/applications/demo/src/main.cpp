#include "baremetal/ARMInstructions.h"
#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Console.h"
#include "baremetal/Logger.h"
#include "baremetal/Mailbox.h"
#include "baremetal/MemoryManager.h"
#include "baremetal/RPIProperties.h"
#include "baremetal/Serialization.h"
#include "baremetal/SysConfig.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"

LOG_MODULE("main");

using namespace baremetal;

void MyHandler(const char* expression, const char* fileName, int lineNumber)
{
    LOG_INFO("An assertion failed on location %s:%d, %s", fileName, lineNumber, expression);
}

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
    RPIProperties properties(mailbox);

    uint64 serial{};
    if (properties.GetBoardSerial(serial))
    {
        LOG_INFO("Mailbox call succeeded");
        LOG_INFO("Serial: %016llx", serial);
    }
    else
    {
        LOG_ERROR("Mailbox call failed");
    }

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }
    if (ch == 'p')
    {
        SetAssertionCallback(MyHandler);
        assert(false);
        ResetAssertionCallback();
        assert(false);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
