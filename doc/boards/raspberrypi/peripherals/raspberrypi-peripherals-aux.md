# Raspberry PI auxiliary peripheral {#RPI_AUX}

See [documentation](BCM2837-peripherals.pdf), page 8

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
