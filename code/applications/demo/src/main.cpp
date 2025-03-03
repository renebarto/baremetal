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

void InterruptHandlerCLK(IGPIOPin* pin, void *param)
{
    LOG_DEBUG("GPIO CLK");
    if (pin->GetEvent())
    {
        auto value = pin->Get();
        LOG_DEBUG("CLK=%d", value);
        pin->ClearEvent();
    }
}

void InterruptHandlerDT(IGPIOPin* pin, void *param)
{
    LOG_DEBUG("GPIO DT");
    if (pin->GetEvent())
    {
        auto value = pin->Get();
        LOG_DEBUG("DT=%d", value);
        pin->ClearEvent();
    }
}

void InterruptHandlerSW(IGPIOPin* pin, void *param)
{
    LOG_DEBUG("GPIO SW");
    if (pin->GetEvent())
    {
        auto value = pin->Get();
        LOG_DEBUG("SW=%d", value);
        pin->ClearEvent();
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

    pinCLK.ConnectInterrupt(InterruptHandlerCLK, nullptr);
    pinCLK.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
    pinDT.ConnectInterrupt(InterruptHandlerDT, nullptr);
    pinDT.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
    pinSW.ConnectInterrupt(InterruptHandlerSW, nullptr);
    pinSW.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);

    LOG_INFO("Wait 5 seconds");
    Timer::WaitMilliSeconds(5000);

    GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_GPIO3);

    pinCLK.DisconnectInterrupt();
    pinDT.DisconnectInterrupt();
    pinSW.DisconnectInterrupt();

    console.Write("Press r to reboot, h to halt\n");
    char ch{};
    while ((ch != 'r') && (ch != 'h'))
    {
        ch = console.ReadChar();
        console.WriteChar(ch);
    }

    return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
}
