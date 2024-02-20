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
#include <unittest/TestFixture.h>
#include <unittest/TestFixtureInfo.h>
#include <unittest/TestSuiteInfo.h>
#include <unittest/TestSuite.h>

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
        LOG_DEBUG("FixtureMyTest1 SetUp");
    }
    void TearDown() override
    {
        LOG_DEBUG("FixtureMyTest1 TearDown");
    }
};

class FixtureMyTest1Helper
    : public FixtureMyTest1
{
public:
    FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
    explicit FixtureMyTest1Helper(unittest::TestDetails const& details)
        : m_details{ details }
    {
        SetUp();
    }
    virtual ~FixtureMyTest1Helper()
    {
        TearDown();
    }
    void RunImpl() const;
    unittest::TestDetails const& m_details;
};
void FixtureMyTest1Helper::RunImpl() const
{
    LOG_DEBUG("MyTestHelper 1");
}

class MyTest1
    : public TestBase
{
public:
    MyTest1()
        : TestBase("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__)
    {

    }
    void RunImpl() const override
    {
        LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
        FixtureMyTest1Helper fixtureHelper(Details());
        fixtureHelper.RunImpl();
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
    explicit FixtureMyTest2Helper(unittest::TestDetails const& details)
        : m_details{ details }
    {
        SetUp();
    }
    virtual ~FixtureMyTest2Helper()
    {
        TearDown();
    }
    void RunImpl() const;
    unittest::TestDetails const& m_details;
};
void FixtureMyTest2Helper::RunImpl() const
{
    LOG_DEBUG("MyTestHelper 2");
}

class MyTest2
    : public TestBase
{
public:
    MyTest2()
        : TestBase("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__)
    {

    }
    void RunImpl() const override
    {
        LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
        FixtureMyTest2Helper fixtureHelper(Details());
        fixtureHelper.RunImpl();
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
    explicit FixtureMyTest3Helper(unittest::TestDetails const& details)
        : m_details{ details }
    {
        SetUp();
    }
    virtual ~FixtureMyTest3Helper()
    {
        TearDown();
    }
    void RunImpl() const;
    unittest::TestDetails const& m_details;
};
void FixtureMyTest3Helper::RunImpl() const
{
    LOG_DEBUG("MyTestHelper 3");
}

class MyTest3
    : public TestBase
{
public:
    MyTest3()
        : TestBase("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
    {

    }
    void RunImpl() const override
    {
        LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
        FixtureMyTest3Helper fixtureHelper(Details());
        fixtureHelper.RunImpl();
    }
};

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    TestBase* test1 = new Suite1::MyTest1;
    TestBase* test2 = new Suite2::MyTest2;
    TestBase* test3 = new MyTest3;
    TestFixtureInfo* fixture1 = new TestFixtureInfo("MyFixture1");
    fixture1->AddTest(test1);
    TestFixtureInfo* fixture2 = new TestFixtureInfo("MyFixture2");
    fixture2->AddTest(test2);
    TestFixtureInfo* fixture3 = new TestFixtureInfo("MyFixture3");
    fixture3->AddTest(test3);
    TestSuiteInfo* suite1 = new TestSuiteInfo("MySuite1");
    suite1->AddFixture(fixture1);
    TestSuiteInfo* suite2 = new TestSuiteInfo("MySuite2");
    suite2->AddFixture(fixture2);
    TestSuiteInfo* suiteDefault = new TestSuiteInfo("");
    suiteDefault->AddFixture(fixture3);
    suite1->Run();
    suite2->Run();
    suiteDefault->Run();
    delete suite1;
    delete suite2;
    delete suiteDefault;

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
