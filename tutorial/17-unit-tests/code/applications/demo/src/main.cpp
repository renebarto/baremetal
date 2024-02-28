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
    FAIL();
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
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_TRUE(false);
    EXPECT_FALSE(true);
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
    int x = 0;
    int y = 1;
    int z = 1;
    EXPECT_EQ(x, y);
    EXPECT_EQ(y, z);
    EXPECT_NE(x, y);
    EXPECT_NE(y, z);
}

TEST(Test4)
{
    int* p = nullptr;
    int dd = 123;
    int* q = &dd;
    int* r = &dd;
    ASSERT_NULL(p);
    EXPECT_NULL(p);
    ASSERT_NULL(q);
    EXPECT_NULL(q);
    ASSERT_NOT_NULL(p);
    EXPECT_NOT_NULL(p);
    ASSERT_NOT_NULL(q);
    EXPECT_NOT_NULL(q);
    baremetal::string s1 = "A";
    baremetal::string s2 = "B";
    baremetal::string s3 = "B";
    baremetal::string s4 = "b";
    ASSERT_EQ(s1, s2);
    EXPECT_EQ(s1, s2);
    ASSERT_EQ(s2, s3);
    EXPECT_EQ(s2, s3);
    ASSERT_NE(s1, s2);
    EXPECT_NE(s1, s2);
    ASSERT_NE(s2, s3);
    EXPECT_NE(s2, s3);
    ASSERT_EQ_IGNORE_CASE(s1, s2);
    EXPECT_EQ_IGNORE_CASE(s1, s2);
    ASSERT_EQ_IGNORE_CASE(s2, s3);
    EXPECT_EQ_IGNORE_CASE(s2, s3);
    ASSERT_NE_IGNORE_CASE(s1, s2);
    EXPECT_NE_IGNORE_CASE(s1, s2);
    ASSERT_NE_IGNORE_CASE(s2, s3);
    EXPECT_NE_IGNORE_CASE(s2, s3);
    ASSERT_EQ_IGNORE_CASE(s2, s4);
    EXPECT_EQ_IGNORE_CASE(s2, s4);
    ASSERT_NE_IGNORE_CASE(s2, s4);
    EXPECT_NE_IGNORE_CASE(s2, s4);
    char t[] = { 'A', '\0' };
    char u[] = { 'B', '\0' };
    char v[] = { 'B', '\0' };
    char w[] = { 'b', '\0' };
    const char* tC = "A";
    const char* uC = "B";
    const char* vC = "B";
    const char* wC = "b";
    ASSERT_EQ(t, u);
    EXPECT_EQ(t, u);
    ASSERT_EQ(u, v);
    EXPECT_EQ(u, v);
    ASSERT_EQ(t, u);
    EXPECT_EQ(t, uC);
    ASSERT_EQ(uC, v);
    EXPECT_EQ(uC, vC);
    ASSERT_EQ(t, w);
    EXPECT_EQ(t, wC);
    ASSERT_EQ(uC, w);
    EXPECT_EQ(uC, wC);
    ASSERT_NE(t, u);
    EXPECT_NE(t, u);
    ASSERT_NE(u, v);
    EXPECT_NE(u, v);
    ASSERT_NE(t, u);
    EXPECT_NE(t, uC);
    ASSERT_NE(uC, v);
    EXPECT_NE(uC, vC);
    ASSERT_NE(t, w);
    EXPECT_NE(t, wC);
    ASSERT_NE(uC, w);
    EXPECT_NE(uC, wC);
    ASSERT_EQ_IGNORE_CASE(t, u);
    EXPECT_EQ_IGNORE_CASE(t, u);
    ASSERT_EQ_IGNORE_CASE(u, v);
    EXPECT_EQ_IGNORE_CASE(u, v);
    ASSERT_EQ_IGNORE_CASE(t, u);
    EXPECT_EQ_IGNORE_CASE(t, uC);
    ASSERT_EQ_IGNORE_CASE(uC, v);
    EXPECT_EQ_IGNORE_CASE(uC, vC);
    ASSERT_EQ_IGNORE_CASE(t, w);
    EXPECT_EQ_IGNORE_CASE(t, wC);
    ASSERT_EQ_IGNORE_CASE(uC, w);
    EXPECT_EQ_IGNORE_CASE(uC, wC);
    ASSERT_NE_IGNORE_CASE(t, u);
    EXPECT_NE_IGNORE_CASE(t, u);
    ASSERT_NE_IGNORE_CASE(u, v);
    EXPECT_NE_IGNORE_CASE(u, v);
    ASSERT_NE_IGNORE_CASE(t, u);
    EXPECT_NE_IGNORE_CASE(t, uC);
    ASSERT_NE_IGNORE_CASE(uC, v);
    EXPECT_NE_IGNORE_CASE(uC, vC);
    ASSERT_NE_IGNORE_CASE(t, w);
    EXPECT_NE_IGNORE_CASE(t, wC);
    ASSERT_NE_IGNORE_CASE(uC, w);
    EXPECT_NE_IGNORE_CASE(uC, wC);

    double a = 0.123;
    double b = 0.122;
    ASSERT_EQ(a, b);
    EXPECT_EQ(a, b);
    ASSERT_NEAR(a, b, 0.0001);
    EXPECT_NEAR(a, b, 0.0001);
    ASSERT_NEAR(a, b, 0.001);
    EXPECT_NEAR(a, b, 0.001);
}

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    ConsoleTestReporter reporter;
    auto numFailures = RunAllTests(&reporter);
    LOG_INFO("Failures found: %d", numFailures);

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
