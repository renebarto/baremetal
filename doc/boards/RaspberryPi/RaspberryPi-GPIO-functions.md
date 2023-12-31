# GPIO functions

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

## Alternative functions for GPIO

\page BCM_GPIO_ALTERNATIVE_FUNCTIONS Alternative functions for GPIO
@{
@details Alternative functions for GPIO\n
See [documentation](BCM2837-peripherals.pdf), page 102

<table>
<caption id="multi_row">GPIO alternative functions</caption>
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
@}

