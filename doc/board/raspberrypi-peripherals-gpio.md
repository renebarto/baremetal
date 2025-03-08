# Raspberry Pi GPIO {#RASPBERRY_PI_GPIO}

See [documentation](pdf/bcm2837-peripherals.pdf), page 90

Pinout:

 <img src="images/rpi-gpio-pinout.png" alt="Raspberry Pi GPIO pins" width="1000"/>

Raspberry Pi 3 has 54 GPIO, Raspberry Pi 4 and later have 57. Most however are not attached to the GPIO header.

| Register           | Address         | Bits  | Name                                  | Acc | Meaning |
|--------------------|-----------------|-------|---------------------------------------|-----|---------|
| RPI_GPIO_GPFSEL0   | Base+0x00200000 | 31:30 | -                                     | R   | Reserved. Reset level: 0
|                    |                 | 29:27 | FSEL9                                 | R/W | Function select GPIO9<br/>000 = input<br/>001 = output<br/>010 = alt function 5<br/>011 = alt function 4<br/>100 = alt function 0<br/>101 = alt function 1<br/>110 = alt function 2<br/>111 = alt function 3<br/>Reset level: 0
|                    |                 | 26:24 | FSEL8                                 | R/W | Function select GPIO8. Reset level: 0
|                    |                 | 23:21 | FSEL7                                 | R/W | Function select GPIO7. Reset level: 0
|                    |                 | 20:18 | FSEL6                                 | R/W | Function select GPIO6. Reset level: 0
|                    |                 | 17:15 | FSEL5                                 | R/W | Function select GPIO5. Reset level: 0
|                    |                 | 14:12 | FSEL4                                 | R/W | Function select GPIO4. Reset level: 0
|                    |                 | 11:9  | FSEL3                                 | R/W | Function select GPIO3. Reset level: 0
|                    |                 | 8:56  | FSEL2                                 | R/W | Function select GPIO2. Reset level: 0
|                    |                 | 5:3   | FSEL1                                 | R/W | Function select GPIO1. Reset level: 0
|                    |                 | 2:0   | FSEL0                                 | R/W | Function select GPIO0. Reset level: 0
| RPI_GPIO_GPFSEL1   | Base+0x00200004 | 31:30 | -                                     | R   | Reserved. Reset level: 0
|                    |                 | 29:27 | FSEL19                                | R/W | Function select GPIO19<br/>000 = input<br/>001 = output<br/>010 = alt function 5<br/>011 = alt function 4<br/>100 = alt function 0<br/>101 = alt function 1<br/>110 = alt function 2<br/>111 = alt function 3<br/>Reset level: 0
|                    |                 | 26:24 | FSEL18                                | R/W | Function select GPIO18. Reset level: 0
|                    |                 | 23:21 | FSEL17                                | R/W | Function select GPIO17. Reset level: 0
|                    |                 | 20:18 | FSEL16                                | R/W | Function select GPIO16. Reset level: 0
|                    |                 | 17:15 | FSEL15                                | R/W | Function select GPIO15. Reset level: 0
|                    |                 | 14:12 | FSEL14                                | R/W | Function select GPIO14. Reset level: 0
|                    |                 | 11:9  | FSEL13                                | R/W | Function select GPIO13. Reset level: 0
|                    |                 | 8:56  | FSEL12                                | R/W | Function select GPIO12. Reset level: 0
|                    |                 | 5:3   | FSEL11                                | R/W | Function select GPIO11. Reset level: 0
|                    |                 | 2:0   | FSEL10                                | R/W | Function select GPIO10. Reset level: 0
| RPI_GPIO_GPFSEL2   | Base+0x00200008 | 31:30 | -                                     | R   | Reserved. Reset level: 0
|                    |                 | 29:27 | FSEL29                                | R/W | Function select GPIO29<br/>000 = input<br/>001 = output<br/>010 = alt function 5<br/>011 = alt function 4<br/>100 = alt function 0<br/>101 = alt function 1<br/>110 = alt function 2<br/>111 = alt function 3<br/>Reset level: 0
|                    |                 | 26:24 | FSEL28                                | R/W | Function select GPIO28. Reset level: 0
|                    |                 | 23:21 | FSEL27                                | R/W | Function select GPIO27. Reset level: 0
|                    |                 | 20:18 | FSEL26                                | R/W | Function select GPIO26. Reset level: 0
|                    |                 | 17:15 | FSEL25                                | R/W | Function select GPIO25. Reset level: 0
|                    |                 | 14:12 | FSEL24                                | R/W | Function select GPIO24. Reset level: 0
|                    |                 | 11:9  | FSEL23                                | R/W | Function select GPIO23. Reset level: 0
|                    |                 | 8:56  | FSEL22                                | R/W | Function select GPIO22. Reset level: 0
|                    |                 | 5:3   | FSEL21                                | R/W | Function select GPIO21. Reset level: 0
|                    |                 | 2:0   | FSEL20                                | R/W | Function select GPIO20. Reset level: 0
| RPI_GPIO_GPFSEL3   | Base+0x0020000C | 31:30 | -                                     | R   | Reserved. Reset level: 0
|                    |                 | 29:27 | FSEL39                                | R/W | Function select GPIO39<br/>000 = input<br/>001 = output<br/>010 = alt function 5<br/>011 = alt function 4<br/>100 = alt function 0<br/>101 = alt function 1<br/>110 = alt function 2<br/>111 = alt function 3<br/>Reset level: 0
|                    |                 | 26:24 | FSEL38                                | R/W | Function select GPIO38. Reset level: 0
|                    |                 | 23:21 | FSEL37                                | R/W | Function select GPIO37. Reset level: 0
|                    |                 | 20:18 | FSEL36                                | R/W | Function select GPIO36. Reset level: 0
|                    |                 | 17:15 | FSEL35                                | R/W | Function select GPIO35. Reset level: 0
|                    |                 | 14:12 | FSEL34                                | R/W | Function select GPIO34. Reset level: 0
|                    |                 | 11:9  | FSEL33                                | R/W | Function select GPIO33. Reset level: 0
|                    |                 | 8:56  | FSEL32                                | R/W | Function select GPIO32. Reset level: 0
|                    |                 | 5:3   | FSEL31                                | R/W | Function select GPIO31. Reset level: 0
|                    |                 | 2:0   | FSEL30                                | R/W | Function select GPIO30. Reset level: 0
| RPI_GPIO_GPFSEL4   | Base+0x00200010 | 31:30 | -                                     | R   | Reserved. Reset level: 0
|                    |                 | 29:27 | FSEL49                                | R/W | Function select GPIO49<br/>000 = input<br/>001 = output<br/>010 = alt function 5<br/>011 = alt function 4<br/>100 = alt function 0<br/>101 = alt function 1<br/>110 = alt function 2<br/>111 = alt function 3<br/>Reset level: 0
|                    |                 | 26:24 | FSEL48                                | R/W | Function select GPIO48. Reset level: 0
|                    |                 | 23:21 | FSEL47                                | R/W | Function select GPIO47. Reset level: 0
|                    |                 | 20:18 | FSEL46                                | R/W | Function select GPIO46. Reset level: 0
|                    |                 | 17:15 | FSEL45                                | R/W | Function select GPIO45. Reset level: 0
|                    |                 | 14:12 | FSEL44                                | R/W | Function select GPIO44. Reset level: 0
|                    |                 | 11:9  | FSEL43                                | R/W | Function select GPIO43. Reset level: 0
|                    |                 | 8:56  | FSEL42                                | R/W | Function select GPIO42. Reset level: 0
|                    |                 | 5:3   | FSEL41                                | R/W | Function select GPIO41. Reset level: 0
|                    |                 | 2:0   | FSEL40                                | R/W | Function select GPIO40. Reset level: 0
| RPI_GPIO_GPFSEL5   | Base+0x00200014 | 31:21 | -                                     | R   | Reserved. Reset level: 0
|                    |                 | 20:18 | FSEL56 (RPI 4 and later only)         | R/W | Function select GPIO56<br/>000 = input<br/>001 = output<br/>010 = alt function 5<br/>011 = alt function 4<br/>100 = alt function 0<br/>101 = alt function 1<br/>110 = alt function 2<br/>111 = alt function 3<br/>Reset level: 0
|                    |                 | 17:15 | FSEL55 (RPI 4 and later only)         | R/W | Function select GPIO55. Reset level: 0
|                    |                 | 14:12 | FSEL54 (RPI 4 and later only)         | R/W | Function select GPIO54. Reset level: 0
|                    |                 | 11:9  | FSEL53                                | R/W | Function select GPIO53. Reset level: 0
|                    |                 | 8:56  | FSEL52                                | R/W | Function select GPIO52. Reset level: 0
|                    |                 | 5:3   | FSEL51                                | R/W | Function select GPIO51. Reset level: 0
|                    |                 | 2:0   | FSEL50                                | R/W | Function select GPIO50. Reset level: 0
| -                  | Base+0x00200018 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPSET0    | Base+0x0020001C | 31    | SET31                                 | R/W | Set GPIO31 (0 = no effect, 1 = set GPIO)
|                    |                 | 30    | SET30                                 | R/W | Set GPIO30 (0 = no effect, 1 = set GPIO)
|                    |                 | 29    | SET29                                 | R/W | Set GPIO29 (0 = no effect, 1 = set GPIO)
|                    |                 | 28    | SET28                                 | R/W | Set GPIO28 (0 = no effect, 1 = set GPIO)
|                    |                 | 27    | SET27                                 | R/W | Set GPIO27 (0 = no effect, 1 = set GPIO)
|                    |                 | 26    | SET26                                 | R/W | Set GPIO26 (0 = no effect, 1 = set GPIO)
|                    |                 | 25    | SET25                                 | R/W | Set GPIO25 (0 = no effect, 1 = set GPIO)
|                    |                 | 24    | SET24                                 | R/W | Set GPIO24 (0 = no effect, 1 = set GPIO)
|                    |                 | 23    | SET23                                 | R/W | Set GPIO23 (0 = no effect, 1 = set GPIO)
|                    |                 | 22    | SET22                                 | R/W | Set GPIO22 (0 = no effect, 1 = set GPIO)
|                    |                 | 21    | SET21                                 | R/W | Set GPIO21 (0 = no effect, 1 = set GPIO)
|                    |                 | 20    | SET20                                 | R/W | Set GPIO20 (0 = no effect, 1 = set GPIO)
|                    |                 | 19    | SET19                                 | R/W | Set GPIO19 (0 = no effect, 1 = set GPIO)
|                    |                 | 18    | SET18                                 | R/W | Set GPIO18 (0 = no effect, 1 = set GPIO)
|                    |                 | 17    | SET17                                 | R/W | Set GPIO17 (0 = no effect, 1 = set GPIO)
|                    |                 | 16    | SET16                                 | R/W | Set GPIO16 (0 = no effect, 1 = set GPIO)
|                    |                 | 15    | SET15                                 | R/W | Set GPIO15 (0 = no effect, 1 = set GPIO)
|                    |                 | 14    | SET14                                 | R/W | Set GPIO14 (0 = no effect, 1 = set GPIO)
|                    |                 | 13    | SET13                                 | R/W | Set GPIO13 (0 = no effect, 1 = set GPIO)
|                    |                 | 12    | SET12                                 | R/W | Set GPIO12 (0 = no effect, 1 = set GPIO)
|                    |                 | 11    | SET11                                 | R/W | Set GPIO11 (0 = no effect, 1 = set GPIO)
|                    |                 | 10    | SET10                                 | R/W | Set GPIO10 (0 = no effect, 1 = set GPIO)
|                    |                 | 9     | SET9                                  | R/W | Set GPIO9 (0 = no effect, 1 = set GPIO)
|                    |                 | 8     | SET8                                  | R/W | Set GPIO8 (0 = no effect, 1 = set GPIO)
|                    |                 | 7     | SET7                                  | R/W | Set GPIO7 (0 = no effect, 1 = set GPIO)
|                    |                 | 6     | SET6                                  | R/W | Set GPIO6 (0 = no effect, 1 = set GPIO)
|                    |                 | 5     | SET5                                  | R/W | Set GPIO5 (0 = no effect, 1 = set GPIO)
|                    |                 | 4     | SET4                                  | R/W | Set GPIO4 (0 = no effect, 1 = set GPIO)
|                    |                 | 3     | SET3                                  | R/W | Set GPIO3 (0 = no effect, 1 = set GPIO)
|                    |                 | 2     | SET2                                  | R/W | Set GPIO2 (0 = no effect, 1 = set GPIO)
|                    |                 | 1     | SET1                                  | R/W | Set GPIO1 (0 = no effect, 1 = set GPIO)
|                    |                 | 0     | SET0                                  | R/W | Set GPIO0 (0 = no effect, 1 = set GPIO)
| RPI_GPIO_GPSET1    | Base+0x00200020 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | SET56 (RPI 4 and later only)          | R/W | Set GPIO56 (0 = no effect, 1 = set GPIO)
|                    |                 | 23    | SET55 (RPI 4 and later only)          | R/W | Set GPIO55 (0 = no effect, 1 = set GPIO)
|                    |                 | 22    | SET54 (RPI 4 and later only)          | R/W | Set GPIO54 (0 = no effect, 1 = set GPIO)
|                    |                 | 21    | SET53                                 | R/W | Set GPIO53 (0 = no effect, 1 = set GPIO)
|                    |                 | 20    | SET52                                 | R/W | Set GPIO52 (0 = no effect, 1 = set GPIO)
|                    |                 | 19    | SET51                                 | R/W | Set GPIO51 (0 = no effect, 1 = set GPIO)
|                    |                 | 18    | SET50                                 | R/W | Set GPIO50 (0 = no effect, 1 = set GPIO)
|                    |                 | 17    | SET49                                 | R/W | Set GPIO49 (0 = no effect, 1 = set GPIO)
|                    |                 | 16    | SET48                                 | R/W | Set GPIO48 (0 = no effect, 1 = set GPIO)
|                    |                 | 15    | SET47                                 | R/W | Set GPIO47 (0 = no effect, 1 = set GPIO)
|                    |                 | 14    | SET46                                 | R/W | Set GPIO46 (0 = no effect, 1 = set GPIO)
|                    |                 | 13    | SET45                                 | R/W | Set GPIO45 (0 = no effect, 1 = set GPIO)
|                    |                 | 12    | SET44                                 | R/W | Set GPIO44 (0 = no effect, 1 = set GPIO)
|                    |                 | 11    | SET43                                 | R/W | Set GPIO43 (0 = no effect, 1 = set GPIO)
|                    |                 | 10    | SET42                                 | R/W | Set GPIO42 (0 = no effect, 1 = set GPIO)
|                    |                 | 9     | SET41                                 | R/W | Set GPIO41 (0 = no effect, 1 = set GPIO)
|                    |                 | 8     | SET40                                 | R/W | Set GPIO40 (0 = no effect, 1 = set GPIO)
|                    |                 | 7     | SET39                                 | R/W | Set GPIO39 (0 = no effect, 1 = set GPIO)
|                    |                 | 6     | SET38                                 | R/W | Set GPIO38 (0 = no effect, 1 = set GPIO)
|                    |                 | 5     | SET37                                 | R/W | Set GPIO37 (0 = no effect, 1 = set GPIO)
|                    |                 | 4     | SET36                                 | R/W | Set GPIO36 (0 = no effect, 1 = set GPIO)
|                    |                 | 3     | SET35                                 | R/W | Set GPIO35 (0 = no effect, 1 = set GPIO)
|                    |                 | 2     | SET34                                 | R/W | Set GPIO34 (0 = no effect, 1 = set GPIO)
|                    |                 | 1     | SET33                                 | R/W | Set GPIO33 (0 = no effect, 1 = set GPIO)
|                    |                 | 0     | SET32                                 | R/W | Set GPIO32 (0 = no effect, 1 = set GPIO)
| -                  | Base+0x00200024 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPCLR0    | Base+0x00200028 | 31    | CLR31                                 | R/W | Clear GPIO31 (0 = no effect, 1 = clear GPIO)
|                    |                 | 30    | CLR30                                 | R/W | Clear GPIO30 (0 = no effect, 1 = clear GPIO)
|                    |                 | 29    | CLR29                                 | R/W | Clear GPIO29 (0 = no effect, 1 = clear GPIO)
|                    |                 | 28    | CLR28                                 | R/W | Clear GPIO28 (0 = no effect, 1 = clear GPIO)
|                    |                 | 27    | CLR27                                 | R/W | Clear GPIO27 (0 = no effect, 1 = clear GPIO)
|                    |                 | 26    | CLR26                                 | R/W | Clear GPIO26 (0 = no effect, 1 = clear GPIO)
|                    |                 | 25    | CLR25                                 | R/W | Clear GPIO25 (0 = no effect, 1 = clear GPIO)
|                    |                 | 24    | CLR24                                 | R/W | Clear GPIO24 (0 = no effect, 1 = clear GPIO)
|                    |                 | 23    | CLR23                                 | R/W | Clear GPIO23 (0 = no effect, 1 = clear GPIO)
|                    |                 | 22    | CLR22                                 | R/W | Clear GPIO22 (0 = no effect, 1 = clear GPIO)
|                    |                 | 21    | CLR21                                 | R/W | Clear GPIO21 (0 = no effect, 1 = clear GPIO)
|                    |                 | 20    | CLR20                                 | R/W | Clear GPIO20 (0 = no effect, 1 = clear GPIO)
|                    |                 | 19    | CLR19                                 | R/W | Clear GPIO19 (0 = no effect, 1 = clear GPIO)
|                    |                 | 18    | CLR18                                 | R/W | Clear GPIO18 (0 = no effect, 1 = clear GPIO)
|                    |                 | 17    | CLR17                                 | R/W | Clear GPIO17 (0 = no effect, 1 = clear GPIO)
|                    |                 | 16    | CLR16                                 | R/W | Clear GPIO16 (0 = no effect, 1 = clear GPIO)
|                    |                 | 15    | CLR15                                 | R/W | Clear GPIO15 (0 = no effect, 1 = clear GPIO)
|                    |                 | 14    | CLR14                                 | R/W | Clear GPIO14 (0 = no effect, 1 = clear GPIO)
|                    |                 | 13    | CLR13                                 | R/W | Clear GPIO13 (0 = no effect, 1 = clear GPIO)
|                    |                 | 12    | CLR12                                 | R/W | Clear GPIO12 (0 = no effect, 1 = clear GPIO)
|                    |                 | 11    | CLR11                                 | R/W | Clear GPIO11 (0 = no effect, 1 = clear GPIO)
|                    |                 | 10    | CLR10                                 | R/W | Clear GPIO10 (0 = no effect, 1 = clear GPIO)
|                    |                 | 9     | CLR9                                  | R/W | Clear GPIO9 (0 = no effect, 1 = clear GPIO)
|                    |                 | 8     | CLR8                                  | R/W | Clear GPIO8 (0 = no effect, 1 = clear GPIO)
|                    |                 | 7     | CLR7                                  | R/W | Clear GPIO7 (0 = no effect, 1 = clear GPIO)
|                    |                 | 6     | CLR6                                  | R/W | Clear GPIO6 (0 = no effect, 1 = clear GPIO)
|                    |                 | 5     | CLR5                                  | R/W | Clear GPIO5 (0 = no effect, 1 = clear GPIO)
|                    |                 | 4     | CLR4                                  | R/W | Clear GPIO4 (0 = no effect, 1 = clear GPIO)
|                    |                 | 3     | CLR3                                  | R/W | Clear GPIO3 (0 = no effect, 1 = clear GPIO)
|                    |                 | 2     | CLR2                                  | R/W | Clear GPIO2 (0 = no effect, 1 = clear GPIO)
|                    |                 | 1     | CLR1                                  | R/W | Clear GPIO1 (0 = no effect, 1 = clear GPIO)
|                    |                 | 0     | CLR0                                  | R/W | Clear GPIO0 (0 = no effect, 1 = clear GPIO)
| RPI_GPIO_GPCLR1    | Base+0x0020002C | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | CLR56 (RPI 4 and later only)          | R/W | Clear GPIO56 (0 = no effect, 1 = clear GPIO)
|                    |                 | 23    | CLR55 (RPI 4 and later only)          | R/W | Clear GPIO55 (0 = no effect, 1 = clear GPIO)
|                    |                 | 22    | CLR54 (RPI 4 and later only)          | R/W | Clear GPIO54 (0 = no effect, 1 = clear GPIO)
|                    |                 | 21    | CLR53                                 | R/W | Clear GPIO53 (0 = no effect, 1 = clear GPIO)
|                    |                 | 20    | CLR52                                 | R/W | Clear GPIO52 (0 = no effect, 1 = clear GPIO)
|                    |                 | 19    | CLR51                                 | R/W | Clear GPIO51 (0 = no effect, 1 = clear GPIO)
|                    |                 | 18    | CLR50                                 | R/W | Clear GPIO50 (0 = no effect, 1 = clear GPIO)
|                    |                 | 17    | CLR49                                 | R/W | Clear GPIO49 (0 = no effect, 1 = clear GPIO)
|                    |                 | 16    | CLR48                                 | R/W | Clear GPIO48 (0 = no effect, 1 = clear GPIO)
|                    |                 | 15    | CLR47                                 | R/W | Clear GPIO47 (0 = no effect, 1 = clear GPIO)
|                    |                 | 14    | CLR46                                 | R/W | Clear GPIO46 (0 = no effect, 1 = clear GPIO)
|                    |                 | 13    | CLR45                                 | R/W | Clear GPIO45 (0 = no effect, 1 = clear GPIO)
|                    |                 | 12    | CLR44                                 | R/W | Clear GPIO44 (0 = no effect, 1 = clear GPIO)
|                    |                 | 11    | CLR43                                 | R/W | Clear GPIO43 (0 = no effect, 1 = clear GPIO)
|                    |                 | 10    | CLR42                                 | R/W | Clear GPIO42 (0 = no effect, 1 = clear GPIO)
|                    |                 | 9     | CLR41                                 | R/W | Clear GPIO41 (0 = no effect, 1 = clear GPIO)
|                    |                 | 8     | CLR40                                 | R/W | Clear GPIO40 (0 = no effect, 1 = clear GPIO)
|                    |                 | 7     | CLR39                                 | R/W | Clear GPIO39 (0 = no effect, 1 = clear GPIO)
|                    |                 | 6     | CLR38                                 | R/W | Clear GPIO38 (0 = no effect, 1 = clear GPIO)
|                    |                 | 5     | CLR37                                 | R/W | Clear GPIO37 (0 = no effect, 1 = clear GPIO)
|                    |                 | 4     | CLR36                                 | R/W | Clear GPIO36 (0 = no effect, 1 = clear GPIO)
|                    |                 | 3     | CLR35                                 | R/W | Clear GPIO35 (0 = no effect, 1 = clear GPIO)
|                    |                 | 2     | CLR34                                 | R/W | Clear GPIO34 (0 = no effect, 1 = clear GPIO)
|                    |                 | 1     | CLR33                                 | R/W | Clear GPIO33 (0 = no effect, 1 = clear GPIO)
|                    |                 | 0     | CLR32                                 | R/W | Clear GPIO32 (0 = no effect, 1 = clear GPIO)
| -                  | Base+0x00200030 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPLEV0    | Base+0x00200034 | 31    | LEV31                                 | R/W | Level GPIO31 (0 = low, 1 = high)
|                    |                 | 30    | LEV30                                 | R/W | Level GPIO30 (0 = low, 1 = high)
|                    |                 | 29    | LEV29                                 | R/W | Level GPIO29 (0 = low, 1 = high)
|                    |                 | 28    | LEV28                                 | R/W | Level GPIO28 (0 = low, 1 = high)
|                    |                 | 27    | LEV27                                 | R/W | Level GPIO27 (0 = low, 1 = high)
|                    |                 | 26    | LEV26                                 | R/W | Level GPIO26 (0 = low, 1 = high)
|                    |                 | 25    | LEV25                                 | R/W | Level GPIO25 (0 = low, 1 = high)
|                    |                 | 24    | LEV24                                 | R/W | Level GPIO24 (0 = low, 1 = high)
|                    |                 | 23    | LEV23                                 | R/W | Level GPIO23 (0 = low, 1 = high)
|                    |                 | 22    | LEV22                                 | R/W | Level GPIO22 (0 = low, 1 = high)
|                    |                 | 21    | LEV21                                 | R/W | Level GPIO21 (0 = low, 1 = high)
|                    |                 | 20    | LEV20                                 | R/W | Level GPIO20 (0 = low, 1 = high)
|                    |                 | 19    | LEV19                                 | R/W | Level GPIO19 (0 = low, 1 = high)
|                    |                 | 18    | LEV18                                 | R/W | Level GPIO18 (0 = low, 1 = high)
|                    |                 | 17    | LEV17                                 | R/W | Level GPIO17 (0 = low, 1 = high)
|                    |                 | 16    | LEV16                                 | R/W | Level GPIO16 (0 = low, 1 = high)
|                    |                 | 15    | LEV15                                 | R/W | Level GPIO15 (0 = low, 1 = high)
|                    |                 | 14    | LEV14                                 | R/W | Level GPIO14 (0 = low, 1 = high)
|                    |                 | 13    | LEV13                                 | R/W | Level GPIO13 (0 = low, 1 = high)
|                    |                 | 12    | LEV12                                 | R/W | Level GPIO12 (0 = low, 1 = high)
|                    |                 | 11    | LEV11                                 | R/W | Level GPIO11 (0 = low, 1 = high)
|                    |                 | 10    | LEV10                                 | R/W | Level GPIO10 (0 = low, 1 = high)
|                    |                 | 9     | LEV9                                  | R/W | Level GPIO9 (0 = low, 1 = high)
|                    |                 | 8     | LEV8                                  | R/W | Level GPIO8 (0 = low, 1 = high)
|                    |                 | 7     | LEV7                                  | R/W | Level GPIO7 (0 = low, 1 = high)
|                    |                 | 6     | LEV6                                  | R/W | Level GPIO6 (0 = low, 1 = high)
|                    |                 | 5     | LEV5                                  | R/W | Level GPIO5 (0 = low, 1 = high)
|                    |                 | 4     | LEV4                                  | R/W | Level GPIO4 (0 = low, 1 = high)
|                    |                 | 3     | LEV3                                  | R/W | Level GPIO3 (0 = low, 1 = high)
|                    |                 | 2     | LEV2                                  | R/W | Level GPIO2 (0 = low, 1 = high)
|                    |                 | 1     | LEV1                                  | R/W | Level GPIO1 (0 = low, 1 = high)
|                    |                 | 0     | LEV0                                  | R/W | Level GPIO0 (0 = low, 1 = high)
| RPI_GPIO_GPLEV1    | Base+0x00200038 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | LEV56 (RPI 4 and later only)          | R/W | Level GPIO56 (0 = low, 1 = high)
|                    |                 | 23    | LEV55 (RPI 4 and later only)          | R/W | Level GPIO55 (0 = low, 1 = high)
|                    |                 | 22    | LEV54 (RPI 4 and later only)          | R/W | Level GPIO54 (0 = low, 1 = high)
|                    |                 | 21    | LEV53                                 | R/W | Level GPIO53 (0 = low, 1 = high)
|                    |                 | 20    | LEV52                                 | R/W | Level GPIO52 (0 = low, 1 = high)
|                    |                 | 19    | LEV51                                 | R/W | Level GPIO51 (0 = low, 1 = high)
|                    |                 | 18    | LEV50                                 | R/W | Level GPIO50 (0 = low, 1 = high)
|                    |                 | 17    | LEV49                                 | R/W | Level GPIO49 (0 = low, 1 = high)
|                    |                 | 16    | LEV48                                 | R/W | Level GPIO48 (0 = low, 1 = high)
|                    |                 | 15    | LEV47                                 | R/W | Level GPIO47 (0 = low, 1 = high)
|                    |                 | 14    | LEV46                                 | R/W | Level GPIO46 (0 = low, 1 = high)
|                    |                 | 13    | LEV45                                 | R/W | Level GPIO45 (0 = low, 1 = high)
|                    |                 | 12    | LEV44                                 | R/W | Level GPIO44 (0 = low, 1 = high)
|                    |                 | 11    | LEV43                                 | R/W | Level GPIO43 (0 = low, 1 = high)
|                    |                 | 10    | LEV42                                 | R/W | Level GPIO42 (0 = low, 1 = high)
|                    |                 | 9     | LEV41                                 | R/W | Level GPIO41 (0 = low, 1 = high)
|                    |                 | 8     | LEV40                                 | R/W | Level GPIO40 (0 = low, 1 = high)
|                    |                 | 7     | LEV39                                 | R/W | Level GPIO39 (0 = low, 1 = high)
|                    |                 | 6     | LEV38                                 | R/W | Level GPIO38 (0 = low, 1 = high)
|                    |                 | 5     | LEV37                                 | R/W | Level GPIO37 (0 = low, 1 = high)
|                    |                 | 4     | LEV36                                 | R/W | Level GPIO36 (0 = low, 1 = high)
|                    |                 | 3     | LEV35                                 | R/W | Level GPIO35 (0 = low, 1 = high)
|                    |                 | 2     | LEV34                                 | R/W | Level GPIO34 (0 = low, 1 = high)
|                    |                 | 1     | LEV33                                 | R/W | Level GPIO33 (0 = low, 1 = high)
|                    |                 | 0     | LEV32                                 | R/W | Level GPIO32 (0 = low, 1 = high)
| -                  | Base+0x0020003C | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPEDS0    | Base+0x00200040 | 31    | EDS31                                 | R/W | Event detected GPIO31 (0 = event not detected, 1 = event detected)
|                    |                 | 30    | EDS30                                 | R/W | Event detected GPIO30 (0 = event not detected, 1 = event detected)
|                    |                 | 29    | EDS29                                 | R/W | Event detected GPIO29 (0 = event not detected, 1 = event detected)
|                    |                 | 28    | EDS28                                 | R/W | Event detected GPIO28 (0 = event not detected, 1 = event detected)
|                    |                 | 27    | EDS27                                 | R/W | Event detected GPIO27 (0 = event not detected, 1 = event detected)
|                    |                 | 26    | EDS26                                 | R/W | Event detected GPIO26 (0 = event not detected, 1 = event detected)
|                    |                 | 25    | EDS25                                 | R/W | Event detected GPIO25 (0 = event not detected, 1 = event detected)
|                    |                 | 24    | EDS24                                 | R/W | Event detected GPIO24 (0 = event not detected, 1 = event detected)
|                    |                 | 23    | EDS23                                 | R/W | Event detected GPIO23 (0 = event not detected, 1 = event detected)
|                    |                 | 22    | EDS22                                 | R/W | Event detected GPIO22 (0 = event not detected, 1 = event detected)
|                    |                 | 21    | EDS21                                 | R/W | Event detected GPIO21 (0 = event not detected, 1 = event detected)
|                    |                 | 20    | EDS20                                 | R/W | Event detected GPIO20 (0 = event not detected, 1 = event detected)
|                    |                 | 19    | EDS19                                 | R/W | Event detected GPIO19 (0 = event not detected, 1 = event detected)
|                    |                 | 18    | EDS18                                 | R/W | Event detected GPIO18 (0 = event not detected, 1 = event detected)
|                    |                 | 17    | EDS17                                 | R/W | Event detected GPIO17 (0 = event not detected, 1 = event detected)
|                    |                 | 16    | EDS16                                 | R/W | Event detected GPIO16 (0 = event not detected, 1 = event detected)
|                    |                 | 15    | EDS15                                 | R/W | Event detected GPIO15 (0 = event not detected, 1 = event detected)
|                    |                 | 14    | EDS14                                 | R/W | Event detected GPIO14 (0 = event not detected, 1 = event detected)
|                    |                 | 13    | EDS13                                 | R/W | Event detected GPIO13 (0 = event not detected, 1 = event detected)
|                    |                 | 12    | EDS12                                 | R/W | Event detected GPIO12 (0 = event not detected, 1 = event detected)
|                    |                 | 11    | EDS11                                 | R/W | Event detected GPIO11 (0 = event not detected, 1 = event detected)
|                    |                 | 10    | EDS10                                 | R/W | Event detected GPIO10 (0 = event not detected, 1 = event detected)
|                    |                 | 9     | EDS9                                  | R/W | Event detected GPIO9 (0 = event not detected, 1 = event detected)
|                    |                 | 8     | EDS8                                  | R/W | Event detected GPIO8 (0 = event not detected, 1 = event detected)
|                    |                 | 7     | EDS7                                  | R/W | Event detected GPIO7 (0 = event not detected, 1 = event detected)
|                    |                 | 6     | EDS6                                  | R/W | Event detected GPIO6 (0 = event not detected, 1 = event detected)
|                    |                 | 5     | EDS5                                  | R/W | Event detected GPIO5 (0 = event not detected, 1 = event detected)
|                    |                 | 4     | EDS4                                  | R/W | Event detected GPIO4 (0 = event not detected, 1 = event detected)
|                    |                 | 3     | EDS3                                  | R/W | Event detected GPIO3 (0 = event not detected, 1 = event detected)
|                    |                 | 2     | EDS2                                  | R/W | Event detected GPIO2 (0 = event not detected, 1 = event detected)
|                    |                 | 1     | EDS1                                  | R/W | Event detected GPIO1 (0 = event not detected, 1 = event detected)
|                    |                 | 0     | EDS0                                  | R/W | Event detected GPIO0 (0 = event not detected, 1 = event detected)
| RPI_GPIO_GPEDS1    | Base+0x00200044 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | EDS56 (RPI 4 and later only)          | R/W | Event detected GPIO56 (0 = event not detected, 1 = event detected)
|                    |                 | 23    | EDS55 (RPI 4 and later only)          | R/W | Event detected GPIO55 (0 = event not detected, 1 = event detected)
|                    |                 | 22    | EDS54 (RPI 4 and later only)          | R/W | Event detected GPIO54 (0 = event not detected, 1 = event detected)
|                    |                 | 21    | EDS53                                 | R/W | Event detected GPIO53 (0 = event not detected, 1 = event detected)
|                    |                 | 20    | EDS52                                 | R/W | Event detected GPIO52 (0 = event not detected, 1 = event detected)
|                    |                 | 19    | EDS51                                 | R/W | Event detected GPIO51 (0 = event not detected, 1 = event detected)
|                    |                 | 18    | EDS50                                 | R/W | Event detected GPIO50 (0 = event not detected, 1 = event detected)
|                    |                 | 17    | EDS49                                 | R/W | Event detected GPIO49 (0 = event not detected, 1 = event detected)
|                    |                 | 16    | EDS48                                 | R/W | Event detected GPIO48 (0 = event not detected, 1 = event detected)
|                    |                 | 15    | EDS47                                 | R/W | Event detected GPIO47 (0 = event not detected, 1 = event detected)
|                    |                 | 14    | EDS46                                 | R/W | Event detected GPIO46 (0 = event not detected, 1 = event detected)
|                    |                 | 13    | EDS45                                 | R/W | Event detected GPIO45 (0 = event not detected, 1 = event detected)
|                    |                 | 12    | EDS44                                 | R/W | Event detected GPIO44 (0 = event not detected, 1 = event detected)
|                    |                 | 11    | EDS43                                 | R/W | Event detected GPIO43 (0 = event not detected, 1 = event detected)
|                    |                 | 10    | EDS42                                 | R/W | Event detected GPIO42 (0 = event not detected, 1 = event detected)
|                    |                 | 9     | EDS41                                 | R/W | Event detected GPIO41 (0 = event not detected, 1 = event detected)
|                    |                 | 8     | EDS40                                 | R/W | Event detected GPIO40 (0 = event not detected, 1 = event detected)
|                    |                 | 7     | EDS39                                 | R/W | Event detected GPIO39 (0 = event not detected, 1 = event detected)
|                    |                 | 6     | EDS38                                 | R/W | Event detected GPIO38 (0 = event not detected, 1 = event detected)
|                    |                 | 5     | EDS37                                 | R/W | Event detected GPIO37 (0 = event not detected, 1 = event detected)
|                    |                 | 4     | EDS36                                 | R/W | Event detected GPIO36 (0 = event not detected, 1 = event detected)
|                    |                 | 3     | EDS35                                 | R/W | Event detected GPIO35 (0 = event not detected, 1 = event detected)
|                    |                 | 2     | EDS34                                 | R/W | Event detected GPIO34 (0 = event not detected, 1 = event detected)
|                    |                 | 1     | EDS33                                 | R/W | Event detected GPIO33 (0 = event not detected, 1 = event detected)
|                    |                 | 0     | EDS32                                 | R/W | Event detected GPIO32 (0 = event not detected, 1 = event detected)
| -                  | Base+0x00200048 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPREN0    | Base+0x0020004C | 31    | GPREN31                               | R/W | Rising edge detect enable GPIO31 (0 = enabled, 1 = disabled)
|                    |                 | 30    | GPREN30                               | R/W | Rising edge detect enable GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | GPREN29                               | R/W | Rising edge detect enable GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | GPREN28                               | R/W | Rising edge detect enable GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | GPREN27                               | R/W | Rising edge detect enable GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | GPREN26                               | R/W | Rising edge detect enable GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | GPREN25                               | R/W | Rising edge detect enable GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | GPREN24                               | R/W | Rising edge detect enable GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPREN23                               | R/W | Rising edge detect enable GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPREN22                               | R/W | Rising edge detect enable GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPREN21                               | R/W | Rising edge detect enable GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPREN20                               | R/W | Rising edge detect enable GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPREN19                               | R/W | Rising edge detect enable GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPREN18                               | R/W | Rising edge detect enable GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPREN17                               | R/W | Rising edge detect enable GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPREN16                               | R/W | Rising edge detect enable GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPREN15                               | R/W | Rising edge detect enable GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPREN14                               | R/W | Rising edge detect enable GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPREN13                               | R/W | Rising edge detect enable GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPREN12                               | R/W | Rising edge detect enable GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPREN11                               | R/W | Rising edge detect enable GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPREN10                               | R/W | Rising edge detect enable GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPREN9                                | R/W | Rising edge detect enable GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPREN8                                | R/W | Rising edge detect enable GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPREN7                                | R/W | Rising edge detect enable GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPREN6                                | R/W | Rising edge detect enable GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPREN5                                | R/W | Rising edge detect enable GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPREN4                                | R/W | Rising edge detect enable GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPREN3                                | R/W | Rising edge detect enable GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPREN2                                | R/W | Rising edge detect enable GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPREN1                                | R/W | Rising edge detect enable GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPREN0                                | R/W | Rising edge detect enable GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPREN1    | Base+0x00200050 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | GPREN56 (RPI 4 and later only)        | R/W | Rising edge detect enable GPIO56 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPREN55 (RPI 4 and later only)        | R/W | Rising edge detect enable GPIO55 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPREN54 (RPI 4 and later only)        | R/W | Rising edge detect enable GPIO54 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPREN53                               | R/W | Rising edge detect enable GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPREN52                               | R/W | Rising edge detect enable GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPREN51                               | R/W | Rising edge detect enable GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPREN50                               | R/W | Rising edge detect enable GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPREN49                               | R/W | Rising edge detect enable GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPREN48                               | R/W | Rising edge detect enable GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPREN47                               | R/W | Rising edge detect enable GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPREN46                               | R/W | Rising edge detect enable GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPREN45                               | R/W | Rising edge detect enable GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPREN44                               | R/W | Rising edge detect enable GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPREN43                               | R/W | Rising edge detect enable GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPREN42                               | R/W | Rising edge detect enable GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPREN41                               | R/W | Rising edge detect enable GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPREN40                               | R/W | Rising edge detect enable GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPREN39                               | R/W | Rising edge detect enable GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPREN38                               | R/W | Rising edge detect enable GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPREN37                               | R/W | Rising edge detect enable GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPREN36                               | R/W | Rising edge detect enable GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPREN35                               | R/W | Rising edge detect enable GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPREN34                               | R/W | Rising edge detect enable GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPREN33                               | R/W | Rising edge detect enable GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPREN32                               | R/W | Rising edge detect enable GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x00200054 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPFEN0    | Base+0x00200058 | 31    | GPFEN31                               | R/W | Falling edge detect enable GPIO31 (0 = enabled, 1 = disabled)
|                    |                 | 30    | GPFEN30                               | R/W | Falling edge detect enable GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | GPFEN29                               | R/W | Falling edge detect enable GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | GPFEN28                               | R/W | Falling edge detect enable GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | GPFEN27                               | R/W | Falling edge detect enable GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | GPFEN26                               | R/W | Falling edge detect enable GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | GPFEN25                               | R/W | Falling edge detect enable GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | GPFEN24                               | R/W | Falling edge detect enable GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPFEN23                               | R/W | Falling edge detect enable GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPFEN22                               | R/W | Falling edge detect enable GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPFEN21                               | R/W | Falling edge detect enable GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPFEN20                               | R/W | Falling edge detect enable GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPFEN19                               | R/W | Falling edge detect enable GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPFEN18                               | R/W | Falling edge detect enable GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPFEN17                               | R/W | Falling edge detect enable GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPFEN16                               | R/W | Falling edge detect enable GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPFEN15                               | R/W | Falling edge detect enable GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPFEN14                               | R/W | Falling edge detect enable GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPFEN13                               | R/W | Falling edge detect enable GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPFEN12                               | R/W | Falling edge detect enable GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPFEN11                               | R/W | Falling edge detect enable GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPFEN10                               | R/W | Falling edge detect enable GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPFEN9                                | R/W | Falling edge detect enable GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPFEN8                                | R/W | Falling edge detect enable GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPFEN7                                | R/W | Falling edge detect enable GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPFEN6                                | R/W | Falling edge detect enable GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPFEN5                                | R/W | Falling edge detect enable GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPFEN4                                | R/W | Falling edge detect enable GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPFEN3                                | R/W | Falling edge detect enable GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPFEN2                                | R/W | Falling edge detect enable GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPFEN1                                | R/W | Falling edge detect enable GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPFEN0                                | R/W | Falling edge detect enable GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPFEN1    | Base+0x0020005C | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | GPFEN56 (RPI 4 and later only)        | R/W | Falling edge detect enable GPIO56 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPFEN55 (RPI 4 and later only)        | R/W | Falling edge detect enable GPIO55 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPFEN54 (RPI 4 and later only)        | R/W | Falling edge detect enable GPIO54 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPFEN53                               | R/W | Falling edge detect enable GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPFEN52                               | R/W | Falling edge detect enable GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPFEN51                               | R/W | Falling edge detect enable GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPFEN50                               | R/W | Falling edge detect enable GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPFEN49                               | R/W | Falling edge detect enable GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPFEN48                               | R/W | Falling edge detect enable GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPFEN47                               | R/W | Falling edge detect enable GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPFEN46                               | R/W | Falling edge detect enable GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPFEN45                               | R/W | Falling edge detect enable GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPFEN44                               | R/W | Falling edge detect enable GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPFEN43                               | R/W | Falling edge detect enable GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPFEN42                               | R/W | Falling edge detect enable GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPFEN41                               | R/W | Falling edge detect enable GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPFEN40                               | R/W | Falling edge detect enable GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPFEN39                               | R/W | Falling edge detect enable GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPFEN38                               | R/W | Falling edge detect enable GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPFEN37                               | R/W | Falling edge detect enable GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPFEN36                               | R/W | Falling edge detect enable GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPFEN35                               | R/W | Falling edge detect enable GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPFEN34                               | R/W | Falling edge detect enable GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPFEN33                               | R/W | Falling edge detect enable GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPFEN32                               | R/W | Falling edge detect enable GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x00200060 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPHEN0    | Base+0x00200064 | 31    | GPHEN31                               | R/W | High level detect enable GPIO31 (0 = enabled, 1 = disabled)
|                    |                 | 30    | GPHEN30                               | R/W | High level detect enable GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | GPHEN29                               | R/W | High level detect enable GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | GPHEN28                               | R/W | High level detect enable GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | GPHEN27                               | R/W | High level detect enable GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | GPHEN26                               | R/W | High level detect enable GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | GPHEN25                               | R/W | High level detect enable GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | GPHEN24                               | R/W | High level detect enable GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPHEN23                               | R/W | High level detect enable GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPHEN22                               | R/W | High level detect enable GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPHEN21                               | R/W | High level detect enable GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPHEN20                               | R/W | High level detect enable GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPHEN19                               | R/W | High level detect enable GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPHEN18                               | R/W | High level detect enable GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPHEN17                               | R/W | High level detect enable GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPHEN16                               | R/W | High level detect enable GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPHEN15                               | R/W | High level detect enable GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPHEN14                               | R/W | High level detect enable GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPHEN13                               | R/W | High level detect enable GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPHEN12                               | R/W | High level detect enable GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPHEN11                               | R/W | High level detect enable GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPHEN10                               | R/W | High level detect enable GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPHEN9                                | R/W | High level detect enable GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPHEN8                                | R/W | High level detect enable GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPHEN7                                | R/W | High level detect enable GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPHEN6                                | R/W | High level detect enable GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPHEN5                                | R/W | High level detect enable GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPHEN4                                | R/W | High level detect enable GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPHEN3                                | R/W | High level detect enable GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPHEN2                                | R/W | High level detect enable GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPHEN1                                | R/W | High level detect enable GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPHEN0                                | R/W | High level detect enable GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPHEN1    | Base+0x00200068 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | GPHEN56 (RPI 4 and later only)        | R/W | High level detect enable GPIO56 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPHEN55 (RPI 4 and later only)        | R/W | High level detect enable GPIO55 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPHEN54 (RPI 4 and later only)        | R/W | High level detect enable GPIO54 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPHEN53                               | R/W | High level detect enable GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPHEN52                               | R/W | High level detect enable GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPHEN51                               | R/W | High level detect enable GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPHEN50                               | R/W | High level detect enable GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPHEN49                               | R/W | High level detect enable GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPHEN48                               | R/W | High level detect enable GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPHEN47                               | R/W | High level detect enable GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPHEN46                               | R/W | High level detect enable GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPHEN45                               | R/W | High level detect enable GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPHEN44                               | R/W | High level detect enable GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPHEN43                               | R/W | High level detect enable GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPHEN42                               | R/W | High level detect enable GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPHEN41                               | R/W | High level detect enable GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPHEN40                               | R/W | High level detect enable GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPHEN39                               | R/W | High level detect enable GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPHEN38                               | R/W | High level detect enable GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPHEN37                               | R/W | High level detect enable GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPHEN36                               | R/W | High level detect enable GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPHEN35                               | R/W | High level detect enable GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPHEN34                               | R/W | High level detect enable GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPHEN33                               | R/W | High level detect enable GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPHEN32                               | R/W | High level detect enable GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x0020006C | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPLEN0    | Base+0x00200070 | 31    | GPLEN31                               | R/W | Low level detect enable GPIO31 (0 = enabled, 1 = disabled)
|                    |                 | 30    | GPLEN30                               | R/W | Low level detect enable GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | GPLEN29                               | R/W | Low level detect enable GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | GPLEN28                               | R/W | Low level detect enable GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | GPLEN27                               | R/W | Low level detect enable GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | GPLEN26                               | R/W | Low level detect enable GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | GPLEN25                               | R/W | Low level detect enable GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | GPLEN24                               | R/W | Low level detect enable GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPLEN23                               | R/W | Low level detect enable GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPLEN22                               | R/W | Low level detect enable GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPLEN21                               | R/W | Low level detect enable GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPLEN20                               | R/W | Low level detect enable GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPLEN19                               | R/W | Low level detect enable GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPLEN18                               | R/W | Low level detect enable GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPLEN17                               | R/W | Low level detect enable GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPLEN16                               | R/W | Low level detect enable GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPLEN15                               | R/W | Low level detect enable GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPLEN14                               | R/W | Low level detect enable GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPLEN13                               | R/W | Low level detect enable GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPLEN12                               | R/W | Low level detect enable GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPLEN11                               | R/W | Low level detect enable GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPLEN10                               | R/W | Low level detect enable GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPLEN9                                | R/W | Low level detect enable GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPLEN8                                | R/W | Low level detect enable GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPLEN7                                | R/W | Low level detect enable GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPLEN6                                | R/W | Low level detect enable GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPLEN5                                | R/W | Low level detect enable GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPLEN4                                | R/W | Low level detect enable GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPLEN3                                | R/W | Low level detect enable GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPLEN2                                | R/W | Low level detect enable GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPLEN1                                | R/W | Low level detect enable GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPLEN0                                | R/W | Low level detect enable GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPLEN1    | Base+0x00200074 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | GPLEN56 (RPI 4 and later only)        | R/W | Low level detect enable GPIO56 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPLEN55 (RPI 4 and later only)        | R/W | Low level detect enable GPIO55 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPLEN54 (RPI 4 and later only)        | R/W | Low level detect enable GPIO54 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPLEN53                               | R/W | Low level detect enable GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPLEN52                               | R/W | Low level detect enable GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPLEN51                               | R/W | Low level detect enable GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPLEN50                               | R/W | Low level detect enable GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPLEN49                               | R/W | Low level detect enable GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPLEN48                               | R/W | Low level detect enable GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPLEN47                               | R/W | Low level detect enable GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPLEN46                               | R/W | Low level detect enable GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPLEN45                               | R/W | Low level detect enable GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPLEN44                               | R/W | Low level detect enable GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPLEN43                               | R/W | Low level detect enable GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPLEN42                               | R/W | Low level detect enable GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPLEN41                               | R/W | Low level detect enable GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPLEN40                               | R/W | Low level detect enable GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPLEN39                               | R/W | Low level detect enable GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPLEN38                               | R/W | Low level detect enable GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPLEN37                               | R/W | Low level detect enable GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPLEN36                               | R/W | Low level detect enable GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPLEN35                               | R/W | Low level detect enable GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPLEN34                               | R/W | Low level detect enable GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPLEN33                               | R/W | Low level detect enable GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPLEN32                               | R/W | Low level detect enable GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x00200078 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPAREN0   | Base+0x0020007C | 31    | GPAREN31                              | R/W | Asynchronous rising edge detect enable GPIO31 (0 = enabled, 1 = disabled)
|                    |                 | 30    | GPAREN30                              | R/W | Asynchronous rising edge detect enable GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | GPAREN29                              | R/W | Asynchronous rising edge detect enable GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | GPAREN28                              | R/W | Asynchronous rising edge detect enable GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | GPAREN27                              | R/W | Asynchronous rising edge detect enable GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | GPAREN26                              | R/W | Asynchronous rising edge detect enable GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | GPAREN25                              | R/W | Asynchronous rising edge detect enable GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | GPAREN24                              | R/W | Asynchronous rising edge detect enable GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPAREN23                              | R/W | Asynchronous rising edge detect enable GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPAREN22                              | R/W | Asynchronous rising edge detect enable GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPAREN21                              | R/W | Asynchronous rising edge detect enable GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPAREN20                              | R/W | Asynchronous rising edge detect enable GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPAREN19                              | R/W | Asynchronous rising edge detect enable GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPAREN18                              | R/W | Asynchronous rising edge detect enable GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPAREN17                              | R/W | Asynchronous rising edge detect enable GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPAREN16                              | R/W | Asynchronous rising edge detect enable GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPAREN15                              | R/W | Asynchronous rising edge detect enable GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPAREN14                              | R/W | Asynchronous rising edge detect enable GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPAREN13                              | R/W | Asynchronous rising edge detect enable GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPAREN12                              | R/W | Asynchronous rising edge detect enable GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPAREN11                              | R/W | Asynchronous rising edge detect enable GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPAREN10                              | R/W | Asynchronous rising edge detect enable GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPAREN9                               | R/W | Asynchronous rising edge detect enable GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPAREN8                               | R/W | Asynchronous rising edge detect enable GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPAREN7                               | R/W | Asynchronous rising edge detect enable GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPAREN6                               | R/W | Asynchronous rising edge detect enable GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPAREN5                               | R/W | Asynchronous rising edge detect enable GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPAREN4                               | R/W | Asynchronous rising edge detect enable GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPAREN3                               | R/W | Asynchronous rising edge detect enable GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPAREN2                               | R/W | Asynchronous rising edge detect enable GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPAREN1                               | R/W | Asynchronous rising edge detect enable GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPAREN0                               | R/W | Asynchronous rising edge detect enable GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPAREN1   | Base+0x00200080 | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | GPAREN56 (RPI 4 and later only)       | R/W | Asynchronous rising edge detect enable GPIO56 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPAREN55 (RPI 4 and later only)       | R/W | Asynchronous rising edge detect enable GPIO55 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPAREN54 (RPI 4 and later only)       | R/W | Asynchronous rising edge detect enable GPIO54 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPAREN53                              | R/W | Asynchronous rising edge detect enable GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPAREN52                              | R/W | Asynchronous rising edge detect enable GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPAREN51                              | R/W | Asynchronous rising edge detect enable GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPAREN50                              | R/W | Asynchronous rising edge detect enable GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPAREN49                              | R/W | Asynchronous rising edge detect enable GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPAREN48                              | R/W | Asynchronous rising edge detect enable GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPAREN47                              | R/W | Asynchronous rising edge detect enable GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPAREN46                              | R/W | Asynchronous rising edge detect enable GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPAREN45                              | R/W | Asynchronous rising edge detect enable GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPAREN44                              | R/W | Asynchronous rising edge detect enable GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPAREN43                              | R/W | Asynchronous rising edge detect enable GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPAREN42                              | R/W | Asynchronous rising edge detect enable GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPAREN41                              | R/W | Asynchronous rising edge detect enable GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPAREN40                              | R/W | Asynchronous rising edge detect enable GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPAREN39                              | R/W | Asynchronous rising edge detect enable GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPAREN38                              | R/W | Asynchronous rising edge detect enable GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPAREN37                              | R/W | Asynchronous rising edge detect enable GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPAREN36                              | R/W | Asynchronous rising edge detect enable GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPAREN35                              | R/W | Asynchronous rising edge detect enable GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPAREN34                              | R/W | Asynchronous rising edge detect enable GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPAREN33                              | R/W | Asynchronous rising edge detect enable GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPAREN32                              | R/W | Asynchronous rising edge detect enable GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x00200084 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPAFEN0   | Base+0x00200088 | 31    | GPAFEN31                              | R/W | Asynchronous falling edge detect enable GPIO31 (0 = enabled, 1 = disabled)
|                    |                 | 30    | GPAFEN30                              | R/W | Asynchronous falling edge detect enable GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | GPAFEN29                              | R/W | Asynchronous falling edge detect enable GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | GPAFEN28                              | R/W | Asynchronous falling edge detect enable GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | GPAFEN27                              | R/W | Asynchronous falling edge detect enable GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | GPAFEN26                              | R/W | Asynchronous falling edge detect enable GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | GPAFEN25                              | R/W | Asynchronous falling edge detect enable GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | GPAFEN24                              | R/W | Asynchronous falling edge detect enable GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPAFEN23                              | R/W | Asynchronous falling edge detect enable GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPAFEN22                              | R/W | Asynchronous falling edge detect enable GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPAFEN21                              | R/W | Asynchronous falling edge detect enable GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPAFEN20                              | R/W | Asynchronous falling edge detect enable GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPAFEN19                              | R/W | Asynchronous falling edge detect enable GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPAFEN18                              | R/W | Asynchronous falling edge detect enable GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPAFEN17                              | R/W | Asynchronous falling edge detect enable GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPAFEN16                              | R/W | Asynchronous falling edge detect enable GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPAFEN15                              | R/W | Asynchronous falling edge detect enable GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPAFEN14                              | R/W | Asynchronous falling edge detect enable GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPAFEN13                              | R/W | Asynchronous falling edge detect enable GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPAFEN12                              | R/W | Asynchronous falling edge detect enable GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPAFEN11                              | R/W | Asynchronous falling edge detect enable GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPAFEN10                              | R/W | Asynchronous falling edge detect enable GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPAFEN9                               | R/W | Asynchronous falling edge detect enable GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPAFEN8                               | R/W | Asynchronous falling edge detect enable GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPAFEN7                               | R/W | Asynchronous falling edge detect enable GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPAFEN6                               | R/W | Asynchronous falling edge detect enable GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPAFEN5                               | R/W | Asynchronous falling edge detect enable GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPAFEN4                               | R/W | Asynchronous falling edge detect enable GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPAFEN3                               | R/W | Asynchronous falling edge detect enable GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPAFEN2                               | R/W | Asynchronous falling edge detect enable GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPAFEN1                               | R/W | Asynchronous falling edge detect enable GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPAFEN0                               | R/W | Asynchronous falling edge detect enable GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPAFEN1   | Base+0x0020008C | 31:25 | -                                     | R/W | Unused
|                    |                 | 24    | GPAFEN56 (RPI 4 and later only)       | R/W | Asynchronous falling edge detect enable GPIO56 (0 = enabled, 1 = disabled)
|                    |                 | 23    | GPAFEN55 (RPI 4 and later only)       | R/W | Asynchronous falling edge detect enable GPIO55 (0 = enabled, 1 = disabled)
|                    |                 | 22    | GPAFEN54 (RPI 4 and later only)       | R/W | Asynchronous falling edge detect enable GPIO54 (0 = enabled, 1 = disabled)
|                    |                 | 21    | GPAFEN53                              | R/W | Asynchronous falling edge detect enable GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | GPAFEN52                              | R/W | Asynchronous falling edge detect enable GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | GPAFEN51                              | R/W | Asynchronous falling edge detect enable GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | GPAFEN50                              | R/W | Asynchronous falling edge detect enable GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | GPAFEN49                              | R/W | Asynchronous falling edge detect enable GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | GPAFEN48                              | R/W | Asynchronous falling edge detect enable GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | GPAFEN47                              | R/W | Asynchronous falling edge detect enable GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | GPAFEN46                              | R/W | Asynchronous falling edge detect enable GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | GPAFEN45                              | R/W | Asynchronous falling edge detect enable GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | GPAFEN44                              | R/W | Asynchronous falling edge detect enable GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | GPAFEN43                              | R/W | Asynchronous falling edge detect enable GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | GPAFEN42                              | R/W | Asynchronous falling edge detect enable GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | GPAFEN41                              | R/W | Asynchronous falling edge detect enable GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | GPAFEN40                              | R/W | Asynchronous falling edge detect enable GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | GPAFEN39                              | R/W | Asynchronous falling edge detect enable GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | GPAFEN38                              | R/W | Asynchronous falling edge detect enable GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | GPAFEN37                              | R/W | Asynchronous falling edge detect enable GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | GPAFEN36                              | R/W | Asynchronous falling edge detect enable GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | GPAFEN35                              | R/W | Asynchronous falling edge detect enable GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | GPAFEN34                              | R/W | Asynchronous falling edge detect enable GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | GPAFEN33                              | R/W | Asynchronous falling edge detect enable GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | GPAFEN32                              | R/W | Asynchronous falling edge detect enable GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x00200090 | 31:0  | -                                     | -   | Reserved
| RPI_GPIO_GPPUD     | Base+0x00200094 | 31:2  | -                                     | -   | Unused
|                    |                 | 1:0   | PUD       (RPI 3)                     | R/W | Pull up/down mode<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
| RPI_GPIO_GPPUDCLK0 | Base+0x00200098 | 31    | PUDCLK31  (RPI 3)                     | R/W | Pull up/down set GPIO31 (0 = no effect, 1 = assert change on GPIO)<br/>Process:<br/>1. Write to GPPUD register to set pull up/ down<br/>2. Wait 150 cycles<br/>3. Write to GPPUDCLK0/1 to assert the clock for selected GPIO(s)<br/>4. Wait 150 cycles<br/>5. Write 00 to GPPUD to remove control signal<br/>6. Write to GPPUDCLK0/1 to remove the clock for selected GPIO(s)
|                    |                 | 30    | PUDCLK30  (RPI 3)                     | R/W | Pull up/down set GPIO30 (0 = enabled, 1 = disabled)
|                    |                 | 29    | PUDCLK29  (RPI 3)                     | R/W | Pull up/down set GPIO29 (0 = enabled, 1 = disabled)
|                    |                 | 28    | PUDCLK28  (RPI 3)                     | R/W | Pull up/down set GPIO28 (0 = enabled, 1 = disabled)
|                    |                 | 27    | PUDCLK27  (RPI 3)                     | R/W | Pull up/down set GPIO27 (0 = enabled, 1 = disabled)
|                    |                 | 26    | PUDCLK26  (RPI 3)                     | R/W | Pull up/down set GPIO26 (0 = enabled, 1 = disabled)
|                    |                 | 25    | PUDCLK25  (RPI 3)                     | R/W | Pull up/down set GPIO25 (0 = enabled, 1 = disabled)
|                    |                 | 24    | PUDCLK24  (RPI 3)                     | R/W | Pull up/down set GPIO24 (0 = enabled, 1 = disabled)
|                    |                 | 23    | PUDCLK23  (RPI 3)                     | R/W | Pull up/down set GPIO23 (0 = enabled, 1 = disabled)
|                    |                 | 22    | PUDCLK22  (RPI 3)                     | R/W | Pull up/down set GPIO22 (0 = enabled, 1 = disabled)
|                    |                 | 21    | PUDCLK21  (RPI 3)                     | R/W | Pull up/down set GPIO21 (0 = enabled, 1 = disabled)
|                    |                 | 20    | PUDCLK20  (RPI 3)                     | R/W | Pull up/down set GPIO20 (0 = enabled, 1 = disabled)
|                    |                 | 19    | PUDCLK19  (RPI 3)                     | R/W | Pull up/down set GPIO19 (0 = enabled, 1 = disabled)
|                    |                 | 18    | PUDCLK18  (RPI 3)                     | R/W | Pull up/down set GPIO18 (0 = enabled, 1 = disabled)
|                    |                 | 17    | PUDCLK17  (RPI 3)                     | R/W | Pull up/down set GPIO17 (0 = enabled, 1 = disabled)
|                    |                 | 16    | PUDCLK16  (RPI 3)                     | R/W | Pull up/down set GPIO16 (0 = enabled, 1 = disabled)
|                    |                 | 15    | PUDCLK15  (RPI 3)                     | R/W | Pull up/down set GPIO15 (0 = enabled, 1 = disabled)
|                    |                 | 14    | PUDCLK14  (RPI 3)                     | R/W | Pull up/down set GPIO14 (0 = enabled, 1 = disabled)
|                    |                 | 13    | PUDCLK13  (RPI 3)                     | R/W | Pull up/down set GPIO13 (0 = enabled, 1 = disabled)
|                    |                 | 12    | PUDCLK12  (RPI 3)                     | R/W | Pull up/down set GPIO12 (0 = enabled, 1 = disabled)
|                    |                 | 11    | PUDCLK11  (RPI 3)                     | R/W | Pull up/down set GPIO11 (0 = enabled, 1 = disabled)
|                    |                 | 10    | PUDCLK10  (RPI 3)                     | R/W | Pull up/down set GPIO10 (0 = enabled, 1 = disabled)
|                    |                 | 9     | PUDCLK9   (RPI 3)                     | R/W | Pull up/down set GPIO9 (0 = enabled, 1 = disabled)
|                    |                 | 8     | PUDCLK8   (RPI 3)                     | R/W | Pull up/down set GPIO8 (0 = enabled, 1 = disabled)
|                    |                 | 7     | PUDCLK7   (RPI 3)                     | R/W | Pull up/down set GPIO7 (0 = enabled, 1 = disabled)
|                    |                 | 6     | PUDCLK6   (RPI 3)                     | R/W | Pull up/down set GPIO6 (0 = enabled, 1 = disabled)
|                    |                 | 5     | PUDCLK5   (RPI 3)                     | R/W | Pull up/down set GPIO5 (0 = enabled, 1 = disabled)
|                    |                 | 4     | PUDCLK4   (RPI 3)                     | R/W | Pull up/down set GPIO4 (0 = enabled, 1 = disabled)
|                    |                 | 3     | PUDCLK3   (RPI 3)                     | R/W | Pull up/down set GPIO3 (0 = enabled, 1 = disabled)
|                    |                 | 2     | PUDCLK2   (RPI 3)                     | R/W | Pull up/down set GPIO2 (0 = enabled, 1 = disabled)
|                    |                 | 1     | PUDCLK1   (RPI 3)                     | R/W | Pull up/down set GPIO1 (0 = enabled, 1 = disabled)
|                    |                 | 0     | PUDCLK0   (RPI 3)                     | R/W | Pull up/down set GPIO0 (0 = enabled, 1 = disabled)
| RPI_GPIO_GPPUDCLK1 | Base+0x0020009C | 31:22 | -         (RPI 3)                     | R/W | Unused
|                    |                 | 21    | PUDCLK53  (RPI 3)                     | R/W | Pull up/down set GPIO53 (0 = enabled, 1 = disabled)
|                    |                 | 20    | PUDCLK52  (RPI 3)                     | R/W | Pull up/down set GPIO52 (0 = enabled, 1 = disabled)
|                    |                 | 19    | PUDCLK51  (RPI 3)                     | R/W | Pull up/down set GPIO51 (0 = enabled, 1 = disabled)
|                    |                 | 18    | PUDCLK50  (RPI 3)                     | R/W | Pull up/down set GPIO50 (0 = enabled, 1 = disabled)
|                    |                 | 17    | PUDCLK49  (RPI 3)                     | R/W | Pull up/down set GPIO49 (0 = enabled, 1 = disabled)
|                    |                 | 16    | PUDCLK48  (RPI 3)                     | R/W | Pull up/down set GPIO48 (0 = enabled, 1 = disabled)
|                    |                 | 15    | PUDCLK47  (RPI 3)                     | R/W | Pull up/down set GPIO47 (0 = enabled, 1 = disabled)
|                    |                 | 14    | PUDCLK46  (RPI 3)                     | R/W | Pull up/down set GPIO46 (0 = enabled, 1 = disabled)
|                    |                 | 13    | PUDCLK45  (RPI 3)                     | R/W | Pull up/down set GPIO45 (0 = enabled, 1 = disabled)
|                    |                 | 12    | PUDCLK44  (RPI 3)                     | R/W | Pull up/down set GPIO44 (0 = enabled, 1 = disabled)
|                    |                 | 11    | PUDCLK43  (RPI 3)                     | R/W | Pull up/down set GPIO43 (0 = enabled, 1 = disabled)
|                    |                 | 10    | PUDCLK42  (RPI 3)                     | R/W | Pull up/down set GPIO42 (0 = enabled, 1 = disabled)
|                    |                 | 9     | PUDCLK41  (RPI 3)                     | R/W | Pull up/down set GPIO41 (0 = enabled, 1 = disabled)
|                    |                 | 8     | PUDCLK40  (RPI 3)                     | R/W | Pull up/down set GPIO40 (0 = enabled, 1 = disabled)
|                    |                 | 7     | PUDCLK39  (RPI 3)                     | R/W | Pull up/down set GPIO39 (0 = enabled, 1 = disabled)
|                    |                 | 6     | PUDCLK38  (RPI 3)                     | R/W | Pull up/down set GPIO38 (0 = enabled, 1 = disabled)
|                    |                 | 5     | PUDCLK37  (RPI 3)                     | R/W | Pull up/down set GPIO37 (0 = enabled, 1 = disabled)
|                    |                 | 4     | PUDCLK36  (RPI 3)                     | R/W | Pull up/down set GPIO36 (0 = enabled, 1 = disabled)
|                    |                 | 3     | PUDCLK35  (RPI 3)                     | R/W | Pull up/down set GPIO35 (0 = enabled, 1 = disabled)
|                    |                 | 2     | PUDCLK34  (RPI 3)                     | R/W | Pull up/down set GPIO34 (0 = enabled, 1 = disabled)
|                    |                 | 1     | PUDCLK33  (RPI 3)                     | R/W | Pull up/down set GPIO33 (0 = enabled, 1 = disabled)
|                    |                 | 0     | PUDCLK32  (RPI 3)                     | R/W | Pull up/down set GPIO32 (0 = enabled, 1 = disabled)
| -                  | Base+0x002000A0 | 31:0  | -                                     | -   | Reserved
| -                  | Base+0x002000A4 | 31:0  | -                                     | -   | Reserved
| -                  | Base+0x002000A8 | 31:0  | -                                     | -   | Reserved
| -                  | Base+0x002000AC | 31:0  | -                                     | -   | Reserved
| -                  | Base+0x002000B0 | 31:0  | Undocumented                          | -   | Test
| RPI_GPIO_GPPINMUXSD| Base+0x002000D0 | -     | Undocumented                          | -   | GPIO pull up/down pin multiplexer register
| RPI_GPIO_GPPUPPDN0 | Base+0x002000E4 | 31:30 | PUD15 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO15<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 29:28 | PUD14 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO14<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 27:26 | PUD13 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO13<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 25:24 | PUD12 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO12<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 23:22 | PUD11 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO11<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 21:20 | PUD10 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO10<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 19:18 | PUD9  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO9<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 17:16 | PUD8  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO8<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 15:14 | PUD7  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO7<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 13:12 | PUD6  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO6<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 11:10 | PUD5  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO5<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 9:8   | PUD4  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO4<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 7:6   | PUD3  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO3<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 5:4   | PUD2  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO2<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 3:2   | PUD1  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO1<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 1:0   | PUD0  (RPI 4 and later only)          | R/W | Pull up/down mode GPIO0<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
| RPI_GPIO_GPPUPPDN1 | Base+0x002000E8 | 31:30 | PUD31 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO31<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 29:28 | PUD30 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO30<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 27:26 | PUD29 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO29<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 25:24 | PUD28 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO28<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 23:22 | PUD27 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO27<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 21:20 | PUD26 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO26<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 19:18 | PUD25 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO25<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 17:16 | PUD24 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO24<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 15:14 | PUD23 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO23<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 13:12 | PUD22 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO22<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 11:10 | PUD21 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO21<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 9:8   | PUD20 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO20<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 7:6   | PUD19 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO19<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 5:4   | PUD18 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO18<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 3:2   | PUD17 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO17<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 1:0   | PUD16 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO16<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
| RPI_GPIO_GPPUPPDN2 | Base+0x002000EC | 31:30 | PUD47 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO47<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 29:28 | PUD46 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO46<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 27:26 | PUD45 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO45<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 25:24 | PUD44 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO44<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 23:22 | PUD43 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO43<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 21:20 | PUD42 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO42<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 19:18 | PUD41 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO41<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 17:16 | PUD40 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO40<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 15:14 | PUD39 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO39<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 13:12 | PUD38 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO38<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 11:10 | PUD37 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO37<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 9:8   | PUD36 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO36<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 7:6   | PUD35 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO35<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 5:4   | PUD34 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO34<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 3:2   | PUD33 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO33<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 1:0   | PUD32 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO32<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
| RPI_GPIO_GPPUPPDN3 | Base+0x002000F0 | 31:18 | -                                     | R   | Unused
|                    |                 | 17:16 | PUD56 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO56<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 15:14 | PUD55 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO55<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 13:12 | PUD54 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO54<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 11:10 | PUD53 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO53<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 9:8   | PUD52 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO52<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 7:6   | PUD51 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO51<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 5:4   | PUD50 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO50<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 3:2   | PUD49 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO49<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved
|                    |                 | 1:0   | PUD48 (RPI 4 and later only)          | R/W | Pull up/down mode GPIO48<br/>00 = Off (disable pull up/down)<br/>01 - enable pull down control<br/>10 - enable pull up control<br/>11 = reserved

