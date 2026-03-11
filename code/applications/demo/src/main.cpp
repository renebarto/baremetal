#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/PhysicalGPIOPin.h"
#include "baremetal/String.h"
#include "baremetal/System.h"
#include "baremetal/Timer.h"
#include "device/spi/MCP23008SPI.h"

LOG_MODULE("main");

using namespace baremetal;
using namespace device;

int main()
{
    GetLogger().SetLogLevel(LogSeverity::Info);

    uint8 spiDevice = 0;
    SPI_CEIndex spiCE = SPI_CEIndex::CE1;
    SPIMaster device;
    if (!device.Initialize(spiDevice))
    {
        LOG_INFO("Cannot initialize SPI device");
    }
    uint8 readBuffer[3] {};
    //for (int i = 0; i < 11; i++)
    //{
    //    writeBuffer[1] = static_cast<uint8>(i);
    //    device.WriteRead(spiCE, writeBuffer, readBuffer, 3);
    //    TRACE_INFO("Write %02x %02x %02x", writeBuffer[0], writeBuffer[1], writeBuffer[2]);
    //    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);
    //}

    TRACE_INFO("Set IODIR 1");
    uint8 writeSetIODIR[3] {0b01000000, 0x00, 0x00};
    device.WriteRead(spiCE, writeSetIODIR, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetIODIR[0], writeSetIODIR[1], writeSetIODIR[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    TRACE_INFO("Set GPIO 1");
    uint8 writeSetGPIO[3] {0b01000000, 0x09, 0xFF};
    device.WriteRead(spiCE, writeSetGPIO, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetGPIO[0], writeSetGPIO[1], writeSetGPIO[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    Timer::WaitMilliSeconds(500);

    TRACE_INFO("Set IODIR 0");
    device.WriteRead(SPI_CEIndex::CE0, writeSetIODIR, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetIODIR[0], writeSetIODIR[1], writeSetIODIR[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    TRACE_INFO("Set GPIO 0");
    device.WriteRead(SPI_CEIndex::CE0, writeSetGPIO, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetGPIO[0], writeSetGPIO[1], writeSetGPIO[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    Timer::WaitMilliSeconds(5000);

    TRACE_INFO("Set IODIR 1");
    writeSetIODIR[2] = 0xFF;
    device.WriteRead(spiCE, writeSetIODIR, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetIODIR[0], writeSetIODIR[1], writeSetIODIR[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    TRACE_INFO("Set GPIO 1");
    writeSetGPIO[2] = 0x00;
    device.WriteRead(spiCE, writeSetGPIO, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetGPIO[0], writeSetGPIO[1], writeSetGPIO[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    Timer::WaitMilliSeconds(500);

    TRACE_INFO("Set IODIR 0");
    device.WriteRead(SPI_CEIndex::CE0, writeSetIODIR, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetIODIR[0], writeSetIODIR[1], writeSetIODIR[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    TRACE_INFO("Set GPIO 0");
    device.WriteRead(SPI_CEIndex::CE0, writeSetGPIO, readBuffer, 3);
    TRACE_INFO("Write %02x %02x %02x", writeSetGPIO[0], writeSetGPIO[1], writeSetGPIO[2]);
    TRACE_INFO("Read %02x %02x %02x", readBuffer[0], readBuffer[1], readBuffer[2]);

    uint8 readRegCommand[2] {0b01000001, 0x00};
    uint8 readRegResponse[1] {};
    for (int i = 0; i < 11; i++)
    {
        readRegCommand[1] = static_cast<uint8>(i);
        device.Write(spiCE, readRegCommand, 2);
        TRACE_INFO("Write %02x %02x", readRegCommand[0], readRegCommand[1]);
        device.Read(spiCE, readRegResponse, 1);
        TRACE_INFO("Read %02x", readRegResponse[0]);
    }
    for (int i = 0; i < 11; i++)
    {
        readRegCommand[1] = static_cast<uint8>(i);
        device.Write(SPI_CEIndex::CE0, readRegCommand, 2);
        TRACE_INFO("Write %02x %02x", readRegCommand[0], readRegCommand[1]);
        device.Read(SPI_CEIndex::CE0, readRegResponse, 1);
        TRACE_INFO("Read %02x", readRegResponse[0]);
    }

    //uint8 spiDevice = 0;
    //MCP23008SPI expander;

    //if (!expander.Initialize(spiDevice, SPI_CEIndex::CE0))
    //{
    //    LOG_INFO("Cannot initialize expander");
    //}

    //expander.GetPortValue();
    //expander.SetPortDirections(MCP23008PinDirection::Out);
    //for (int i = 0; i < 10; i++)
    //{
    //    expander.SetPortValue(0x55);
    //    Timer::WaitMilliSeconds(500);
    //    expander.SetPortValue(0xAA);
    //    Timer::WaitMilliSeconds(500);
    //}
    //expander.SetPortValue(0x00);

    LOG_INFO("Rebooting");

    return static_cast<int>(ReturnCode::ExitReboot);
}
