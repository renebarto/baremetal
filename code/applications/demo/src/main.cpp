#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>

#include <unittest/CurrentTest.h>
#include <unittest/Test.h>
#include <unittest/TestFixture.h>
#include <unittest/TestFixtureInfo.h>
#include <unittest/TestInfo.h>
#include <unittest/TestResults.h>
#include <unittest/TestSuite.h>
#include <unittest/TestSuiteInfo.h>

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
    CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 1");
}

class MyTest1
    : public Test
{
    void RunImpl() const override
    {
        LOG_DEBUG("Test 1");
        FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
        fixtureHelper.RunImpl();
    }
} myTest1;
class MyTestInfo1
    : public TestInfo
{
public:
    MyTestInfo1(Test* testInstance)
        : TestInfo("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__)
    {
        SetTest(testInstance);
    }
};

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
    CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
}

class MyTest2
    : public Test
{
    void RunImpl() const override
    {
        LOG_DEBUG("Test 2");
        FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
        fixtureHelper.RunImpl();
    }
} myTest2;
class MyTestInfo2
    : public TestInfo
{
public:
    MyTestInfo2(Test* testInstance)
        : TestInfo("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__)
    {
        SetTest(testInstance);
    }
};

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
    CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
}

class MyTest3
    : public Test
{
    void RunImpl() const override
    {
        LOG_DEBUG("Test 3");
        FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
        fixtureHelper.RunImpl();
    }
} myTest3;
class MyTestInfo3
    : public TestInfo
{
public:
    MyTestInfo3(Test* testInstance)
        : TestInfo("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
    {
        SetTest(testInstance);
    }
};


class MyTest
    : public Test
{
public:
    void RunImpl() const override;
} myTest;
class MyTestInfo
    : public TestInfo
{
public:
    MyTestInfo(Test* testInstance)
        : TestInfo("MyTest3", "", "", __FILE__, __LINE__)
    {
        SetTest(testInstance);
    }
};

void MyTest::RunImpl() const
{
    CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
}

int main()
{
    auto& console = GetConsole();

    TestInfo* test1 = new Suite1::MyTestInfo1(&Suite1::myTest1);
    TestInfo* test2 = new Suite2::MyTestInfo2(&Suite2::myTest2);
    TestInfo* test3 = new MyTestInfo3(&myTest3);
    TestFixtureInfo* fixture1 = new TestFixtureInfo("MyFixture1");
    fixture1->AddTest(test1);
    TestFixtureInfo* fixture2 = new TestFixtureInfo("MyFixture2");
    fixture2->AddTest(test2);
    TestFixtureInfo* fixture3 = new TestFixtureInfo("MyFixture3");
    fixture3->AddTest(test3);
    unittest::TestResults results;
    TestSuiteInfo* suite1 = new TestSuiteInfo("MySuite1");
    suite1->AddFixture(fixture1);
    TestSuiteInfo* suite2 = new TestSuiteInfo("MySuite2");
    suite2->AddFixture(fixture2);
    TestSuiteInfo* suiteDefault = new TestSuiteInfo("");
    suiteDefault->AddFixture(fixture3);
    suite1->Run(results);
    suite2->Run(results);
    suiteDefault->Run(results);
    delete suite1;
    delete suite2;
    delete suiteDefault;
    unittest::TestInfo myTestInfo("MyTest", "", "", __FILE__, __LINE__);
    myTestInfo.SetTest(&myTest);

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
