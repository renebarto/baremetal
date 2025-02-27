# Raspberry Pi Interrupt Control {#RASPBERRY_PI_INTERRUPT_CONTROL}

## Raspberry Pi 3

See [documentation](pdf/bcm2835-peripherals.pdf), section 7.

### Interrupt ID

Raspberry Pi 3 has an interrupt controller as part of the specific Broadcom SoC, BCM2835/6/7. It supports a total of 84 interrupts:
- 32 in the IRQ1 control group (also named GPU 0..31 IRQ, linked to GPU pending 0)
- 32 in the IRQ2 control group (also named GPU 32..63 IRQ, linked to GPU pending 1)
- 8 in the basic IRQ control group  (also named ARM IRQ, linked to basic pending)
- 12 ARM local interrupts in the ARM local control group (outside image below)

<img src="images/rpi3-interrupt.png"  alt="Memory map" width="400"/>

| IRQ name            | IRQ# | Group    | Index | Source and type |
|---------------------|------|----------|-------|-----------------|
| IRQ_TIMER0          | 0x00 | IRQ1     | 0     | BCM system timer 0 interrupt, when compare value it hit
| IRQ_TIMER1          | 0x01 | IRQ1     | 1     | BCM system timer 1 interrupt, when compare value it hit
| IRQ_TIMER2          | 0x02 | IRQ1     | 2     | BCM system timer 2 interrupt, when compare value it hit
| IRQ_TIMER3          | 0x03 | IRQ1     | 3     | BCM system timer 3 interrupt, when compare value it hit
| IRQ_CODEC0          | 0x04 | IRQ1     | 4     | H.264 codec 0 interrupt, undocumented
| IRQ_CODEC1          | 0x05 | IRQ1     | 5     | H.264 codec 1 interrupt, undocumented
| IRQ_CODEC2          | 0x06 | IRQ1     | 6     | H.264 codec 2 interrupt, undocumented
| IRQ_JPEG            | 0x07 | IRQ1     | 7     | JPEG interrupt, undocumented
| IRQ_ISP             | 0x08 | IRQ1     | 8     | ISP interrupt, undocumented
| IRQ_USB             | 0x09 | IRQ1     | 9     | USB interrupt
| IRQ_3D              | 0x0A | IRQ1     | 10    | VideoCore 3D interrupt, undocumented
| IRQ_TRANSPOSER      | 0x0B | IRQ1     | 11    | TXP / Transposer interrupt, undocumented
| IRQ_MULTICORESYNC0  | 0x0C | IRQ1     | 12    | Multicore sync 0 interrupt, undocumented
| IRQ_MULTICORESYNC1  | 0x0D | IRQ1     | 13    | Multicore sync 1 interrupt, undocumented
| IRQ_MULTICORESYNC2  | 0x0E | IRQ1     | 14    | Multicore sync 2 interrupt, undocumented
| IRQ_MULTICORESYNC3  | 0x0F | IRQ1     | 15    | Multicore sync 3 interrupt, undocumented
| IRQ_DMA0            | 0x10 | IRQ1     | 16    | DMA channel 0 interrupt, undocumented
| IRQ_DMA1            | 0x11 | IRQ1     | 17    | DMA channel 1 interrupt, undocumented
| IRQ_DMA2            | 0x12 | IRQ1     | 18    | DMA channel 2 interrupt, I2S PCM TX
| IRQ_DMA3            | 0x13 | IRQ1     | 19    | DMA channel 3 interrupt, I2S PCM RX
| IRQ_DMA4            | 0x14 | IRQ1     | 20    | DMA channel 4 interrupt, SMI
| IRQ_DMA5            | 0x15 | IRQ1     | 21    | DMA channel 5 interrupt, PWM
| IRQ_DMA6            | 0x16 | IRQ1     | 22    | DMA channel 6 interrupt, SPI TX
| IRQ_DMA7            | 0x17 | IRQ1     | 23    | DMA channel 7 interrupt, SPI RX
| IRQ_DMA8            | 0x18 | IRQ1     | 24    | DMA channel 8 interrupt, undocumented
| IRQ_DMA9            | 0x19 | IRQ1     | 25    | DMA channel 9 interrupt, undocumented
| IRQ_DMA10           | 0x1A | IRQ1     | 26    | DMA channel 10 interrupt, undocumented
| IRQ_DMA11-14        | 0x1B | IRQ1     | 27    | DMA channel 11-14 interrupt, EMMC / UART TX / undocumented / UART RX
| IRQ_DMA_SHARED      | 0x1C | IRQ1     | 28    | DMA channel shared interrupt
| IRQ_AUX             | 0x1D | IRQ1     | 29    | AUX interrupt (UART1, SPI0/1)
| IRQ_ARM             | 0x1E | IRQ1     | 30    | ARM interrupt, undocumented
| IRQ_VPUDMA          | 0x1F | IRQ1     | 31    | VPU interrupt, undocumented
| IRQ_HOSTPORT        | 0x20 | IRQ2     | 0     | USB Host Port interrupt, undocumented
| IRQ_VIDEOSCALER     | 0x21 | IRQ2     | 1     | HVS interrupt, undocumented
| IRQ_CCP2TX          | 0x22 | IRQ2     | 2     | CCP2TX interrupt, undocumented
| IRQ_SDC             | 0x23 | IRQ2     | 3     | SDC interrupt, undocumented
| IRQ_DSI0            | 0x24 | IRQ2     | 4     | DSI 0 (display) interrupt, undocumented
| IRQ_AVE             | 0x25 | IRQ2     | 5     | AVE interrupt, undocumented
| IRQ_CAM0            | 0x26 | IRQ2     | 6     | CSI 0 (camera) interrupt, undocumented
| IRQ_CAM1            | 0x27 | IRQ2     | 7     | CSI 1 (camera) interrupt, undocumented
| IRQ_HDMI0           | 0x28 | IRQ2     | 8     | HDMI 0 interrupt, undocumented
| IRQ_HDMI1           | 0x29 | IRQ2     | 9     | HDMI 1 interrupt, undocumented
| IRQ_PIXELVALVE2     | 0x2A | IRQ2     | 10    | GPU pixel valve 2 interrupt, undocumented
| IRQ_I2CSPISLV       | 0x2B | IRQ2     | 11    | I2C / SPI slave interrupt, undocumented
| IRQ_DSI1            | 0x2C | IRQ2     | 12    | DSI 1 (display) interrupt, undocumented
| IRQ_PIXELVALVE0     | 0x2D | IRQ2     | 13    | GPU pixel valve 0 interrupt, undocumented
| IRQ_PIXELVALVE1     | 0x2E | IRQ2     | 14    | GPU pixel valve 1 interrupt, undocumented
| IRQ_CPR             | 0x2F | IRQ2     | 15    | CPR interrupt, undocumented
| IRQ_FIRMWARE        | 0x30 | IRQ2     | 16    | SMI (firmware) interrupt, undocumented
| IRQ_GPIO0           | 0x31 | IRQ2     | 17    | GPIO 0 interrupt, undocumented
| IRQ_GPIO1           | 0x32 | IRQ2     | 18    | GPIO 1 interrupt, undocumented
| IRQ_GPIO2           | 0x33 | IRQ2     | 19    | GPIO 2 interrupt, undocumented
| IRQ_GPIO3           | 0x34 | IRQ2     | 20    | GPIO 3 interrupt, undocumented
| IRQ_I2C             | 0x35 | IRQ2     | 21    | I2C interrupt, undocumented
| IRQ_SPI             | 0x36 | IRQ2     | 22    | SPI interrupt, undocumented
| IRQ_I2SPCM          | 0x37 | IRQ2     | 23    | I2S interrupt, undocumented
| IRQ_SDHOST          | 0x38 | IRQ2     | 24    | SD host interrupt, undocumented
| IRQ_UART            | 0x39 | IRQ2     | 25    | PL011 UART interrupt (UART0)
| IRQ_SLIMBUS         | 0x3A | IRQ2     | 26    | SLIMBUS interrupt, undocumented
| IRQ_VEC             | 0x3B | IRQ2     | 27    | GPU? vector interrupt, undocumented
| IRQ_CPG             | 0x3C | IRQ2     | 28    | CPG interrupt, undocumented
| IRQ_RNG             | 0x3D | IRQ2     | 29    | RNG (random number generator) interrupt, undocumented
| IRQ_ARASANSDIO      | 0x3E | IRQ2     | 30    | EMMC interrupt, undocumented
| IRQ_AVSPMON         | 0x3F | IRQ2     | 31    | AVSPMON interrupt, undocumented
| IRQ_ARM_TIMER       | 0x40 | IRQBASIC | 0     | ARM timer interrupt, undocumented
| IRQ_ARM_MAILBOX     | 0x41 | IRQBASIC | 1     | ARM mailbox interrupt, undocumented
| IRQ_ARM_DOORBELL_0  | 0x42 | IRQBASIC | 2     | ARM doorbell (VCHIQ) 0 interrupt, undocumented
| IRQ_ARM_DOORBELL_1  | 0x43 | IRQBASIC | 3     | ARM doorbell (VCHIQ) 1 interrupt, undocumented
| IRQ_VPU0_HALTED     | 0x44 | IRQBASIC | 4     | VPU halted 0 interrupt, undocumented
| IRQ_VPU1_HALTED     | 0x45 | IRQBASIC | 5     | VPU halted 1 interrupt, undocumented
| IRQ_ILLEGAL_TYPE0   | 0x46 | IRQBASIC | 6     | Illegal type 0 interrupt, undocumented
| IRQ_ILLEGAL_TYPE1   | 0x47 | IRQBASIC | 7     | Illegal type 1 interrupt, undocumented
| IRQ_LOCAL_CNTPS     | 0x48 | ARMLOCAL | 0     | ARM Secure Physical Timer interrupt (CNTPS_ELn)
| IRQ_LOCAL_CNTPNS    | 0x49 | ARMLOCAL | 1     | ARM Non-secure Physical Timer interrupt (CNTPNS_ELn)
| IRQ_LOCAL_CNTHP     | 0x4A | ARMLOCAL | 2     | ARM Hypervisor Physical Timer interrupt (CNTHP_ELn)
| IRQ_LOCAL_CNTV      | 0x4B | ARMLOCAL | 3     | ARM Virtual Timer interrupt (CNTV_ELn)
| IRQ_LOCAL_MAILBOX0  | 0x4C | ARMLOCAL | 4     | Mailbox 0 interrupt, undocumented
| IRQ_LOCAL_MAILBOX1  | 0x4D | ARMLOCAL | 5     | Mailbox 1 interrupt, undocumented
| IRQ_LOCAL_MAILBOX2  | 0x4E | ARMLOCAL | 6     | Mailbox 2 interrupt, undocumented
| IRQ_LOCAL_MAILBOX3  | 0x4F | ARMLOCAL | 7     | Mailbox 3 interrupt, undocumented
| IRQ_LOCAL_GPU       | 0x50 | ARMLOCAL | 8     | GPU interrupt, undocumented
| IRQ_LOCAL_PMU       | 0x51 | ARMLOCAL | 9     | PMU (performance monitoring unit) interrupt
| IRQ_LOCAL_AXI_IDLE  | 0x52 | ARMLOCAL | 10    | AXI bus idle (core 0 only) interrupt, undocumented
| IRQ_LOCAL_LOCALTIMER| 0x53 | ARMLOCAL | 11    | ARM local timer interrupt, undocumented