## Pull up-down mode {#RASPBERRY_PI_GPIO_PULL_UP_DOWN_MODE}

Details GPIO pin pull up/down mode

| Value | Mode                       |
|-------|----------------------------|
| 00    | Off (disable pull up/down) |
| 01    | Enable pull down control   |
| 10    | Enable pull up control     |
| 11    | Reserved                   |

## Alternative functions for GPIO {#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO}

See [documentation](pdf/bcm2837-peripherals.pdf), page 102

Every GPIO may have one of multiple functions
- Input
- Output
- One of six alternative functions (0-5)

These functions are coded as follows:

| Value | Functions      |
|-------|----------------|
| 000   | input          |
| 001   | output         |
| 010   | alt function 5 |
| 011   | alt function 4 |
| 100   | alt function 0 |
| 101   | alt function 1 |
| 110   | alt function 2 |
| 111   | alt function 3 |

### Raspberry Pi 3 {#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO_RASPBERRY_PI_3}

<table>
<caption id="GPIO_alternative_functions_rpi3"></caption>
<tr><th>GPIO<th>Pull up / down<th>Alt 0     <th>Alt 1        <th>Alt 2     <th>Alt 3           <th>Alt 4     <th>Alt 5</tr>
<tr><td>0 <td>High<td>SDA0      <td>SA5          <td>reserved                                             </tr>
<tr><td>1 <td>High<td>SCL0      <td>SA4          <td>reserved                                             </tr>
<tr><td>2 <td>High<td>SDA1      <td>SA3          <td>reserved                                             </tr>
<tr><td>3 <td>High<td>SCL1      <td>SA2          <td>reserved                                             </tr>
<tr><td>4 <td>High<td>GPCLK0    <td>SA1          <td>reserved<td>                <td>          <td>ARM_TDI</tr>
<tr><td>5 <td>High<td>GPCLK1    <td>SA0          <td>reserved<td>                <td>          <td>ARM_TDO</tr>
<tr><td>6 <td>High<td>GPCLK2    <td>SOE_N / SE   <td>reserved<td>                <td>          <td>ARM_RTC</tr>
<tr><td>7 <td>High<td>SPI0_CE1_N<td>SWE_N / SRW_N<td>reserved                                             </tr>
<tr><td>8 <td>High<td>SPI0_CE0_N<td>SD0          <td>reserved                                             </tr>
<tr><td>9 <td>Low <td>SPI0_MISO <td>SD1          <td>reserved                                             </tr>
<tr><td>10<td>Low <td>SPI0_MOSI <td>SD2          <td>reserved                                             </tr>
<tr><td>11<td>Low <td>SPI0_SCLK <td>SD3          <td>reserved                                             </tr>
<tr><td>12<td>Low <td>PWM0      <td>SD4          <td>reserved<td>                <td>          <td>ARM_TMS</tr>
<tr><td>13<td>Low <td>PWM1      <td>SD5          <td>reserved<td>                <td>          <td>ARM_TCK</tr>
<tr><td>14<td>Low <td>TXD0      <td>SD6          <td>reserved<td>                <td>          <td>TXD1   </tr>
<tr><td>15<td>Low <td>RXD0      <td>SD7          <td>reserved<td>                <td>          <td>RXD1   </tr>
<tr><td>16<td>Low <td>reserved  <td>SD8          <td>reserved<td>CTS0            <td>SPI1_CE2_N<td>CTS1   </tr>
<tr><td>17<td>Low <td>reserved  <td>SD9          <td>reserved<td>RTS0            <td>SPI1_CE1_N<td>RTS1   </tr>
<tr><td>18<td>Low <td>PCM_CLK   <td>SD10         <td>reserved<td>BSCSL SDA / MOSI<td>SPI1_CE0_N<td>PWM0   </tr>
<tr><td>19<td>Low <td>PCM_FS    <td>SD11         <td>reserved<td>BSCSL SCL / SCLK<td>SPI1_MISO <td>PWM1   </tr>
<tr><td>20<td>Low <td>PCM_DIN   <td>SD12         <td>reserved<td>BSCSL / MISO    <td>SPI1_MOSI <td>GPCLK0 </tr>
<tr><td>21<td>Low <td>PCM_DOUT  <td>SD13         <td>reserved<td>BSCSL / CE_N    <td>SPI1_SCLK <td>GPCLK1 </tr>
<tr><td>22<td>Low <td>reserved  <td>SD14         <td>reserved<td>SD1_CLK         <td>ARM_TRST             </tr>
<tr><td>23<td>Low <td>reserved  <td>SD15         <td>reserved<td>SD1_CMD         <td>ARM_RTCK             </tr>
<tr><td>24<td>Low <td>reserved  <td>SD16         <td>reserved<td>SD1_DAT0        <td>ARM_TDO              </tr>
<tr><td>25<td>Low <td>reserved  <td>SD17         <td>reserved<td>SD1_DAT1        <td>ARM_TCK              </tr>
<tr><td>26<td>Low <td>reserved  <td>reserved     <td>reserved<td>SD1_DAT2        <td>ARM_TDI              </tr>
<tr><td>27<td>Low <td>reserved  <td>reserved     <td>reserved<td>SD1_DAT3        <td>ARM_TMS              </tr>
<tr><td>28<td>-   <td>SDA0      <td>SA5          <td>PCM_CLK <td>reserved                                 </tr>
<tr><td>29<td>-   <td>SCL0      <td>SA4          <td>PCM_FS  <td>reserved                                 </tr>
<tr><td>30<td>Low <td>reserved  <td>SA3          <td>PCM_DIN <td>CTS0            <td>          <td>CTS1   </tr>
<tr><td>31<td>Low <td>reserved  <td>SA2          <td>PCM_DOUT<td>RTS0            <td>          <td>RTS1   </tr>
<tr><td>32<td>Low <td>GPCLK0    <td>SA1          <td>reserved<td>TXD0            <td>          <td>TXD1   </tr>
<tr><td>33<td>Low <td>reserved  <td>SA0          <td>reserved<td>RXD0            <td>          <td>RXD1   </tr>
<tr><td>34<td>High<td>GPCLK0    <td>SOE_N / SE   <td>reserved<td>reserved                                 </tr>
<tr><td>35<td>High<td>SPI0_CE1_N<td>SWE_N / SRW_N<td>        <td>reserved                                 </tr>
<tr><td>36<td>High<td>SPI0_CE0_N<td>SD0<td>TXD0  <td>reserved                                             </tr>
<tr><td>37<td>Low <td>SPI0_MISO <td>SD1<td>RXD0  <td>reserved                                             </tr>
<tr><td>38<td>Low <td>SPI0_MOSI <td>SD2<td>RTS0  <td>reserved                                             </tr>
<tr><td>39<td>Low <td>SPI0_SCLK <td>SD3<td>CTS0  <td>reserved                                             </tr>
<tr><td>40<td>Low <td>PWM0      <td>SD4          <td>        <td>reserved        <td>SPI2_MISO<td>TXD1    </tr>
<tr><td>41<td>Low <td>PWM1      <td>SD5          <td>reserved<td>reserved        <td>SPI2_MOSI<td>RXD1    </tr>
<tr><td>42<td>Low <td>GPCLK1    <td>SD6          <td>reserved<td>reserved        <td>SPI2_SCLK<td>RTS1    </tr>
<tr><td>43<td>Low <td>GPCLK2    <td>SD7          <td>reserved<td>reserved        <td>SPI2_CE0_N<td>CTS1   </tr>
<tr><td>44<td>-   <td>GPCLK1    <td>SDA0<td>SDA1 <td>reserved<td>SPI2_CE1_N                               </tr>
<tr><td>45<td>-   <td>PWM1      <td>SCL0<td>SCL1 <td>reserved<td>SPI2_CE2_N                               </tr>
<tr><td>46<td>High<td>Internal                                                                            </tr>
<tr><td>47<td>High<td>Internal                                                                            </tr>
<tr><td>48<td>High<td>Internal                                                                            </tr>
<tr><td>49<td>High<td>Internal                                                                            </tr>
<tr><td>50<td>High<td>Internal                                                                            </tr>
<tr><td>51<td>High<td>Internal                                                                            </tr>
<tr><td>52<td>High<td>Internal                                                                            </tr>
<tr><td>53<td>High<td>Internal                                                                            </tr>
</table>

