#include <baremetal/ARMInstructions.h>
#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/Format.h>
#include <baremetal/I2CMasterEmulator.h>
#include <baremetal/Logger.h>
#include <baremetal/String.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/stubs/MemoryAccessStubGPIO.h>
#include <device/gpio/KY-040.h>

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

static int value = 0;
static int holdCounter = 0;
static const int HoldThreshold = 2;
static bool reboot = false;

void OnEvent(KY040::Event event, void *param)
{
    LOG_INFO("Event %s", KY040::EventToString(event));
    switch (event)
    {
        case KY040::Event::SwitchDown:
            LOG_INFO("Value %d", value);
            break;
        case KY040::Event::RotateClockwise:
            value++;
            break;
        case KY040::Event::RotateCounterclockwise:
            value--;
            break;
        case KY040::Event::SwitchHold:
            holdCounter++;
            if (holdCounter >= HoldThreshold)
            {
                reboot = true;
                LOG_INFO("Reboot triggered");
            }
            break;
        default:
            break;
    }
}

//bool WriteI2CAddress(uint8 address)
//{
//    LOG_INFO("I2C Write address %02x", address);
//    return false;
//}
//
//bool ReadI2CData(uint8& data)
//{
//    data = {};
//    LOG_INFO("I2C Read data %02x", data);
//    return false;
//}
//
//bool WriteI2CData(uint8 data)
//{
//    LOG_INFO("I2C Write data %02x", data);
//    return false;
//}

int main()
{
    auto& console = GetConsole();
    GetLogger().SetLogLevel(LogSeverity::Info);

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

//    MemoryAccessStubGPIO fakeMemoryAccess;

    KY040 rotarySwitch(11, 9, 10/*, fakeMemoryAccess*/);
    rotarySwitch.Initialize();
    rotarySwitch.RegisterEventHandler(OnEvent, nullptr);

    I2CMaster/*Emulator*/ master;
    //master.SetSendAddressByteCallback(WriteI2CAddress);
    //master.SetRecvDataByteCallback(ReadI2CData);
    //master.SetSendDataByteCallback(WriteI2CData);
    uint8 busIndex = 1;

    if (!master.Initialize(busIndex))
    {
        LOG_ERROR("Cannot scan bus %d", busIndex);
    }
    LOG_INFO("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    uint8 AddressGroupSize = 16;
    for (uint8 addressGroup = 32; addressGroup < 40; addressGroup += AddressGroupSize)
    {
        string line = Format("%02x: ", addressGroup);
        for (uint8 addressOffset = 0; addressOffset < AddressGroupSize; ++addressOffset)
        {
            uint8 address = addressGroup + addressOffset;
            if ((address < 8) || (address >= 120))
            {
                line += "   ";
            }
            else if (master.Scan(address))
            {
                line += Format("%02x ", address);
            }
            else
            {
                line += "-- ";
            }
        }
        LOG_INFO(line.c_str());
    }

    uint8 address{ 0x20 };
    const size_t BufferSize{ 2 };
    uint8 buffer[BufferSize];
    buffer[0] = 0x05;
    buffer[1] = 0x2c;
    auto bytesWritten = master.Write(address, buffer, BufferSize);

    buffer[0] = 0x0C;
    buffer[1] = 0x00;
    bytesWritten = master.Write(address, buffer, BufferSize);
    LOG_INFO("Written %d bytes", bytesWritten);
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    bytesWritten = master.Write(address, buffer, BufferSize);
    LOG_INFO("Written %d bytes", bytesWritten);
    buffer[0] = 0x0D;
    buffer[1] = 0xFF;
    bytesWritten = master.Write(address, buffer, BufferSize);
    LOG_INFO("Written %d bytes", bytesWritten);
    buffer[0] = 0x01;
    buffer[1] = 0xFF;
    bytesWritten = master.Write(address, buffer, BufferSize);
    LOG_INFO("Written %d bytes", bytesWritten);
    buffer[0] = 0x12;
    buffer[1] = 0x0F;
    bytesWritten = master.Write(address, buffer, BufferSize);
    LOG_INFO("Written %d bytes", bytesWritten);

    LOG_INFO("Hold down switch button for %d seconds to reboot", HoldThreshold);
    while (!reboot)
    {
        WaitForInterrupt();
    }

    rotarySwitch.UnregisterEventHandler(OnEvent);
    rotarySwitch.Uninitialize();

    LOG_INFO("Rebooting");

    return static_cast<int>(ReturnCode::ExitReboot);
}
