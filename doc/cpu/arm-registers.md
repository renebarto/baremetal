# ARM registers {#ARM_REGISTERS}

@tableofcontents

See [documentation](pdf/arm-architecture-registers.pdf) and [reference manual](pdf/arm-aarch64-reference-manual.pdf).

\todo Selectively copy registers used in code.

This page enumerates the definition of some of the registers inside the ARM processor.
The list will never be complete, please refer to the documentation for that, here I just list very common registers, and registers needed to explain certain code.

## Register overview {#ARM_REGISTERS_REGISTER_OVERVIEW}

### Common registers {#ARM_REGISTERS_REGISTER_OVERVIEW_COMMON_REGISTERS}

<table>
<caption id="arm_register_overview">Register overview</caption>
<tr><th>Register name<th>Width<th>Access<th>Reset value<th>Name                                                    <th>Meaning</tr>
<!-- <tr><td>MIDR_EL1     <td>32   <td>R  <td>0x410FD083 <td>Main ID Register                                        <td>Provides identification information for the processor, including an implementer code for the device and a device ID number.</tr>
<tr><td>MPIDR_EL1    <td>64   <td>R  <td>0x80000003 <td>Multiprocessor Affinity Register                        <td>Provides an additional core identification mechanism for scheduling purposes in a cluster system. EDDEVAFF0 is a read-only copy of MPIDR_EL1[31:0] accessible from the external debug interface.</tr>
<tr><td>L2CTLR_EL1   <td>32   <td>R/W<td>?          <td>L2 Control register                                     <td>?</tr>
<tr><td>SP_EL0       <td>64   <td>R/W<td>?          <td>Stack Pointer (EL0)                                     <td>Holds the stack pointer associated with EL0. At higher Exception levels, this is used as the current stack pointer when the value of SPSel.SP is 0.</tr>
<tr><td>SP_EL1       <td>64   <td>R/W<td>?          <td>Stack Pointer (EL1)                                     <td>Holds the stack pointer associated with EL1. When executing at EL1, the value of SPSel.SP determines the current stack pointer:<br>
SPSel.SP   Current stack pointer<br>
0b0        SP_EL0<br>
0b1        SP_EL1
<tr><td>ELR_EL1      <td>64   <td>R/W<td>?          <td>Exception Link Register (EL1)                           <td>When taking an exception to EL1, holds the address to return to.</tr>
<tr><td>ESR_EL1      <td>64   <td>R  <td>?          <td>Exception Syndrome Register (EL1)                       <td>Holds syndrome information for an exception taken to EL1</tr>
<tr><td>SPSR_EL1     <td>64   <td>R/W<td>?          <td>Saved Program Status Register (EL1)                     <td>Holds the saved process state when an exception is taken to EL1.</tr>
<tr><td>FAR_EL1      <td>64   <td>R/W<td>?          <td>Fault Address Register (EL1)                            <td>Holds the faulting Virtual Address for all synchronous Instruction or Data Abort, PC alignment fault and Watchpoint exceptions that are taken to EL1.</tr>
<tr><td>VBAR_EL1     <td>64   <td>R/W<td>?          <td>Vector Base Address Register (EL1)                      <td>Holds the vector base address for the exception vector table for EL1. As this table needs to be aligned to 2048 bytes, the low 11 bits must be 0.</tr>
<tr><td>DAIF         <td>64   <td>R/W<td>0x000003C0 <td>Interrupt Mask Bits                                     <td>Allows access to the interrupt mask bits.</tr>
<tr><td>DAIFClr      <td>4    <td>R/W<td>0x00000000 <td>Interrupt Mask Bits Reset Register                      <td></tr>
<tr><td>DAIFSet      <td>4    <td>R/W<td>0x00000000 <td>Interrupt Mask Bits Set Register                        <td></tr>
<tr><td>CNTFRQ_EL0   <td>4    <td>R/W<td>0x00000000 <td>Counter-timer Frequency register (EL0)                  <td>This register is provided so that software can discover the frequency of the system counter. It must be programmed with this value as part of system initialization. The value of the register is not interpreted by hardware.</tr>
<tr><td>CNTPCT_EL0   <td>4    <td>R/W<td>0x00000000 <td>Counter-timer Physical Count register (EL0)             <td>Holds the 64-bit physical count value.</tr>
<tr><td>CNTP_CTL_EL0 <td>4    <td>R/W<td>0x00000000 <td>Counter-timer Physical Timer Control register (EL0)     <td>Control register for the EL0 physical timer.<br>Bit 2: If 1, compare value condition is met, if 0, it is not met (yet)<br>Bit 1: If 1, interrupt on timer condition is masked, if 0, the interrupt is not masked<br>Bit 0: If 1, the timer is enabled, if 0, it is disabled</tr>
<tr><td>CNTP_CVAL_EL0<td>4    <td>R/W<td>0x00000000 <td>Counter-timer Physical Timer CompareValue register (EL0)<td>Holds the compare value for the EL0 physical timer.</tr>
<tr><td>CTR_EL0      <td>4    <td>R/W<td>0x00000000 <td>Cache Type Register (EL0)                               <td>Provides information about the architecture of the caches.</tr> -->
</table>

