//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : TestMacros.h
//
// Namespace   : unittest
//
// Class       : -
//
// Description : Macros for specifyig tests
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and /or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------

#pragma once

#define TEST_EX(TestName, Registry)                                                                         \
class Test##TestName : public unittest::TestBase                                                            \
{                                                                                                           \
public:                                                                                                     \
    Test##TestName() : TestBase(baremetal::string(#TestName), baremetal::string(""), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__) {} \
private:                                                                                                    \
    virtual void RunImpl() const override;                                                                  \
} test##TestName##Instance;                                                                                 \
                                                                                                            \
static unittest::TestRegistrar registrar##TestName(Registry, &test##TestName##Instance);                    \
                                                                                                            \
void Test##TestName::RunImpl() const

#define TEST(TestName) TEST_EX(TestName, unittest::TestRegistry::GetTestRegistry())

#define TEST_FIXTURE_EX(FixtureClass,TestName,Registry) \
class FixtureClass##TestName##Helper : public FixtureClass                                                  \
{                                                                                                           \
public:                                                                                                     \
    FixtureClass##TestName##Helper(const FixtureClass##TestName##Helper &) = delete;                        \
    explicit FixtureClass##TestName##Helper(unittest::TestDetails const & details) :                        \
        m_details{ details }                                                                                \
    {                                                                                                       \
        SetUp();                                                                                            \
    }                                                                                                       \
    virtual ~FixtureClass##TestName##Helper()                                                               \
    {                                                                                                       \
        TearDown();                                                                                         \
    }                                                                                                       \
    FixtureClass##TestName##Helper & operator = (const FixtureClass##TestName##Helper &) = delete;          \
    virtual void SetUp() {}                                                                                 \
    virtual void TearDown() {}                                                                              \
    void RunImpl() const;                                                                                   \
    unittest::TestDetails const & m_details;                                                                \
};                                                                                                          \
                                                                                                            \
class Test##FixtureClass##TestName : public unittest::TestBase                                              \
{                                                                                                           \
public:                                                                                                     \
    Test##FixtureClass##TestName() :                                                                        \
        TestBase(baremetal::string(#TestName), baremetal::string(#FixtureClass), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__) \
    {                                                                                                       \
    }                                                                                                       \
private:                                                                                                    \
    void RunImpl() const override;                                                                          \
} test##FixtureClass##TestName##Instance;                                                                   \
                                                                                                            \
unittest::TestRegistrar registrar##FixtureClass##TestName(Registry, &test##FixtureClass##TestName##Instance); \
                                                                                                            \
void Test##FixtureClass##TestName::RunImpl() const                                                          \
{                                                                                                           \
    bool ctorOk = false;                                                                                    \
    FixtureClass##TestName##Helper fixtureHelper(Details());                                                \
    unittest::ExecuteTest(fixtureHelper, Details());                                                        \
}                                                                                                           \
void FixtureClass##TestName##Helper::RunImpl() const

#define TEST_FIXTURE(FixtureClass,TestName) TEST_FIXTURE_EX(FixtureClass,TestName,unittest::TestRegistry::GetTestRegistry())

#define TEST_SUITE(SuiteName)                                                                               \
    namespace Suite##SuiteName                                                                              \
    {                                                                                                       \
        inline char const* GetSuiteName()                                                                   \
        {                                                                                                   \
            return baremetal::string(#SuiteName);                                                           \
        }                                                                                                   \
    }                                                                                                       \
    namespace Suite##SuiteName
