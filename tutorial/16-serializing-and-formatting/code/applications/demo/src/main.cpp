#include <stdlib/Util.h>
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

LOG_MODULE("main");

using namespace baremetal;

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    char c = 'A';
    assert(Serialize(c) ==                          "65");
    assert(Serialize(c, 4) ==                       "  65");
    int8 i8 = 123;
    assert(Serialize(i8) ==                         "123");
    assert(Serialize(i8, 8) ==                      "     123");
    assert(Serialize(i8, -8) ==                     "123     ");
    assert(Serialize(i8, 8, 16) ==                  "      7B");
    assert(Serialize(i8, 8, 16, true) ==            "    0x7B");
    assert(Serialize(i8, 8, 16, true, true) ==      "0x0000007B");
    uint8 u8 = 234;
    assert(Serialize(u8) ==                         "234");
    assert(Serialize(u8, 8) ==                      "     234");
    assert(Serialize(u8, -8) ==                     "234     ");
    assert(Serialize(u8, 8, 16) ==                  "      EA");
    assert(Serialize(u8, 8, 16, true) ==            "    0xEA");
    assert(Serialize(u8, 8, 16, true, true) ==      "0x000000EA");
    int16 i16 = 12345;
    assert(Serialize(i16) ==                        "12345");
    assert(Serialize(i16, 8) ==                     "   12345");
    assert(Serialize(i16, -8) ==                    "12345   ");
    assert(Serialize(i16, 8, 16) ==                 "    3039");
    assert(Serialize(i16, 8, 16, true) ==           "  0x3039");
    assert(Serialize(i16, 8, 16, true, true) ==     "0x00003039");
    uint16 u16 = 34567;
    assert(Serialize(u16) ==                        "34567");
    assert(Serialize(u16, 8) ==                     "   34567");
    assert(Serialize(u16, -8) ==                    "34567   ");
    assert(Serialize(u16, 8, 16) ==                 "    8707");
    assert(Serialize(u16, 8, 16, true) ==           "  0x8707");
    assert(Serialize(u16, 8, 16, true, true) ==     "0x00008707");
    int32 i32 = 1234567890l;
    assert(Serialize(i32) ==                        "1234567890");
    assert(Serialize(i32, 12) ==                    "  1234567890");
    assert(Serialize(i32, -12) ==                   "1234567890  ");
    assert(Serialize(i32, 12, 16) ==                "    499602D2");
    assert(Serialize(i32, 12, 16, true) ==          "  0x499602D2");
    assert(Serialize(i32, 12, 16, true, true) ==    "0x0000499602D2");
    uint32 u32 = 2345678900ul;
    assert(Serialize(u32) ==                        "2345678900");
    assert(Serialize(u32, 12) ==                    "  2345678900");
    assert(Serialize(u32, -12) ==                   "2345678900  ");
    assert(Serialize(u32, 12, 16) ==                "    8BD03834");
    assert(Serialize(u32, 12, 16, true) ==          "  0x8BD03834");
    assert(Serialize(u32, 12, 16, true, true) ==    "0x00008BD03834");
    int64 i64 = 9223372036854775807ll;
    assert(Serialize(i64) ==                        "9223372036854775807");
    assert(Serialize(i64, 20) ==                    " 9223372036854775807");
    assert(Serialize(i64, -20) ==                   "9223372036854775807 ");
    assert(Serialize(i64, 20, 16) ==                "    7FFFFFFFFFFFFFFF");
    assert(Serialize(i64, 20, 16, true) ==          "  0x7FFFFFFFFFFFFFFF");
    assert(Serialize(i64, 20, 16, true, true) ==    "0x00007FFFFFFFFFFFFFFF");
    uint64 u64 = 9223372036854775808ull;
    assert(Serialize(u64) ==                        "9223372036854775808");
    assert(Serialize(u64, 20) ==                    " 9223372036854775808");
    assert(Serialize(u64, -20) ==                   "9223372036854775808 ");
    assert(Serialize(u64, 20, 16) ==                "    8000000000000000");
    assert(Serialize(u64, 20, 16, true) ==          "  0x8000000000000000");
    assert(Serialize(u64, 20, 16, true, true) ==    "0x00008000000000000000");
    float f = 1.23456789F;
    assert(Serialize(f) ==                          "1.2345679");
    assert(Serialize(f, 12) ==                      "   1.2345679");
    assert(Serialize(f, -12) ==                     "1.2345679   ");
    assert(Serialize(f, 12, 2) ==                   "        1.23");
    assert(Serialize(f, 12, 7) ==                   "   1.2345679");
    assert(Serialize(f, 12, 8) ==                   "   1.2345679");

    double d = 1.234567890123456;
    assert(Serialize(d) ==                          "1.23456789012346");
    assert(Serialize(d, 18) ==                      "  1.23456789012346");
    assert(Serialize(d, -18) ==                     "1.23456789012346  ");
    assert(Serialize(d, 18, 5) ==                   "           1.23457");
    assert(Serialize(d, 18, 7) ==                   "         1.2345679");
    assert(Serialize(d, 18, 12) ==                  "    1.234567890123");

    string s("hello world");
    assert(Serialize(s) ==                          "hello world");
    assert(Serialize(s, 15) ==                      "    hello world");
    assert(Serialize(s, -15) ==                     "hello world    ");
    assert(Serialize(s, 15, true) ==                "  \"hello world\"");

    const char* str = "hello world";
    assert(Serialize(str) ==                        "hello world");
    assert(Serialize(str, 15) ==                    "    hello world");
    assert(Serialize(str, -15) ==                   "hello world    ");
    assert(Serialize(str, 15, true) ==              "  \"hello world\"");

    const void* pvc = reinterpret_cast<const void*>(0x0123456789ABCDEF);
    assert(Serialize(pvc) ==                        "0x0123456789ABCDEF");
    assert(Serialize(pvc, 20) ==                    "  0x0123456789ABCDEF");
    assert(Serialize(pvc, -20) ==                   "0x0123456789ABCDEF  ");

    void* pv = reinterpret_cast<void*>(0x0123456789ABCDEF);
    assert(Serialize(pv) == "0x0123456789ABCDEF");
    assert(Serialize(pv, 20) == "  0x0123456789ABCDEF");
    assert(Serialize(pv, -20) == "0x0123456789ABCDEF  ");

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
