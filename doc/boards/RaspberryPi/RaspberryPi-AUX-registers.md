# UART1 registers

See [documentation](BCM2837-peripherals.pdf), page 8

## AUX registers

| Register        | Address         | Bits  | Name                                  | Acc | Meaning |
|-----------------|-----------------|-------|---------------------------------------|-----|---------|
| AUX_IRQ         | Base+0x00215000 | 3     | Aux interrupt status                  | R   | Interrupt status for Mini UART, SPI 1, SPI 2
|                 |                 | 31:3  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 2     | SPI2 IRQ                              | R   | SPI2 interrupt pending<br/>1 = interrupt pending<br/>0 = no interrupt pending
|                 |                 | 1     | SPI1 IRQ                              | R   | SPI1 interrupt pending<br/>1 = interrupt pending<br/>0 = no interrupt pending
|                 |                 | 0     | Mini UART IRQ                         | R   | UART1 interrupt pending<br/>1 = interrupt pending<br/>0 = no interrupt pending
| AUX_ENABLES     | Base+0x00215004 | 3     | Auxiliary enables                     | R/W | Enable Mini UART, SPI 1, SPI 2
|                 |                 | 31:3  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 2     | SPI2 enable                           | R/W | SPI2 enable<br/>1 = enabled<br/>0 = disabled<br/>When disabled, all SPI 2 register access is disabled
|                 |                 | 1     | SPI1 enable                           | R/W | SPI1 enable<br/>1 = enabled<br/>0 = disabled<br/>When disabled, all SPI 1 register access is disabled
|                 |                 | 0     | Mini UART enable                      | R/W | UART1 enable<br/>1 = enabled<br/>0 = disabled<br/>When disabled, all UART1 register access is disabled

