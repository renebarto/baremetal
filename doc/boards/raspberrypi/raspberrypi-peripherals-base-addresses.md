# Raspberry Pi peripheral base addresses {#RASPBERRY_PI_PERIPHERAL_BASE_ADDRESSES}

@todo Detail out, add correct addresses

| Device                                | Base address RPI3B | Base address RPI4B | Base address RPI5 |
|---------------------------------------|--------------------|--------------------|-------------------|
| Local Interrupt                       | 40000000           |                    |                   |
| Timer                                 | 7E003000           | 7E003000           | 7C003000
| TXP                                   | 7E004000           |                    |                   |
| DMA                                   | 7E007000           |                    |                   |
| Interrupt controller                  | 7E00b200           |                    |                   |
| Mailbox                               | 7E00b840           |                    |                   |
| Mailbox                               | 7E00b880           |                    |                   |
| Watchdog                              | 7E100000           |                    |                   |
| PM                                    | 7E100000           |                    |                   |
| CPRMAN                                | 7E101000           |                    |                   |
| Random                                | 7E104000           |                    |                   |
| [GPIO](#RASPBERRY_PI_GPIO)            | 7E200000           |                    |                   |
| UART 0                                | 7E201000           |                    |                   |
| Bluetooth                             | 7E201000           |                    |                   |
| SD Host                               | 7E202000           |                    |                   |
| MMC 0                                 | 7E202000           |                    |                   |
| I2S                                   | 7E203000           |                    |                   |
| SPI 0                                 | 7E204000           |                    |                   |
| I2C                                   | 7E205000           |                    |                   |
| PixelValve 0                          | 7E206000           |                    |                   |
| PixelValve 1                          | 7E207000           |                    |                   |
| DPI                                   | 7E208000           |                    |                   |
| DSI 0                                 | 7E209000           |                    |                   |
| PWM                                   | 7E20C000           |                    |                   |
| Thermal                               | 7E212000           |                    |                   |
| Aux                                   | 7E215000           |                    |                   |
| [UART 1](#RASPBERRY_PI_UART1)         | 7E215040           |                    |                   |
| [SPI 1](#RASPBERRY_PI_SPI0)           | 7E215080           |                    |                   |
| [SPI 2](#RASPBERRY_PI_SPI1)           | 7E2150C0           |                    |                   |
| MMC 1                                 | 7E300000           |                    |                   |
| HVS                                   | 7E400000           |                    |                   |
| KMS                                   | 7E600000           |                    |                   |
| SMI                                   | 7E600000           |                    |                   |
| DSI 1                                 | 7E700000           |                    |                   |
| CSI 0                                 | 7E800000           |                    |                   |
| CSI 1                                 | 7E801000           |                    |                   |
| I2C 1 (ARM)                           | 7E804000           |                    |                   |
| I2C 2                                 | 7E805000           |                    |                   |
| Vec                                   | 7E806000           |                    |                   |
| PixelValve 2                          | 7E807000           |                    |                   |
| HDMI                                  | 7E902000           |                    |                   |
| Ethernet                              | 7E980000           |                    |                   |
| USB                                   | 7E980000           |                    |                   |
| V3D                                   | 7EC00000           |                    |                   |

Note: On Raspberry Pi 3, the device addresses are mapped to base 0x3F000000 instead of 0x7E000000. So even though the physical addresses are equal, the ARM uses a different base address.