<!--### ELR_EL1 register {#ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER}

When taking an exception to EL1, holds the address to return to.

<table>
<caption id="ELR_EL1 bits">Exception Link Register (EL1)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:0<td>Return address<td>Return address.</tr>
</table

### ESR_EL1 register {#ARM_REGISTERS_REGISTER_OVERVIEW_ESR_EL1_REGISTER}

Holds syndrome information for an exception taken to EL1.

<table>
<caption id="ESR_EL1 bits">Exception Syndrome Register (EL1)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:56<td>Res0<td>Reserved, must be 0.</tr>
<tr><td>55:32<td>ISS2<td>ISS2 encoding for an exception, the bit assignments are:<br>
<table>
<caption id="multi_row">ISS2 encoding for an exception from a Data Abort</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>23:11<td>Res0<td>Reserved, must be 0.</tr>
<tr><td>10<td>TnD<td>   Tag not Data.<br>
                        If a memory access generates a Data Abort for a stage 1 Permission fault, this field indicates whether the fault is due to an Allocation Tag access.<br>
                        0 = Permission fault is not due to a write of an Allocation Tag to Canonically Tagged memory.<br>
                        1 = Permission fault is due to a write of an Allocation Tag to Canonically Tagged memory.</tr>
<tr><td>9<td>TagAccess<td>NoTagAccess fault.<br>
                        When EL2 provides information to EL1 regarding a Stage 2 Data Abort, this field indicates whether the fault is due to the NoTagAccess memory attribute.<br>
                        0 = Permission fault is not due to the NoTagAccess memory attribute.<br>
                        1 = Permission fault is due to the NoTagAccess memory attribute.</tr>
<tr><td>8<td>GCS<td>Guarded Control Stack data access.<br>
                        If a memory access generates a Data Abort, this field indicates whether the fault is due to a Guarded Control Stack data access.<br>
                        0 = The Data Abort is not due to a Guarded control stack data access.<br>
                        1 = The Data Abort is due to a Guarded control stack data access.</tr>
<tr><td>7<td>AssuredOnly<td>AssuredOnly flag.<br>
                        If EL2 provides information regarding a stage 2 Data Abort to EL1, then this field holds information about the fault.<br>
                        0 = The Data Abort is not due to AssuredOnly.<br>
                        1 = The Data Abort is due to AssuredOnly.</tr>
<tr><td>6<td>Overlay<td>Overlay flag.<br>
                        If a memory access generates a Data Abort for a Permission fault, then this field holds information about the fault.<br>
                        0 = Data Abort is not due to Overlay Permissions.<br>
                        1 = Data Abort is due to Overlay Permissions.</tr>
<tr><td>5<td>DirtyBit<td>DirtyBit flag.<br>
                        If a write access to memory generates a Data Abort for a Permission fault using Indirect Permission, then this field holds information about the fault.<br>
                        0 = Permission Fault is not due to dirty state.<br>
                        1 = Permission Fault is due to dirty state.</tr>
<tr><td>4:0<td>Xs<td>When FEAT_LS64_V is implemented, if a memory access generated by an ST64BV instruction generates a Data Abort exception for a Translation fault, Access flag fault, or Permission fault, then this field holds register specifier, Xs.<br>
                        When FEAT_LS64_ACCDATA is implemented, if a memory access generated by an ST64BV0 instruction generates a Data Abort exception for a Translation fault, Access flag fault, or Permission fault, then this field holds register specifier, Xs.</tr>
