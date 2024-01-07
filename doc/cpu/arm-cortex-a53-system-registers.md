# ARM Cortex-A53 System Registers

## Register overview 

| Register name | Width | Acc | Reset value | Meaning |
|---------------|-------|-----|-------------|---------|
| MIDR_EL1      | 32    | R   | 0x410FD083  | Provides identification information for the processor, including an implementer code for the device and a device ID number.
| MPIDR_EL1     | 64    | R   | 0x80000003  | Provides an additional core identification mechanism for scheduling purposes in a cluster system. EDDEVAFF0 is a read-only copy of MPIDR_EL1[31:0] accessible from the external debug interface.
| L2CTLR_EL1    | 32    | R/W | ?           | L2 Control register

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
|               | 25:24 | #cores            | R   | Number of cores (0 = 1 core, 1 = 2 cores, 2 = 3 cores, 3 = 4 cores)
|               | 23    | -                 |     | Reserved
|               | 22    | CPU cache prot    |     | CPU RAM protection (0 = without ECC, 1 = with ECC)
|               | 21    | SCU-L2 cache prot |     | L2 cache protection (0 = without ECC, 1 = with ECC)
|               | 20:6  | -                 |     | Reserved
|               | 5     | L2 input latency  |     | L2 data RAM input latency ( 0 = 1 cycle, 1 = 2 cycles)
|               | 4:1   | -                 |     | Reserved
|               | 0     | L2 output latency |     | L2 data RAM output latency ( 0 = 2 cycles, 1 = 3 cycles)

