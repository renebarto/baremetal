#include "baremetal/ARMInstructions.h"
#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Console.h"
#include "baremetal/Logger.h"
#include "baremetal/Mailbox.h"
#include "baremetal/MemoryManager.h"
#include "baremetal/New.h"
#include "baremetal/RPIProperties.h"
#include "baremetal/Serialization.h"
#include "baremetal/String.h"
#include "baremetal/SysConfig.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"
#include "stdlib/Util.h"

LOG_MODULE("main");

using namespace baremetal;

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Debug);

    MemoryManager& memoryManager = GetMemoryManager();
    LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
    LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
    LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));

    memoryManager.DumpStatus();

    String s1{"a"};
    String s2{"a"};
    String s3{"aaaa", 3};
    String s4{4, 'b'};
    String s5{s3};
    String s6{s3, 1};
    String s7{s3, 1, 1};
    String s8{nullptr};
    String s9{""};
    String s10{nullptr, 3};

    LOG_INFO("s4");
    for (auto ch : s4)
    {
        LOG_INFO("%c", ch);
    }
    assert(strcmp(s1, "a") == 0);
    assert(strcmp(s2, "a") == 0);
    assert(strcmp(s3, "aaa") == 0);
    assert(strcmp(s4, "bbbb") == 0);
    assert(strcmp(s5, "aaa") == 0);
    assert(strcmp(s6, "aa") == 0);
    assert(strcmp(s7, "a") == 0);
    assert(strcmp(s8, "") == 0);
    assert(strcmp(s9, "") == 0);
    assert(strcmp(s10, "") == 0);

    s7 = "abcde";
    assert(strcmp(s7, "abcde") == 0);
    s7 = nullptr;
    assert(strcmp(s7, "") == 0);
    s7 = "";
    assert(strcmp(s7, "") == 0);
    s6 = s4;
    assert(strcmp(s6, "bbbb") == 0);
    {
        String s99{"cccc"};
        s6 = s99;
    }
    assert(strcmp(s6, "cccc") == 0);

    const char* s = "abcdefghijklmnopqrstuvwxyz";
    s1.assign(s);
    assert(strcmp(s1, "abcdefghijklmnopqrstuvwxyz") == 0);
    s1.assign("");
    assert(strcmp(s1, "") == 0);
    s1.assign(nullptr);
    assert(strcmp(s1, "") == 0);
    s1.assign(s, 6);
    assert(strcmp(s1, "abcdef") == 0);
    s1.assign("", 6);
    assert(strcmp(s1, "") == 0);
    s1.assign(nullptr, 6);
    assert(strcmp(s1, "") == 0);
    s8 = s;
    s1.assign(s8, 3);
    assert(strcmp(s1, "defghijklmnopqrstuvwxyz") == 0);
    s1.assign(s8, 4, 6);
    assert(strcmp(s1, "efghij") == 0);
    s1.assign(6, 'c');
    assert(strcmp(s1, "cccccc") == 0);

    const String s8c{s8};
    assert(s8.at(3) == 'd');
    assert(s8c.at(3) == 'd');
    assert(s8.front() == 'a');
    assert(s8c.front() == 'a');
    assert(s8.back() == 'z');
    assert(s8c.back() == 'z');
    assert(s8[3] == 'd');
    assert(s8c[3] == 'd');
    assert(s8c.capacity() == 256);
    assert(s8.reserve(1024) == 1024);
    assert(s8.capacity() == 1024);

    s1 = "a";
    assert(strcmp(s1, "a") == 0);
    s1 += 'b';
    assert(strcmp(s1, "ab") == 0);
    s2 = "a";
    s1 += s2;
    assert(strcmp(s1, "aba") == 0);
    s1 += "abcde";
    assert(strcmp(s1, "abaabcde") == 0);
    s1 = "a";
    s1 += "";
    assert(strcmp(s1, "a") == 0);
    s1 += nullptr;
    assert(strcmp(s1, "a") == 0);

    s3 = "";
    s4 = s1 + s2;
    assert(strcmp(s4, "aa") == 0);
    s4 = s1 + s3;
    assert(strcmp(s4, "a") == 0);
    s4 = s1 + "b";
    assert(strcmp(s4, "ab") == 0);
    s4 = s1 + "";
    assert(strcmp(s4, "a") == 0);
    s4 = s1 + nullptr;
    assert(strcmp(s4, "a") == 0);
    s4 = "b" + s1;
    assert(strcmp(s4, "ba") == 0);
    s4 = "" + s1;
    assert(strcmp(s4, "a") == 0);
    s4 = nullptr + s1;
    assert(strcmp(s4, "a") == 0);

    s1 = "a";
    s1.append(4, 'b');
    assert(strcmp(s1, "abbbb") == 0);
    s1.append(s2);
    assert(strcmp(s1, "abbbba") == 0);
    s1.append(s8, 3, 5);
    assert(strcmp(s1, "abbbbadefgh") == 0);
    s1.append("ccc");
    assert(strcmp(s1, "abbbbadefghccc") == 0);
    s1.append("dddddd", 3);
    assert(strcmp(s1, "abbbbadefghcccddd") == 0);
    s1.clear();
    assert(strcmp(s1, "") == 0);
    s1.append("");
    assert(strcmp(s1, "") == 0);
    s1.append(nullptr);
    assert(strcmp(s1, "") == 0);
    s1.append("", 3);
    assert(strcmp(s1, "") == 0);
    s1.append(nullptr, 3);
    assert(strcmp(s1, "") == 0);

    s1 = s;
    s2 = "c";
    auto pos = s1.find(s2);
    assert(pos == 2);
    pos = s1.find(s2, 1);
    assert(pos == 2);
    pos = s1.find(s2, 3);
    assert(pos == String::npos);
    s2 = "deg";
    pos = s1.find(s2, 3);
    assert(pos == String::npos);
    pos = s1.find(s2, 3, 2);
    assert(pos == 3);
    s2 = "xyz";
    pos = s1.find(s2);
    assert(pos == 23);

    pos = s1.find("d");
    assert(pos == 3);
    pos = s1.find("d", 1);
    assert(pos == 3);
    pos = s1.find("d", 4);
    assert(pos == String::npos);
    pos = s1.find("def", 2);
    assert(pos == 3);
    pos = s1.find("deg", 2);
    assert(pos == String::npos);
    pos = s1.find("deg", 2, 2);
    assert(pos == 3);
    pos = s1.find("xyz");
    assert(pos == 23);
    pos = s1.find("");
    assert(pos == 0);
    pos = s1.find(nullptr);
    assert(pos == 0);
    pos = s1.find("", 2);
    assert(pos == 2);
    pos = s1.find(nullptr, 2);
    assert(pos == 2);
    pos = s1.find(nullptr, 26);
    assert(pos == String::npos);
    pos = s1.find("", 2, 2);
    assert(pos == 2);
    pos = s1.find(nullptr, 2, 2);
    assert(pos == 2);
    pos = s1.find(nullptr, 26, 1);
    assert(pos == String::npos);

    pos = s1.find('d');
    assert(pos == 3);
    pos = s1.find('d', 2);
    assert(pos == 3);
    pos = s1.find('d', 4);
    assert(pos == String::npos);
    pos = s1.find('A');
    assert(pos == String::npos);
    pos = s1.find("z");
    assert(pos == 25);

    s2 = "abc";
    s3 = "xyz";
    auto isTrue = s1.starts_with('a');
    assert(isTrue);
    isTrue = s1.starts_with('z');
    assert(!isTrue);
    isTrue = s1.starts_with("abc");
    assert(isTrue);
    isTrue = s1.starts_with("xyz");
    assert(!isTrue);
    isTrue = s1.starts_with("");
    assert(isTrue);
    isTrue = s1.starts_with(nullptr);
    assert(isTrue);
    isTrue = s1.starts_with(s2);
    assert(isTrue);
    isTrue = s1.starts_with(s3);
    assert(!isTrue);

    isTrue = s1.ends_with('a');
    assert(!isTrue);
    isTrue = s1.ends_with('z');
    assert(isTrue);
    isTrue = s1.ends_with("abc");
    assert(!isTrue);
    isTrue = s1.ends_with("xyz");
    assert(isTrue);
    isTrue = s1.ends_with("");
    assert(isTrue);
    isTrue = s1.ends_with(nullptr);
    assert(isTrue);
    isTrue = s1.ends_with(s2);
    assert(!isTrue);
    isTrue = s1.ends_with(s3);
    assert(isTrue);

    isTrue = s1.contains('a');
    assert(isTrue);
    isTrue = s1.contains('A');
    assert(!isTrue);
    isTrue = s1.contains("abc");
    assert(isTrue);
    isTrue = s1.contains("XYZ");
    assert(!isTrue);
    isTrue = s1.contains("");
    assert(isTrue);
    isTrue = s1.contains(nullptr);
    assert(isTrue);
    isTrue = s1.contains(s2);
    assert(isTrue);
    isTrue = s1.contains(s3);
    assert(isTrue);

    s2 = s1.substr();
    assert(strcmp(s2, "abcdefghijklmnopqrstuvwxyz") == 0);
    s2 = s1.substr(6);
    assert(strcmp(s2, "ghijklmnopqrstuvwxyz") == 0);
    s2 = s1.substr(6, 6);
    assert(strcmp(s2, "ghijkl") == 0);

    s1 = "abcdefg";
    s2 = "abcdefG";
    s3 = "abcdefg";
    isTrue = s1.equals(s2);
    assert(!isTrue);
    isTrue = s1.equals(s3);
    assert(isTrue);
    isTrue = s1.equals("abcefg");
    assert(!isTrue);
    isTrue = s1.equals("abcdefg");
    assert(isTrue);
    isTrue = s1.equals("");
    assert(!isTrue);
    isTrue = s1.equals(nullptr);
    assert(!isTrue);
    s4 = "";
    isTrue = s4.equals_case_insensitive(s3);
    assert(!isTrue);
    isTrue = s4.equals("");
    assert(isTrue);
    isTrue = s4.equals(nullptr);
    assert(isTrue);

    isTrue = s1.equals_case_insensitive(s2);
    assert(isTrue);
    isTrue = s1.equals_case_insensitive(s3);
    assert(isTrue);
    isTrue = s1.equals_case_insensitive("abcefg");
    assert(!isTrue);
    isTrue = s1.equals_case_insensitive("abcdefg");
    assert(isTrue);
    isTrue = s1.equals_case_insensitive("");
    assert(!isTrue);
    isTrue = s1.equals_case_insensitive(nullptr);
    assert(!isTrue);
    s4 = "";
    isTrue = s4.equals_case_insensitive(s3);
    assert(!isTrue);
    isTrue = s4.equals_case_insensitive("");
    assert(isTrue);
    isTrue = s4.equals_case_insensitive(nullptr);
    assert(isTrue);

    assert(s1 == s3);
    assert(s1 != s2);
    assert(s1 == "abcdefg");
    assert(s1 != "abcdefG");
    assert(s1 != "");
    assert(s1 != nullptr);
    assert("abcdefg" == s1);
    assert("abcdefG" != s1);
    assert("" != s1);
    assert(nullptr != s1);
    assert(s4 != s3);
    assert(s4 == "");
    assert(s4 == nullptr);
    assert("" == s4);
    assert(nullptr == s4);

    s4 = "bcdefg";
    s5 = "def";
    auto result = s1.compare(s2);
    assert(result == 1);
    result = s2.compare(s1);
    assert(result == -1);
    result = s1.compare(s3);
    assert(result == 0);
    result = s3.compare(s1);
    assert(result == 0);
    result = s1.compare(1, 6, s4);
    assert(result == 0);
    result = s1.compare(1, 5, s4);
    assert(result == -1);
    result = s1.compare(3, 6, s4, 2);
    assert(result == 0);
    result = s1.compare(3, 6, s4, 2, 1);
    assert(result == 1);
    result = s1.compare(3, 3, s4, 2, 6);
    assert(result == -1);

    result = s1.compare("a");
    assert(result == 1);
    result = s1.compare("Abcdefg");
    assert(result == 1);
    result = s1.compare("abdecfg");
    assert(result == -1);
    result = s1.compare("");
    assert(result == 1);
    result = s1.compare(nullptr);
    assert(result == 1);
    s2 = "";
    result = s2.compare("a");
    assert(result == -1);
    result = s2.compare("");
    assert(result == 0);
    result = s2.compare(nullptr);
    assert(result == 0);

    s1 = "abcde";
    s2 = "fghijk";
    s3 = s1.replace(0, 1, s2);
    assert(s1.equals("fghijkbcde"));
    assert(s3.equals("fghijkbcde"));
    s1 = "abcde";
    s3 = s1.replace(1, 2, s2, 2);
    assert(s1.equals("ahijkde"));
    assert(s3.equals("ahijkde"));
    s1 = "abcde";
    s3 = s1.replace(1, 2, s2, 2, 2);
    assert(s1.equals("ahide"));
    assert(s3.equals("ahide"));
    s1 = "abcde";
    s3 = s1.replace(0, 1, "uvwxyz");
    assert(s1.equals("uvwxyzbcde"));
    assert(s3.equals("uvwxyzbcde"));
    s1 = "abcde";
    s3 = s1.replace(1, 2, "uvwxyz", 2);
    assert(s1.equals("auvde"));
    assert(s3.equals("auvde"));
    s1 = "abcde";
    s3 = s1.replace(0, 1, 'x');
    assert(s1.equals("xbcde"));
    assert(s3.equals("xbcde"));
    s1 = "abcde";
    s3 = s1.replace(1, 2, 'x', 3);
    assert(s1.equals("axxxde"));
    assert(s3.equals("axxxde"));

    s1 = "abcde";
    s2 = "cd";
    s3 = "xy";
    int count = s1.replace(s2, s3);
    assert(count == 1);
    assert(s1.equals("abxye"));
    s1 = "abababab";
    s2 = "ab";
    s3 = "cd";
    count = s1.replace(s2, s3);
    assert(count == 4);
    assert(s1.equals("cdcdcdcd"));
    s1 = "abcde";
    count = s1.replace("cd", "xy");
    assert(count == 1);
    assert(s1.equals("abxye"));
    s1 = "abababab";
    count = s1.replace("ab", "cd");
    assert(count == 4);
    assert(s1.equals("cdcdcdcd"));

    s1 = "abcd";
    s2 = s1.align(8);
    assert(s2.equals("    abcd"));
    s2 = s1.align(-8);
    assert(s2.equals("abcd    "));
    s2 = s1.align(0);
    assert(s2.equals("abcd"));

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

    LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
    LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
    LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));

    memoryManager.DumpStatus();

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
