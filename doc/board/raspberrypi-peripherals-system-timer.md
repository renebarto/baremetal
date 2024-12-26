# Raspberry Pi System timer {#RASPBERRY_PI_SYSTEM_TIMER}

See documentation:
- [Broadcom documentation BCM2837 (Raspberry Pi 3)](pdf/bcm2837-peripherals.pdf) (section `12 System Timer`)
- [Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/bcm2711-peripherals.pdf) (`Chapter 10. System Timer`)
- [Broadcom documentation RP1 (Raspberry Pi 5)](pdf/rp1-peripherals.pdf) (section `3.8. TICKS`).

<table>
<caption id="System_timer_registers">System timer registers</caption>
<tr><th>Register       <th>Address        <th>Bits<th>Name<th>Access<td>Meaning</tr>
<tr><td>ARM_SYSTMR_CS  <td>Base+0x00003000<td>31:4<td>-   <td>R/W   <td>Reserved, write as 0, read as don't care. Reset level: 0</tr>
<tr><td>               <td>               <td>3   <td>M3  <td>R/W   <td>System Timer Match 3. Timer match since last cleared (0 = no match 1 = match). Reset level: 0</tr>
<tr><td>               <td>               <td>2   <td>M2  <td>R/W   <td>System Timer Match 2. Timer match since last cleared (0 = no match 1 = match). Reset level: 0</tr>
<tr><td>               <td>               <td>1   <td>M1  <td>R/W   <td>System Timer Match 1. Timer match since last cleared (0 = no match 1 = match). Reset level: 0</tr>
<tr><td>               <td>               <td>0   <td>M0  <td>R/W   <td>System Timer Match 0. Timer match since last cleared (0 = no match 1 = match). Reset level: 0</tr>
<tr><td>ARM_SYSTMR_LO  <td>Base+0x00003004<td>31:0<td>CLO <td>R     <td>Free running timer low 32 bits</tr>
<tr><td>ARM_SYSTMR_HI  <td>Base+0x00003008<td>31:0<td>CHI <td>R     <td>Free running timer high 32 bits</tr>
<tr><td>ARM_SYSTMR_CMP0<td>Base+0x0000300C<td>31:0<td>C0  <td>R/W   <td>Time match value 0, will match when low 32 bits are equal to value.</tr>
<tr><td>ARM_SYSTMR_CMP1<td>Base+0x00003010<td>31:0<td>C1  <td>R/W   <td>Time match value 1, will match when low 32 bits are equal to value.</tr>
<tr><td>ARM_SYSTMR_CMP2<td>Base+0x00003014<td>31:0<td>C2  <td>R/W   <td>Time match value 2, will match when low 32 bits are equal to value.</tr>
<tr><td>ARM_SYSTMR_CMP3<td>Base+0x00003018<td>31:0<td>C3  <td>R/W   <td>Time match value 3, will match when low 32 bits are equal to value.</tr>
</table>
