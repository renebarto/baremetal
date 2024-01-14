# Mailbox {#RPI_MAILBOX}

For more information:
- [Raspberry Pi firmware wiki - Mailboxes](https://github.com/raspberrypi/firmware/wiki/Mailboxes)
- [Raspberry Pi firmware wiki - Accessing mailboxes](https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes)
- [Raspberry Pi firmware wiki - Mailbox property interface](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)

| Register        | Address         | Bits  | Name                                  | Acc | Meaning |
|-----------------|-----------------|-------|---------------------------------------|-----|---------|
| READ_DATA       | Base+0x0000B880 | 31:4  | Data to be read                       | -   | Data read from mailbox (address to buffer containing mailbox data, converted to GPU address space). Buffer must be aligned on 4 byte boundary
| READ_CHANNEL    |                 | 3:0   | Read channel                          | R   | Read channel
| ...             |                 | -     | -                                     | -   | Unused
| POLL            | Base+0x0000B890 | ?     | ?                                     | -   | Unknown
| SENDER          | Base+0x0000B894 | ?     | ?                                     | -   | Unknown
| STATUS_RESERVED | Base+0x0000B898 | 31:2  | Reserved                              | -   | Reserved
| STATUS_EMPTY    |                 | 1     | Read empty                            | R   | Read buffer empty<br/>1 = read buffer is empty<br/>0 = read buffer filled (data ready to be read)
| STATUS_FULL     |                 | 0     | Write full                            | R   | Write buffer full<br/1 = write buffer full<br/>0 = write buffer empty (data can be written)
| CONFIG          | Base+0x0000B89C | -     | -                                     | -   | Unused
| WRITE_DATA      | Base+0x0000B8A0 | 31:4  | Data to be written                    | -   | Data written to mailbox (address to buffer containing mailbox data, converted to GPU address space). Buffer must be aligned on 4 byte boundary
| WRITE_CHANNEL   |                 | 3:0   | Write channel                         | R   | Write channel
| STATUS1         | Base+0x0000B8B8 | ?     | ?                                     | -   | Unknown

