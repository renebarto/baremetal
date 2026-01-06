#include "baremetal/System.h"
#include "baremetal/Logger.h"
#include "unittest/unittest.h"

using namespace baremetal;
using namespace unittest;

int main()
{
    ConsoleTestReporter reporter;
    GetLogger().SetLogLevel(LogSeverity::Debug);
    RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessMockTest", nullptr));
    RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessGPIOMockTest", nullptr));
//    RunAllTests(&reporter);

    return static_cast<int>(ReturnCode::ExitHalt);
}