</table><br>
<table>
<caption id="ISS2 encoding for an exception from an Instruction Abort">ISS2 encoding for an exception from an Instruction Abort</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>23:8<td>Res0<td>Reserved, must be 0.</tr>
<tr><td>7<td>AssuredOnly<td>AssuredOnly flag.<br>
                        If EL2 provides information regarding a stage 2 Instruction Abort to EL1, then this field holds information about the fault.<br>
                        0 = The Instruction Abort is not due to AssuredOnly.<br>
                        1 = The Instruction Abort is due to AssuredOnly.</tr>
<tr><td>6<td>Overlay<td>Overlay flag.<br>
                        If a memory access generates a Instruction Abort for a Permission fault, then this field holds information about the fault.<br>
                        0 = Instruction Abort is not due to Overlay Permissions.<br>
                        1 = Instruction Abort is due to Overlay Permissions.</tr>
<tr><td>5:0<td>Res0<td>Reserved, must be 0.</tr>
</table><br>
<table>
<caption id="ISS2 encoding for an exception from a Watchpoint exception">ISS2 encoding for an exception from a Watchpoint exception</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>23:9<td>Res0<td>Reserved, must be 0.</tr>
<tr><td>8<td>GCS<td>Guarded control stack data access.<br>
                        Indicates that the Watchpoint exception is due to a Guarded control stack data access.<br>
                        0 = The Watchpoint exception is not due to a Guarded control stack data access.<br>
                        1 = The Watchpoint exception is due to a Guarded control stack data access.</tr>
<tr><td>7:0<td>Res0<td>Reserved, must be 0.</tr>
</table><br>
<table>
<caption id="ISS2 encoding for all other exceptions">ISS2 encoding for all other exceptions</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>23:0<td>Res0<td>Reserved, must be 0.</tr>
</table><br>
</tr>
<tr><td>31:26<td>EC<td>Exception Class. Indicates the reason for the exception that this register holds information about.<br>
                        For each EC value, the table references a subsection that gives information about :<br>
                        The cause of the exception, for example the configuration required to enable the trap.<br>
                        The encoding of the associated ISS.<br>
                        Possible values of the EC field are :<br>
<table>
<caption id="Exception Class encoding">Exception Class encoding</caption>
<tr><th>EC<th>Meaning<th>ISS<th>ISS2<th>Applies when</tr>
<tr><td>000000<td>Unknown reason
    <td>ISS encoding for exceptions with an unknown reason
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>000001<td>Trapped WF* instruction execution<br>
                  Conditional WF * instructions that fail their condition code check do not cause an exception.
    <td>ISS encoding for an exception from a WF* instruction
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>000011<td>Trapped MCR or MRC access with(coproc == 0b1111) that is not reported using EC 0b000000.<br>
                  Conditional WF * instructions that fail their condition code check do not cause an exception.
    <td>ISS encoding for an exception from an MCR or MRC access
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>000100<td>Trapped MCRR or MRRC access with(coproc == 0b1111) that is not reported using EC 0b000000.
    <td>ISS encoding for an exception from an MCRR or MRRC access
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>000101<td>Trapped MCR or MRC access with(coproc == 0b1110).
    <td>ISS encoding for an exception from an MCR or MRC access
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>000110<td>Trapped LDC or STC access.<br>
                  The only architected uses of these instruction are :<br>
                  An STC to write data to memory from DBGDTRRXint.<br>
                  An LDC to read data from memory to DBGDTRTXint.
    <td>ISS encoding for an exception from an LDC or STC instruction
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>000111<td>Access to SME, SVE, Advanced SIMD or floating - point functionality trapped by CPACR_EL1.FPEN, CPTR_EL2.FPEN, CPTR_EL2.TFP, or CPTR_EL3.TFP control.<br>
                  Excludes exceptions resulting from CPACR_EL1 when the value of HCR_EL2.TGE is 1, or because SVE or Advanced SIMD and floating - point are not implemented.<br>
                  These are reported with EC value 0b000000.
    <td>ISS encoding for an exception from an access to SVE, Advanced SIMD or floating - point functionality, resulting from the FPEN and TFP traps
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>001010<td>Trapped execution of an LD64B or ST64B * instruction.
    <td>ISS encoding for an exception from an LD64B or ST64B * instruction
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_LS64 is implemented</tr>
<tr><td>001100<td>Trapped MRRC access with(coproc == 0b1110).
    <td>ISS encoding for an exception from an MCRR or MRRC access
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>001110<td>Illegal Execution state.
    <td>ISS encoding for an exception from an Illegal Execution state, or a PC or SP alignment fault
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>010001<td>SVC instruction execution in AArch32 state.
    <td>ISS encoding for an exception from HVC or SVC instruction execution
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>010100<td>Trapped MSRR, MRRS or System instruction execution in AArch64 state, that is not reported using EC 0b000000.
    <td>ISS encoding for an exception from MSRR, MRRS, or 128 - bit System instruction execution in AArch64 state
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_SYSREG128 is implemented or FEAT_SYSINSTR128 is implemented</tr>
<tr><td>010101<td>SVC instruction execution in AArch64 state.
    <td>ISS encoding for an exception from HVC or SVC instruction execution
    <td>ISS2 encoding for all other exceptions
    <td>When AArch64 is supported</tr>
