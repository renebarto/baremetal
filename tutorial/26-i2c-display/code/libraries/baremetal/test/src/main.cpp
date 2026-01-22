#include "baremetal/System.h"
#include "baremetal/Logger.h"
#include "unittest/unittest.h"

using namespace baremetal;
using namespace unittest;

int main()
{
    ConsoleTestReporter reporter;
    GetLogger().SetLogLevel(LogSeverity::Info);
    RunAllTests(&reporter);

    return static_cast<int>(ReturnCode::ExitHalt);
}