| Register        | Address         | Bits  | Name                                  | Acc | Meaning |
|-----------------|-----------------|-------|---------------------------------------|-----|---------|
| AUX_MU_IO_REG   | Base+0x00215040 | 8     | Mini Uart I/O Data                    | R/W | I/O register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:0   | LS bits baudrate                      | R/W | Low 8 bits of baudrate, both R/W, if DLAB = 1
|                 |                 | 7:0   | Write data                            | W   | Data written to transmit FIFO, if DLAB = 0
|                 |                 | 7:0   | Read data                             | R   | Data read from receive FIFO, if DLAB = 0
| AUX_MU_IER_REG  | Base+0x00215044 | 8     | Mini Uart Interrupt Enable            | R/W | Interrupt enable register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:0   | MS bits baudrate                      | R/W | High 8 bits of baudrate, both R/W, if DLAB = 1
|                 |                 | 7:4   | -                                     | R/W | Ignored, read 0
|                 |                 | 3:2   | -                                     | R/W | Must be set to 1 to receive interrupts
|                 |                 | 1     | Enable transmit int                   | R/W | Enable transmit interrupts, if DLAB = 0
|                 |                 | 0     | Enable receive int                    | R/W | Enable receive interrupts, if DLAB = 0
| AUX_MU_IIR_REG  | Base+0x00215048 | 8     | Mini Uart Interrupt Identify          | R/W | Interrupt status register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:6   | FIFO enables                          | R   | Always 11 as FIFO are always enabled
|                 |                 | 5:4   | -                                     | R   | Always 00
|                 |                 | 3     | -                                     | R   | Always 0 as UART has not timeout function
|                 |                 | 2:1   | Interrupt ID                          | R   | 00: no interrupts<br/>01: transmit buffer empty<br/>10: receive buffer full<br/>11: not possible
|                 |                 | 2:1   | Clear FIFO                            | W   | bit 2: clear transmit FIFO<br/>bit 1: clear receive FIFO
|                 |                 | 0     | Interrupt pending                     | R   | Interrupt pending<br/>0 = interrupt pending<br/>1 = no interrupt pending
| AUX_MU_LCR_REG  | Base+0x0021504C | 8     | Mini Uart Line Control                | R/W | Line control register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7     | DLAB access                           | R/W | DLAB bit (0 = normal operation, 1 = set baud rate)<br/>See also AUX_MU_IER_REG and AUX_MU_IIR_REG
|                 |                 | 6     | Break                                 | R/W | Break, setting TX line low. If done for at least 12 bit times, this indicates a break condition (1 = pull TX low, 0 = no operation)
|                 |                 | 5:2   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 1:0   | Data size                             | R/W | Data size<br/>00 = 7 bits<br/>01 = invalid<br/>10 = invalid<br/>11 = 8 bits
| AUX_MU_MCR_REG  | Base+0x00215050 | 8     | Mini Uart Modem Control               | R/W | Modem control register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:2   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 1     | RTS                                   | R/W | RTS signal level, ignore for auto flow control (0 = high, 1 = low)
|                 |                 | 0     | -                                     | -   | Reserved (write 0, read X)
| AUX_MU_LSR_REG  | Base+0x00215054 | 8     | Mini Uart Line Status                 | R   | Line status register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7     | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 6     | Transmitter idle                      | R   | Transmit FIFO empty and transmitter idle (1 = idle, 0 = not idle). Reset: 1
|                 |                 | 5     | Transmitter empty                     | R   | Transmit FIFO empty (1 = empty, 0 = not empty). Reset: 0
|                 |                 | 4:2   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 1     | Receiver overrun                      | R/C | Receive buffer full and more data received (discard)<br/>Bit is reset on read. To perform a non-destructive read use AUX_MU_STAT_REG (1 = overrun, 0 = no overrun)
|                 |                 | 0     | Receive data ready                    | R   | FIFO contains received data (1 = data ready, 0 = no data ready)
| AUX_MU_MSR_REG  | Base+0x00215058 | 8     | Mini Uart Modem Status                | R   | Modem status register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:6   | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 5     | CTS status                            | R   | CTS status (1 = CTS low, 0 = CTS high). Reset: 1)
|                 |                 | 4:0   | -                                     | -   | Reserved (write 0, read X)
| AUX_MU_SCRATCH  | Base+0x0021505C | 8     | Mini Uart Scratch                     | R/W | Scratch register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7:0   | Extra storage                         | R/W | Free storage space
| AUX_MU_CNTL_REG | Base+0x00215060 | 8     | Mini Uart Extra Control               | R/W | Exta control register
|                 |                 | 31:8  | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 7     | CTS assert level                      | R/W | CTS auto flow assert level (1 = low, 0 = high)<br/>Reset level: 0
|                 |                 | 6     | RTS assert level                      | R/W | RTS auto flow assert level (1 = low, 0 = high)<br/>Reset level: 0
|                 |                 | 5:4   | RTS auto flow level                   | R/W | RTS auto flow level<br/>00 = de-assert when FIFO has 3 empty spaces<br/>01 = de-assert when FIFO has 2 empty spaces<br/>10 = de-assert when FIFO has 1 empty space<br/>11 = de-assert when FIFO has 4 empty spaces<br/>Reset level: 00
|                 |                 | 3     | Enable CTS transmit auto flow control | R/W | Enable transmit auto flow-control using CTS<br/>1 = stop transmit when CTS de-asserted<br/>0 = ignore CTS status<br/>Reset level: 0
|                 |                 | 2     | Enable RTS receive auto flow control  | R/W | Enable receive auto flow-control using RTS<br/>1 = de-assert RTS if FIFO reaches auto flow level<br/>0 = RTS level determined by AUX_MU_MCR_REG bit 1<br/>Reset level: 0
|                 |                 | 1     | Transmit enable                       | R/W | Enable transmit<br/>1 = enabled<br/>0 = disabled<br/>Reset level: 1
|                 |                 | 0     | Receive enable                        | R/W | Enable receive<br/>1 = enabled<br/>0 = disabled<br/>Reset level: 1
| AUX_MU_STAT_REG | Base+0x00215060 | 32    | Mini Uart Extra Status                | R   | Extra status register
|                 |                 | 31:28 | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 27:24 | Transmit FIFO level                   | R   | Count of symbols in transmit FIFO (0-7)<br/>Reset level: 0
|                 |                 | 23:20 | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 19:16 | Receive FIFO level                    | R   | Count of symbols in receive FIFO (0-7)<br/>Reset level: 0
|                 |                 | 15:10 | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 9     | Transmitter done                      | R   | This bit is set if the transmitter is idle and the transmit FIFO is empty. It is a logic AND of bits 3 and 8<br/>Reset level: 1
|                 |                 | 8     | Transmit FIFO empty                   | R   | If this bit is set the transmitter FIFO is empty. Thus it can accept 8 symbols.<br/>Reset level: 0
|                 |                 | 7     | CTS line level                        | R   | UART1 CTS line level<br/>Reset level: 0
|                 |                 | 6     | RTS line level                        | R   | UART1 RTS line level<br/>Reset level: 0
|                 |                 | 5     | Transmit FIFO full                    | R   | If this bit is set the transmitter FIFO is full. This is the inverse of bit 1. Thus it can accept 8 symbols.<br/>Reset level: 0
|                 |                 | 4     | Receiver overrun                      | R   | This bit is set if there was a receiver overrun. That is: one or more characters arrived whilst the receive FIFO was full. The newly arrived characters have been discarded. This bit is cleared each time the AUX_MU_LSR_REG register is read<br/>Reset level: 0
|                 |                 | 3     | Transmitter is idle                   | R   | If this bit is set the transmitter is idle<br/>Reset level: 1
|                 |                 | 2     | Receive is idle                       | R   | If this bit is set the receiver is idle<br/>Reset level: 1
|                 |                 | 1     | Space available                       | R   | If this bit is set the mini UART transmitter FIFO can accept at least one more symbol<br/>Reset level: 0
|                 |                 | 0     | Symbol available                      | R   | If this bit is set the mini UART receive FIFO contains at least 1 symbol<br/>Reset level: 0
| AUX_MU_BAUD_REG | Base+0x00215064 | 16    | Mini Uart Baudrate                    | R/W | Baud rate register
|                 |                 | 31:16 | -                                     | -   | Reserved (write 0, read X)
|                 |                 | 15:0  | Baud rate                             | R/W | UART1 baud rate counter<br/>Reset level: 0