IRQ are enabled and disabled by writing a 1 bit to the corresponnding bit in the corresponding register, e.g. for `IRQ_ARM_TIMER`:
- Enabled by writing 1 to bit 0 of `RPI_INTRCTRL_ENABLE_BASIC_IRQS`
- disabled by writing 1 to bit 0 of `RPI_INTRCTRL_DISABLE_BASIC_IRQS`
- Interrupt pending is checked by reading bit 0 of `RPI_INTRCTRL_IRQ_BASIC_PENDING`

For the interrupts connected to ARM local, this is different, e.g. `IRQ_LOCAL_CNTPNS`:
- Enabled by writing 1 to bit 0 of `ARM_LOCAL_TIMER_INT_CONTROL0`
- Disabled by writing 0 to bit 0 of `ARM_LOCAL_TIMER_INT_CONTROL0`
- Interrupt pending is checked by reading bit 0 of `ARM_LOCAL_IRQ_PENDING0`

Interrupts are prefixed in code with `RPI_INTRCTRL_`.

### Interrupt control registers

<table>
<caption id="Interrupt_control_registers">Interrupt control registers</caption>
<tr><th>Register          <th>Address        <th>Bits <th>Name                               <th>Acc<th>Meaning</tr>
<tr><td>IRQ_BASIC_PENDING <td>Base+0x0000B200<td>31:0 <td>Basic IRQ pending                  <td>R  <td>Group Basic IRQ pending</tr>
<tr><td>                  <td>               <td>31:21<td>Unused                             <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>20   <td>GPU IRQ 62                         <td>R  <td>GPU IRQ 62 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>19   <td>GPU IRQ 57                         <td>R  <td>GPU IRQ 57 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>18   <td>GPU IRQ 56                         <td>R  <td>GPU IRQ 56 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>17   <td>GPU IRQ 55                         <td>R  <td>GPU IRQ 55 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>16   <td>GPU IRQ 54                         <td>R  <td>GPU IRQ 54 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>15   <td>GPU IRQ 53                         <td>R  <td>GPU IRQ 53 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>14   <td>GPU IRQ 19                         <td>R  <td>GPU IRQ 19 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>13   <td>GPU IRQ 18                         <td>R  <td>GPU IRQ 18 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>12   <td>GPU IRQ 10                         <td>R  <td>GPU IRQ 10 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>11   <td>GPU IRQ 9                          <td>R  <td>GPU IRQ 9 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>10   <td>GPU IRQ 7                          <td>R  <td>GPU IRQ 7 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>9    <td>IRQ pending register 2             <td>R  <td>Group IRQ2 pending<br>
1 = one or more interrupts pending<br>
0 = no interrupts pending<br>
<tr><td>                  <td>               <td>8    <td>IRQ pending register 1             <td>R  <td>Group IRQ1 pending<br>
1 = one or more interrupts pending<br>
0 = no interrupts pending<br>
<tr><td>                  <td>               <td>7    <td>Illegal access type 0 IRQ pending  <td>R  <td>Illegal access type 0 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>6    <td>Illegal access type 1 IRQ pending  <td>R  <td>Illegal access type 1 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>5    <td>GPU1 halted IRQ pending            <td>R  <td>GPU 1 halted interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>4    <td>GPU0 halted IRQ pending            <td>R  <td>GPU 0 halted interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>3    <td>ARM Doorbell 1 IRQ pending         <td>R  <td>ARM Doorbell 1 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>2    <td>ARM Doorbell 0 IRQ pending         <td>R  <td>ARM Doorbell 0 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>1    <td>ARM Mailbox IRQ pending            <td>R  <td>ARM Mailbox interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>0    <td>ARM Timer IRQ pending              <td>R  <td>ARM Timer interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>IRQ_PENDING_1     <td>Base+0x0000B204<td>31:0 <td>GPU IRQ 0..31 pending (group IRQ1) <td>R  <td>Group IRQ1 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>IRQ_PENDING_2     <td>Base+0x0000B208<td>31:0 <td>GPU IRQ 32..63 pending (group IRQ2)<td>R  <td>Group IRQ2 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>FIQ_CONTROL       <td>Base+0x0000B20C<td>31:0 <td>FIQ control                        <td>R/W<td>FIQ control</tr>
<tr><td>                  <td>               <td>31:8 <td>Unused                             <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>7    <td>FIQ enable                         <td>R/W<td>Enable FIQ. Only 1 FIQ can be enabled at any time<br>
1 = FIQ enabled<br>
0 = FIQ disabled)</tr>
<tr><td>                  <td>               <td>6:0  <td>Select FIQ source                  <td>R/W<td>Select FIQ source<br>
0-83: See IRQ table<br>
84-127: Do Not Use</tr>
<tr><td>ENABLE_IRQS_1     <td>Base+0x0000B210<td>31:0 <td>GPU IRQ 31:0 enable (group IRQ1)   <td>R/W<td>Group IRQ1 enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>ENABLE_IRQS_2     <td>Base+0x0000B214<td>31:0 <td>GPU IRQ 63:32 enable (group IRQ2)  <td>R/W<td>Group IRQ2 enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>ENABLE_BASIC_IRQS <td>Base+0x0000B218<td>31:0 <td>Basic IRQ enable                   <td>-  <td>Group Basic IRQ enable</tr>
<tr><td>                  <td>               <td>31:8 <td>Unused                             <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>7    <td>Illegal access type 0 IRQ enable   <td>R/W<td>Illegal access type 0 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>6    <td>Illegal access type 1 IRQ enable <td>R/W<td>Illegal access type 1 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>5    <td>GPU 1 Halted IRQ enable          <td>R/W<td>GPU 1 Halted IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>4    <td>GPU 0 Halted IRQ enable          <td>R/W<td>GPU 0 Halted IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>3    <td>ARM Doorbell 1 IRQ enable        <td>R/W<td>ARM Doorbell 1 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>2    <td>ARM Doorbell 0 IRQ enable        <td>R/W<td>ARM Doorbell 0 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>1    <td>ARM Mailbox IRQ enable            <td>R/W<td>ARM Mailbox IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>0    <td>ARM Timer IRQ enable              <td>R/W<td>ARM Timer IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>DISABLE_IRQS_1    <td>Base+0x0000B21C<td>31:0 <td>GPU IRQ 31:0 disable (group IRQ1) <td>R/W<td>Group IRQ1 disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>DISABLE_IRQS_2    <td>Base+0x0000B220<td>31:0 <td>GPU IRQ 63:32 disable (group IRQ2)<td>R/W<td>Group IRQ2 disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>DISABLE_BASIC_IRQS<td>Base+0x0000B224<td>31:0 <td>Basic IRQ disable                 <td>-  <td>Group Basic IRQ disable</tr>
<tr><td>                  <td>               <td>31:8 <td>Unused                            <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>7    <td>Illegal access type 0 IRQ disable <td>R/W<td>Illegal access type 0 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>6    <td>Illegal access type 1 IRQ disable <td>R/W<td>Illegal access type 1 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>5    <td>GPU 1 Halted IRQ disable          <td>R/W<td>GPU 1 Halted IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>4    <td>GPU 0 Halted IRQ disable          <td>R/W<td>GPU 0 Halted IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>3    <td>ARM Doorbell 1 IRQ disable        <td>R/W<td>ARM Doorbell 1 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>2    <td>ARM Doorbell 0 IRQ disable        <td>R/W<td>ARM Doorbell 0 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>1    <td>ARM Mailbox IRQ disable           <td>R/W<td>ARM Mailbox IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>0    <td>ARM Timer IRQ disable             <td>R/W<td>ARM Timer IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
</table>

