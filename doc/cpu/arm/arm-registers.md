# ARM registers {#ARM_REGISTERS}

@tableofcontents

This page enumarates the definition of some of the registers inside the ARM processor.

## Register overview {#ARM_REGISTERS_REGISTER_OVERVIEW}

| Register name | Width | Acc | Reset value | Meaning |
|---------------|-------|-----|-------------|---------|
| MIDR_EL1      | 32    | R   | 0x410FD083  | Provides identification information for the processor, including an implementer code for the device and a device ID number.
| MPIDR_EL1     | 64    | R   | 0x80000003  | Provides an additional core identification mechanism for scheduling purposes in a cluster system. EDDEVAFF0 is a read-only copy of MPIDR_EL1[31:0] accessible from the external debug interface.
| L2CTLR_EL1    | 32    | R/W | ?           | L2 Control register
| DAIF          | 64    | R/W | 0x000003C0  | Interrupt Mask Bits
| DAIFClr       | 4     | R/W | 0x00000000  | Interrupt Mask Bits Set Register
| DAIFSet       | 4     | R/W | 0x00000000  | Interrupt Mask Bits Reset Register
| CNTFRQ_EL0    | 64    |
| CNTPCT_EL0    | 64    |
| CNTP_CTL_EL0  | 64    | R/W
| CNTP_CVAL_EL0 | 64    |

### Common registers {#ARM_REGISTERS_REGISTER_OVERVIEW_COMMON_REGISTERS}

| Register name | Bits  | Name              | Acc | Meaning |
|---------------|-------|-------------------|-----|---------|
| MIDR_EL1      | 31:24 | Implementer       |     | Implementer code (ARM = 0x41)
|               | 24:20 | Variant           |     | Major revision number (0x0 = Major revision number)
|               | 19:16 | Architecture      |     | Architecture code (0xF = Defined by CPUID scheme)
|               | 15::4 | Primary part      |     | Primary part number (0xD03 = Cortex A-53, 0xD08 = Cortex A-72)
|               | 3:0   | Revision          |     | Minor revision number (0x2 = r0p2)
| MPIDR_EL1     | 63:40 | -                 |     | Reserved
|               | 39:32 | Aff3              |     | Affinity level 3, highest affinity level field, reserved
|               | 31    | -                 |     | Reserved
|               | 30    | U                 |     | Indicates a single core system, as distinct from core 0 in a cluster (0 = part of cluster, 1 = single core)
|               | 29:25 | -                 |     | Reserved
|               | 24    | MT                |     | Indicates whether the lowest level of affinity consists of logical cores that are implemented using a multi-threading type approach ( 0 = performance of cores at lowest affinity level largely independent)
|               | 23:16 | Aff2              |     | Affinity level 2, CLUSTERIDAFF2 configuration signal value
|               | 15:8  | Aff1              |     | Affinity level 1, CLUSTERIDAFF1 configuration signal value
|               | 7:0   | Aff0              |     | Affinity level 0, core id (0, 1, 2, 3)
| L2CTLR_EL1    | 31:26 | -                 |     | Reserved
|               | 25:24 | number of cores   | R   | Number of cores (0 = 1 core, 1 = 2 cores, 2 = 3 cores, 3 = 4 cores)
|               | 23    | -                 |     | Reserved
|               | 22    | CPU cache prot    |     | CPU RAM protection (0 = without ECC, 1 = with ECC)
|               | 21    | SCU-L2 cache prot |     | L2 cache protection (0 = without ECC, 1 = with ECC)
|               | 20:6  | -                 |     | Reserved
|               | 5     | L2 input latency  |     | L2 data RAM input latency ( 0 = 1 cycle, 1 = 2 cycles)
|               | 4:1   | -                 |     | Reserved
|               | 0     | L2 output latency |     | L2 data RAM output latency ( 0 = 2 cycles, 1 = 3 cycles)

### DAIF register {#ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER}

<table>
<caption id="DAIF_bits">DAIF bits</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>9<td>D, Process state D mask     <td>0 = Watchpoint, Breakpoint, and Software Step exceptions targeted at the current Exception level are not masked.\n
                                             1 = Watchpoint, Breakpoint, and Software Step exceptions targeted at the current Exception level are masked.</tr>
<tr><td>8<td>A, SError exception mask bit<td>0 = Exception not masked.\n
                                             1 = Exception masked.</tr>
<tr><td>7<td>I, IRQ mask bit             <td>0 = IRQ not masked.\n
                                             1 = IRQ masked.</tr>
<tr><td>6<td>F, FIQ mask bit             <td>0 = FIQ not masked.\n
                                             1 = FIQ masked.</tr>
</table>

### CNTFRQ_EL0 register {#ARM_REGISTERS_REGISTER_OVERVIEW_CNTFRQ_EL0_REGISTER}

This register is provided so that software can discover the frequency of the system counter.
It must be programmed with this value as part of system initialization.
The value of the register is not interpreted by hardware.

<table>
<caption id="CNTFRQ_EL0 bits">System Control Register(EL1)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:32<td>Res0<td>Reserved, must be 0</tr>
<tr><td>31:0<td>Clock frequency<td>Clock frequency. Indicates the system counter clock frequency, in Hz.</tr>
</table>

### CNTPCT_EL0 register {#ARM_REGISTERS_REGISTER_OVERVIEW_CNTPCT_EL0_REGISTER}

Holds the 64-bit physical count value.

<table>
<caption id="CNTPCT_EL0 bits">System Control Register(EL1)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:0<td>Physical count value<td>Physical count value.</tr>
</table>

### CNTP_CTL_EL0 register {#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER}

Holds the 64-bit physical count value.

<table>
<caption id="CNTP_CTL_EL0 bits">Counter-timer Physical Timer Control Register (EL0)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:3<td>Res 0<td>Reserved, must be 0.</tr>
<tr><td>2<td>ISTATUS<td>The status of the timer. This bit indicates whether the timer condition is met:\n
                        0 = Timer condition is not met.\n
                        1 = Timer condition is met.</tr>
<tr><td>1<td>IMASK  <td>Timer interrupt mask bit. Permitted values are:\n
                        0 = Timer interrupt is not masked by the IMASK bit.\n
                        1 = Timer interrupt is masked by the IMASK bit.</tr>
<tr><td>0<td>ENABLE <td>Enables the timer. Permitted values are:\n
                        0 = Timer disabled.\n
                        1 = Timer enabled.</tr>
</table>

### CNTP_CVAL_EL0 register {#ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER}

Holds the compare value for the EL1 physical timer.

<table>
<caption id="CNTP_CVAL_EL0 bits">Counter-timer Physical Timer Control Register (EL0)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:0<td>CompareValue<td>Holds the EL1 physical timer CompareValue.\n
When CNTP_CTL_EL0.ENABLE is 1, the timer condition is met when(CNTPCT_EL0 - CompareValue) is greater than or equal to zero.This means that CompareValue acts like a 64 - bit upcounter timer.When the timer condition is met:\n
CNTP_CTL_EL0.ISTATUS is set to 1.\n
If CNTP_CTL_EL0.IMASK is 0, an interrupt is generated.\n
When CNTP_CTL_EL0.ENABLE is 0, the timer condition is not met, but CNTPCT_EL0 continues to count.</tr>
</table>
