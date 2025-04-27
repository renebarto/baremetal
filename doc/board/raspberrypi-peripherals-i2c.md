# Raspberry Pi I2C {#RASPBERRY_PI_I2C}

See [documentation](pdf/bcm2837-peripherals.pdf), section 3 (page 28) for Raspberry Pi 3.
See [documentation](pdf/bcm2711-peripherals.pdf), section 3 (page 24) for Raspberry Pi 4.
See [documentation](pdf/bcm2837-peripherals.pdf), section 3.5 (page 48) for Raspberry Pi 5.

The BSC is the Broadcom Serial Controller, which handles the I2C communication.
Depending on the model, Raspberry Pi has 3 to 7 I2C buses.

| Raspberry Pi | # I2C buses |
|--------------|------------|
| 3            | 3
| 4            | 6
| 5            | 7

Register base addresses differ per bus, and per model.

| Raspberry Pi | Bus # | Base address | Base address seen from ARM |
|--------------|-------|--------------|----------------------------|
| 3            | 0     | 0x7E205000   | 0x3F205000                 |
| 3            | 1     | 0x7E804000   | 0x3F804000                 |
| 3            | 2     | 0x7E805000   | 0x3F805000                 |
| 4            | 0     | 0x7E205000   | 0xFE205000                 |
| 4            | 1     | 0x7E804000   | 0xFE804000                 |
| 4            | 3     | 0x7E205600   | 0xFE205600                 |
| 4            | 4     | 0x7E205800   | 0xFE205800                 |
| 4            | 5     | 0x7E205A80   | 0xFE205A80                 |
| 4            | 6     | 0x7E205C00   | 0xFE205C00                 |
| 5            | 0     | 0x40070000   | 0x107C205000               |
| 5            | 1     | 0x40074000   | 0x107C804000               |
| 5            | 2     | 0x40078000   | 0x107C205C00               |
| 5            | 3     | 0x4007c000   | 0x107C205600               |
| 5            | 4     | 0x40080000   | 0x107C205800               |
| 5            | 5     | 0x40084000   | 0x107C205A80               |
| 5            | 6     | 0x40088000   | 0x107C205C00               |

## BSC GPIO pins {#RASPBERRY_PI_I2C_BSC_GPIO_PINS}

Not all I2C buses can be connected to the GPIO header.
The GPIO connections per bus are shown in the table below.

| Bus #  | config 0      | config 1      | config 2      | Boards    |
|--------|---------------|---------------|---------------|-----------|
| Signal | SDA    SCL    | SDA    SCL    | SDA    SCL    |           |
| 0      | GPIO0  GPIO1  | GPIO28 GPIO29 | GPIO44 GPIO45 | Raspberry Pi 3 only
| 1      | GPIO2  GPIO3  |               |               | Raspberry Pi 3 only
| 2      |               |               |               | None
| 3      | GPIO2  GPIO3  | GPIO4  GPIO5  |               | Raspberry Pi 4 only
| 4      | GPIO6  GPIO7  | GPIO8  GPIO9  |               | Raspberry Pi 4 only
| 5      | GPIO10 GPIO11 | GPIO12 GPIO13 |               | Raspberry Pi 4 only
| 6      | GPIO22 GPIO23 |               |               | Raspberry Pi 4 only
| 0      | GPIO0  GPIO1  | GPIO8  GPIO9  |               | Raspberry Pi 5 only
| 1      | GPIO2  GPIO3  | GPIO10 GPIO11 |               | Raspberry Pi 5 only
| 2      | GPIO4  GPIO5  | GPIO12 GPIO13 |               | Raspberry Pi 5 only
| 3      | GPIO6  GPIO7  | GPIO14 GPIO15 | GPIO22 GPIO23 | Raspberry Pi 5 only

## BSC control register {#RASPBERRY_PI_I2C_BSC_CONTROL_REGISTER}

The control register is used to enable interrupts, clear the FIFO, define a read or write operation and start a transfer.