## Raspberry Pi 4/5

See [documentation](pdf/bcm2711-peripherals.pdf), section 6.

Raspberry Pi 4 and 5 use the GIC400 standard interrupt controller integrated into the SoC.
This is a building block well documented by ARM.

The GIC supports three kinds of interrupts:
- 16 Software Generated Interrupts (SGI)
- 16 Private Peripheral Interrupts (PPI)
- 224 Shared Peripheral Interrupts (SPI)

This makes for a total of 256 IRQ, where the GIC can actually handle up to 1024 interrupt sources.

See the image below for the interrupt controller layout.

<img src="images/gic400-interrupt-sources.png"  alt="Memory map" width="600"/>

As can been seen in the image, the GIC400 has 5 sources of interrupt:
- ARM core specific interrupts, mapped to PPI interrupts
  - Physical secure timer interrupt
  - Physical non-secure timer interrupt
  - Physical hypervisor timer interrupt
  - Virtual timer interrupt
  - Performance Monitoring Unit (PMU) interrupt

- ARM local interrupts
  - 4 mailbox interrupts for each of the 4 cores (so 16 in total)
  - 2 related to the AXI bus (AXI bus idle and AXI bus error)
  - 1 local timer interrupt
- ARM peripheral interrupts
  - ARM timer interrupt
  - Mailbox interrupt (for the VC mailbox)
  - ARM doorbell 0 interrupt
  - ARM doorbell 1 interrupt
  - VPU halted 0 interrupt
  - VPU halted 1 interrupt
  - ARM address error interrupt
  - ARM AXI error interrupt
  - 8 software interrupts
