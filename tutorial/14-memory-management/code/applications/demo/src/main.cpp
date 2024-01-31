#include <baremetal/ARMInstructions.h>
#include <baremetal/Assert.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/Console.h>
#include <baremetal/Logger.h>
#include <baremetal/Mailbox.h>
#include <baremetal/MemoryManager.h>
#include <baremetal/New.h>
#include <baremetal/RPIProperties.h>
#include <baremetal/SysConfig.h>
#include <baremetal/Serialization.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/Util.h>

LOG_MODULE("main");

using namespace baremetal;

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    MemoryManager& memoryManager = GetMemoryManager();
    LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
    LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
    LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));

    auto ptr = memoryManager.HeapAllocate(4, HeapType::LOW);
    LOG_INFO("Allocated block %llx", ptr);
    memoryManager.DumpStatus();

    memoryManager.HeapFree(ptr);
    memoryManager.DumpStatus();

    class X {};

    X* x = new (HeapType::LOW) X;
    LOG_INFO("Allocated block %llx", x);
    memoryManager.DumpStatus();
    delete x;
    memoryManager.DumpStatus();

    void*p = malloc(256);
    LOG_INFO("Allocated block %llx", p);
    memoryManager.DumpStatus();
    free(p);
    memoryManager.DumpStatus();

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