- The READ field specifies the type of transfer.
- The CLEAR field is used to clear the FIFO. Writing to this field is a one-shot operation which will always read back as zero.
The CLEAR bit can set at the same time as the start transfer bit, and will result in the FIFO being
cleared just prior to the start of transfer. Note that clearing the FIFO during a transfer will result in the transfer being aborted.
- The ST field starts a new BSC transfer. This is a one-shot action, and so the bit will always read back as 0.
- The INTD field enables interrupts at the end of a transfer - the DONE condition.
The interrupt remains active until the DONE condition is cleared by writing a 1 to the I2C Status register DONE field.
Writing a 0 to the INTD field disables interrupts on DONE.
- The INTT field enables interrupts whenever the FIFO is 3/4 or more empty and needs writing (i.e. during a write transfer) - the TXW condition.
The interrupt remains active until the TXW condition is cleared by writing sufficient data to the FIFO to complete the transfer.
Writing a 0 to the INTT field disables interrupts on TXW.
- The INTR field enables interrupts whenever the FIFO is 3/4 or more full and needs reading (i.e.during a read transfer) - the RXR condition.
The interrupt remains active until the RXW condition is cleared by reading sufficient data from the FIFO.
Writing a 0 to the INTR field disables interrupts on RXR.

The I2CEN field enables BSC operations. If this bit is 0 then transfers will not be performed.All register accesses are still permitted however.
<table>
<caption id="BSC_C_bits">BSC C bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:16<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>15   <td>I2CEN   <td>I2C Enable<br/>
                             0 = BSC controller is disabled<br/>
                             1 = BSC controller is enabled          <td>RW  <td>0x0</tr>
<tr><td>14:11<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>10   <td>INTR    <td>Interrupt on RX<br/>
                             0 = Don't generate interrupts on RXR condition.<br/>
                             1 = Generate interrupt while RXR = 1.  <td>RW  <td>0x0</tr>
<tr><td>9    <td>INTT    <td>Interrupt on TX<br/>
                             0 = Don't generate interrupts on TXW condition.<br/>
                             1 = Generate interrupt while TXW = 1.  <td>RW  <td>0x0</tr>
<tr><td>8    <td>INTD    <td>Interrupt on DONE<br/>
                             0 = Don't generate interrupts on DONE condition.<br/>
                             1 = Generate interrupt while DONE = 1. <td>RW  <td>0x0</tr>
<tr><td>7    <td>ST      <td>Start Transfer<br/>
                             0 = No action.<br/>
                             1 = Start a new transfer. One-shot operation. Read back as 0.
                                                                    <td>W1SC<td>0x0</tr>
<tr><td>6    <td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>5:4  <td>CLEAR   <td>FIFO Clear<br/>
                             00 = No action.<br/>
                             x1 = Clear FIFO. One-shot operation.<br/>
                             1x = Clear FIFO. One-shot operation.<br/>
                             If CLEAR and ST are both set in the same operation,
                             the FIFO is cleared before the new frame is started.<br/>
                             Read back as 0.<br/>
                             Note: 2 bits are used to maintain compatibility with the previous version.
                                                                    <td>W1SC<td>0x0</tr>
<tr><td>3:1  <td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>0    <td>READ    <td>Read Transfer<br/>
                             0 = Write Packet Transfer.<br/>
                             1 = Read Packet Transfer.              <td>RW  <td>0x0</tr>
</table>

## BSC status register {#RASPBERRY_PI_I2C_BSC_STATUS_REGISTER}

The status register is used to record activity status, errors and interrupt requests.
- The TA field indicates the activity status of the BSC controller. This read-only field returns a 1 when the controller is in
the middle of a transfer and a 0 when idle.
- The DONE field is set when the transfer completes. The DONE condition can be used with the I2C Control Register INTD field to generate an
interrupt on transfer completion. The DONE field is reset by writing a 1, writing a 0 to the field has no effect.
- The read-only TXW bit is set during a write transfer and the FIFO is less than 3/4 full and needs writing. Writing
sufficient data (i.e. enough data to either fill the FIFO more than 3/4 full or complete the transfer) to the FIFO will clear
the field. When the I2C Control Register INTT control bit is set, the TXW condition can be used to generate an interrupt to write more
data to the FIFO to complete the current transfer. If the I2C controller runs out of data to send, it will wait for more
data to be written into the FIFO.
- The read-only RXR field is set during a read transfer and the FIFO is 3/4 or more full and needs reading. Reading
sufficient data to bring the depth below 3/4 will clear the field.
When the I2C Control Register INTR control bit is set, the RXR condition can be used to generate an interrupt to read data from the FIFO
before it becomes full. In the event that the FIFO does become full, all I2C operations will stall until data is removed
from the FIFO.
- The read-only TXD field is set when the FIFO has space for at least one byte of data.
TXD is clear when the FIFO is full. The TXD field can be used to check that the FIFO can accept data before any is
written. Any writes to a full TX FIFO will be ignored.
- The read-only RXD field is set when the FIFO contains at least one byte of data. RXD is cleared when the FIFO
becomes empty. The RXD field can be used to check that the FIFO contains data before reading. Reading from an
empty FIFO will return invalid data.
- The read-only TXE field is set when the FIFO is empty. No further data will be transmitted until more data is written to
the FIFO.
- The read-only RXF field is set when the FIFO is full. No more clocks will be generated until space is available in the
FIFO to receive more data.
- The ERR field is set when the slave fails to acknowledge either its address or a data byte written to it. The ERR field is
reset by writing a 1, writing a 0 to the field has no effect.
- The CLKT field is set when the slave holds the SCL signal high for too long (clock stretching). The CLKT field is reset
by writing a 1, writing a 0 to the field has no effect.

