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

LOG_MODULE("main");

using namespace baremetal;

int main()
{
    auto& console = GetConsole();
    LOG_DEBUG("Hello World!");

    String s1{ "a" };
    String s2{ "a" };
    String s3{ "aaaa", 3 };
    String s4{ 4, 'b' };
    String s5{ s3 };
    String s6{ s3, 1 };
    String s7{ s3, 1, 1 };
    String s8{ nullptr };
    String s9{ "" };
    String s10{ nullptr, 3 };

    LOG_INFO("s1");
    for (auto ch : s1)
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

    const char* s = "abcdefghijklmnopqrstuvwxyz";
    s1.Assign(s);
    assert(strcmp(s1, "abcdefghijklmnopqrstuvwxyz") == 0);
    s1.Assign("");
    assert(strcmp(s1, "") == 0);
    s1.Assign(nullptr);
    assert(strcmp(s1, "") == 0);
    s1.Assign(s, 6);
    assert(strcmp(s1, "abcdef") == 0);
    s1.Assign("", 6);
    assert(strcmp(s1, "") == 0);
    s1.Assign(nullptr, 6);
    assert(strcmp(s1, "") == 0);
    s8 = s;
    s1.Assign(s8, 3);
    assert(strcmp(s1, "defghijklmnopqrstuvwxyz") == 0);
    s1.Assign(s8, 4, 6);
    assert(strcmp(s1, "efghij") == 0);
    s1.Assign(6, 'c');
    assert(strcmp(s1, "cccccc") == 0);

    const String s8c{ s8 };
    assert(s8.At(3) == 'd');
    assert(s8c.At(3) == 'd');
    assert(s8.Front() == 'a');
    assert(s8c.Front() == 'a');
    assert(s8.Back() == 'z');
    assert(s8c.Back() == 'z');
    assert(s8[3] == 'd');
    assert(s8c[3] == 'd');
    assert(s8c.Capacity() == 64);
    assert(s8.Reserve(128) == 128);
    assert(s8.Capacity() == 128);

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
    s1.Append(4, 'b');
    assert(strcmp(s1, "abbbb") == 0);
    s1.Append(s2);
    assert(strcmp(s1, "abbbba") == 0);
    s1.Append(s8, 3, 5);
    assert(strcmp(s1, "abbbbadefgh") == 0);
    s1.Append("ccc");
    assert(strcmp(s1, "abbbbadefghccc") == 0);
    s1.Append("dddddd", 3);
    assert(strcmp(s1, "abbbbadefghcccddd") == 0);
    s1.Clear();
    assert(strcmp(s1, "") == 0);
    s1.Append("");
    assert(strcmp(s1, "") == 0);
    s1.Append(nullptr);
    assert(strcmp(s1, "") == 0);
    s1.Append("", 3);
    assert(strcmp(s1, "") == 0);
    s1.Append(nullptr, 3);
    assert(strcmp(s1, "") == 0);

    s1 = s;
    s2 = "c";
    auto pos = s1.Find(s2);
    assert(pos == 2);
    pos = s1.Find(s2, 1);
    assert(pos == 2);
    pos = s1.Find(s2, 3);
    assert(pos == String::npos);
    s2 = "deg";
    pos = s1.Find(s2, 3);
    assert(pos == String::npos);
    pos = s1.Find(s2, 3, 2);
    assert(pos == 3);
    s2 = "xyz";
    pos = s1.Find(s2);
    assert(pos == 23);

    pos = s1.Find("d");
    assert(pos == 3);
    pos = s1.Find("d", 1);
    assert(pos == 3);
    pos = s1.Find("d", 4);
    assert(pos == String::npos);
    pos = s1.Find("def", 2);
    assert(pos == 3);
    pos = s1.Find("deg", 2);
    assert(pos == String::npos);
    pos = s1.Find("deg", 2, 2);
    assert(pos == 3);
    pos = s1.Find("xyz");
    assert(pos == 23);
    pos = s1.Find("");
    assert(pos == 0);
    pos = s1.Find(nullptr);
    assert(pos == 0);
    pos = s1.Find("", 2);
    assert(pos == 2);
    pos = s1.Find(nullptr, 2);
    assert(pos == 2);
    pos = s1.Find(nullptr, 26);
    assert(pos == String::npos);
    pos = s1.Find("", 2, 2);
    assert(pos == 2);
    pos = s1.Find(nullptr, 2, 2);
    assert(pos == 2);
    pos = s1.Find(nullptr, 26, 1);
    assert(pos == String::npos);

    pos = s1.Find('d');
    assert(pos == 3);
    pos = s1.Find('d', 2);
    assert(pos == 3);
    pos = s1.Find('d', 4);
    assert(pos == String::npos);
    pos = s1.Find('A');
    assert(pos == String::npos);
    pos = s1.Find("z");
    assert(pos == 25);

    s2 = "abc";
    s3 = "xyz";
    auto isTrue = s1.StartsWith('a');
    assert(isTrue);
    isTrue = s1.StartsWith('z');
    assert(!isTrue);
    isTrue = s1.StartsWith("abc");
    assert(isTrue);
    isTrue = s1.StartsWith("xyz");
    assert(!isTrue);
    isTrue = s1.StartsWith("");
    assert(isTrue);
    isTrue = s1.StartsWith(nullptr);
    assert(isTrue);
    isTrue = s1.StartsWith(s2);
    assert(isTrue);
    isTrue = s1.StartsWith(s3);
    assert(!isTrue);

    isTrue = s1.EndsWith('a');
    assert(!isTrue);
    isTrue = s1.EndsWith('z');
    assert(isTrue);
    isTrue = s1.EndsWith("abc");
    assert(!isTrue);
    isTrue = s1.EndsWith("xyz");
    assert(isTrue);
    isTrue = s1.EndsWith("");
    assert(isTrue);
    isTrue = s1.EndsWith(nullptr);
    assert(isTrue);
    isTrue = s1.EndsWith(s2);
    assert(!isTrue);
    isTrue = s1.EndsWith(s3);
    assert(isTrue);

    isTrue = s1.Contains('a');
    assert(isTrue);
    isTrue = s1.Contains('A');
    assert(!isTrue);
    isTrue = s1.Contains("abc");
    assert(isTrue);
    isTrue = s1.Contains("XYZ");
    assert(!isTrue);
    isTrue = s1.Contains("");
    assert(isTrue);
    isTrue = s1.Contains(nullptr);
    assert(isTrue);
    isTrue = s1.Contains(s2);
    assert(isTrue);
    isTrue = s1.Contains(s3);
    assert(isTrue);

    s2 = s1.SubStr();
    assert(strcmp(s2, "abcdefghijklmnopqrstuvwxyz") == 0);
    s2 = s1.SubStr(6);
    assert(strcmp(s2, "ghijklmnopqrstuvwxyz") == 0);
    s2 = s1.SubStr(6, 6);
    assert(strcmp(s2, "ghijkl") == 0);

    s1 = "abcdefg";
    s2 = "abcdefG";
    s3 = "abcdefg";
    isTrue = s1.Equals(s2);
    assert(!isTrue);
    isTrue = s1.Equals(s3);
    assert(isTrue);
    isTrue = s1.Equals("abcefg");
    assert(!isTrue);
    isTrue = s1.Equals("abcdefg");
    assert(isTrue);
    isTrue = s1.Equals("");
    assert(!isTrue);
    isTrue = s1.Equals(nullptr);
    assert(!isTrue);
    s4 = "";
    isTrue = s4.EqualsCaseInsensitive(s3);
    assert(!isTrue);
    isTrue = s4.Equals("");
    assert(isTrue);
    isTrue = s4.Equals(nullptr);
    assert(isTrue);

    isTrue = s1.EqualsCaseInsensitive(s2);
    assert(isTrue);
    isTrue = s1.EqualsCaseInsensitive(s3);
    assert(isTrue);
    isTrue = s1.EqualsCaseInsensitive("abcefg");
    assert(!isTrue);
    isTrue = s1.EqualsCaseInsensitive("abcdefg");
    assert(isTrue);
    isTrue = s1.EqualsCaseInsensitive("");
    assert(!isTrue);
    isTrue = s1.EqualsCaseInsensitive(nullptr);
    assert(!isTrue);
    s4 = "";
    isTrue = s4.EqualsCaseInsensitive(s3);
    assert(!isTrue);
    isTrue = s4.EqualsCaseInsensitive("");
    assert(isTrue);
    isTrue = s4.EqualsCaseInsensitive(nullptr);
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
    auto result = s1.Compare(s2);
    assert(result == 1);
    result = s2.Compare(s1);
    assert(result == -1);
    result = s1.Compare(s3);
    assert(result == 0);
    result = s3.Compare(s1);
    assert(result == 0);
    result = s1.Compare(1, 6, s4);
    assert(result == 0);
    result = s1.Compare(1, 5, s4);
    assert(result == -1);
    result = s1.Compare(3, 6, s4, 2);
    assert(result == 0);
    result = s1.Compare(3, 6, s4, 2, 1);
    assert(result == 1);
    result = s1.Compare(3, 3, s4, 2, 6);
    assert(result == -1);

    result = s1.Compare("a");
    assert(result == 1);
    result = s1.Compare("Abcdefg");
    assert(result == 1);
    result = s1.Compare("abdecfg");
    assert(result == -1);
    result = s1.Compare("");
    assert(result == 1);
    result = s1.Compare(nullptr);
    assert(result == 1);
    s2 = "";
    result = s2.Compare("a");
    assert(result == -1);
    result = s2.Compare("");
    assert(result == 0);
    result = s2.Compare(nullptr);
    assert(result == 0);

    s1 = "abcde";
    s2 = "fghijk";
    s3 = s1.Replace(0, 1, s2);
    assert(s1.Equals("fghijkbcde"));
    assert(s3.Equals("fghijkbcde"));
    s1 = "abcde";
    s3 = s1.Replace(1, 2, s2, 2);
    assert(s1.Equals("ahijkde"));
    assert(s3.Equals("ahijkde"));
    s1 = "abcde";
    s3 = s1.Replace(1, 2, s2, 2, 2);
    assert(s1.Equals("ahide"));
    assert(s3.Equals("ahide"));
    s1 = "abcde";
    s3 = s1.Replace(0, 1, "uvwxyz");
    assert(s1.Equals("uvwxyzbcde"));
    assert(s3.Equals("uvwxyzbcde"));
    s1 = "abcde";
    s3 = s1.Replace(1, 2, "uvwxyz", 2);
    assert(s1.Equals("auvde"));
    assert(s3.Equals("auvde"));
    s1 = "abcde";
    s3 = s1.Replace(0, 1, 'x');
    assert(s1.Equals("xbcde"));
    assert(s3.Equals("xbcde"));
    s1 = "abcde";
    s3 = s1.Replace(1, 2, 'x', 3);
    assert(s1.Equals("axxxde"));
    assert(s3.Equals("axxxde"));

    s1 = "abcde";
    s2 = "cd";
    s3 = "xy";
    int count = s1.Replace(s2, s3);
    assert(count == 1);
    assert(s1.Equals("abxye"));
    s1 = "abababab";
    s2 = "ab";
    s3 = "cd";
    count = s1.Replace(s2, s3);
    assert(count == 4);
    assert(s1.Equals("cdcdcdcd"));
    s1 = "abcde";
    count = s1.Replace("cd", "xy");
    assert(count == 1);
    assert(s1.Equals("abxye"));
    s1 = "abababab";
    count = s1.Replace("ab", "cd");
    assert(count == 4);
    assert(s1.Equals("cdcdcdcd"));

    s1 = "abcd";
    s2 = s1.Align(8);
    assert(s2.Equals("    abcd"));
    s2 = s1.Align(-8);
    assert(s2.Equals("abcd    "));
    s2 = s1.Align(0);
    assert(s2.Equals("abcd"));

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
