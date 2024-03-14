#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>

#include <unittest/unittest.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace unittest;

namespace Suite1 {

inline char const* GetSuiteName()
{
    return baremetal::string("Suite1");
}

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

class FixtureMyTest1Helper
    : public FixtureMyTest1
{
public:
    FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
    explicit FixtureMyTest1Helper(const TestDetails& details)
        : m_details{ details }
    {
        SetUp();
    }
    virtual ~FixtureMyTest1Helper()
    {
        TearDown();
    }
    void RunImpl() const;
    const TestDetails& m_details;
};
void FixtureMyTest1Helper::RunImpl() const
{
    LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 1");
}

class MyTest1
    : public Test
{
    void RunImpl() const override;
} myTest1;

TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__));

void MyTest1::RunImpl() const
{
    LOG_DEBUG("Test 1");
    FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
    fixtureHelper.RunImpl();
}

} // namespace Suite1

namespace Suite2 {

inline char const* GetSuiteName()
{
    return baremetal::string("Suite2");
}

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

class FixtureMyTest2Helper
    : public FixtureMyTest2
{
public:
    FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
    explicit FixtureMyTest2Helper(const TestDetails& details)
        : m_details{ details }
    {
        SetUp();
    }
    virtual ~FixtureMyTest2Helper()
    {
        TearDown();
    }
    void RunImpl() const;
    const TestDetails& m_details;
};
void FixtureMyTest2Helper::RunImpl() const
{
    LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 2");
}

class MyTest2
    : public Test
{
    void RunImpl() const override;
} myTest1;

TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__));

void MyTest2::RunImpl() const
{
    LOG_DEBUG("Test 2");
    FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
    fixtureHelper.RunImpl();
}

} // namespace Suite2

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

class FixtureMyTest3Helper
    : public FixtureMyTest3
{
public:
    FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
    explicit FixtureMyTest3Helper(const TestDetails& details)
        : m_details{ details }
    {
        SetUp();
    }
    virtual ~FixtureMyTest3Helper()
    {
        TearDown();
    }
    void RunImpl() const;
    const TestDetails& m_details;
};
void FixtureMyTest3Helper::RunImpl() const
{
    LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 3");
}

class MyTest3
    : public Test
{
    void RunImpl() const override;
} myTest3;

TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &myTest3, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__));

void MyTest3::RunImpl() const
{
    LOG_DEBUG("Test 3");
    FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
    fixtureHelper.RunImpl();
}

class MyTest
    : public Test
{
public:
    void RunImpl() const override;
} myTest;

TestRegistrar registrarFixtureMyTest(TestRegistry::GetTestRegistry(), &myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));

void MyTest::RunImpl() const
{
    LOG_DEBUG("Running test");
    CurrentTest::Results()->OnTestFailure(*CurrentTest::Details(), "Failure");
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
