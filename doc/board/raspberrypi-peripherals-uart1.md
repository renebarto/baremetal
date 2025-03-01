# Raspberry Pi UART1 {#RASPBERRY_PI_UART1}

See [documentation](pdf/bcm2837-peripherals.pdf), page 10

Also refer to [aux device](#RASPBERRY_PI_AUXILIARY_PERIPHERAL).

| Register        | Address         | Bits  | Name                                  | Acc | Meaning |
|-----------------|-----------------|-------|---------------------------------------|-----|---------|
| RPI_AUX_MU_IO   | Base+0x00215040 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:0   | LS bits baudrate                      | R/W | Low 8 bits of baudrate, both R/W, if DLAB = 1
|                 |                 | 7:0   | Write data                            | W   | Data written to transmit FIFO, if DLAB = 0
|                 |                 | 7:0   | Read data                             | R   | Data read from receive FIFO, if DLAB = 0
| RPI_AUX_MU_IER  | Base+0x00215044 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:0   | MS bits baudrate                      | R/W | High 8 bits of baudrate, both R/W, if DLAB = 1
|                 |                 | 7:4   | -                                     | R/W | Ignored, read 0
|                 |                 | 3:2   | -                                     | R/W | Must be set to 1 to receive interrupts
|                 |                 | 1     | Enable transmit int                   | R/W | Enable transmit interrupts, if DLAB = 0
|                 |                 | 0     | Enable receive int                    | R/W | Enable receive interrupts, if DLAB = 0
| RPI_AUX_MU_IIR  | Base+0x00215048 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:6   | FIFO enables                          | R   | Always 11 as FIFO are always enabled
|                 |                 | 5:4   | -                                     | R   | Always 00
|                 |                 | 3     | -                                     | R   | Always 0 as UART has not timeout function
|                 |                 | 2:1   | Interrupt ID                          | R   | 00: no interrupts<br/>01: transmit buffer empty<br/>10: receive buffer full<br/>11: not possible
|                 |                 | 2:1   | Clear FIFO                            | W   | bit 2: clear transmit FIFO<br/>bit 1: clear receive FIFO
|                 |                 | 0     | Interrupt pending                     | R   | Interrupt pending<br/>0 = interrupt pending<br/>1 = no interrupt pending
| RPI_AUX_MU_LCR  | Base+0x0021504C | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7     | DLAB access                           | R/W | DLAB bit (0 = normal operation, 1 = set baud rate)<br/>See also AUX_MU_IER_REG and AUX_MU_IIR_REG
|                 |                 | 6     | Break                                 | R/W | Break, setting TX line low. If done for at least 12 bit times, this indicates a break condition (1 = pull TX low, 0 = no operation)
|                 |                 | 5:2   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 1:0   | Data size                             | R/W | Data size<br/>00 = 7 bits<br/>01 = invalid<br/>10 = invalid<br/>11 = 8 bits
| RPI_AUX_MU_MCR  | Base+0x00215050 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:2   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 1     | RTS                                   | R/W | RTS signal level, ignore for auto flow control (0 = high, 1 = low)
|                 |                 | 0     | -                                     | -   | Reserved (write 0, read X)
| RPI_AUX_MU_LSR  | Base+0x00215054 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7     | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 6     | Transmitter idle                      | R   | Transmit FIFO empty and transmitter idle (1 = idle, 0 = not idle). Reset: 1
|                 |                 | 5     | Transmitter empty                     | R   | Transmit FIFO empty (1 = empty, 0 = not empty). Reset: 0
|                 |                 | 4:2   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 1     | Receiver overrun                      | R/C | Receive buffer full and more data received (discard)<br/>Bit is reset on read. To perform a non-destructive read use AUX_MU_STAT_REG (1 = overrun, 0 = no overrun)
|                 |                 | 0     | Receive data ready                    | R   | FIFO contains received data (1 = data ready, 0 = no data ready)
| RPI_AUX_MU_MSR  | Base+0x00215058 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:6   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 5     | CTS status                            | R   | CTS status (1 = CTS low, 0 = CTS high). Reset: 1)
|                 |                 | 4:0   | -                                     | -   | Reserved (write 0, read X)
| RPI_AUX_MU_CNTL | Base+0x00215060 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7     | CTS assert level                      | R/W | CTS auto flow assert level (1 = low, 0 = high)<br/>Reset level: 0
|                 |                 | 6     | RTS assert level                      | R/W | RTS auto flow assert level (1 = low, 0 = high)<br/>Reset level: 0
|                 |                 | 5:4   | RTS auto flow level                   | R/W | RTS auto flow level<br/>00 = de-assert when FIFO has 3 empty spaces<br/>01 = de-assert when FIFO has 2 empty spaces<br/>10 = de-assert when FIFO has 1 empty space<br/>11 = de-assert when FIFO has 4 empty spaces<br/>Reset level: 00
|                 |                 | 3     | Enable CTS transmit auto flow control | R/W | Enable transmit auto flow-control using CTS<br/>1 = stop transmit when CTS de-asserted<br/>0 = ignore CTS status<br/>Reset level: 0
|                 |                 | 2     | Enable RTS receive auto flow control  | R/W | Enable receive auto flow-control using RTS<br/>1 = de-assert RTS if FIFO reaches auto flow level<br/>0 = RTS level determined by AUX_MU_MCR_REG bit 1<br/>Reset level: 0
|                 |                 | 1     | Transmit enable                       | R/W | Enable transmit<br/>1 = enabled<br/>0 = disabled<br/>Reset level: 1
|                 |                 | 0     | Receive enable                        | R/W | Enable receive<br/>1 = enabled<br/>0 = disabled<br/>Reset level: 1
| RPI_AUX_MU_BAUD | Base+0x00215068 | 31:16 | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 15:0  | Baudrate counter                      | R/W | Baud rate delay counter<br/>Reset level: 0