<tr><td>011000<td>Trapped MSR, MRS or System instruction execution in AArch64 state, that is not reported using EC 0b000000, 0b000001, or 0b000111.<br>
                  This includes all instructions that cause exceptions that are part of the encoding space defined in 'System instruction class encoding overview', except for those exceptions reported using EC values 0b000000, 0b000001, or 0b000111.
    <td>ISS encoding for an exception from MSR, MRS, or System instruction execution in AArch64 state
    <td>ISS2 encoding for all other exceptions
    <td>When AArch64 is supported</tr>
<tr><td>011001<td>Access to SVE functionality trapped as a result of CPACR_EL1.ZEN, CPTR_EL2.ZEN, CPTR_EL2.TZ, or CPTR_EL3.EZ, that is not reported using EC 0b000000.
    <td>ISS encoding for an exception from an access to SVE functionality, resulting from CPACR_EL1.ZEN, CPTR_EL2.ZEN, CPTR_EL2.TZ, or CPTR_EL3.EZ
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_SVE is implemented</tr>
<tr><td>011011<td>Exception from an access to a TSTART instruction at EL0 when SCTLR_EL1.TME0 == 0, EL0 when SCTLR_EL2.TME0 == 0, at EL1 when SCTLR_EL1.TME == 0, at EL2 when SCTLR_EL2.TME == 0 or at EL3 when SCTLR_EL3.TME == 0.
    <td>ISS encoding for an exception from a TSTART instruction
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_TME is implemented</tr>
<tr><td>011100<td>Exception from a PAC Fail
    <td>ISS encoding for a PAC Fail exception
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_FPAC is implemented</tr>
<tr><td>011101<td>Access to SME functionality trapped as a result of CPACR_EL1.SMEN, CPTR_EL2.SMEN, CPTR_EL2.TSM, CPTR_EL3.ESM, or an attempted execution of an instruction that is illegal because of the value of PSTATE.SM or PSTATE.ZA, that is not reported using EC 0b000000.
    <td>ISS encoding for an exception due to SME functionality
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_SME is implemented</tr>
<tr><td>100000<td>Instruction Abort from a lower Exception level.<br>
                  Used for MMU faults generated by instruction accesses and synchronous External aborts, including synchronous parity or ECC errors.Not used for debug - related exceptions.
    <td>ISS encoding for an exception from an Instruction Abort
    <td>ISS2 encoding for an exception from an Instruction Abort</tr>
<tr><td>100001<td>Instruction Abort taken without a change in Exception level.<br>
                  Used for MMU faults generated by instruction accesses and synchronous External aborts, including synchronous parity or ECC errors.Not used for debug - related exceptions.
    <td>ISS encoding for an exception from an Instruction Abort
    <td>ISS2 encoding for an exception from an Instruction Abort</tr>
