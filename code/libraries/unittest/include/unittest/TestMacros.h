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

#define TEST_EX(Name, List)                                                                                 \
class Test##Name : public unittest::TestBase                                                                \
{                                                                                                           \
public:                                                                                                     \
    Test##Name() : TestBase(baremetal::string(#Name), baremetal::string(""), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__) {} \
private:                                                                                                    \
    virtual void RunImpl() const override;                                                                  \
} test##Name##Instance;                                                                                     \
                                                                                                            \
static unittest::TestRegistrar registrar##Name(List, &test##Name##Instance);                                \
                                                                                                            \
void Test##Name::RunImpl() const

#define TEST(Name) TEST_EX(Name, unittest::TestRegistry::GetTestRegistry())

#define TEST_FIXTURE_EX(Fixture,Name,List) \
class Fixture##Name##Helper : public Fixture                                                                \
{                                                                                                           \
public:                                                                                                     \
    Fixture##Name##Helper(const Fixture##Name##Helper &) = delete;                                          \
    explicit Fixture##Name##Helper(unittest::TestDetails const & details) :                                 \
        m_details{ details }                                                                                \
    {                                                                                                       \
        SetUp();                                                                                            \
    }                                                                                                       \
    virtual ~Fixture##Name##Helper()                                                                        \
    {                                                                                                       \
        TearDown();                                                                                         \
    }                                                                                                       \
    Fixture##Name##Helper & operator = (const Fixture##Name##Helper &) = delete;                            \
    virtual void SetUp() {}                                                                                 \
    virtual void TearDown() {}                                                                              \
    void RunImpl() const;                                                                          \
    unittest::TestDetails const & m_details;                                                                \
};                                                                                                          \
                                                                                                            \
class Test##Fixture##Name : public unittest::TestBase                                                       \
{                                                                                                           \
public:                                                                                                     \
    Test##Fixture##Name() :                                                                                 \
        TestBase(baremetal::string(#Name), baremetal::string(#Fixture), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__) \
    {                                                                                                       \
    }                                                                                                       \
private:                                                                                                    \
    void RunImpl() const override;                                                                          \
} test##Fixture##Name##Instance;                                                                            \
                                                                                                            \
unittest::TestRegistrar registrar##Fixture##Name(List, &test##Fixture##Name##Instance);                     \
                                                                                                            \
void Test##Fixture##Name::RunImpl() const                                                                   \
{                                                                                                           \
    bool ctorOk = false;                                                                                    \
    Fixture##Name##Helper fixtureHelper(Details());                                                         \
    unittest::ExecuteTest(fixtureHelper, Details());                                                        \
}                                                                                                           \
void Fixture##Name##Helper::RunImpl() const

#define TEST_FIXTURE(Fixture,Name) TEST_FIXTURE_EX(Fixture,Name,unittest::TestRegistry::GetTestRegistry())

#define TEST_SUITE(Name)                                                                                    \
    namespace Suite##Name                                                                                   \
    {                                                                                                       \
        inline char const* GetSuiteName()                                                                   \
        {                                                                                                   \
            return baremetal::string(#Name);                                                                \
        }                                                                                                   \
    }                                                                                                       \
    namespace Suite##Name
