# Raspberry PI SPI0 {#RPI_SPI0}

\todo Add register details

See [documentation](BCM2837-peripherals.pdf), page 20

Also refer to [aux device](#RPI_AUX).

| Register           | Address         | Bits  | Name                                  | Acc | Meaning |
|--------------------|-----------------|-------|---------------------------------------|-----|---------|
| AUX_SPI0_CNTL0_REG | Base+0x00215080 | 32    | SPI 1 Control register 0              |     | 
| AUX_SPI0_CNTL1_REG | Base+0x00215084 | 8     | SPI 1 Control register 1              |     | 
| AUX_SPI0_STAT_REG  | Base+0x00215088 | 32    | SPI 1 Status                          |     | 
| AUX_SPI0_IO_REG    | Base+0x00215090 | 32    | SPI 1 Data                            |     | 
| AUX_SPI0_PEEK_REG  | Base+0x00215094 | 16    | SPI 1 Peek                            |     | 

