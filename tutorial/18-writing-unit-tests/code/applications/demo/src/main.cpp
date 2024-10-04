#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>

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

TEST_FIXTURE(FixtureMyTest2, MyTest2)
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

TEST(MyTest)
{
   int* p = nullptr;
   int dd = 123;
   int* q = &dd;
   int* r = &dd;
   EXPECT_NULL(p);
   EXPECT_NULL(q);
   EXPECT_NOT_NULL(p);
   EXPECT_NOT_NULL(q);
   baremetal::string s1 = "A";
   baremetal::string s2 = "B";
   baremetal::string s3 = "B";
   baremetal::string s4 = "b";
   EXPECT_EQ(s1, s2);
   EXPECT_EQ(s2, s3);
   EXPECT_NE(s1, s2);
   EXPECT_NE(s2, s3);
   EXPECT_EQ_IGNORE_CASE(s1, s2);
   EXPECT_EQ_IGNORE_CASE(s2, s3);
   EXPECT_NE_IGNORE_CASE(s1, s2);
   EXPECT_NE_IGNORE_CASE(s2, s3);
   EXPECT_EQ_IGNORE_CASE(s2, s4);
   EXPECT_NE_IGNORE_CASE(s2, s4);
   char t[] = { 'A', '\0' };
   char u[] = { 'B', '\0' };
   char v[] = { 'B', '\0' };
   char w[] = { 'b', '\0' };
   const char* tC = "A";
   const char* uC = "B";
   const char* vC = "B";
   const char* wC = "b";
   EXPECT_EQ(t, u);
   EXPECT_EQ(u, v);
   EXPECT_EQ(t, uC);
   EXPECT_EQ(uC, vC);
   EXPECT_EQ(t, wC);
   EXPECT_EQ(uC, wC);
   EXPECT_NE(t, u);
   EXPECT_NE(u, v);
   EXPECT_NE(t, uC);
   EXPECT_NE(uC, vC);
   EXPECT_NE(t, wC);
   EXPECT_NE(uC, wC);
   EXPECT_EQ_IGNORE_CASE(t, u);
   EXPECT_EQ_IGNORE_CASE(u, v);
   EXPECT_EQ_IGNORE_CASE(t, uC);
   EXPECT_EQ_IGNORE_CASE(uC, vC);
   EXPECT_EQ_IGNORE_CASE(t, wC);
   EXPECT_EQ_IGNORE_CASE(uC, wC);
   EXPECT_NE_IGNORE_CASE(t, u);
   EXPECT_NE_IGNORE_CASE(u, v);
   EXPECT_NE_IGNORE_CASE(t, uC);
   EXPECT_NE_IGNORE_CASE(uC, vC);
   EXPECT_NE_IGNORE_CASE(t, wC);
   EXPECT_NE_IGNORE_CASE(uC, wC);

   double a = 0.123;
   double b = 0.122;
   EXPECT_EQ(a, b);
   EXPECT_NEAR(a, b, 0.0001);
   EXPECT_NEAR(a, b, 0.001);
}

int main()
{
    auto& console = GetConsole();

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