<table>
<caption id="BSC_S_bits">BSC S bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:10<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>9<td>CLKT<td>Clock Stretch Timeout<br/>
0 = No errors detected.<br/>
1 = Slave has held the SCL signal low (clock stretching) for longer and that specified in the I2CCLKT register.
Cleared by writing 1 to the field.<td>W1C<td>0x0</tr>
<tr><td>8<td>ERR<td>ACK Error<br/>
0 = No errors detected.<br/>
1 = Slave has not acknowledged its address. Cleared by writing 1 to the field.<td>W1C<td>0x0</tr>
<tr><td>7<td>RXF<td>FIFO Full<br/>
0 = FIFO is not full.<br/>
1 = FIFO is full. If a read is underway, no further serial data will be received until data is read from FIFO.<td>RO<td>0x0</tr>
<tr><td>6<td>TXE<td>FIFO Empty<br/>
0 = FIFO is not empty.<br/>
1 = FIFO is empty. If a write is underway, no further serial data can be transmitted until data is written to the FIFO.<td>RO<td>0x1</tr>
<tr><td>5<td>RXD<td>FIFO contains Data<br/>
0 = FIFO is empty.<br/>
1 = FIFO contains at least 1 byte. Cleared by reading sufficient data from FIFO.<td>RO<td>0x0</tr>
<tr><td>4<td>TXD<td>FIFO can accept Data<br/>
0 = FIFO is full. The FIFO cannot accept more data.<br/>
1 = FIFO has space for at least 1 byte.<td>RO<td>0x1</tr>
<tr><td>3<td>RXR<td>FIFO needs Reading (3/4 full)<br/>
0 = FIFO is less than 3/4 full and a read is underway.<br/>
1 = FIFO is 3/4 or more full and a read is underway. Cleared by reading sufficient data from the FIFO.<td>RO<td>0x0</tr>
<tr><td>2<td>TXW<td>FIFO needs Writing (1/4 full)<br/>
0 = FIFO is at least 1/4 full and a write is underway (or sufficient data to send).<br/>
1 = FIFO is less than 1/4 full and a write is underway. Cleared by writing sufficient data to the FIFO.<td>RO<td>0x0</tr>
<tr><td>1<td>DONE<td>Transfer Done<br/>
0 = Transfer not completed.<br/>
1 = Transfer complete. Cleared by writing 1 to the field.<td>W1C<td>0x0</tr>
<tr><td>0<td>TA<td>Transfer Active<br/>
0 = Transfer not active.<br/>
1 = Transfer active.<td>RO<td>0x0</tr>
</table>

## BSC data length register {#RASPBERRY_PI_I2C_BSC_DATA_LENGTH_REGISTER}

The data length register defines the number of bytes of data to transmit or receive in the I2C transfer.
Reading the register gives the number of bytes remaining in the current transfer.
The DLEN field specifies the number of bytes to be transmitted/received.
Reading the DLEN field when a transfer is in progress (TA = 1) returns the number of bytes still to be transmitted or received.
Reading the DLEN field when the transfer has just completed (DONE = 1) returns zero as there are no more bytes to transmit or receive.
Finally, reading the DLEN field when TA = 0 and DONE = 0 returns the last value written.
The DLEN field can be left over multiple transfers.

