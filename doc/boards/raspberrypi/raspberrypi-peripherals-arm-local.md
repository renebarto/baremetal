# Raspberry Pi ARM local device registers {#RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS}

See [documentation](pdf/bcm2836-peripherals.pdf), page 173

For the introduction of the Quad core CPU in Raspberry Pi 2, some new registers were added, which are not well described.
Their address range is also different from the BCM peripheral registers
The base address for Raspberry Pi 3 and below is 0x40000000, for Raspberry 4 and later this is 0xFF800000

<table>
<caption id="ARM_local_registers">ARM local registers</caption>
<tr><th>Register                    <th>Address        <th>Bits<th>Name                  <th>Acc<td>Meaning</tr>
<tr><td>ARM_LOCAL_TIMER_INT_CONTROL0<td>Base+0x00000040<td>31:8<td>-                     <td>-  <td>Reserved</tr>
<tr><td>                            <td>               <td>7   <td>nCNTVIRQ FIQ control  <td>R/W<td>Counter-timer Virtual Timer Core 0 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>6   <td>nCNTHPIRQ FIQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 0 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>5   <td>nCNTPNSIRQ FIQ control<td>R/W<td>Counter-timer Physical Timer Core 0 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>4   <td>nCNTPSIRQ FIQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 0 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>3   <td>nCNTVIRQ IRQ control  <td>R/W<td>Counter-timer Virtual Timer Core 0 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>2   <td>nCNTHPIRQ IRQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 0 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>1   <td>nCNTPNSIRQ IRQ control<td>R/W<td>Counter-timer Physical Timer Core 0 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>0   <td>nCNTPSIRQ IRQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 0 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>ARM_LOCAL_TIMER_INT_CONTROL1<td>Base+0x00000044<td>31:8<td>-                     <td>-  <td>Reserved</tr>
<tr><td>                            <td>               <td>7   <td>nCNTVIRQ FIQ control  <td>R/W<td>Counter-timer Virtual Timer Core 1 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>6   <td>nCNTHPIRQ FIQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 1 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>5   <td>nCNTPNSIRQ FIQ control<td>R/W<td>Counter-timer Physical Timer Core 1 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>4   <td>nCNTPSIRQ FIQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 1 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>3   <td>nCNTVIRQ IRQ control  <td>R/W<td>Counter-timer Virtual Timer Core 1 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>2   <td>nCNTHPIRQ IRQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 1 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>1   <td>nCNTPNSIRQ IRQ control<td>R/W<td>Counter-timer Physical Timer Core 1 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>0   <td>nCNTPSIRQ IRQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 1 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>ARM_LOCAL_TIMER_INT_CONTROL2<td>Base+0x00000048<td>31:8<td>-                     <td>-  <td>Reserved</tr>
<tr><td>                            <td>               <td>7   <td>nCNTVIRQ FIQ control  <td>R/W<td>Counter-timer Virtual Timer Core 2 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>6   <td>nCNTHPIRQ FIQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 2 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>5   <td>nCNTPNSIRQ FIQ control<td>R/W<td>Counter-timer Physical Timer Core 2 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>4   <td>nCNTPSIRQ FIQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 2 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>3   <td>nCNTVIRQ IRQ control  <td>R/W<td>Counter-timer Virtual Timer Core 2 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>2   <td>nCNTHPIRQ IRQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 2 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>1   <td>nCNTPNSIRQ IRQ control<td>R/W<td>Counter-timer Physical Timer Core 2 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>0   <td>nCNTPSIRQ IRQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 2 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>ARM_LOCAL_TIMER_INT_CONTROL3<td>Base+0x0000004C<td>31:8<td>-                     <td>-  <td>Reserved</tr>
<tr><td>                            <td>               <td>7   <td>nCNTVIRQ FIQ control  <td>R/W<td>Counter-timer Virtual Timer Core 3 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>6   <td>nCNTHPIRQ FIQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 3 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>5   <td>nCNTPNSIRQ FIQ control<td>R/W<td>Counter-timer Physical Timer Core 3 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>4   <td>nCNTPSIRQ FIQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 3 FIQ enable<br>
0 = FIQ disabled<br>
1 = FIQ enabled (overrides IRQ enable)</tr>
<tr><td>                            <td>               <td>3   <td>nCNTVIRQ IRQ control  <td>R/W<td>Counter-timer Virtual Timer Core 3 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>2   <td>nCNTHPIRQ IRQ control <td>R/W<td>Counter-timer Hypervisor Physical Timer Core 3 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>1   <td>nCNTPNSIRQ IRQ control<td>R/W<td>Counter-timer Physical Timer Core 3 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
<tr><td>                            <td>               <td>0   <td>nCNTPSIRQ IRQ control <td>R/W<td>Counter-timer Physical Secure Timer Core 3 IRQ enable<br>
0 = IRQ disabled<br>
1 = IRQ enabled (if FIQ not enabled)</tr>
</table>
