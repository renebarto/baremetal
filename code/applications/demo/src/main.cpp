#include <baremetal/ARMInstructions.h>
#include <baremetal/Assert.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/Mailbox.h>
#include <baremetal/MemoryManager.h>
#include <baremetal/New.h>
#include <baremetal/RPIProperties.h>
#include <baremetal/Serialization.h>
#include <baremetal/String.h>
#include <baremetal/SysConfig.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/Util.h>

#include <unittest/TestBase.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace unittest;

class MyTest
    : public TestBase
{
public:
    MyTest()
        : TestBase("MyTest", "", "", __FILE__, __LINE__)
    {

    }
    void RunImpl() const override
    {
        LOG_DEBUG("In RunImpl");
    }
};

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    MyTest test;
    test.Run();

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
        assert(false);

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