- VC (VideoCore) peripheral interrupts
  - 62 interrupts from different peripherals
  - 2 interrupts injected from Ethernet PCIe device
- PCIe ethernet interrupts
  - 57 so called L2 interrupt
  - 1 secure interrupt

The image below explains the interrupt routing for legacy interrupts.

<img src="images/gic400-legacy-interrupts.png"  alt="Memory map" width="800"/>

The image below explains the interrupt routing for the GIC400.

<img src="images/gic400-interrupts.png"  alt="Memory map" width="400"/>

The image below explains the combining of peripheral interrups for the Raspberry Pi.

<img src="images/gic400-peripheral-interrupts.png"  alt="Memory map" width="600"/>

| IRQ name              | IRQ# | Type | Index | Source and type |
|-----------------------|------|------|-------|-----------------|
|                       | 0x00 | SGI  | 0     | Undocumented
|                       | 0x01 | SGI  | 1     | Undocumented
|                       | 0x02 | SGI  | 2     | Undocumented
|                       | 0x03 | SGI  | 3     | Undocumented
|                       | 0x04 | SGI  | 4     | Undocumented
|                       | 0x05 | SGI  | 5     | Undocumented
|                       | 0x06 | SGI  | 6     | Undocumented
|                       | 0x07 | SGI  | 7     | Undocumented
|                       | 0x08 | SGI  | 8     | Undocumented
|                       | 0x09 | SGI  | 9     | Undocumented
|                       | 0x0A | SGI  | 10    | Undocumented
|                       | 0x0B | SGI  | 11    | Undocumented
|                       | 0x0C | SGI  | 12    | Undocumented
|                       | 0x0D | SGI  | 13    | Undocumented
|                       | 0x0E | SGI  | 14    | Undocumented
|                       | 0x0F | SGI  | 15    | Undocumented
|                       | 0x10 | PPI  | 0     | Undocumented
|                       | 0x11 | PPI  | 1     | Undocumented
|                       | 0x12 | PPI  | 2     | Undocumented
|                       | 0x13 | PPI  | 3     | Undocumented
|                       | 0x14 | PPI  | 4     | Undocumented
|                       | 0x15 | PPI  | 5     | Undocumented
|                       | 0x16 | PPI  | 6     | Undocumented
|                       | 0x17 | PPI  | 7     | Undocumented
|                       | 0x18 | PPI  | 8     | Undocumented
|                       | 0x19 | PPI  | 9     | Undocumented
| IRQ_LOCAL_CNTHP       | 0x1A | PPI  | 10    | ARM Hypervisor Physical Timer interrupt (CNTHP_ELn)
| IRQ_LOCAL_CNTV        | 0x1B | PPI  | 11    | ARM Virtual Timer interrupt (CNTV_ELn)
|                       | 0x1C | PPI  | 12    | Undocumented
| IRQ_LOCAL_CNTPS       | 0x1D | PPI  | 13    | ARM Secure Physical Timer interrupt (CNTPS_ELn)
| IRQ_LOCAL_CNTPNS      | 0x1E | PPI  | 14    | ARM Non-secure Physical Timer interrupt (CNTPNS_ELn)
|                       | 0x1F | PPI  | 15    | Undocumented
| IRQ_LOCAL_MAILBOX_0_0 | 0x20 | SPI  | 0     | ARM core mailbox 0, core 0
| IRQ_LOCAL_MAILBOX_1_0 | 0x21 | SPI  | 1     | ARM core mailbox 1, core 0
| IRQ_LOCAL_MAILBOX_2_0 | 0x22 | SPI  | 2     | ARM core mailbox 2, core 0
| IRQ_LOCAL_MAILBOX_3_0 | 0x23 | SPI  | 3     | ARM core mailbox 3, core 0
| IRQ_LOCAL_MAILBOX_0_1 | 0x24 | SPI  | 4     | ARM core mailbox 0, core 1
| IRQ_LOCAL_MAILBOX_1_1 | 0x25 | SPI  | 5     | ARM core mailbox 1, core 1
| IRQ_LOCAL_MAILBOX_2_1 | 0x26 | SPI  | 6     | ARM core mailbox 2, core 1
| IRQ_LOCAL_MAILBOX_3_1 | 0x27 | SPI  | 7     | ARM core mailbox 3, core 1
| IRQ_LOCAL_MAILBOX_0_2 | 0x28 | SPI  | 8     | ARM core mailbox 0, core 2
| IRQ_LOCAL_MAILBOX_1_2 | 0x29 | SPI  | 9     | ARM core mailbox 1, core 2
| IRQ_LOCAL_MAILBOX_2_2 | 0x2A | SPI  | 10    | ARM core mailbox 2, core 2
| IRQ_LOCAL_MAILBOX_3_2 | 0x2B | SPI  | 11    | ARM core mailbox 3, core 2
| IRQ_LOCAL_MAILBOX_0_3 | 0x2C | SPI  | 12    | ARM core mailbox 0, core 3
| IRQ_LOCAL_MAILBOX_1_3 | 0x2D | SPI  | 13    | ARM core mailbox 1, core 3
| IRQ_LOCAL_MAILBOX_2_3 | 0x2E | SPI  | 14    | ARM core mailbox 2, core 3
| IRQ_LOCAL_MAILBOX_3_3 | 0x2F | SPI  | 15    | ARM core mailbox 3, core 3
| IRQ_LOCAL_PMU0        | 0x30 | PPI  | 16    | PMU (Performance Monitoring Unit) core 0 interrupt
| IRQ_LOCAL_PMU1        | 0x31 | PPI  | 17    | PMU (Performance Monitoring Unit) core 1 interrupt
| IRQ_LOCAL_PMU2        | 0x32 | PPI  | 18    | PMU (Performance Monitoring Unit) core 2 interrupt
| IRQ_LOCAL_PMU3        | 0x33 | PPI  | 19    | PMU (Performance Monitoring Unit) core 3 interrupt
| IRQ_LOCAL_AXI_ERR     | 0x34 | SPI  | 20    | AXI bus error, undocumented
|                       | 0x35 | SPI  | 21    | Undocumented
|                       | 0x36 | SPI  | 22    | Undocumented
|                       | 0x37 | SPI  | 23    | Undocumented
|                       | 0x38 | SPI  | 24    | Undocumented
|                       | 0x39 | SPI  | 25    | Undocumented
|                       | 0x3A | SPI  | 26    | Undocumented
|                       | 0x3B | SPI  | 27    | Undocumented
|                       | 0x3C | SPI  | 28    | Undocumented
|                       | 0x3D | SPI  | 29    | Undocumented
|                       | 0x3E | SPI  | 30    | Undocumented
|                       | 0x3F | SPI  | 31    | Undocumented
| IRQ_ARM_TIMER         | 0x40 | SPI  | 32    | ARM timer interrupt, undocumented
| IRQ_ARM_MAILBOX       | 0x41 | SPI  | 33    | ARM Mailbox interrupt, undocumented
| IRQ_ARM_DOORBELL_0    | 0x42 | SPI  | 34    | ARM doorbell (VCHIQ) 0 interrupt, undocumented
| IRQ_ARM_DOORBELL_1    | 0x43 | SPI  | 35    | ARM doorbell (VCHIQ) 1 interrupt, undocumented
| IRQ_VPU0_HALTED       | 0x44 | SPI  | 36    | VPU halted 0 interrupt, undocumented
| IRQ_VPU1_HALTED       | 0x45 | SPI  | 37    | VPU halted 1 interrupt, undocumented
| IRQ_ILLEGAL_TYPE0     | 0x46 | SPI  | 38    | ARM address error interrupt, undocumented
| IRQ_ILLEGAL_TYPE1     | 0x47 | SPI  | 39    | ARM AXI error interrupt, undocumented
| IRQ_HOSTPORT          | 0x48 | SPI  | 40    | USB Host port interrupt, undocumented
|                       | 0x49 | SPI  | 41    | Undocumented
|                       | 0x4A | SPI  | 42    | Undocumented
|                       | 0x4B | SPI  | 43    | Undocumented
|                       | 0x4C | SPI  | 44    | Undocumented
|                       | 0x4D | SPI  | 45    | Undocumented
|                       | 0x4E | SPI  | 46    | Undocumented
|                       | 0x4F | SPI  | 47    | Undocumented
|                       | 0x50 | SPI  | 48    | Undocumented
|                       | 0x51 | SPI  | 49    | Undocumented
|                       | 0x52 | SPI  | 50    | Undocumented
|                       | 0x53 | SPI  | 51    | Undocumented
|                       | 0x54 | SPI  | 52    | Undocumented
|                       | 0x55 | SPI  | 53    | Undocumented
|                       | 0x56 | SPI  | 54    | Undocumented
|                       | 0x57 | SPI  | 55    | Undocumented
|                       | 0x58 | SPI  | 56    | Undocumented
|                       | 0x59 | SPI  | 57    | Undocumented
|                       | 0x5A | SPI  | 58    | Undocumented
|                       | 0x5B | SPI  | 59    | Undocumented
|                       | 0x5C | SPI  | 60    | Undocumented
|                       | 0x5D | SPI  | 61    | Undocumented
|                       | 0x5E | SPI  | 62    | Undocumented
|                       | 0x5F | SPI  | 63    | Undocumented
| IRQ_TIMER0            | 0x60 | SPI  | 64    | BCM system timer 0 interrupt, when compare value is hit
| IRQ_TIMER1            | 0x61 | SPI  | 65    | BCM system timer 1 interrupt, when compare value is hit
| IRQ_TIMER2            | 0x62 | SPI  | 66    | BCM system timer 2 interrupt, when compare value is hit
| IRQ_TIMER3            | 0x63 | SPI  | 67    | BCM system timer 3 interrupt, when compare value is hit
| IRQ_CODEC0            | 0x64 | SPI  | 68    | H.264 codec 0 interrupt, undocumented
| IRQ_CODEC1            | 0x65 | SPI  | 69    | H.264 codec 1 interrupt, undocumented
| IRQ_CODEC2            | 0x66 | SPI  | 70    | H.264 codec 2 interrupt, undocumented
| IRQ_JPEG              | 0x67 | SPI  | 71    | JPEG interrupt, undocumented
| IRQ_ISP               | 0x68 | SPI  | 72    | ISP interrupt, undocumented
| IRQ_USB               | 0x69 | SPI  | 73    | USB interrupt, undocumented
| IRQ_3D                | 0x6A | SPI  | 74    | VideoCore 3D interrupt, undocumented
| IRQ_TRANSPOSER        | 0x6B | SPI  | 75    | GPU transposer interrupt, undocumented
| IRQ_MULTICORESYNC0    | 0x6C | SPI  | 76    | Multicore sync 0 interrupt, undocumented
| IRQ_MULTICORESYNC1    | 0x6D | SPI  | 77    | Multicore sync 1 interrupt, undocumented
| IRQ_MULTICORESYNC2    | 0x6E | SPI  | 78    | Multicore sync 2 interrupt, undocumented
| IRQ_MULTICORESYNC3    | 0x6F | SPI  | 79    | Multicore sync 3 interrupt, undocumented
| IRQ_DMA0              | 0x70 | SPI  | 80    | DMA channel 0 interrupt, undocumented
| IRQ_DMA1              | 0x71 | SPI  | 81    | DMA channel 1 interrupt, PWM1
| IRQ_DMA2              | 0x72 | SPI  | 82    | DMA channel 2 interrupt, I2S PCM TX
| IRQ_DMA3              | 0x73 | SPI  | 83    | DMA channel 3 interrupt, I2S PCM RX
| IRQ_DMA4              | 0x74 | SPI  | 84    | DMA channel 4 interrupt, SMI
| IRQ_DMA5              | 0x75 | SPI  | 85    | DMA channel 5 interrupt, PWM
| IRQ_DMA6              | 0x76 | SPI  | 86    | DMA channel 6 interrupt, SPI TX
| IRQ_DMA7/8            | 0x77 | SPI  | 87    | DMA channel 7/8 interrupt, SPI RX /undocumented
| IRQ_DMA9/10           | 0x78 | SPI  | 88    | DMA channel 9/10 interrupt, undocumented / HDMI
| IRQ_DMA11             | 0x79 | SPI  | 89    | DMA channel 11 interrupt, EMMC
| IRQ_DMA12             | 0x7A | SPI  | 90    | DMA channel 12 interrupt, UART TX
| IRQ_DMA13             | 0x7B | SPI  | 91    | DMA channel 13 interrupt, undocumented
| IRQ_DMA14             | 0x7C | SPI  | 92    | DMA channel 14 interrupt, UART RX
| IRQ_AUX               | 0x7D | SPI  | 93    | AUX UART 1 / SPI 0/1 interrupt
| IRQ_ARM               | 0x7E | SPI  | 94    | ARM interrupt, undocumented
| IRQ_DMA15             | 0x7F | SPI  | 95    | DMA channel 15 interrupt, undocumented
| IRQ_HDMI_CEC          | 0x80 | SPI  | 96    | HMDI CEC interrupt, undocumented
| IRQ_VIDEOSCALER       | 0x81 | SPI  | 97    | HVS (video scaler) interrupt, undocumented
| IRQ_DECODER           | 0x82 | SPI  | 98    | Video decoder interrupt, undocumented
| IRQ_SDC               | 0x83 | SPI  | 99    | SDC interrupt, undocumented
| IRQ_DSI0              | 0x84 | SPI  | 100   | DSI 0 (display) interrupt, undocumented
| IRQ_PIXELVALVE2       | 0x85 | SPI  | 101   | GPU pixel valve 2 interrupt, undocumented
| IRQ_CAM0              | 0x86 | SPI  | 102   | CSI 0 (camera) interrupt, undocumented
| IRQ_CAM1              | 0x87 | SPI  | 103   | CSI 1 (camera) interrupt, undocumented
| IRQ_HDMI0             | 0x88 | SPI  | 104   | HDMI 0 interrupt, undocumented
| IRQ_HDMI1             | 0x89 | SPI  | 105   | HDMI 1 interrupt, undocumented
| IRQ_PIXELVALVE3       | 0x8A | SPI  | 106   | GPU pixel valve 3 interrupt, undocumented
| IRQ_SPI_BSC           | 0x8B | SPI  | 107   | SPI BSC slave interrupt, undocumented
| IRQ_DSI1              | 0x8C | SPI  | 108   | DSI 1 (display) interrupt, undocumented
| IRQ_PIXELVALVE0       | 0x8D | SPI  | 109   | GPU pixel valve 0 interrupt, undocumented
| IRQ_PIXELVALVE1/4     | 0x8E | SPI  | 110   | GPU pixel valve 1/4 interrupt, undocumented
| IRQ_CPR               | 0x8F | SPI  | 111   | CPR interrupt, undocumented
| IRQ_FIRMWARE          | 0x90 | SPI  | 112   | SMI (firmware) interrupt, undocumented
| IRQ_GPIO0             | 0x91 | SPI  | 113   | GPIO 0 interrupt, undocumented
| IRQ_GPIO1             | 0x92 | SPI  | 114   | GPIO 1 interrupt, undocumented
| IRQ_GPIO2             | 0x93 | SPI  | 115   | GPIO 2 interrupt, undocumented
| IRQ_GPIO3             | 0x94 | SPI  | 116   | GPIO 3 interrupt, undocumented
| IRQ_I2C               | 0x95 | SPI  | 117   | I2C interrupt (logical OR of all I2C bus interrupts)
| IRQ_SPI               | 0x96 | SPI  | 118   | SPI interrupt (logical OR of all SPI bus interrupts)
| IRQ_I2SPCM            | 0x97 | SPI  | 119   | I2S interrupt, undocumented
| IRQ_SDHOST            | 0x98 | SPI  | 120   | SD host interrupt, undocumented
| IRQ_UART              | 0x99 | SPI  | 121   | PL011 UART interrupt (logical OR of all SPI bus interrupts)
| IRQ_SLIMBUS           | 0x9A | SPI  | 122   | SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?), undocumented
| IRQ_VEC               | 0x9B | SPI  | 123   | GPU? VEC interrupt, undocumented
| IRQ_CPG               | 0x9C | SPI  | 124   | CPG interrupt, undocumented
| IRQ_RNG               | 0x9D | SPI  | 125   | RNG (random number generator) interrupt, undocumented
| IRQ_ARASANSDIO        | 0x9E | SPI  | 126   | EMMC / EMMC2 interrupt, undocumented
| IRQ_ETH_PCIE_S        | 0x9F | SPI  | 127   | Ethernet PCIe secure interrupt, undocumented
|                       | 0xA0 | SPI  | 128   | Undocumented
|                       | 0xA1 | SPI  | 129   | Undocumented
|                       | 0xA2 | SPI  | 130   | Undocumented
|                       | 0xA3 | SPI  | 131   | Undocumented
|                       | 0xA4 | SPI  | 132   | Undocumented
|                       | 0xA5 | SPI  | 133   | Undocumented
|                       | 0xA6 | SPI  | 134   | Undocumented
|                       | 0xA7 | SPI  | 135   | Undocumented
|                       | 0xA8 | SPI  | 136   | Undocumented
| IRQ_AVS               | 0xA9 | SPI  | 137   | PCI Express AVS interrupt, undocumented
|                       | 0xAA | SPI  | 138   | Undocumented
|                       | 0xAB | SPI  | 139   | Undocumented
|                       | 0xAC | SPI  | 140   | Undocumented
|                       | 0xAD | SPI  | 141   | Undocumented
|                       | 0xAE | SPI  | 142   | Undocumented
| IRQ_PCIE_INTA         | 0xAF | SPI  | 143   | PCI Express Ethernet A interrupt, undocumented
| IRQ_PCIE_INTB         | 0xB0 | SPI  | 144   | PCI Express Ethernet B interrupt, undocumented
| IRQ_PCIE_INTC         | 0xB1 | SPI  | 145   | PCI Express Ethernet C interrupt, undocumented
| IRQ_PCIE_INTD         | 0xB2 | SPI  | 146   | PCI Express Ethernet D interrupt, undocumented
| IRQ_PCIE_HOST_INTA    | 0xB3 | SPI  | 147   | PCI Express Host A interrupt, undocumented
| IRQ_PCIE_HOST_MSI     | 0xB4 | SPI  | 148   | PCI Express Host MSI interrupt, undocumented
|                       | 0xB5 | SPI  | 149   | Undocumented
|                       | 0xB6 | SPI  | 150   | Undocumented
|                       | 0xB7 | SPI  | 151   | Undocumented
|                       | 0xB8 | SPI  | 152   | Undocumented
|                       | 0xB9 | SPI  | 153   | Undocumented
|                       | 0xBA | SPI  | 154   | Undocumented
|                       | 0xBB | SPI  | 155   | Undocumented
|                       | 0xBC | SPI  | 156   | Undocumented
| IRQ_GENET_0_A         | 0xBD | SPI  | 157   | Ethernet interrupt, undocumented
| IRQ_GENET_0_B         | 0xBE | SPI  | 158   | Ethernet interrupt, undocumented
|                       | 0xBF | SPI  | 159   | Undocumented
|                       | 0xC0 | SPI  | 160   | Undocumented
|                       | 0xC1 | SPI  | 161   | Undocumented
|                       | 0xC2 | SPI  | 162   | Undocumented
|                       | 0xC3 | SPI  | 163   | Undocumented
|                       | 0xC4 | SPI  | 164   | Undocumented
|                       | 0xC5 | SPI  | 165   | Undocumented
|                       | 0xC6 | SPI  | 166   | Undocumented
|                       | 0xC7 | SPI  | 167   | Undocumented
|                       | 0xC8 | SPI  | 168   | Undocumented
|                       | 0xC9 | SPI  | 169   | Undocumented
|                       | 0xCA | SPI  | 170   | Undocumented
|                       | 0xCB | SPI  | 171   | Undocumented
|                       | 0xCC | SPI  | 172   | Undocumented
|                       | 0xCD | SPI  | 173   | Undocumented
|                       | 0xCE | SPI  | 174   | Undocumented
|                       | 0xCF | SPI  | 175   | Undocumented
| IRQ_XHCI_INTERNAL     | 0xD0 | SPI  | 176   | USB XHCI interrupt, undocumented
|                       | 0xD1 | SPI  | 177   | Undocumented
|                       | 0xD2 | SPI  | 178   | Undocumented
|                       | 0xD3 | SPI  | 179   | Undocumented
|                       | 0xD4 | SPI  | 180   | Undocumented
|                       | 0xD5 | SPI  | 181   | Undocumented
|                       | 0xD6 | SPI  | 182   | Undocumented
|                       | 0xD7 | SPI  | 183   | Undocumented
|                       | 0xD8 | SPI  | 184   | Undocumented
|                       | 0xD9 | SPI  | 185   | Undocumented
|                       | 0xDA | SPI  | 186   | Undocumented
|                       | 0xDB | SPI  | 187   | Undocumented
|                       | 0xDC | SPI  | 188   | Undocumented
|                       | 0xDD | SPI  | 189   | Undocumented
|                       | 0xDE | SPI  | 190   | Undocumented
|                       | 0xDF | SPI  | 191   | Undocumented
|                       | 0xE0 | SPI  | 192   | Undocumented
|                       | 0xE1 | SPI  | 193   | Undocumented
|                       | 0xE2 | SPI  | 194   | Undocumented
|                       | 0xE3 | SPI  | 195   | Undocumented
|                       | 0xE4 | SPI  | 196   | Undocumented
|                       | 0xE5 | SPI  | 197   | Undocumented
|                       | 0xE6 | SPI  | 198   | Undocumented
|                       | 0xE7 | SPI  | 199   | Undocumented
|                       | 0xE8 | SPI  | 200   | Undocumented
|                       | 0xE9 | SPI  | 201   | Undocumented
|                       | 0xEA | SPI  | 202   | Undocumented
|                       | 0xEB | SPI  | 203   | Undocumented
|                       | 0xEC | SPI  | 204   | Undocumented
|                       | 0xED | SPI  | 205   | Undocumented
|                       | 0xEE | SPI  | 206   | Undocumented
|                       | 0xEF | SPI  | 207   | Undocumented
|                       | 0xF0 | SPI  | 208   | Undocumented
|                       | 0xF1 | SPI  | 209   | Undocumented
|                       | 0xF2 | SPI  | 210   | Undocumented
|                       | 0xF3 | SPI  | 211   | Undocumented
|                       | 0xF4 | SPI  | 212   | Undocumented
|                       | 0xF5 | SPI  | 213   | Undocumented
|                       | 0xF6 | SPI  | 214   | Undocumented
|                       | 0xF7 | SPI  | 215   | Undocumented
|                       | 0xF8 | SPI  | 216   | Undocumented
|                       | 0xF9 | SPI  | 217   | Undocumented
|                       | 0xFA | SPI  | 218   | Undocumented
|                       | 0xFB | SPI  | 219   | Undocumented
|                       | 0xFC | SPI  | 220   | Undocumented
|                       | 0xFD | SPI  | 221   | Undocumented
|                       | 0xFE | SPI  | 222   | Undocumented
|                       | 0xFF | SPI  | 223   | Undocumented

