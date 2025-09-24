# Raspberry Pi Power Management {#RASPBERRY_PI_POWER_MANAGEMENT}

The Raspberry Pi power management is not well described, it is not part of the peripheral description of BCM2835/2836/2837/2711/2712.

Information is mostly extracted from watchdog driver source code in the kernel, even that is hard to find.

For more information see:
- https://elixir.bootlin.com/linux/latest/source/drivers/watchdog/bcm2835_wdt.c
- https://github.com/torvalds/linux/blob/master/drivers/watchdog/bcm2835_wdt.c 

| Register | Address         | Bits  | Name                   | Acc | Meaning |
|----------|-----------------|-------|------------------------|-----|---------|
| RSTC     | Base+0x0010001C | 31:24 | Unknown                | R/W | When writing must be set to watchdog magic number 0x5A
|          |                 | 23:6  | Unknown                | R/W | Unknown
|          |                 | 5:4   | Function               | R/W | 0x00 : Unknown
|          |                 |       |                        |     | 0x01 : Unknown
|          |                 |       |                        |     | 0x02 : Signifies that the watchdog timer is running on read, and needs this value to be started on write
|          |                 |       |                        |     | 0x03 : Configure, unknown
| RSTS     | Base+0x00100020 | 31:24 | Unknown                | R/W | When writing must be set to watchdog magic number 0x5A
|          |                 | 23:11 | Unknown                | R/W | Unknown
|          |                 | 10    | Sector bit 5           | R/W | Bit 5 of sector to boot from on write
|          |                 | 9     | Unknown                | R/W | Unknown
|          |                 | 8     | Sector bit 4           | R/W | Bit 4 of sector to boot from on write
|          |                 | 7     | Unknown                | R/W | Unknown
|          |                 | 6     | Sector bit 3           | R/W | Bit 3 of sector to boot from on write
|          |                 | 5     | Unknown                | R/W | Unknown
|          |                 | 4     | Sector bit 2           | R/W | Bit 2 of sector to boot from on write
|          |                 | 3     | Unknown                | R/W | Unknown
|          |                 | 2     | Sector bit 1           | R/W | Bit 1 of sector to boot from on write
|          |                 | 1     | Unknown                | R/W | Unknown
|          |                 | 0     | Sector bit 0           | R/W | Bit 0 of sector to boot from on write
| WDOG     | Base+0x00100024 | 31:24 | Unknown                | R/W | When writing must be set to watchdog magic number 0x5A
|          |                 | 23:20 | Unknown                | R/W | Unknown
|          |                 | 19:0  | Watchdog timeout value | R/W | On write sets watchdog timeout value in seconds, on read specifies timeout value left

Note: Sector number 0x3F is a special number, instructing the system to halt.

@todo Document Power management