<table>
<caption id="BSC_DLEN_bits">BSC DLEN bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:16<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>15:0<td>DLEN<td>Data Length.<br/>
Writing to DLEN specifies the number of bytes to be transmitted/received.<br/>
Reading from DLEN when TA = 1 or DONE = 1, returns the number of bytes still to be transmitted or received.<br/>
Reading from DLEN when TA = 0 and DONE = 0, returns the last DLEN value written.<br/>
DLEN can be left over multiple packets.<td>RW<td>0x0000</tr>
</table>

## BSC address register {#RASPBERRY_PI_I2C_BSC_ADDRESS_REGISTER}

The slave address register specifies the slave address and cycle type.
The address register can be left across multiple transfers.
The ADDR field specifies the slave address of the I2C device.

In case of a 10 bit address, the address register holds the first byte to be transmitted for the address, the second byte is part of the data to be sent.

<table>
<caption id="BSC_A_bits">BSC A bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:7<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>6:0<td>ADDR<td>Slave Address.<td>RW<td>0x00</tr>
</table>

## BSC FIFO register {#RASPBERRY_PI_I2C_BSC_FIFO_REGISTER}

The Data FIFO register is used to access the FIFO.
Write cycles to this address place data in the 16-byte FIFO, ready to transmit on the BSC bus.
Read cycles access data received from the bus.
Data writes to a full FIFO will be ignored and data reads from an empty FIFO will result in invalid data.
The FIFO can be cleared using the I2C Control Register CLEAR field.
The DATA field specifies the data to be transmitted or received.

<table>
<caption id="BSC_FIFO_bits">BSC FIFO bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:8<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>7:0<td>DATA<td>Writes to the register write transmit data to the FIFO.<br/>
Reads from register read received data from the FIFO.<td>RW<td>0x00</tr>
</table>

## BSC clock divider register {#RASPBERRY_PI_I2C_BSC_CLOCK_DIVIDER_REGISTER}

The clock divider register is used to define the clock speed of the BSC peripheral.
The CDIV field specifies the core clock divider used by the BSC.

<table>
<caption id="BSC_DIV_bits">BSC DIV bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:16<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>15:0<td>CDIV<td>Clock Divider<br/>
SCL = core_clock / CDIV<br/>
Where core_clk is nominally 150 MHz. If CDIV is set to 0, the divisor is 32768.<br/>
CDIV is always rounded down to an even number.<br/>
The default value should result in a 100 kHz I2C clock frequency.<td>RW<td>0x05dc</tr>
</table>

## BSC delay register {#RASPBERRY_PI_I2C_BSC_DELAY_REGISTER}

The data delay register provides fine control over the sampling / launch point of the data.
The REDL field specifies the number core clocks to wait after the rising edge before sampling the incoming data.
The FEDL field specifies the number core clocks to wait after the falling edge before outputting the next data bit.
Note: Care must be taken in choosing values for FEDL and REDL as it is possible to cause the BSC master to
malfunction by setting values of CDIV/2 or greater. Therefore the delay values should always be set to less than CDIV/2.

<table>
<caption id="BSC_DEL_bits">BSC DEL bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:16<td>FEDL<td>Falling Edge Delay<br/>
Number of core clock cycles to wait after the falling edge of SCL before outputting next bit of data.<td>RW<td>0x0030</tr>
<tr><td>15:0<td>REDL<td>Rising Edge Delay<br/>
Number of core clock cycles to wait after the rising edge of SCL before reading the next bit of data.<td>RW<td>0x0030</tr>
</table>

## BSC clock stretch register {#RASPBERRY_PI_I2C_BSC_CLOCK_STRETCH_REGISTER}

The clock stretch timeout register provides a timeout on how long the master waits for the slave to stretch the clock before deciding that the slave has hung.
The TOUT field specifies the number I2C SCL clocks to wait after releasing SCL high and finding that the SCL is still low before deciding that the slave is not responding and moving the I2C machine forward.
When a timeout occurs, the I2C Status rgister CLKT bit is set.
Writing 0x0 to TOUT will result in the Clock Stretch Timeout being disabled.

<table>
<caption id="BSC_CLKT_bits">BSC CLKT bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:16<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>15:0<td>TOUT<td>Clock Stretch Timeout Value<br/>
Number of SCL clock cycles to wait after the rising edge of SCL before deciding that the slave is not responding.<td>RW<td>0x0040</tr>
</table>

@todo Add RPI5 register descriptions for I2C. See Synopsys DW_apb_i2c.pdf
