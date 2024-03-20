#include <baremetal/System.h>
#include <unittest/unittest.h>

using namespace baremetal;
using namespace unittest;

int main()
{
    ConsoleTestReporter reporter;
    RunAllTests(&reporter);

    return static_cast<int>(ReturnCode::ExitHalt);
}
