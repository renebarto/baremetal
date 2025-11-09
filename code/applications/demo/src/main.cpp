#include "baremetal/Console.h"
#include "baremetal/Logger.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"

#include "unittest/unittest.h"

LOG_MODULE("main");

using namespace baremetal;
using namespace unittest;

TEST_SUITE(Suite1)
{

    class FixtureMyTest1 : public TestFixture
    {
    public:
        void SetUp() override
        {
            LOG_DEBUG("MyTest SetUp");
        }
        void TearDown() override
        {
            LOG_DEBUG("MyTest TearDown");
        }
    };

    TEST_FIXTURE(FixtureMyTest1, MyTest1)
    {
        FAIL("For some reason");
    }

} // Suite1

TEST_SUITE(Suite2)
{

    class FixtureMyTest2 : public TestFixture
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

    TEST_FIXTURE(FixtureMyTest2, MyTest2)
    {
        EXPECT_TRUE(true);
        EXPECT_FALSE(false);
        EXPECT_TRUE(false);
        EXPECT_FALSE(true);
    }

} // Suite2

class FixtureMyTest3 : public TestFixture
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

TEST_FIXTURE(FixtureMyTest3, MyTest3)
{
    int x = 0;
    int y = 1;
    int z = 1;
    EXPECT_EQ(x, y);
    EXPECT_EQ(y, z);
    EXPECT_NE(x, y);
    EXPECT_NE(y, z);
}

TEST(MyTest4)
{
    ASSERT_TRUE(false);
}

int main()
{
    auto& console = GetConsole();

    ConsoleTestReporter reporter;
    RunAllTests(&reporter);

    LOG_INFO("Wait 5 seconds");
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
