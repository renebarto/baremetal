#include <baremetal/ARMInstructions.h>
#include <baremetal/Assert.h>
#include <baremetal/Console.h>
#include <baremetal/InterruptHandler.h>
#include <baremetal/Interrupts.h>
#include <baremetal/Logger.h>
#include <baremetal/PhysicalGPIOPin.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>

LOG_MODULE("main");

using namespace baremetal;

struct GPIOPins
{
    IGPIOPin& pinCLK;
    IGPIOPin& pinDT;
    IGPIOPin& pinSW;
};

void InterruptHandler(void *param)
{
    LOG_DEBUG("GPIO3");
    GPIOPins* pins = reinterpret_cast<GPIOPins*>(param);
    if (pins->pinCLK.GetEvent())
    {
        auto value = pins->pinCLK.Get();
        LOG_DEBUG("CLK=%d", value);
        pins->pinCLK.ClearEvent();
    }
    if (pins->pinDT.GetEvent())
    {
        auto value = pins->pinDT.Get();
        LOG_DEBUG("DT=%d", value);
        pins->pinDT.ClearEvent();
    }
    if (pins->pinSW.GetEvent())
    {
        auto value = pins->pinSW.Get();
        LOG_DEBUG("SW=%d", value);
        pins->pinSW.ClearEvent();
    }
}

int main()
{
    auto& console = GetConsole();

    auto exceptionLevel = CurrentEL();
    LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));

    PhysicalGPIOPin pinCLK(11, GPIOMode::InputPullUp);
    PhysicalGPIOPin pinDT(9, GPIOMode::InputPullUp);
    PhysicalGPIOPin pinSW(10, GPIOMode::InputPullUp);
    GPIOPins pins { pinCLK, pinDT, pinSW };

    GetInterruptSystem().RegisterIRQHandler(IRQ_ID::IRQ_GPIO3, InterruptHandler, &pins);

    pinCLK.EnableInterrupt(GPIOInterruptType::RisingEdge);
    pinCLK.EnableInterrupt(GPIOInterruptType::FallingEdge);
    pinDT.EnableInterrupt(GPIOInterruptType::RisingEdge);
    pinDT.EnableInterrupt(GPIOInterruptType::FallingEdge);
    pinSW.EnableInterrupt(GPIOInterruptType::RisingEdge);
    pinSW.EnableInterrupt(GPIOInterruptType::FallingEdge);

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_GPIO3);

    pinCLK.DisableAllInterrupts();
    pinDT.DisableAllInterrupts();
    pinSW.DisableAllInterrupts();

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
