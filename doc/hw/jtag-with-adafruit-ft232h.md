# Using Adafruit FT232H for JTAG Programming {#USING_ADAFRUIT_FT232H_FOR_JTAG_PROGRAMMING}

## Setting up OpenOCD with Adafruit FT232H for JTAG {#USING_ADAFRUIT_FT232H_FOR_JTAG_PROGRAMMING_SETTING_UP_OPENOCD_WITH_ADAFRUIT_FT232H_FOR_JTAG}

First download and unpack [USB Driver Tool](https://sysprogs.com/getfile/1372/UsbDriverTool-2.1.exe) and [OpenOCD](https://sysprogs.com/getfile/2532/openocd-20251211.7z)

Then connect your FT232H board to your PC via USB and run UsbDriverTool. There will be a device named "USB Serial Converter" in the list. Select it and right click, then select `Install Libusb - WinUSB`.
Create a configuration file named `ft232h.cfg` with the following contents:

### ft232h.cfg {#USING_ADAFRUIT_FT232H_FOR_JTAG_PROGRAMMING_SETTING_UP_OPENOCD_WITH_ADAFRUIT_FT232H_FOR_JTAG_FT232HCFG}

```text
#
# FTDI USB Hi-Speed to MPSSE Breakout from Adafruit
#
# This should work for any bare FT232H
#
 
# Setup driver type
adapter driver ftdi
 
# 30000 kHZ -> 30MHz
adapter speed 30000
 
# Using JTAG (also could be SWD)
transport select jtag
 
# Common PID for FT232H
ftdi vid_pid 0x0403 0x6014
 
# Set sampling to allow higher clock speed
ftdi tdo_sample_edge falling
 
 
# Layout
# On this breakout, the LEDs are on ACBUS8 and ACBUS9, can't assign them
# registers are <ACVALUE><ADVALUE> <ACCONFIG><ADCONFIG>
# so we set 0x0308 to mean only ACBUS nTRST and nSRST, ADBUS3 (TMS) asserted high
# and we set 0x000B to mean only AC3,AC2,AC0 outputs -> (TMS,TD0, TCK)
ftdi layout_init 0x0308 0x000b
 
# Pins
# pin name      | func. |
# --------------|-------|
# ADBUS0        | TCK   |
# ADBUS1        | TDI   |
# ADBUS2        | TDO   |
# ADBUS3        | TMS   |
# ACBUS0        | nTRST |
# ACBUS1        | nSRST |
#---------------|-------|
 
# When data == oe -> pins are switched from output to input to give
# the tri state (L, H, Hi-Z) effect 
ftdi layout_signal nTRST -data 0x0100 -oe 0x0100
ftdi layout_signal nSRST -data 0x0200 -oe 0x0200
```

Do net connect the FT232H to the target device yet.

Now you can run OpenOCD with this configuration file:
```bat
cd D:\Projects\Private\tools\OpenOCD-20251211-0.12.0\share\openocd\scripts
..\..\..\bin\openocd.exe -f .\interface\ft232h.cfg -f .\target\raspberry.cfg
```

The output should look like this:
```text
Open On-Chip Debugger 0.12.0 (2025-12-11) [https://github.com/sysprogs/openocd]
Licensed under GNU GPL v2
libusb1 d52e355daa09f17ce64819122cb067b8a2ee0d4b
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : clock speed 1000 kHz
Error: JTAG scan chain interrogation failed: all ones
Error: Check JTAG interface, timings, target power, etc.
Error: Trying to use configured scan chain anyway...
Error: rspi.arm: IR capture error; saw 0x1f not 0x01
Warn : Bypassing JTAG setup events due to errors
Error: 'arm11 target' JTAG error SCREG OUT 0x1f
Error: unexpected ARM11 ID code
Error: [rspi.arm] Examination failed
Warn : target rspi.arm examination failed
Info : [rspi.arm] starting gdb server on 3333
Info : Listening on port 3333 for gdb connections
```

As the device is not connected, it is expected to see errors about JTAG scan chain interrogation failing.

## Connecting to the Target Device {#USING_ADAFRUIT_FT232H_FOR_JTAG_PROGRAMMING_CONNECTING_TO_THE_TARGET_DEVICE}

Now connect the FT232H to the target device using the following connections:

| Signal name | FT232H Pin | Target Pin       | Color Code |
|-------------|------------|------------------|------------|
| TCK         | ADBUS0     | GPIO 25 (Pin 22) | Brown      |
| TDI         | ADBUS1     | GPIO 26 (Pin 37) | Red        |
| TDO         | ADBUS2     | GPIO 24 (Pin 18) | Orange     |
| TMS         | ADBUS3     | GPIO 27 (Pin 13) | Yellow     |
| TRST        | -          | 3.3V (Pin 17)    | Blue       |
| RTCK        | -          | GPIO 23 (Pin 16) | -          |
| SRST        | ACBUS1     | 3.3V (Pin 17)    | Green      |
| GND         | GND        | GND (Pin 14)     | Black      |

Make sure to connect the GND pins of both devices.

## Update Raspberry Pi Configuration {#USING_ADAFRUIT_FT232H_FOR_JTAG_PROGRAMMING_UPDATE_RASPBERRY_PI_CONFIGURATION}

On the Raspberry Pi, you need to enable the JTAG interface. You can do this by adding the following lines to `/boot/config.txt`:
```text
[all]
enable_jtag_gpio=1
gpio=22-27=a4
```

I bought the Adafruit FT232H (https://www.adafruit.com/product/2264) last week with the hope of using its MPSSE (Multi-Protocol Synchronous Serial Engine) mode as a "JTAG Pod" with OpenOCD... to debug a target device (in the first case, a Raspberry Pi 3b+).

I've read through several datasheets (on the FT232H) and the info posted by Adafruit. I was able to quickly/easily use the UART mode of the FT232H immediately; I hooked it up to the RPi's debug console GPIO Rx/Tx pins without problems.

I've run through the "Adafruit Learn" info--that largely uses CircuitPython Libraries (on PC)--and the C0-C7, D0-D7, Gnd, Vcc 3.5, Vcc 5 pins all seem to be functioning... I can control an LED from each of them through GPIO mode using Blinka.

So, it *seems* like my soldering job was ok.

However...

I'm having the worst time trying to configure OpenOCD to use the FT232H as JTAG. I've checked and rechecked the wiring between the FT232H and the target (RPi)... many, many times. It seems correct.

All I get back from OpenOCD is infinitely frustrating errors:

Error: JTAG scan chain interrogation failed: all ones
Error: Check JTAG interface, timings, target power, etc.
Error: Trying to use configured scan chain anyway...

Is there any chance you guys can help me over this hurdle, please?

I'm using only 4 pins, TCK, TDO, TDI, TMS... no SRST, no TRST... and GND.
I'm running the target Rpi with a vanilla Raspberry Pi OS Lite, 64 bit... just to have a valid target and all of its utilities available to help get the FT232H configured correctly. The Rpi target has been properly configured to use "alt 4" for GPIO pins 22 - 27 (verified with "pinctrl -p"); so, it seems to be in the right state for JTAG debugging.

For the OpenOCD .cfgs, I'm using a modified version of the interface/ftdi/ft232h-module-swd.cfg that comes with OpenOCD (which seems to have originally come from you guys?) and the target/bcm2837.cfg that also comes with OpenOCD.

The modifications to the ft232h-module-swd.cfg file were:
* commenting out the "ftdi layout_signal" lines
* commenting out the "transport select swd" line

I suspect part of the problem is associated with the "ftdi layout_init" since I'm not using the SRST and TRST pins. It's not clear to me how to interpret and modify this, though, and anything I've tried doesn't change the error results.

I've also tried following a few tutorials that people have posted... they're usually not exactly the same situation... and all of them end up generating the same error results for me.

I'm not sure what else to try at this point... other than spending more $$ to buy a dedicated JTAG device :-(

What do you think? Can you help? :-)

I've found out how to interpret the OpenOCD "ftdi layout_init" data--how each bit corresponds to specific FT232H pins, how to set the initial values, and how to set the correct directions.

I found out that the FT232H and all of its pins have been working correctly all along.

It turns out that the Rpi 3b+, itself, is setting its TDO to "hi" for some unknown reason. It never changes. This is definitely not behaving correctly.

I guess the plan, now, is to ask the Rpi guys, directly.

Hopefully this little nugget will help save someone else some time ;-)

Even though it's not explicitly mentioned anywhere, the TRST pin is NOT optional (at least for Rpi 3b+). The TRST pin on the RPi needs to be driven high (1).

When the RPi's TRST is not driven high, the RPi's TDO outputs a constant high (1)... in effect non-responsive to any JTAG commands.

When the RPi's TRST is driven high, the RPi's TDO behaves correctly, sending responses to JTAG commands.

Who knew?!

I ran FT_Prog from FTDI. It turns out that the EEPROM on my FT232H was blank for some reason.

I programmed it with defaults--which will generate a serial number if one is not provided explicitly--and now it shows an English description and has a serial number (and all of the settings are set to known/expected values).

Did Adafruit expect to ship the breakout device with an empty EEPROM? If not, it's something to investigate :-)
