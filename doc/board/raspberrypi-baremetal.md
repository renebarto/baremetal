# Raspberry Pi baremetal development {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT}

\todo Sort out and redistribute over other documents

## Reference material {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_REFERENCE_MATERIAL}

 - [Raspberry Pi documentation](https://github.com/raspberrypi/documentation/tree/develop/documentation/asciidoc/computers/raspberry-pi)
 - [Raspberry Pi 3B schematics](pdf/rpi-3b-v1_2-schematics-reduced.pdf)
 - [Raspberry Pi 3B physical layout](pdf/rpi-3b-v1_2-mechanical-drawing.pdf)
 - [Raspberry Pi 4B schematics](pdf/rpi-4b-schematics-reduced.pdf)
 - [Raspberry Pi 4B physical layout](pdf/rpi-4b-mechanical-drawing.pdf)
 - [ARM architecture registers](pdf/arm-architecture-registers.pdf)
 - [Aarch64 reference manual](pdf/arm-aarch64-reference-manual.pdf)
 - [ARM Cortex A-53 reference r0p4](pdf/arm-cortex-a53-r0p4.pdf)
 - [ARM Cortex A-72 reference r0p3](pdf/arm-cortex-a72-r0p3.pdf)
 - [ARM Cortex A-76 reference r4p1](pdf/arm-cortex-a76-r4p1.pdf)
 - [BCM2835 peripherals specification](pdf/bcm2835-peripherals.pdf)
 - [BCM2836 additional information](pdf/bcm2836-additional-info.pdf)
 - [BCM2837 peripherals specification](pdf/bcm2837-peripherals.pdf)
 - [BCM2711 peripherals specification](pdf/bcm2711-peripherals.pdf)
 - [RP1 peripherals specification](pdf/rp1-peripherals.pdf)
 - [VideoCore IV specification](pdf/videocore-iv-3d-architecture-reference-guide.pdf)

## SoC for each board {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_SOC_FOR_EACH_BOARD}

The Raspberry Pi boards uses different types and versions of SoC (System-on-Chip)

| Board                          | SoC       | Processor              | Number of cores |
|--------------------------------|-----------|------------------------|-----------------|
| Raspberry Pi 1A                | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi 1A+               | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi 1B                | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi 1B+               | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi Zero              | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi Zero W            | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi Compute Module 1  | BCM2835   | ARM1176JZF-S           | 1 |
| Raspberry Pi 2B                | BCM2836   | ARM Cortex-A7          | 4 |
| Raspberry Pi 3B                | BCM2837   | ARM Cortex A53 (ARMv8) | 4 |
| Raspberry Pi 2B (later models) | BCM2837   | ARM Cortex A53 (ARMv8) | 4 |
| Raspberry Pi Compute Module 3  | BCM2837   | ARM Cortex A53 (ARMv8) | 4 |
| Raspberry Pi 3A+               | BCM2837B0 | ARM Cortex A53 (ARMv8) | 4 |
| Raspberry Pi 3B+               | BCM2837B0 | ARM Cortex A53 (ARMv8) | 4 |
| Raspberry Pi Compute Module 3+ | BCM2837B0 | ARM Cortex A53 (ARMv8) | 4 |
| Raspberry Pi 4B                | BCM2711   | ARM Cortex A72 (ARMv8) | 4 |
| Raspberry Pi 400               | BCM2711   | ARM Cortex A72 (ARMv8) | 4 |
| Raspberry Pi Compute Module 4  | BCM2711   | ARM Cortex A72 (ARMv8) | 4 |
| Raspberry Pi 5B                | BCM2712   | ARM Cortex A76 (ARMv8) | 4 |

## CPU <-> GPU communication {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION}

CPU and GPU communicate through a mailbox interface:
 - CPU writes data
 - CPU flags to GPU that data is ready
 - GPU reads data and processes
 - GPU writes response
 - CPU polls for ready response and reads it

The mailbox interface is just another peripheral.

### CPU <-> Peripheral communication {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_CPU___PERIPHERAL_COMMUNICATION}

Every peripheral has a slot of memory mapped I/O (MMIO). The start address depends on the SoC:

| SoC           | Peripheral address start | Peripheral address size |
|---------------|--------------------------|-------------------------|
| BCM2835       | 0x20000000               | 0x01000000
| BCM2836       | 0x3F000000               | 0x01000000
| BCM2837       | 0x3F000000               | 0x01000000
| BCM2711       | 0xFE000000               | 0x01800000
| BCM2712       | 0xFE000000               | 0x01800000
 
Peripheral data is written in 32 bit words, aligned on 4 bytes.
Every peripheral has control/status and data words.

Peripheral MMIO slots for BCM2837:

| Start address | Peripheral |
|---------------|------------|
| 0x3F003000    | System Timer 
| 0x3F00B000    | Interrupt controller
| 0x3F00B880    | VideoCore mailbox 
| 0x3F100000    | Power management 
| 0x3F104000    | Random Number Generator
| 0x3F200000    | General Purpose IO controller
| 0x3F201000    | UART0 (serial port, PL011)
| 0x3F215000    | UART1 (serial port, AUX mini UART)
| 0x3F300000    | External Mass Media Controller (SD card reader)
| 0x3F980000    | Universal Serial Bus controller

### Memory Management Unit (MMU) {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MANAGEMENT_UNIT_MMU}

The CPU also has a MMU, which can be used to map addresses to virtual addresses.

### Code execution {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_CODE_EXECUTION}

After the GPU initializes, all cores are started, however depending on whether the core is core 0 or a secondary core:

| Core | Start address |
|------|---------------|
| 0    | 0x00000000 -> 0x00008000 or 0x00080000
| 1    | Address read from memory location 0x000000E0
| 2    | Address read from memory location 0x000000E8
| 3    | Address read from memory location 0x000000F0

To determine which core is currently running, use the MPIDR_EL1 system register, e.g.:

```asm
asm volatile ("mrs %0, mpidr_el1" : "=r" (nMPIDR));
```

### Memory mapping {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING}

Memory is mapping in ranges, with different address mapping for each range:

| Physical address      | VC address                          | Use                                |
|-----------------------|-------------------------------------|------------------------------------|
| 0x00000000-end        | -                                   | User space (Physical memory depending on memory size)
| 0x00000000-split      | 0xC0000000-0xC0000000 + split       | Kernel space of ARM assigned memory
| split-end             | 0xC0000000 + split-0xC0000000 + end | Kernel space of VC assigned memory
| 0x20000000-0x21000000 | 0x7E000000-0x7F000000               | I/O peripherals (RPI 1)
| 0x3F000000-0x40000000 | 0x7E000000-0x7F000000               | I/O peripherals (RPI 2/3)
| 0xFE000000-0xFF000000 | 0x7E000000-0x7F000000               | I/O peripherals (RPI 4)

Mapping from BCM2835 peripherals specification.

<img src="images/rpi-memory-mapping.png"  alt="Memory map" width="800"/>
Linux uses a virtual address mapping, however that is not relevant here.

Circle uses the following layout of the ARM memory:

#### RPI 1 {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING_RPI_1}

| Base     | Size         | Contents               | Remarks                      |
|----------|--------------|------------------------|------------------------------|
| 00000000 | 32 Bytes     | Exception vector table |                              |
| ...      |              |                        |                              |
| 00000100 | variable     | ATAGS                  | unused                       |
| 00008000 | max. 2 MByte | Kernel image           |                              |
|          |              | .init                  | startup code                 |
|          |              | .text                  |                              |
|          |              | .rodata                |                              |
|          |              | .init_array            | static constructors          |
|          |              | .ARM.exidx             | C++ exception index          |
|          |              | .eh_frame              | unused                       |
|          |              | .data                  |                              |
|          |              | .bss                   |                              |
| ...      |              |                        |                              |
| 00208000 | 128 KByte    | Kernel stack           |                              |
| 00228000 | 32 KByte     | Abort exception stack  |                              |
| 00230000 | 32 KByte     | IRQ exception stack    |                              |
| 00238000 | 32 KByte     | FIQ exception stack    |                              |
| 00240000 | 16 KByte     | Page table 1           |                              |
| ...      |              |                        |                              |
| 00400000 | 1 MByte      | Coherent region        | for property mailbox, VCHIQ  |
| 00500000 | variable     | Heap allocator         | malloc()                     |
| ???????? | 4 MByte      | Page allocator         | palloc()                     |
| split    | variable     | GPU memory             | depending on split location  |
| end RAM  |              | End of physical RAM    |                              |
| 20000000 |              | Peripherals            |                              |
| ...      |              |                        |                              |

#### RPI 2/3 32 bit {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING_RPI_23_32_BIT}

| Base     | Size         | Contents                     | Remarks                      |
|----------|--------------|------------------------------|------------------------------|
| 00000000 | 32 Bytes     | Exception vector table       |                              |
| 00000000 | 256 Bytes    | ARM stub                     | Contains spinlock for cores 1-3 |
| ...      |              |                              |                              |
| 00000100 | variable     | ATAGS                        | unused                       |
| 00008000 | max. 2 MByte | Kernel image                 | Can be larger if KERNEL_MAX_SIZE redefined
|          |              | .init                        | startup code                 |
|          |              | .text                        |                              |
|          |              | .rodata                      |                              |
|          |              | .init_array                  | static constructors          |
|          |              | .ARM.exidx                   | C++ exception index          |
|          |              | .eh_frame                    | unused                       |
|          |              | .data                        |                              |
|          |              | .bss                         |                              |
| ...      |              |                              |                              |
| 00208000 | 128 KByte    | Kernel stack core 0          |                              |
| 00228000 | 128 KByte    | Kernel stack core 1          |                              |
| 00248000 | 128 KByte    | Kernel stack core 2          |                              |
| 00268000 | 128 KByte    | Kernel stack core 3          |                              |
| 00288000 | 32 KByte     | Abort exception stack core 0 |                              |
| 00290000 | 32 KByte     | Abort exception stack core 1 |                              |
| 00298000 | 32 KByte     | Abort exception stack core 2 |                              |
| 002A0000 | 32 KByte     | Abort exception stack core 3 |                              |
| 002A8000 | 32 KByte     | IRQ exception stack core 0   |                              |
| 002B0000 | 32 KByte     | IRQ exception stack core 1   |                              |
| 002B8000 | 32 KByte     | IRQ exception stack core 2   |                              |
| 002C0000 | 32 KByte     | IRQ exception stack core 3   |                              |
| 002C8000 | 32 KByte     | FIQ exception stack core 0   |                              |
| 002D0000 | 32 KByte     | FIQ exception stack core 1   |                              |
| 002D8000 | 32 KByte     | FIQ exception stack core 2   |                              |
| 002E0000 | 32 KByte     | FIQ exception stack core 3   |                              |
| 002E8000 | 16 KByte     | Page table 1                 |                              |
| ...      |              |                              |                              |
| 00400000 | 1 MByte      | Coherent region              | for property mailbox, VCHIQ  |
| 00500000 | variable     | Heap allocator               | malloc()                     |
| ???????? | 4 MByte      | Page allocator               | palloc()                     |
| 1F000000 | variable     | RPI stub                     | if used for debugging        |
| split    | variable     | GPU memory                   | depending on split location  |
| end RAM  |              | End of physical RAM          |                              |
| 3F000000 | 16 MByte     | Peripherals                  |                              |
| 40000000 |              | Local peripherals            |                              |
| ...      |              |                              |                              |

#### RPI 2/3 64 bit {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING_RPI_23_64_BIT}

| Base     | Size         | Contents                     | Remarks                      |
|----------|--------------|------------------------------|------------------------------|
| 00000000 | 256 Bytes    | ARM stub                     | Contains spinlock for cores 1-3 |
| ...      |              |                              |                              |
| 00000100 | variable     | ATAGS                        | unused                       |
| 00080000 | max. 2 MByte | Kernel image                 | Can be larger if KERNEL_MAX_SIZE redefined |
|          |              | .init                        | startup code                 |
|          |              | .text                        |                              |
|          |              | .rodata                      |                              |
|          |              | .init_array                  | static constructors          |
|          |              | .ARM.exidx                   | C++ exception index          |
|          |              | .eh_frame                    | unused                       |
|          |              | .data                        |                              |
|          |              | .bss                         |                              |
| ...      |              |                              |                              |
| 00208000 | 128 KByte    | Kernel stack core 0          |                              |
| 00228000 | 128 KByte    | Kernel stack core 1          |                              |
| 00248000 | 128 KByte    | Kernel stack core 2          |                              |
| 00268000 | 128 KByte    | Kernel stack core 3          |                              |
| ...      |              |                              |                              |
| 00500000 | 1 MByte      | Coherent region              | for property mailbox, VCHIQ  |
| 00500000 | variable     | Heap allocator               | malloc()                     |
| ???????? | 16 MByte     | Page allocator               | palloc()                     |
| split    | variable     | GPU memory                   | depending on split location  |
| end RAM  |              | End of physical RAM          |                              |
| 3F000000 | 16 MByte     | Peripherals                  |                              |
| 40000000 |              | Local peripherals            |                              |
| ...      |              |                              |                              |

#### RPI 4 32 bit {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING_RPI_4_32_BIT}

| Base     | Size         | Contents                     | Remarks                      |
|----------|--------------|------------------------------|------------------------------|
| 00000000 | 32 Bytes     | Exception vector table       |                              |
| 00000000 | 256 Bytes    | ARM stub                     | Contains spinlock for cores 1-3
| ...      |              |                              |                              |
| 00000100 | variable     | ATAGS                        | unused                       |
| 00008000 | max. 2 MByte | Kernel image                 | Can be larger if KERNEL_MAX_SIZE redefined |
|          |              | .init                        | startup code                 |
|          |              | .text                        |                              |
|          |              | .rodata                      |                              |
|          |              | .init_array                  | static constructors          |
|          |              | .ARM.exidx                   | C++ exception index          |
|          |              | .eh_frame                    | unused                       |
|          |              | .data                        |                              |
|          |              | .bss                         |                              |
| ...      |              |                              |                              |
| 00208000 | 128 KByte    | Kernel stack core 0          |                              |
| 00228000 | 128 KByte    | Kernel stack core 1          |                              |
| 00248000 | 128 KByte    | Kernel stack core 2          |                              |
| 00268000 | 128 KByte    | Kernel stack core 3          |                              |
| 00288000 | 32 KByte     | Abort exception stack core 0 |                              |
| 00290000 | 32 KByte     | Abort exception stack core 1 |                              |
| 00298000 | 32 KByte     | Abort exception stack core 2 |                              |
| 002A0000 | 32 KByte     | Abort exception stack core 3 |                              |
| 002A8000 | 32 KByte     | IRQ exception stack core 0   |                              |
| 002B0000 | 32 KByte     | IRQ exception stack core 1   |                              |
| 002B8000 | 32 KByte     | IRQ exception stack core 2   |                              |
| 002C0000 | 32 KByte     | IRQ exception stack core 3   |                              |
| 002C8000 | 32 KByte     | FIQ exception stack core 0   |                              |
| 002D0000 | 32 KByte     | FIQ exception stack core 1   |                              |
| 002D8000 | 32 KByte     | FIQ exception stack core 2   |                              |
| 002E0000 | 32 KByte     | FIQ exception stack core 3   |                              |
| 002E8000 | 16 KByte     | Page table 1                 |                              |
| ...      |              |                              |                              |
| 00500000 | 4 MByte      | Coherent region              | for property mailbox, VCHIQ, xHCI |
| 00500000 | variable     | Heap allocator               | "new" and malloc()           |
| ???????? | 4 MByte      | Page allocator               | palloc()                     |
| split    | variable     | GPU memory                   | depending on split location  |
| 40000000 | variable     | high heap allocator          | unused above 0xC0000000      |
| end RAM  |              | End of physical RAM          |                              |
| ...      |              |                              |                              |
| FA000000 | 2 MByte      | xHCI controller              | mapped from 0x600000000      |
| ...      |              |                              |                              |
| FE000000 | 64 MByte     | Peripherals                  |                              |

#### RPI 4 64 bit {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING_RPI_4_64_BIT}

| Base     | Size         | Contents                     | Remarks                      |
|----------|--------------|------------------------------|------------------------------|
| 00000000 | 256 Bytes    | ARM stub                     | Contains spinlock for cores 1-3 |
| ...      |              |                              |                              |
| 00000100 | variable     | ATAGS                        | unused                       |
| 0006F000 | 4 KByte      | EL3 stack                    |                              |
| 00070000 | 2 KByte      | Exception vector table EL3   |                              |
| 00080000 | max. 2 MByte | Kernel image                 | Can be larger if KERNEL_MAX_SIZE redefined |
|          |              | .init                        | startup code                 |
|          |              | .text                        |                              |
|          |              | .rodata                      |                              |
|          |              | .init_array                  | static constructors          |
|          |              | .ARM.exidx                   | unused                       |
|          |              | .eh_frame                    | C++ exception frames         |
|          |              | .data                        |                              |
|          |              | .bss                         |                              |
| ...      |              |                              |                              |
| 00208000 | 128 KByte    | Kernel stack core 0          |                              |
| 00228000 | 128 KByte    | Kernel stack core 1          |                              |
| 00248000 | 128 KByte    | Kernel stack core 2          |                              |
| 00268000 | 128 KByte    | Kernel stack core 3          |                              |
| 00288000 | 32 KByte     | Exception stack core 0       |                              |
| 00290000 | 32 KByte     | Exception stack core 1       |                              |
| 00298000 | 32 KByte     | Exception stack core 2       |                              |
| 002A0000 | 32 KByte     | Exception stack core 3       |                              |
| ...      |              |                              |                              |
| 00500000 | 4 MByte      | Coherent region              | for property mailbox, VCHIQ, xHCI |
| 00900000 | variable     | Heap allocator               | "new" and malloc()           |
| ???????? | 16 MByte     | Page allocator               | palloc()                     |
| split    | variable     | GPU memory                   | depending on split location  |
| 40000000 | variable     | high heap allocator          | unused above 0xC0000000      |
| end RAM  |              | End of physical RAM          |                              |
| ...      |              |                              |                              |
| FE000000 | 64 MByte     | Peripherals                  |                              |
| ...      |              |                              |                              |
| 600000000 | 64 MByte    | xHCI controller              | mapped from 0x600000000      |
| ...      |              |                              |                              |
| FC000000 | 64 MByte     | Peripherals                  |                              |
| ...      |              |                              |                              |
| 600000000 | 64 MByte    | xHCI controller              |                              |

#### RPI 5 64 bit {#RASPBERRY_PI_BAREMETAL_DEVELOPMENT_CPU___GPU_COMMUNICATION_MEMORY_MAPPING_RPI_5_64_BIT}

\todo Add memory mapping for RPI 5
