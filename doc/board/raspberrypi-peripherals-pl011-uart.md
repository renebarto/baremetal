# Raspberry Pi PL011 UART {#RASPBERRY_PI_PL011_UART}

See [documentation Raspberry Pi 3](pdf/bcm2837-peripherals.pdf), section 13 and [documentation Raspberry Pi 4](pdf/bcm2711-peripherals.pdf), section 11.

Raspberry Pi UART0 is one of multiple UARTs available on the Raspberry Pi.
It is a full-duplex UART with a baud rate of up to 4 Mbps. It is used for serial communication with other devices.
The UART is based on the ARM PrimeCell PL011 UART.

Raspberry Pi 3 supports only UART0 next to the mini-UART.
Raspberry Pi 4 supports UART0, 2, 3, 4, 5, and mini-UART.
Raspberry Pi 5 supports UART0, 1, 2, 3, 4, 5, and no mini-UART.

## GPIO pins (Raspberry Pi 3/4) {#RASPBERRY_PI_PL011_UART_GPIO_PINS_RASPBERRY_PI_34}

| GPIO#  |Pull  | Alt 0 | Alt 1 | Alt 2 | Alt 3 | Alt 4 | Alt 5 | Board type
|--------|------|-------|-------|-------|-------|-------|-------|------------
| GPIO0  | High |       |       |       |       | TXD2  |       | RPI 4 only
| GPIO1  | High |       |       |       |       | RXD2  |       | RPI 4 only
| GPIO2  | High |       |       |       |       | CTS2  |       | RPI 4 only
| GPIO3  | High |       |       |       |       | RTS2  |       | RPI 4 only
| GPIO4  | High |       |       |       |       | TXD3  |       | RPI 4 only
| GPIO5  | High |       |       |       |       | RXD3  |       | RPI 4 only
| GPIO6  | High |       |       |       |       | CTS3  |       | RPI 4 only
| GPIO7  | High |       |       |       |       | RTS3  |       | RPI 4 only
| GPIO8  | High |       |       |       |       | TXD4  |       | RPI 4 only
| GPIO9  | High |       |       |       |       | RXD4  |       | RPI 4 only
| GPIO10 | High |       |       |       |       | CTS4  |       | RPI 4 only
| GPIO11 | High |       |       |       |       | RTS4  |       | RPI 4 only
| GPIO12 | High |       |       |       |       | TXD5  |       | RPI 4 only
| GPIO13 | High |       |       |       |       | RXD5  |       | RPI 4 only
| GPIO14 | Low  | TXD0  |       |       |       | CTS5  | TXD1  | CTS 5 RPI 4 only
| GPIO15 | Low  | RXD0  |       |       |       | RTS5  | RXD1  | RTS 5 RPI 4 only
| GPIO16 | Low  |       |       |       | CTS0  |       | CTS1  | RPI 4 only
| GPIO17 | Low  |       |       |       | RTS0  |       | RTS1  | RPI 4 only
| GPIO30 | Low  |       |       |       | CTS0  |       | CTS1  | RPI 4 only
| GPIO31 | Low  |       |       |       | RTS0  |       | RTS1  | RPI 4 only
| GPIO32 | Low  |       |       |       | TXD0  |       | TXD1  | RPI 4 only
| GPIO33 | Low  |       |       |       | RXD0  |       | RXD1  | RPI 4 only
| GPIO36 | High |       |       | TXD0  |       |       |       |
| GPIO37 | Low  |       |       | RXD0  |       |       |       |
| GPIO38 | Low  |       |       | RTS0  |       |       |       | RPI 4 only
| GPIO39 | Low  |       |       | CTS0  |       |       |       | RPI 4 only
| GPIO40 | Low  |       |       |       |       |       | TXD1  |
| GPIO41 | Low  |       |       |       |       |       | RXD1  |
| GPIO42 | Low  |       |       |       |       |       | RTS1  | RPI 4 only
| GPIO43 | Low  |       |       |       |       |       | CTS1  | RPI 4 only

Each UART has its own register address space.

| UART | Bases register address |
|------|------------------------|
| 0    | 0x3F201000             |
| 1    | 0x3F201200             |
| 2    | 0x3F201400             |
| 3    | 0x3F201600             |
| 4    | 0x3F201800             |
| 5    | 0x3F201A00             |

| Register name | Register offset | Bits  | Name                                  | Acc | Meaning |
|---------------|-----------------|-------|---------------------------------------|-----|---------|
| DR            | 0x00            | 31:0  | Data Register                         | R/W | Data to be transmitted or received
| RSRECR        | 0x04            | 31:0  | Receive status register/error clear   | R/W | Receive status register/error clear
| FR            | 0x18            | 31:0  | Flag register
| ILPR          | 0x20            | 31:0  | IrDA low-power counter register       | -   | Not in use
| IBRD          | 0x24            | 31:0  | Integer Baud rate divisor             | R/W | Integer part of the baud rate divisor
| FBRD          | 0x28            | 31:0  | Fractional Baud rate divisor          | R/W | Fractional part of the baud rate divisor
| LCRH          | 0x2C            | 31:0  | Line control register                 | R/W | Line control register
| CR            | 0x30            | 31:0  | Control register                      | R/W | Control register
| IFLS          | 0x34            | 31:0  | Interrupt FIFO level select register  | R/W | Interrupt FIFO level select register
| IMSC          | 0x38            | 31:0  | Interrupt mask set clear register     | R/W | Interrupt mask set clear register
| RIS           | 0x3C            | 31:0  | Raw interrupt status register         | R   | Raw interrupt status register
| MIS           | 0x40            | 31:0  | Masked interrupt status register      | R   | Masked interrupt status register
| ICR           | 0x44            | 31:0  | Interrupt clear register              | W   | Interrupt clear register
| DMACR         | 0x48            | 31:0  | DMA control register                  | R/W | DMA control register
| ITCR          | 0x80            | 31:0  | Test control register                 | R/W | Test control register
| ITIP          | 0x84            | 31:0  | Integration test input register       | R   | Integration test input register
| ITOP          | 0x88            | 31:0  | Integration test output register      | R/W | Integration test output register
| TDR           | 0x8C            | 31:0  | Test data register                    | R/W | Test data register

@todo Add register details, see BCM2837 peripherals documentation, page 179 and further.

## GPIO pins (Raspberry Pi 5) {#RASPBERRY_PI_PL011_UART_GPIO_PINS_RASPBERRY_PI_5}

@todo Describe RPI 5 UARTs

