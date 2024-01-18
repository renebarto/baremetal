# Raspberry Pi I2C registers {#RASPBERRY_PI_I2C_REGISTERS}

See [documentation](pdf/bcm2837-peripherals.pdf), page 28

The BSC is the Broadcom Serial Controller, which handles the I2C communication.
There are three BSC masters inside BCM. The register addresses start from
- BSC0: 0x7E205000
- BSC1: 0x7E804000
- BSC2: 0x7E805000

## BSC control register {#RASPBERRY_PI_I2C_REGISTERS_BSC_CONTROL_REGISTER}

The control register is used to enable interrupts, clear the FIFO, define a read or write operation and start a transfer.
The READ field specifies the type of transfer.The CLEAR field is used to clear the FIFO.Writing to this field is a one-shot operation
which will always read back aszero.The CLEAR bit can set at the same time as the start transfer bit, and will result in the FIFO being
cleared just prior to the start of transfer. Note that clearing the FIFO during a transfer will result in the transfer being aborted.

The ST field starts a new BSC transfer.This is a one-shot action, and so the bit will always read back as 0. The INTD field enables
interrupts at the end of a transfer - the DONE condition.
The interrupt remains active until the DONE condition is cleared by writing a 1 to the I2CS.DONE field.
Writing a 0 to the INTD field disables interrupts on DONE.

The INTT field enables interrupts whenever the FIFO is 3/4 or more empty and needs writing (i.e.during a write transfer) - the TXW condition.
The interrupt remains active until the TXW condition is cleared by writing sufficient data to the FIFO to complete the transfer.
Writing a 0 to the INTT field disables interrupts on TXW.

The INTR field enables interrupts whenever the FIFO is 3/4 or more full and needs reading (i.e.during a read transfer) - the RXR condition.
The interrupt remains active until the RXW condition is cleared by reading sufficient data from the FIFO.
Writing a 0 to the INTR field disables interrupts on RXR.

The I2CEN field enables BSC operations. If this bit is 0 then transfers will not be performed.All register accesses are still permitted however.
<table>
<caption id="BSC_C_bits">BSC C bits</caption>
<tr><th>Bits<th>ID<th>Description<th>Type<th>Reset</tr>
<tr><td>31:16<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>15   <td>I2CEN   <td>I2C Enable\n
                             0 = BSC controller is disabled\n
                             1 = BSC controller is enabled          <td>RW  <td>0x0</tr>
<tr><td>14:11<td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>10   <td>INTR    <td>Interrupt on RX\n
                             0 = Don't generate interrupts on RXR condition.\n
                             1 = Generate interrupt while RXR = 1.  <td>RW  <td>0x0</tr>
<tr><td>9    <td>INTT    <td>Interrupt on TX\n
                             0 = Don't generate interrupts on TXW condition.\n
                             1 = Generate interrupt while TXW = 1.  <td>RW  <td>0x0</tr>
<tr><td>8    <td>INTD    <td>Interrupt on DONE\n
                             0 = Don't generate interrupts on DONE condition.\n
                             1 = Generate interrupt while DONE = 1. <td>RW  <td>0x0</tr>
<tr><td>7    <td>ST      <td>Start Transfer\n
                             0 = No action.\n
                             1 = Start a new transfer. One-shot operation. Read back as 0.
                                                                    <td>W1SC<td>0x0</tr>
<tr><td>6    <td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>5:4  <td>CLEAR   <td>FIFO Clear\n
                             00 = No action.\n
                             x1 = Clear FIFO. One-shot operation.\n
                             1x = Clear FIFO. One-shot operation.\n
                             If CLEAR and ST are both set in the same operation,
                             the FIFO is cleared before the new frame is started.\n
                             Read back as 0.\n
                             Note: 2 bits are used to maintain compatibility with the previous version.
                                                                    <td>W1SC<td>0x0</tr>
<tr><td>3:1  <td>Reserved<td>-                                      <td>-   <td>-  </tr>
<tr><td>0    <td>READ    <td>Read Transfer\n
                             0 = Write Packet Transfer.\n
                             1 = Read Packet Transfer.              <td>RW  <td>0x0</tr>
</table>

## BSC status register {#RASPBERRY_PI_I2C_REGISTERS_BSC_STATUS_REGISTER}

- The status register is used to record activity status, errors and interrupt requests.
- The TA field indicates the activity status of the BSC controller. This read-only field returns a 1 when the controller is in
the middle of a transfer and a 0 when idle.
- The DONE field is set when the transfer completes. The DONE condition can be used with I2CC.INTD to generate an
interrupt on transfer completion. The DONE field is reset by writing a 1, writing a 0 to the field has no effect.
- The read-only TXW bit is set during a write transfer and the FIFO is less than 3/4 full and needs writing. Writing
sufficient data (i.e. enough data to either fill the FIFO more than 3/4 full or complete the transfer) to the FIFO will clear
the field. When the I2CC.INTT control bit is set, the TXW condition can be used to generate an interrupt to write more
data to the FIFO to complete the current transfer. If the I2C controller runs out of data to send, it will wait for more
data to be written into the FIFO.
- The read-only RXR field is set during a read transfer and the FIFO is 3/4 or more full and needs reading. Reading
sufficient data to bring the depth below 3/4 will clear the field.
When I2CC.INTR control bit is set, the RXR condition can be used to generate an interrupt to read data from the FIFO
before it becomes full. In the event that the FIFO does become full, all I2C operations will stall until data is removed
from the FIFO.
- The read-only TXD field is set when the FIFO has space for at least one byte of data.
- TXD is clear when the FIFO is full. The TXD field can be used to check that the FIFO can accept data before any is
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
</table>
