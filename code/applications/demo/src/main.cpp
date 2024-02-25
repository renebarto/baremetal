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

#include <unittest/unittest.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace unittest;

TEST_SUITE(Suite1)
{

class FixtureMyTest1
    : public TestFixture
{
public:
    void SetUp() override
    {
        LOG_DEBUG("FixtureMyTest1 SetUp");
    }
    void TearDown() override
    {
        LOG_DEBUG("FixtureMyTest1 TearDown");
    }
};

TEST_FIXTURE(FixtureMyTest1,Test1)
{
    LOG_DEBUG(m_details.FixtureName().c_str());
}

} // Suite1

TEST_SUITE(Suite2)
{

class FixtureMyTest2
    : public TestFixture
{
public:
    void SetUp() override
    {
        LOG_DEBUG("FixtureMyTest2 SetUp");
    }
    void TearDown() override
    {
        LOG_DEBUG("FixtureMyTest2 TearDown");
    }
};

TEST_FIXTURE(FixtureMyTest2, Test2)
{
    LOG_DEBUG(m_details.FixtureName().c_str());
}

} // Suite2

class FixtureMyTest3
    : public TestFixture
{
public:
    void SetUp() override
    {
        LOG_DEBUG("FixtureMyTest3 SetUp");
    }
    void TearDown() override
    {
        LOG_DEBUG("FixtureMyTest3 TearDown");
    }
};

TEST_FIXTURE(FixtureMyTest3, Test3)
{
    LOG_DEBUG(m_details.FixtureName().c_str());
    CurrentTest::Results()->OnTestFailure(m_details, "Fail");
}

TEST(Test4)
{
    LOG_DEBUG(Details().FixtureName().c_str());
    CurrentTest::Results()->OnTestFailure(Details(), "Fail");
    CurrentTest::Results()->OnTestFailure(Details(), "FailAgain");
}

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    ConsoleTestReporter reporter;
    RunAllTests(&reporter);

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