<tr><td>100010<td>PC alignment fault exception.
    <td>ISS encoding for an exception from an Illegal Execution state, or a PC or SP alignment fault
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>100101<td>Data Abort exception taken without a change in Exception level.<br>
                  Used for MMU faults generated by data accesses, alignment faults other than those caused by Stack Pointer misalignment, and synchronous External aborts, including synchronous parity or ECC errors.Not used for debug - related exceptions.
    <td>ISS encoding for an exception from a Data Abort
    <td>ISS2 encoding for an exception from a Data Abort</tr>
<tr><td>100110<td>SP alignment fault exception.
    <td>ISS encoding for an exception from an Illegal Execution state, or a PC or SP alignment fault
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>100111<td>Memory Operation Exception.
    <td>ISS encoding for an exception from the Memory Copy and Memory Set instructions
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_MOPS is implemented</tr>
<tr><td>101000<td>Trapped floating-point exception taken from AArch32 state.<br>
                  This EC value is valid if the implementation supports trapping of floating-point exceptions, otherwise it is reserved.<br>
                  Whether a floating-point implementation supports trapping of floating-point exceptions is IMPLEMENTATION DEFINED.
    <td>ISS encoding for an exception from a trapped floating-point exception
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>101000<td>Trapped floating-point exception taken from AArch64 state.<br>
                  This EC value is valid if the implementation supports trapping of floating-point exceptions, otherwise it is reserved.<br>
                  Whether a floating-point implementation supports trapping of floating-point exceptions is IMPLEMENTATION DEFINED.
    <td>ISS encoding for an exception from a trapped floating-point exception
    <td>ISS2 encoding for all other exceptions
    <td>When AArch64 is supported</tr>
<tr><td>101101<td>GCS exception.
    <td>ISS encoding for a GCS exception
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_GCS is implemented</tr>
<tr><td>101111<td>SError exception.
    <td>ISS encoding for an SError exception
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>110000<td>Breakpoint exception from a lower Exception level.
    <td>ISS encoding for an exception from a Breakpoint or Vector Catch debug exception
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>110001<td>Breakpoint exception taken without a change in Exception level.
    <td>ISS encoding for an exception from a Breakpoint or Vector Catch debug exception
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>110010<td>Software Step exception from a lower Exception level.
    <td>ISS encoding for an exception from a Software Step exception
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>110011<td>Software Step exception taken without a change in Exception level.
    <td>ISS encoding for an exception from a Software Step exception
    <td>ISS2 encoding for all other exceptions</tr>
<tr><td>110100<td>Watchpoint exception from a lower Exception level.
    <td>ISS encoding for an exception from a Watchpoint exception
    <td>ISS2 encoding for an exception from a Watchpoint exception</tr>
<tr><td>110101<td>Watchpoint exception taken without a change in Exception level.
    <td>ISS encoding for an exception from a Watchpoint exception
    <td>ISS2 encoding for an exception from a Watchpoint exception</tr>
<tr><td>111000<td>BKPT instruction execution in AArch32 state.
    <td>ISS encoding for an exception from execution of a Breakpoint instruction
    <td>ISS2 encoding for all other exceptions
    <td>When AArch32 is supported</tr>
<tr><td>111100<td>BRK instruction execution in AArch64 state.
    <td>ISS encoding for an exception from execution of a Breakpoint instruction
    <td>ISS2 encoding for all other exceptions
    <td>When AArch64 is supported</tr>
<tr><td>111101<td>PMU exception
    <td>ISS encoding for a PMU exception
    <td>ISS2 encoding for all other exceptions
    <td>When FEAT_EBEP is implemented</tr>
</table><br>
<tr><td>25<td>IL<td>Instruction Length for synchronous exceptions. Possible values of this bit are:<br>
                    0 = 16-bit instruction trapped.<br>
                    1 = 32-bit instruction trapped. This value is also used when the exception is one of the following :<br>
                        - An SError exception.<br>
                        - An Instruction Abort exception.<br>
                        - A PC alignment fault exception.<br>
                        - An SP alignment fault exception.<br>
                        - A Data Abort exception for which the value of the ISV bit is 0.<br>
                        - An Illegal Execution state exception.<br>
                        - Any debug exception except for Breakpoint instruction exceptions.For Breakpoint instruction exceptions, this bit has its standard meaning :<br>
                          - 0 = 16-bit T32 BKPT instruction.<br>
                          - 1 = 32-bit A32 BKPT instruction or A64 BRK instruction.<br>
                        - An exception reported using EC value 0b000000.</tr>
