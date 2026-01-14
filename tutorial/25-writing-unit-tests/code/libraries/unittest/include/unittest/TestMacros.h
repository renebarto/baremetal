//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : TestMacros.h
//
// Namespace   : unittest
//
// Class       : -
//
// Description : Macros for specifying tests
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

/// @file
/// Test macros

/// @brief Register test
///
/// Register the test named TestName with the registry Registry. This will declare a test class Test[TestName] and instantiate it as
/// test[TestName]Instance. It will also instantiate a TestRegistrar which will register the test instance with the test registry. It will then define
/// the RunImpl() method of the Test[TestName] class, which is formed by the following code between curly brackets
#define TEST_EX(TestName, Registry)                                                                                                                  \
    class Test##TestName : public unittest::Test                                                                                                     \
    {                                                                                                                                                \
    private:                                                                                                                                         \
        virtual void RunImpl() const override;                                                                                                       \
    } test##TestName##Instance;                                                                                                                      \
                                                                                                                                                     \
    static unittest::TestRegistrar registrar##TestName(Registry, &test##TestName##Instance,                                                          \
                                                       unittest::TestDetails(baremetal::String(#TestName), baremetal::String(""),                    \
                                                                             baremetal::String(GetSuiteName()), baremetal::String(__FILE__),         \
                                                                             __LINE__));                                                             \
                                                                                                                                                     \
    void Test##TestName::RunImpl() const

/// @brief Register test
///
/// Register the test named TestName with the singleton test registry. This will use the TEST_EX macro to do the actual test registration
#define TEST(TestName) TEST_EX(TestName, unittest::TestRegistry::GetTestRegistry())

/// @brief Register test inside test fixture
///
/// Register the test inside a test fixture named FixtureClass for the test named TestName with the registry Registry.
/// This will declare a class [FixtureClass][TestName]Helper, which calls the Setup() method in the constructor, and the TearDown() method in the
/// destructor. It also declares a test class Test[FixtureClass][TestName] and instantiate it as test[FixtureClass][TestName]Instance. It will also
/// instantiate a TestRegistrar which will register the test  with the test registry. The RunImpl() method of the Test[FixtureClass][TestName] is
/// implemented as creating an instance of the test fixture, and invoking its RunImpl() method. It will then define the RunImpl() method of the
/// Test[FixtureClass][TestName] class, which is formed by the following code between curly brackets
#define TEST_FIXTURE_EX(FixtureClass, TestName, Registry)                                                                                            \
    class FixtureClass##TestName##Helper : public FixtureClass                                                                                       \
    {                                                                                                                                                \
    public:                                                                                                                                          \
        FixtureClass##TestName##Helper(const FixtureClass##TestName##Helper&) = delete;                                                              \
        explicit FixtureClass##TestName##Helper(unittest::TestDetails const& details)                                                                \
            : m_details{details}                                                                                                                     \
        {                                                                                                                                            \
            SetUp();                                                                                                                                 \
        }                                                                                                                                            \
        virtual ~FixtureClass##TestName##Helper()                                                                                                    \
        {                                                                                                                                            \
            TearDown();                                                                                                                              \
        }                                                                                                                                            \
        FixtureClass##TestName##Helper& operator=(const FixtureClass##TestName##Helper&) = delete;                                                   \
        void RunImpl() const;                                                                                                                        \
        unittest::TestDetails const& m_details;                                                                                                      \
    };                                                                                                                                               \
                                                                                                                                                     \
    class Test##FixtureClass##TestName : public unittest::Test                                                                                       \
    {                                                                                                                                                \
    private:                                                                                                                                         \
        void RunImpl() const override;                                                                                                               \
    } test##FixtureClass##TestName##Instance;                                                                                                        \
                                                                                                                                                     \
    unittest::TestRegistrar registrar##FixtureClass##TestName(Registry, &test##FixtureClass##TestName##Instance,                                     \
                                                              TestDetails(baremetal::String(#TestName), baremetal::String(#FixtureClass),            \
                                                                          baremetal::String(GetSuiteName()), baremetal::String(__FILE__),            \
                                                                          __LINE__));                                                                \
                                                                                                                                                     \
    void Test##FixtureClass##TestName::RunImpl() const                                                                                               \
    {                                                                                                                                                \
        FixtureClass##TestName##Helper fixtureHelper(*CurrentTest::Details());                                                                       \
        fixtureHelper.RunImpl();                                                                                                                     \
    }                                                                                                                                                \
    void FixtureClass##TestName##Helper::RunImpl() const

/// @brief Register test inside test fixture
///
/// Register the test named TestName inside a test fixture class named Fixture with the singleton test registry. This will use the TEST_FIXTURE_EX
/// macro to do the actual test registration
#define TEST_FIXTURE(FixtureClass, TestName) TEST_FIXTURE_EX(FixtureClass, TestName, unittest::TestRegistry::GetTestRegistry())

/// @brief Create test suite
///
/// Creates a test suite named SuiteName. This simply creates a namespace inside which tests and test fixtures are placed
#define TEST_SUITE(SuiteName)                                                                                                                        \
    namespace Suite##SuiteName                                                                                                                       \
    {                                                                                                                                                \
        inline char const* GetSuiteName()                                                                                                            \
        {                                                                                                                                            \
            return baremetal::String(#SuiteName);                                                                                                    \
        }                                                                                                                                            \
    }                                                                                                                                                \
    namespace Suite##SuiteName
