# Raspberry PI SPI1 {#RASPBERRY_PI_SPI1}

\todo Add register details

See [documentation](bcm2837-peripherals.pdf), page 20

Also refer to [aux device](#RASPBERRY_PI_AUXILIARY_PERIPHERAL).

| Register           | Address         | Bits  | Name                                  | Acc | Meaning |
|--------------------|-----------------|-------|---------------------------------------|-----|---------|
| AUX_SPI1_CNTL0_REG | Base+0x002150C0 | 32    | SPI 2 Control register 0              |     |         |
| AUX_SPI1_CNTL1_REG | Base+0x002150C4 | 8     | SPI 2 Control register 1              |     |         |
| AUX_SPI1_STAT_REG  | Base+0x002150C8 | 32    | SPI 2 Status                          |     |         |
| AUX_SPI1_IO_REG    | Base+0x002150D0 | 32    | SPI 2 Data                            |     |         |
| AUX_SPI1_PEEK_REG  | Base+0x002150D4 | 16    | SPI 2 Peek                            |     |         |