<tr><td>24:0<td>ISS<td>Instruction Specific Syndrome. Architecturally, this field can be defined independently for each defined Exception class. However, in practice, some ISS encodings are used for more than one Exception class.<br>
                    Typically, an ISS encoding has a number of subfields.When an ISS subfield holds a register number, the value returned in that field is the AArch64 view of the register number.<br>
                    For an exception taken from AArch32 state, see 'Mapping of the general-purpose registers between the Execution states'.<br>
                    If the AArch32 register descriptor is 0b1111, then:<br>
                    - If the instruction that generated the exception was not UNPREDICTABLE, the field takes the value 0b11111.<br>
                    - If the instruction that generated the exception was UNPREDICTABLE, the field takes an UNKNOWN value that must be either :<br>
                      - The AArch64 view of the register number of a register that might have been used at the Exception level from which the exception was taken.<br>
                      - The value 0b11111.</tr>
</table>
</table>

### SPSR_EL1 register {#ARM_REGISTERS_REGISTER_OVERVIEW_SPSR_EL1_REGISTER}

Holds the saved process state when an exception is taken to EL1.

When AArch32 is supported and exception taken from AArch32 state:<br>
<table>
<caption id="SPSR_EL1 bits AArch32">Saved Program Status Register (EL1)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:34<td>Res0<td>   Reserved, must be 0.</tr>
<tr><td>33<td>PPEND<td>     PMU exception pending bit. Set to the value of PSTATE.PPEND on taking an exception to EL1, and conditionally copied to PSTATE.PPEND on executing an exception return operation in EL1.</tr>
<tr><td>32<td>Res0<td>      Reserved, must be 0.</tr>
<tr><td>31<td>N<td>         Negative Condition flag. Set to the value of PSTATE.N on taking an exception to EL1, and copied to PSTATE.N on executing an exception return operation in EL1.</tr>
<tr><td>30<td>Z<td>         Zero Condition flag. Set to the value of PSTATE.Z on taking an exception to EL1, and copied to PSTATE.Z on executing an exception return operation in EL1.</tr>
<tr><td>29<td>C<td>         Carry Condition flag. Set to the value of PSTATE.C on taking an exception to EL1, and copied to PSTATE.C on executing an exception return operation in EL1.</tr>
<tr><td>28<td>V<td>         Overflow Condition flag. Set to the value of PSTATE.V on taking an exception to EL1, and copied to PSTATE.V on executing an exception return operation in EL1.</tr>
<tr><td>27<td>Q<td>         Overflow or saturation flag. Set to the value of PSTATE.Q on taking an exception to EL1, and copied to PSTATE.Q on executing an exception return operation in EL1.</tr>
<tr><td>26:25<td>IT[1:0]<td>If-Then. Set to the value of PSTATE.IT on taking an exception to EL1, and copied to PSTATE.IT on executing an exception return operation in EL1.<br>
                            SPSR_EL1.IT must contain a value that is valid for the instruction being returned to.<br>
                            The IT field is split as follows :<br>
                            IT[1:0] is SPSR_EL1[26:25].<br>
                            IT[7:2] is SPSR_EL1[15:10].</tr>
