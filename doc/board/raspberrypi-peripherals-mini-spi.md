# Raspberry Pi Mini SPI (SPI1/2) {#RASPBERRY_PI_MINI_SPI_SPI12}

Also refer to [Raspberry Pi auxiliary peripheral](#RASPBERRY_PI_AUXILIARY_PERIPHERAL) for register in the auxiliary peripheral that enable the mini-SPI devices and shows their interrupt status.

For Raspberry Pi 3 see [documentation](pdf/bcm2837-peripherals.pdf), section 2, page 8 and section 2.3, page 20. Also see [errata](https://elinux.org/BCM2835_datasheet_errata).

For Raspberry Pi 4 see [documentation](pdf/bcm2711-peripherals.pdf), section 2, page 8 and section 2.3, page 16.

Raspberry Pi 5 has no mini-SPI device.

Base address of the SPI devices is shown in the table below.

| Device | Board version | Base address |
|--------|---------------|--------------|
| SPI1   | 3             | 3F215080
| SPI1   | 3             | 3F2150C0
| SPI2   | 4             | FE215080
| SPI2   | 4             | FE2150C0

| Register           | Address   | Bits  | Name                | Acc | Meaning |
|--------------------|-----------|-------|---------------------|-----|---------|
| AUX_SPIn_CNTL0_REG | Base+0x00 | 31:20 | Speed               | R/W | Sets the SPI clock speed. spi clk freq = system_clock_freq/2*(speed+1).<br/>Reset level: 0
|                    |           | 19:17 | Chip selects        | R/W | The pattern output on the CS pins when active.<br/>Reset level: 0
|                    |           | 16    | Post-input mode     | R/W | If set the SPI input works in post input mode.<br/>Some rare SPI devices output data on the falling clock edge which then has to be picked up on the next falling clock edge. There are two problems with this:<br/>1. The very first falling clock edge there is no valid data arriving.<br/>2. After the last clock edge there is one more 'dangling' bit to pick up.<br>The post-input mode is specifically to deal with this sort of data. If the post-input mode bit is set, the data arriving at the first falling clock edge is ignored. Then after the last falling clock edge the CS remain asserted and after a full bit time the last data bit is picked up.<br/>Reset level: 0
|                    |           | 15    | Variable CS         | R/W | If 1 the SPI takes the CS pattern and the data from the TX fifo.<br/>If 0 the SPI takes the CS pattern from bits 17-19 of this register Set this bit only if also bit 14 (variable width) is set.<br/>Reset level: 0
|                    |           | 14    | Variable width      | R/W | If 1 the SPI takes the shift length and the data from the TX fifo.<br/>If 0 the SPI takes the shift length from bits 0-5 of this register.<br/>Reset level: 0
|                    |           | 13:12 | DOUT hold time      | R/W | Controls the extra DOUT hold time in system clock cycles.<br/>00 : No extra hold time<br/>01 : 1 system clock extra hold time<br/>10 : 4 system clocks extra hold time<br/>11 : 7 system clocks extra hold time<br/>Reset level: 0
|                    |           | 11    | Enable              | R/W | Enables the SPI interface. Whilst disabled the FIFOs can still be written to or read from. This bit should be 1 during normal operation.<br/>Reset level: 0
|                    |           | 10    | In rising           | R/W | If 1 data is clocked in on the rising edge of the SPI clock.<br/>If 0 data is clocked in on the falling edge of the SPI clock.<br/>Reset level: 0
|                    |           | 9     | Clear FIFO          | R/W | If 1 the receive and transmit FIFOs are held in reset (and thus flushed).<br/>This bit should be 0 during normal operation.<br/>Reset level: 0
|                    |           | 8     | Out rising          | R/W | If 1 data is clocked out on the rising edge of the SPI clock.<br/>If 0 data is clocked out on the falling edge of the SPI clock.<br/>Reset level: 0
|                    |           | 7     | Invert SPI clk      | R/W | If 1 the 'idle' clock line state is high.<br/>If 0 the 'idle' clock line state is low.<br/>Reset level: 0
|                    |           | 6     | Shift out MSB first | R/W | If 1 the data is shifted out starting with the MS bit (bit 15 or bit 11).<br/>If 0 the data is shifted out starting with the LS bit (bit 0).<br/>Reset level: 0
|                    |           | 5:0   | Shift length        | R/W | Specifies the number of bits to shift. This field is ignored when using 'variable shift' mode.<br/>Reset level: 0
| AUX_SPIn_CNTL1_REG | Base+0x04 | 31:11 | -                   |     | Reserved. Write 0, read don't care.
|                    |           | 10:8  | CS high time        | R/W | Additional SPI clock cycles where the CS is high.<br/>Reset level: 0
|                    |           | 7     | TX empty IRQ        | R/W | If 1 the interrupt line is high when the transmit FIFO.is empty<br/>Reset level: 0
|                    |           | 6     | Done IRQ            | R/W | If 1 the interrupt line is high when the interface is idle.<br/>Reset level: 0
|                    |           | 5:2   | -                   | R/W | Reserved. Write 0, read don't care.
|                    |           | 1     | Shift in MSB first  | R/W | If 1 the data is shifted in starting with the MS bit (bit 15).<br/>If 0 the data is shifted in starting with the LS bit (bit 0).<br/>Reset level: 0
|                    |           | 0     | Keep input          | R/W | If 1 the receiver shift register is NOT cleared. Thus new data is concatenated to old data.<br/>If 0 the receiver shift register is cleared before each transaction.<br/>Reset level: 0
| AUX_SPIn_STAT_REG  | Base+0x08 | 31:28 | -                   |     | Reserved. Write 0, read don't care.
|                    |           | 27:24 | TX FIFO level       | R/W | The number of data units in the transmit data FIFO. Reset level: 0
|                    |           | 23:20 | -                   |     | Reserved. Write 0, read don't care.
|                    |           | 19:16 | RX FIFO level       | R/W | The number of data units in the receive data FIFO. Reset level: 0
|                    |           | 15:11 | -                   |     | Reserved. Write 0, read don't care.
|                    |           | 10    | TX full             | R/W | If 1 the transmit FIFO is full.<br/>If 0 the transmit FIFO can accept at least 1 data unit.<br/>Reset level: 0
|                    |           | 9     | TX empty            | R/W | If 1 the transmit FIFO is empty.<br/>If 0 the transmit FIFO holds at least 1 data unit.<br/>Reset level: 1
|                    |           | 8     | RX full             | R/W | If 1 the receiver FIFO is full.<br/>If 0 the receiver FIFO can accept at least 1 data unit.<br/>Reset level: 0
|                    |           | 7     | RX empty            | R/W | If 1 the receiver FIFO is empty.<br/>If 0 the receiver FIFO holds at least 1 data unit.<br/>Reset level: 1
|                    |           | 6     | Busy                | R/W | Indicates the module is busy transferring data.<br/>Reset level: 0
|                    |           | 5:0   | Bit count           | R/W | The number of bits still to be processed. Starts with 'shift-length' and counts down.<br/>Reset level: 0
| AUX_SPIn_PEEK_REG  | Base+0x0C | 31:16 | -                   |     | Reserved. Write 0, read don't care.
|                    |           | 15:0  | Data                | R   | Reads from this address will show the top entry from the receive FIFO, but the data is not taken from the FIFO. This provides a means of inspecting the data but not removing it from the FIFO.<br/>Reset level: 0
| AUX_SPIn_IO_REG    | Base+0x2C | 31:16 | -                   |     | Reserved. Write 0, read don't care.
|                    |           | 15:0  | Data                | R/W | Writes to this address range end up in the transmit FIFO. Data is lost when writing whilst the transmit FIFO is full.<br/>Reads from this address will take the top entry from the receive FIFO. Reading whilst the receive FIFO is empty will return the last data received.<br/>Writing to this register causes the SPI CS_n pins to be de-asserted at the end of the access.<br/>Reset level: 0