Enabling / disabling interrupts on the GIC is a little more complicated, but the same for all interrupts, e.g. for `IRQ_LOCAL_CNTPNS`:

- Enabling the interrupt by writing a 1 to bit 14 (0x0E) + 16 (offset for PPI) = bit 30 in `RPI_GICD_ISENABLER0`
- Disabling the interrupt by writing a 1 to bit 14 (0x0E) + 16 (offset for PPI) = bit 30 in `RPI_GICD_ICENABLER0`
- Determining whether the interrupt is pending by reading bit 14 (0x0E) + 16 (offset for PPI) = bit 30 in `RPI_GICD_ISPEND0`

The GIC-400 interrupt controller is an external interrupt controller to the ARM SoC.
Its registers are memory mapped into the ARM address space.

For RPI4 the base address is 0xff840000, for RPI 5 this is 0x107FFF8000.

<table>
<caption id="Interrupt_control_gic_address_mapping_">GIC-400 address mapping</caption>
<tr><th>Address range<th>GIC-400 functional block</tr>
<tr><td>Base + 0x0000-0x0FFF<td>Reserved</tr>
<tr><td>Base + 0x1000-0x1FFF<td>Distributor</tr>
<tr><td>Base + 0x2000-0x3FFF<td>CPU interfaces</tr>
<tr><td>Base + 0x4000-0x4FFF<td>Virtual interface control block, for the processor that is performing the access</tr>
<tr><td>Base + 0x5000-0x5FFF<td>Virtual interface control block, for the processor selected by address bits [11:9]:</tr>
<tr><td>Base + 0x5000-0x51FF<td>Alias for Processor 0</tr>
<tr><td>Base + 0x5200-0x53FF<td>Alias for Processor 1</tr>
<tr><td>                    <td>...</tr>
<tr><td>Base + 0x5E00-0x5FFF<td>Alias for Processor 7</tr>
<tr><td>Base + 0x6000-0x7FFF<td>Virtual CPU interfaces</tr>
</table>