<tr><td>24<td>DIT<td>       Data Independent Timing. Set to the value of PSTATE.DIT on taking an exception to EL1, and copied to PSTATE.DIT on executing an exception return operation in EL1.</tr>
<tr><td>23<td>SSBS<td>      Speculative Store Bypass. Set to the value of PSTATE.SSBS on taking an exception to EL1, and copied to PSTATE.SSBS on executing an exception return operation in EL1.</tr>
<tr><td>22<td>PAN<td>       Privileged Access Never. Set to the value of PSTATE.PAN on taking an exception to EL1, and copied to PSTATE.PAN on executing an exception return operation in EL1.</tr>
<tr><td>21<td>SS<td>        Software Step. Set to the value of PSTATE.SS on taking an exception to EL1, and conditionally copied to PSTATE.SS on executing an exception return operation in EL1.</tr>
<tr><td>20<td>IL<td>        Illegal Execution state. Set to the value of PSTATE.IL on taking an exception to EL1, and copied to PSTATE.IL on executing an exception return operation in EL1.</tr>
<tr><td>19:16<td>GE<td>     Greater than or Equal flags. Set to the value of PSTATE.GE on taking an exception to EL1, and copied to PSTATE.GE on executing an exception return operation in EL1.</tr>
<tr><td>15:10<td>IT[7:2]<td>See IT[2:0]</tr>
<tr><td>9<td>E<td>          Endianness. Set to the value of PSTATE.E on taking an exception to EL1, and copied to PSTATE.E on executing an exception return operation in EL1.<br>
                            If the implementation does not support big-endian operation, SPSR_EL1.E is RES0.<br>
                            If the implementation does not support little-endian operation, SPSR_EL1.E is RES1.
                            On executing an exception return operation in EL1, if the implementation does not support big-endian operation at the Exception level being returned to, SPSR_EL1.E is RES0, and if the implementation does not support little-endian operation at the Exception level being returned to, SPSR_EL1.E is RES1.</tr>
<tr><td>8<td>A<td>          SError exception mask. Set to the value of PSTATE.A on taking an exception to EL1, and copied to PSTATE.A on executing an exception return operation in EL1.</tr>
<tr><td>7<td>I<td>          IRQ interrupt mask. Set to the value of PSTATE.I on taking an exception to EL1, and copied to PSTATE.I on executing an exception return operation in EL1.</tr>
<tr><td>6<td>F<td>          FIQ interrupt mask. Set to the value of PSTATE.F on taking an exception to EL1, and copied to PSTATE.F on executing an exception return operation in EL1.</tr>
<tr><td>5<td>T<td>          T32 Instruction set state. Set to the value of PSTATE.T on taking an exception to EL1, and copied to PSTATE.T on executing an exception return operation in EL1.</tr>
<tr><td>4<td>M[4]<td>       Execution state. Set to 0b1, the value of PSTATE.nRW, on taking an exception to EL1 from AArch32 state, and copied to PSTATE.nRW on executing an exception return operation in EL1.<br>
                            1 = AArch32 execution state.</tr>
<tr><td>3:0<td>M[3:0]<td>   AArch32 Mode. Set to the value of PSTATE.M[3:0] on taking an exception to EL1, and copied to PSTATE.M[3:0] on executing an exception return operation in EL1.<br>
                            0000 = User.<br>
                            0001 = FIQ.<br>
                            0010 = IRQ.<br>
                            0011 = Supervisor.<br>
                            0111 = Abort.<br>
                            1011 = Undefined.<br>
                            1111 = System.</tr>
