# Raspberry Pi 3B memory addresses

## Memory map

| 0x0000000000000000 | 0x0000000000001000 |

## Device addresses

| Device               | Base address RPI3B | Base address RPI4B | Base address RPI5 |
|----------------------|--------------------|--------------------|-------------------|
| Local Interrupt      | 40000000           |
| Timer                | 7e003000           | 7e003000           | 7c003000           |
| TXP                  | 7e004000           |
| DMA                  | 7e007000           |
| Interrupt controller | 7e00b200           |
| Mailbox              | 7e00b840           |
| Mailbox              | 7e00b880           |
| Watchdog             | 7e100000           |
| PM                   | 7e100000           |
| CPRMAN               | 7e101000           |
| Random               | 7e104000           |
| GPIO                 | 7e200000           |
| UART 0               | 7e201000           |
| Bluetooth            | 7e201000           |
| SD Host              | 7e202000           |
| MMC 0                | 7e202000           |
| I2S                  | 7e203000           |
| SPI 0                | 7e204000           |
| I2C                  | 7e205000           |
| PixelValve 0         | 7e206000           |
| PixelValve 1         | 7e207000           |
| DPI                  | 7e208000           |
| DSI 0                | 7e209000           |
| PWM                  | 7e20C000           |
| Thermal              | 7e212000           |
| Aux                  | 7e215000           |
| UART 1               | 7e215040           |
| SPI 1                | 7e215080           |
| SPI 2                | 7e2150C0           |
| MMC 1                | 7e300000           |
| HVS                  | 7e400000           |
| KMS                  | 7e600000           |
| SMI                  | 7e600000           |
| DSI 1                | 7e700000           |
| CSI 0                | 7e800000           |
| CSI 1                | 7e801000           |
| I2C 1 (ARM)          | 7e804000           |
| I2C 2                | 7e805000           |
| Vec                  | 7e806000           |
| PixelValve 2         | 7e807000           |
| HDMI                 | 7e902000           |
| Ethernet             | 7e980000           |
| USB                  | 7e980000           |
| V3D                  | 7eC00000           |


