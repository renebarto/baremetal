# Raspberry Pi SPI {#RASPBERRY_PI_SPI}

See [documentation](pdf/bcm2837-peripherals.pdf), page 148 for Raspberry Pi 3, [documentation](pdf/bcm2711-peripherals.pdf), page 133 for Raspberry Pi 4, and [documentation](pdf/bcm2711-peripherals.pdf), page 50 for Raspberry Pi 5.

For Raspberry Pi 3 see [documentation](pdf/bcm2837-peripherals.pdf), section 10, page 148. Also see [errata](https://elinux.org/BCM2835_datasheet_errata).

For Raspberry Pi 4 see [documentation](pdf/bcm2711-peripherals.pdf), section 9, page 133.

For Raspberry Pi 5 see [documentation](pdf/bcm2711-peripherals.pdf), section 3.6, page 50.

\todo Describe Raspberry Pi 5 later on.

Base address of the SPI devices is shown in the table below.

| Device | Board version | Base address |
|--------|---------------|--------------|
| SPI0   | 3             | 3F204000
| SPI0   | 4             | FE204000
| SPI3   | 4             | FE204600
| SPI4   | 4             | FE204800
| SPI5   | 4             | FE204A00
| SPI6   | 4             | FE204C00

| Register | Address   | Bits  | Name     | Acc | Meaning |
|----------|-----------|-------|----------|-----|---------|
| CS       | Base+0x00 | 31:26 | -        |     | Reserved. Write 0, read don't care.
|          |           | 25    | LEN_LONG | R/W | Enable Long data word in LoSSI mode if DMA_LEN is set.<br/>0: writing to the FIFO will write a single byte<br/>1: writing to the FIFO will write a 32-bit word<br/>Reset level: 0
|          |           | 24    | DMA_LEN  | R/W | Enable DMA mode in LoSSI mode.<br/>Reset level: 0
|          |           | 23    | CSPOL2   | R/W | Chip Select 2 Polarity<br/>0= Chip select is active low.<br/>1= Chip select is active high.<br/>Reset level: 0
|          |           | 22    | CSPOL1   | R/W | Chip Select 1 Polarity<br/>0= Chip select is active low.<br/>1= Chip select is active high.<br/>Reset level: 0
|          |           | 21    | CSPOL0   | R/W | Chip Select 0 Polarity<br/>0= Chip select is active low.<br/>1= Chip select is active high.<br/>Reset level: 0
|          |           | 20    | RXF      | R   | RX FIFO Full<br/>0 = RX FIFO is not full.<br/>1 = RX FIFO is full. No further serial data will be sent / received until data is read from FIFO.<br/>Reset level: 0
|          |           | 19    | RXR      | R   | RX FIFO needs Reading (¾ full)<br/>0 = RX FIFO is less than ¾ full (or not active TA = 0).<br/>1 = RX FIFO is ¾ or more full. Cleared by reading sufficient data from the RX FIFO or setting TA to 0.<br/>Reset level: 0
|          |           | 18    | TXD      | R   | TX FIFO can accept Data<br/>0 = TX FIFO is full and so cannot accept more data.<br/>1 = TX FIFO has space for at least 1 byte.<br/>Reset level: 0
|          |           | 17    | RXD      | R   | RX FIFO contains Data<br/>0 = RX FIFO is empty.<br/>1 = RX FIFO contains at least 1 byte.<br/>Reset level: 0
|          |           | 16    | DONE     | R   | Transfer Done<br/>0 = Transfer is in progress (or not active TA = 0).<br/>1 = Transfer is complete. Cleared by writing more data to the TX FIFO or setting TA to 0.<br/>Reset level: 0
|          |           | 15    | TE_EN    | R/W | Unused<br/>Reset level: 0
|          |           | 14    | LMONO    | R/W | Unused<br/>Reset level: 0
|          |           | 13    | LEN      | R/W | LoSSI enable<br/>The serial interface is configured as a LoSSI master.<br/>0 = The serial interface will behave as an SPI master.<br/>1 = The serial interface will behave as a LoSSI master.<br/>Reset level: 0
|          |           | 12    | REN      | R/W | Read Enable<br/>Read enable if you are using bidirectional mode. If this bit is set, the SPI peripheral will be able to send data to this device.<br/>0 = We intend to write to the SPI peripheral.<br/>1 = We intend to read from the SPI peripheral.<br/>Reset level: 1
|          |           | 11    | ADCS     | R/W | Automatically De-assert Chip Select<br/>0 = Don’t automatically de-assert chip select at the end of a DMA transfer; chip select is manually controlled by software.<br/>1 = Automatically de-assert chip select at the end of a DMA transfer (as determined by SPIDLEN)<br/>Reset level: 0
|          |           | 10    | INTR     | R/W | Interrupt on RXR<br/>0 = Don’t generate interrupts on RX FIFO condition.<br/>1 = Generate interrupt while RXR = 1.<br/>Reset level: 0
|          |           | 9     | INTD     | R/W | Interrupt on Done<br/>0 = Don’t generate interrupt on transfer complete.<br/>1 = Generate interrupt when DONE = 1.<br/>Reset level: 0
|          |           | 8     | DMAEN    | R/W | DMA Enable<br/>0 = No DMA requests will be issued.<br/>1 = Enable DMA operation.<br/>Peripheral generates data requests. These will be taken in four-byte words until the SPIDLEN has been reached.<br/>Reset level: 0
|          |           | 7     | TA       | R/W | Transfer Active<br/>0 = Transfer not active. /CS lines are all high (assuming CSPOL = 0). RXR and DONE are 0. Writes to SPI_FIFO write data into bits 15:0 of SPIDLEN and bits 7:0 of SPICS allowing DMA data blocks to set mode before sending data.<br/>1 = Transfer active. /CS lines are set according to CS bits and CSPOL. Writes to SPI_FIFO write data to TX FIFO. TA is cleared by a dma_frame_end pulse from the DMA controller.<br/>Reset level: 0
|          |           | 6     | CSPOL    | R/W | Chip Select Polarity<br/>0 = Chip select lines are active low<br/>1 = Chip select lines are active high<br/>Reset level: 0
|          |           | 5:4   | CLEAR    | W1SC| FIFO Clear<br/>00 = No action.<br/>x1 = Clear TX FIFO. One-shot operation.<br/>1x = Clear RX FIFO. One-shot operation.<br/>If CLEAR and TA are both set in the same operation, the FIFOs are cleared before the new frame is started. Read back as 0.<br/>Reset level: 0
|          |           | 3     | CPOL     | R/W | Clock Polarity<br/>0 = Rest state of clock = low.<br/>1 = Rest state of clock = high.<br/>Reset level: 0
|          |           | 2     | CPHA     | R/W | Clock Phase<br/>0 = First SCLK transition at middle of data bit.<br/>1 = First SCLK transition at beginning of data bit.<br/>Reset level: 0
|          |           | 1:0   | CS       | R/W | Chip Select<br/>00 = Chip select 0<br/>01 = Chip select 1<br/>10 = Chip select 2<br/>11 = Reserved<br/>Reset level: 0
| FIFO     | Base+0x04 | 31:0  | DATA     | R/W | Sets the SPI clock speed. spi clk freq = system_clock_freq/2*(speed+1).<br/>Reset level: 0
| CLK      | Base+0x08 | 31:16 | -        |     | Reserved. Write 0, read don't care.
|          |           | 15:0  | CDIV     | R/W | Clock Divider SCLK = Core Clock / CDIV.<br/>If CDIV is set to 0, the divisor is 65536. The divisor must be a multiple of 2. Odd numbers rounded down. The maximum SPI clock rate is of the APB clock.<br/>Reset level: 0
| DLEN     | Base+0x0C | 31:16 | -        |     | Reserved. Write 0, read don't care.
|          |           | 15:0  | LEN      | R/W | Data Length<br/>The number of bytes to transfer. This field is only valid for DMA mode (DMAEN set) and controls how many bytes to transmit (and therefore receive).<br/>Reset level: 0
| LTOH     | Base+0x10 | 31:4  | -        |     | Reserved. Write 0, read don't care.
|          |           | 3:0   | TOH      | R/W | This sets the Output Hold delay in APB clocks. A value of 0 causes a 1 clock delay.<br/>Reset level: 0
| DC       | Base+0x14 | 31:24 | RPANIC   | R/W | DMA Read Panic Threshold.<br/>Generate the Panic signal to the RX DMA engine whenever the RX FIFO level is greater than this amount.<br/>Reset level: 0x30
|          |           | 23:16 | RDREQ    | R/W | DMA Read Request Threshold.<br/>Generate a DREQ to the RX DMA engine whenever the RX FIFO level is greater than this amount (RX DREQ is also generated if the transfer has finished but the RX FIFO isn't empty).<br/>Reset level: 0x20
|          |           | 15:8  | TPANIC   | R/W | DMA Write Panic Threshold.<br/>Generate the Panic signal to the TX DMA engine whenever the TX FIFO level is less than or equal to this amount.<br/>Reset level: 0x10
|          |           | 7:0   | TDREQ    | R/W | DMA Write Request Threshold.<br/>Generate a DREQ signal to the TX DMA engine whenever the TX FIFO level is less than or equal to this amount.<br/>Reset level: 0x20