</table>
When exception taken from AArch64 state:<br>
<table>
<caption id="SPSR_EL1 bits AArch64">Saved Program Status Register (EL1)</caption>
<tr><th>Bits<th>ID<th>Values</tr>
<tr><td>63:36<td>Res0<td>   Reserved, must be 0.</tr>
<tr><td>35<td>PACM<td>      PACM. Set to the value of PSTATE.PACM on taking an exception to EL1, and copied to PSTATE.PACM on executing an exception return operation in EL1.</tr>
<tr><td>34<td>EXLOCK<td>    Exception return state lock. Set to the value of PSTATE.EXLOCK on taking an exception to EL1, and copied to PSTATE.EXLOCK on executing an exception return operation in EL1.</tr>
<tr><td>33<td>PPEND<td>     PMU exception pending bit. Set to the value of PSTATE.PPEND on taking an exception to EL1, and conditionally copied to PSTATE.PPEND on executing an exception return operation in EL1.</tr>
<tr><td>32<td>PM<td>        PMU exception mask bit. Set to the value of PSTATE.PM on taking an exception to EL1, and copied to PSTATE.PM on executing an exception return operation in EL1.</tr>
<tr><td>31<td>N<td>         Negative Condition flag. Set to the value of PSTATE.N on taking an exception to EL1, and copied to PSTATE.N on executing an exception return operation in EL1.</tr>
<tr><td>30<td>Z<td>         Zero Condition flag. Set to the value of PSTATE.Z on taking an exception to EL1, and copied to PSTATE.Z on executing an exception return operation in EL1.</tr>
<tr><td>29<td>C<td>         Carry Condition flag. Set to the value of PSTATE.C on taking an exception to EL1, and copied to PSTATE.C on executing an exception return operation in EL1.</tr>
<tr><td>28<td>V<td>         Overflow Condition flag. Set to the value of PSTATE.V on taking an exception to EL1, and copied to PSTATE.V on executing an exception return operation in EL1.</tr>
<tr><td>27:26<td>Res0<td>   Reserved, must be 0.</tr>
<tr><td>25<td>TCO<td>       Tag Check Override. Set to the value of PSTATE.TCO on taking an exception to EL1, and copied to PSTATE.TCO on executing an exception return operation in EL1.</tr>
<tr><td>24<td>DIT<td>       Data Independent Timing. Set to the value of PSTATE.DIT on taking an exception to EL1, and copied to PSTATE.DIT on executing an exception return operation in EL1.</tr>
<tr><td>23<td>UAO<td>       User Access Override. Set to the value of PSTATE.UAO on taking an exception to EL1, and copied to PSTATE.UAO on executing an exception return operation in EL1.</tr>
<tr><td>22<td>PAN<td>       Privileged Access Never. Set to the value of PSTATE.PAN on taking an exception to EL1, and copied to PSTATE.PAN on executing an exception return operation in EL1.</tr>
<tr><td>21<td>SS<td>        Software Step. Set to the value of PSTATE.SS on taking an exception to EL1, and conditionally copied to PSTATE.SS on executing an exception return operation in EL1.</tr>
<tr><td>20<td>IL<td>        Illegal Execution state. Set to the value of PSTATE.IL on taking an exception to EL1, and copied to PSTATE.IL on executing an exception return operation in EL1.</tr>
<tr><td>19:14<td>Res0<td>   Reserved, must be 0.</tr>
<tr><td>13<td>ALLINT<td>    All IRQ or FIQ interrupts mask. Set to the value of PSTATE.ALLINT on taking an exception to EL1, and copied to PSTATE.ALLINT on executing an exception return operation in EL1.</tr>
<tr><td>12<td>SSBS<td>      Speculative Store Bypass. Set to the value of PSTATE.SSBS on taking an exception to EL1, and copied to PSTATE.SSBS on executing an exception return operation in EL1.</tr>
<tr><td>11<td>BTYPE<td>     Branch Type Indicator. Set to the value of PSTATE.BTYPE on taking an exception to EL1, and copied to PSTATE.BTYPE on executing an exception return operation in EL1.</tr>
<tr><td>9<td>D<td>          Debug exception mask. Set to the value of PSTATE.D on taking an exception to EL1, and copied to PSTATE.D on executing an exception return operation in EL1.</tr>
<tr><td>8<td>A<td>          SError exception mask. Set to the value of PSTATE.A on taking an exception to EL1, and copied to PSTATE.A on executing an exception return operation in EL1.</tr>
<tr><td>7<td>I<td>          IRQ interrupt mask. Set to the value of PSTATE.I on taking an exception to EL1, and copied to PSTATE.I on executing an exception return operation in EL1.</tr>
<tr><td>6<td>F<td>          FIQ interrupt mask. Set to the value of PSTATE.F on taking an exception to EL1, and copied to PSTATE.F on executing an exception return operation in EL1.</tr>
<tr><td>5<td>Res0<td>       Reserved, must be 0.</tr>
<tr><td>4<td>M[4]<td>       Execution state. Set to 0b0, the value of PSTATE.nRW, on taking an exception to EL1 from AArch64 state, and copied to PSTATE.nRW on executing an exception return operation in EL1.<br>
                            0 = AArch64 execution state.</tr>
<tr><td>3:0<td>M[3:0]<td>   AArch64 Exception level and selected Stack Pointer.<br>
                            0000 = EL0.<br>
                            0100 = EL1 with SP_EL0 (ELt).<br>
                            0101 = EL1 with SP_EL1 (EL1h).<br>
</table>

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

### CurrentEL register {#ARM_REGISTERS_REGISTER_OVERVIEW_CURRENTEL_REGISTER}
 -->
