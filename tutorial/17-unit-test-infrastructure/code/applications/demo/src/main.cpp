#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>

#include <unittest/Test.h>
#include <unittest/TestInfo.h>
#include <unittest/TestResults.h>

LOG_MODULE("main");

using namespace baremetal;

class MyTest
    : public unittest::Test
{
public:
    void RunImpl() const override;
};

void MyTest::RunImpl() const
{
    LOG_INFO("Running test");
}

int main()
{
    auto& console = GetConsole();

    MyTest myTest;
    unittest::TestInfo myTestInfo("MyTest", "DummyFixture", "DummySuite", __FILE__, __LINE__);
    myTestInfo.SetTest(&myTest);
    unittest::TestResults results;

    myTestInfo.Run(results);

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