| Register           | Address         | Bits  | Name                                  | Acc | Meaning |
|--------------------|-----------------|-------|---------------------------------------|-----|---------|
| AUX_SPI0_CNTL0_REG | Base+0x00215080 | 32    | SPI 1 Control register 0              |     | 
| AUX_SPI0_CNTL1_REG | Base+0x00215084 | 8     | SPI 1 Control register 1              |     | 
| AUX_SPI0_STAT_REG  | Base+0x00215088 | 32    | SPI 1 Status                          |     | 
| AUX_SPI0_IO_REG    | Base+0x00215090 | 32    | SPI 1 Data                            |     | 
| AUX_SPI0_PEEK_REG  | Base+0x00215094 | 16    | SPI 1 Peek                            |     | 

| Register           | Address         | Bits  | Name                                  | Acc | Meaning |
|--------------------|-----------------|-------|---------------------------------------|-----|---------|
| AUX_SPI1_CNTL0_REG | Base+0x002150C0 | 32    | SPI 2 Control register 0              |     | 
| AUX_SPI1_CNTL1_REG | Base+0x002150C4 | 8     | SPI 2 Control register 1              |     | 
| AUX_SPI1_STAT_REG  | Base+0x002150C8 | 32    | SPI 2 Status                          |     | 
| AUX_SPI1_IO_REG    | Base+0x002150D0 | 32    | SPI 2 Data                            |     | 
| AUX_SPI1_PEEK_REG  | Base+0x002150D4 | 16    | SPI 2 Peek                            |     | 