### Raspberry Pi 4 / 5 {#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO_RASPBERRY_PI_4__5}

<table>
<caption id="GPIO_alternative_functions_rpi4_5"></caption>
<tr><th>GPIO<th>Pull up / down<th>Alt 0     <th>Alt 1        <th>Alt 2     <th>Alt 3           <th>Alt 4     <th>Alt 5</tr>
<tr><td>0<td>High<td>SDA0<td>SA5<td>PCLK<td>SPI3_CE0_N<td>TXD2<td>SDA6</tr>
<tr><td>1<td>High<td>SCL0<td>SA4<td>DE<td>SPI3_MISO<td>RXD2<td>SCL6</tr>
<tr><td>2<td>High<td>SDA1<td>SA3<td>LCD_VSYNC<td>SPI3_MOSI<td>CTS2<td>SDA3</tr>
<tr><td>3<td>High<td>SCL1<td>SA2<td>LCD_HSYNC<td>SPI3_SCLK<td>RTS2<td>SCL3</tr>
<tr><td>4<td>High<td>GPCLK0<td>SA1<td>DPI_D0<td>SPI4_CE0_N<td>TXD3<td>SDA3</tr>
<tr><td>5<td>High<td>GPCLK1<td>SA0<td>DPI_D1<td>SPI4_MISO<td>RXD3<td>SCL3</tr>
<tr><td>6<td>High<td>GPCLK2<td>SOE_N / SE<td>DPI_D2<td>SPI4_MOSI<td>CTS3<td>SDA4</tr>
<tr><td>7<td>High<td>SPI0_CE1_N<td>SWE_N / SRW_N<td>DPI_D3<td>SPI4_SCLK<td>RTS3<td>SCL4</tr>
<tr><td>8<td>High<td>SPI0_CE0_N<td>SD0<td>DPI_D4<td>BSCSL / CE_N<td>TXD4<td>SDA4</tr>
<tr><td>9<td>Low<td>SPI0_MISO<td>SD1<td>DPI_D5<td>BSCSL / MISO<td>RXD4<td>SCL4</tr>
<tr><td>10<td>Low<td>SPI0_MOSI<td>SD2<td>DPI_D6<td>BSCSL SDA / MOSI<td>CTS4<td>SDA5</tr>
<tr><td>11<td>Low<td>SPI0_SCLK<td>SD3<td>DPI_D7<td>BSCSL SCL / SCLK<td>RTS4<td>SCL5</tr>
<tr><td>12<td>Low<td>PWM0_0<td>SD4<td>DPI_D8<td>SPI5_CE0_N<td>TXD5<td>SDA5</tr>
<tr><td>13<td>Low<td>PWM0_1<td>SD5<td>DPI_D9<td>SPI5_MISO<td>RXD5<td>SCL5</tr>
<tr><td>14<td>Low<td>TXD0<td>SD6<td>DPI_D10<td>SPI5_MOSI<td>CTS5<td>TXD1</tr>
<tr><td>15<td>Low<td>RXD0<td>SD7<td>DPI_D11<td>SPI5_SCLK<td>RTS5<td>RXD1</tr>
<tr><td>16<td>Low<td>\<reserved\><td>SD8<td>DPI_D12<td>CTS0<td>SPI1_CE2_N<td>CTS1</tr>
<tr><td>17<td>Low<td>\<reserved\><td>SD9<td>DPI_D13<td>RTS0<td>SPI1_CE1_N<td>RTS1</tr>
<tr><td>18<td>Low<td>PCM_CLK<td>SD10<td>DPI_D14<td>SPI6_CE0_N<td>SPI1_CE0_N<td>PWM0_0</tr>
<tr><td>19<td>Low<td>PCM_FS<td>SD11<td>DPI_D15<td>SPI6_MISO<td>SPI1_MISO<td>PWM0_1</tr>
<tr><td>20<td>Low<td>PCM_DIN<td>SD12<td>DPI_D16<td>SPI6_MOSI<td>SPI1_MOSI<td>GPCLK0</tr>
<tr><td>21<td>Low<td>PCM_DOUT<td>SD13<td>DPI_D17<td>SPI6_SCLK<td>SPI1_SCLK<td>GPCLK1</tr>
<tr><td>22<td>Low<td>SD0_CLK<td>SD14<td>DPI_D18<td>SD1_CLK<td>ARM_TRST<td>SDA6</tr>
<tr><td>23<td>Low<td>SD0_CMD<td>SD15<td>DPI_D19<td>SD1_CMD<td>ARM_RTCK<td>SCL6</tr>
<tr><td>24<td>Low<td>SD0_DAT0<td>SD16<td>DPI_D20<td>SD1_DAT0<td>ARM_TDO<td>SPI3_CE1_N</tr>
<tr><td>25<td>Low<td>SD0_DAT1<td>SD17<td>DPI_D21<td>SD1_DAT1<td>ARM_TCK<td>SPI4_CE1_N</tr>
<tr><td>26<td>Low<td>SD0_DAT2<td>\<reserved\><td>DPI_D22<td>SD1_DAT2<td>ARM_TDI<td>SPI5_CE1_N</tr>
<tr><td>27<td>Low<td>SD0_DAT3<td>\<reserved\><td>DPI_D23<td>SD1_DAT3<td>ARM_TMS<td>SPI6_CE1_N</tr>
<tr><td>28<td>-<td>SDA0<td>SA5<td>PCM_CLK<td>\<reserved\><td>MII_A_RX_ERR<td>RGMII_MDIO</tr>
<tr><td>29<td>-<td>SCL0<td>SA4<td>PCM_FS<td>\<reserved\><td>MII_A_TX_ERR<td>RGMII_MDC</tr>
<tr><td>30<td>Low<td>\<reserved\><td>SA3<td>PCM_DIN<td>CTS0<td>MII_A_CRS<td>CTS1</tr>
<tr><td>31<td>Low<td>\<reserved\><td>SA2<td>PCM_DOUT<td>RTS0<td>MII_A_COL<td>RTS1</tr>
<tr><td>32<td>Low<td>GPCLK0<td>SA1<td>\<reserved\><td>TXD0<td>SD_CARD_PRES<td>TXD1</tr>
<tr><td>33<td>Low<td>\<reserved\><td>SA0<td>\<reserved\><td>RXD0<td>SD_CARD_WRPROT<td>RXD1</tr>
<tr><td>34<td>High<td>GPCLK0<td>SOE_N / SE<td>\<reserved\><td>SD1_CLK<td>SD_CARD_LED<td>RGMII_IRQ</tr>
<tr><td>35<td>High<td>SPI0_CE1_N<td>SWE_N / SRW_N<td><td>SD1_CMD<td>RGMII_START_STOP</tr>
<tr><td>36<td>High<td>SPI0_CE0_N<td>SD0<td>TXD0<td>SD1_DAT0<td>RGMII_RX_OK<td>MII_A_RX_ERR</tr>
<tr><td>37<td>Low<td>SPI0_MISO<td>SD1<td>RXD0<td>SD1_DAT1<td>RGMII_MDIO<td>MII_A_TX_ERR</tr>
<tr><td>38<td>Low<td>SPI0_MOSI<td>SD2<td>RTS0<td>SD1_DAT2<td>RGMII_MDC<td>MII_A_CRS</tr>
<tr><td>39<td>Low<td>SPI0_SCLK<td>SD3<td>CTS0<td>SD1_DAT3<td>RGMII_IRQ<td>MII_A_COL</tr>
<tr><td>40<td>Low<td>PWM1_0<td>SD4<td><td>SD1_DAT4<td>SPI0_MISO<td>TXD1</tr>
<tr><td>41<td>Low<td>PWM1_1<td>SD5<td>\<reserved\><td>SD1_DAT5<td>SPI0_MOSI<td>RXD1</tr>
<tr><td>42<td>Low<td>GPCLK1<td>SD6<td>\<reserved\><td>SD1_DAT6<td>SPI0_SCLK<td>RTS1</tr>
<tr><td>43<td>Low<td>GPCLK2<td>SD7<td>\<reserved\><td>SD1_DAT7<td>SPI0_CE0_N<td>CTS1</tr>
<tr><td>44<td>-<td>GPCLK1<td>SDA0<td>SDA1<td>\<reserved\><td>SPI0_CE1_N<td>SD_CARD_VOLT</tr>
<tr><td>45<td>-<td>PWM0_1<td>SCL0<td>SCL1<td>\<reserved\><td>SPI0_CE2_N<td>SD_CARD_PWR0</tr>
<tr><td>46<td>High<td>\<Internal\></tr>
<tr><td>47<td>High<td>\<Internal\></tr>
<tr><td>48<td>High<td>\<Internal\></tr>
<tr><td>49<td>High<td>\<Internal\></tr>
<tr><td>50<td>High<td>\<Internal\></tr>
<tr><td>51<td>High<td>\<Internal\></tr>
<tr><td>52<td>High<td>\<Internal\></tr>
<tr><td>53<td>High<td>\<Internal\></tr>
<tr><td>54<td>High<td>\<Internal\></tr>
<tr><td>55<td>High<td>\<Internal\></tr>
<tr><td>56<td>High<td>\<Internal\></tr>
<tr><td>57<td>High<td>\<Internal\></tr>
</table>