All of the GIC-400 registers have short names.
In these names, the first three characters are GIC, and the fourth character indicates the functional block of the GIC-400:

- GICD_ Distributor
- GICC_ CPU interfaces
- GICH_ Virtual interface control blocks
- GICV_ Virtual CPU interfaces

<table>
<caption id="Interrupt_control_registers_gic">Interrupt control registers (GIC-400)</caption>
<tr><th>Offset     <th>            Name            <th>Type<th>Reset                    <th>Full name</tr>
<tr><td>0x000      <td>            GICD_CTLR       <td>RW  <td>0x00000000               <td>Distributor Control Register</tr>
<tr><td>0x004      <td>            GICD_TYPER      <td>RO  <td>Configuration-dependent  <td>Interrupt Controller Type Register</tr>
<tr><td>0x008      <td>            GICD_IIDR       <td>RO  <td>0x0200143B               <td>Distributor Implementer Identification Register, GICD_IIDR on page 3-6</tr>
<tr><td>0x00C-0x01C<td>            -               <td>-   <td>-                        <td>Reserved</tr>
<tr><td>0x020-0x03C<td>            -               <td>-   <td>-                        <td>implementation defined registers</tr>
<tr><td>0x040-0x07C<td>            -               <td>-   <td>-                        <td>Reserved</tr>
<tr><td>0x080-0x0BC<td>            GICD_IGROUPRn   <td>RW  <td>0x00000000               <td>Interrupt Group Registers. This register is only accessible from a Secure access.</tr>
<tr><td>0x0C0-0x0FC<td>                            <td>    <td>                         <td></tr>
<tr><td>0x100      <td rowspan="2">GICD_ISENABLERn <td>RW  <td>SGIs and PPIs: 0x0000FFFF<td rowspan="2">Interrupt Set-Enable Registers. Writes to bits corresponding to the SGIs are ignored.</tr>
<tr><td>0x104-0x13C                                <td>    <td>SPIs: 0x00000000         </tr>
<tr><td>0x140-0x17C<td>                            <td>    <td>                         <td></tr>
<tr><td>0x180      <td rowspan="2">GICD_ICENABLERn <td>RW  <td>0x0000FFFF               rowspan="2">Interrupt Clear-Enable Registers</tr>
<tr><td>0x184-0x1BC                                <td>    <td>0x00000000               <td></tr>
<tr><td>0x1C0-0x1FC<td>                            <td>    <td>                         <td></tr>
<tr><td>0x200-0x23C<td>            GICD_ISPENDRn   <td>RW  <td>0x00000000               <td>Interrupt Set-Pending Registers</tr>
<tr><td>0x240-0x27C<td>                            <td>    <td>                         <td></tr>
<tr><td>0x280-0x2BC<td>            GICD_ICPENDRn   <td>RW  <td>0x00000000               <td>Interrupt Clear-Pending Registers</tr>
<tr><td>0x2C0-0x2FC<td>                            <td>    <td>                         <td></tr>
<tr><td>0x300-0x33C<td>            GICD_ISACTIVERn <td>RW  <td>0x00000000               <td>Interrupt Set-Active Registers</tr>
<tr><td>0x340-0x37C<td>                            <td>    <td>                         <td></tr>
<tr><td>0x380-0x3BC<td>            GICD_ICACTIVERn <td>RW  <td>0x00000000               <td>Interrupt Clear-Active Registers</tr>
<tr><td>0x3C0-0x3FC<td>                            <td>    <td>                         <td></tr>
<tr><td>0x400-0x5FC<td>            GICD_IPRIORITYRn<td>RW  <td>0x00000000               <td>Interrupt Priority Registers</tr>
<tr><td>0x600-0x7FC<td>                            <td>    <td>                         <td></tr>
<tr><td>0x800-0x81C<td rowspan="2">GICD_ITARGETSRn <td>RO  <td>-                        <td rowspan="2">Interrupt Processor Targets Registers.<br>
The registers that contain the SGI and PPI interrupts are read-only and the value is the CPU number of the current access.<br>
It is encoded in an 8-bit one-hot field, for each implemented interrupt, and zero for interrupts that are not implemented.<br>
For more information on CPU targets field bit values, see the ARM Generic Interrupt Controller Architecture Specification.</tr>
<tr><td>0x820-0x9FC                                <td>RW  <td>0x00000000               </tr>
<tr><td>0xA00-0xBFC<td>                            <td>    <td>                         <td></tr>
<tr><td>0xC00      <td rowspan="3">GICD_ICFGRn     <td>RO  <td>SGIs: 0xAAAAAAAA         <td rowspan="3">Interrupt Configuration Registers, GICD_ICFGRn on page 3-6.<br>
The even bits of the SPIs register are RO, see Interrupt Configuration Registers, GICD_ICFGRn on page 3-6.</tr>
<tr><td>0xC04                                      <td>RO  <td>PPIs: 0x55540000         </tr>
<tr><td>0xC08-0xC7C                                <td>RW  <td>SPIs: 0x55555555         </tr>
<tr><td>0xC80-0xCFC<td>                            <td>    <td>                         <td></tr>
<tr><td>0xD00      <td>GICD_PPISR                  <td>RO  <td>0x00000000               <td>Private Peripheral Interrupt Status Register, GICD_PPISR on page 3-7</tr>
<tr><td>0xD04-0xD3C<td>GICD_SPISRn                 <td>RO  <td>0x00000000               <td>Shared Peripheral Interrupt Status Registers, GICD_SPISRn on page 3-7</tr>
<tr><td>0xD40-0xEFC<td>                            <td>    <td>                         <td></tr>
<tr><td>0xF00      <td>GICD_SGIR                   <td>WO  <td>-                        <td>Software Generated Interrupt Register</tr>
<tr><td>0xF10-0xF1C<td>GICD_CPENDSGIRn             <td>RW  <td>0x00000000               <td>SGI Clear-Pending Registers</tr>
<tr><td>0xF20-0xF2C<td>GICD_SPENDSGIRn             <td>RW  <td>0x00000000               <td>SGI Set-Pending Registers</tr>
<tr><td>0xF30-0xFCC<td>                            <td>    <td>                         <td></tr>
<tr><td>0xFD0      <td>GICD_PIDR4                  <td>RO  <td>0x00000004               <td>Peripheral ID 4 Register</tr>
<tr><td>0xFD4      <td>GICD_PIDR5                  <td>RO  <td>0x00000000               <td>Peripheral ID 5 Register</tr>
<tr><td>0xFD8      <td>GICD_PIDR6                  <td>RO  <td>0x00000000               <td>Peripheral ID 6 Register</tr>
<tr><td>0xFDC      <td>GICD_PIDR7                  <td>RO  <td>0x00000000               <td>Peripheral ID 7 Register</tr>
<tr><td>0xFE0      <td>GICD_PIDR0                  <td>RO  <td>0x00000000               <td>Peripheral ID 0 Register</tr>
<tr><td>0xFE4      <td>GICD_PIDR1                  <td>RO  <td>0x00000000               <td>Peripheral ID 1 Register</tr>
<tr><td>0xFE8      <td>GICD_PIDR2                  <td>RO  <td>0x00000000               <td>Peripheral ID 2 Register</tr>
<tr><td>0xFEC      <td>GICD_PIDR3                  <td>RO  <td>0x00000000               <td>Peripheral ID 3 Register</tr>
<tr><td>0xFF0      <td>GICD_CIDR0                  <td>RO  <td>0x00000000               <td>Component ID 0 Register</tr>
<tr><td>0xFF4      <td>GICD_CIDR1                  <td>RO  <td>0x00000000               <td>Component ID 1 Register</tr>
<tr><td>0xFF8      <td>GICD_CIDR2                  <td>RO  <td>0x00000000               <td>Component ID 2 Register</tr>
<tr><td>0xFFC      <td>GICD_CIDR3                  <td>RO  <td>0x00000000               <td>Component ID 3 Register</tr>
</table>

Where n corresponds to a CPU core number

@todo Add more detail to registers.
