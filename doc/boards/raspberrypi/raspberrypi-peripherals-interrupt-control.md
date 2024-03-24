# Raspberry Pi Interrupt Control {#RASPBERRY_PI_INTERRUPT_CONTROL}

See [documentation](pdf/bcm2835-peripherals.pdf), page 109

<table>
<caption id="Interrupt_control_registers">Interrupt control registers</caption>
<tr><th>Register          <th>Address        <th>Bits <th>Name                             <th>Acc<th>Meaning</tr>
<tr><td>IRQ_BASIC_PENDING <td>Base+0x0000B200<td>31-21<td>Unused                           <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>20   <td>GPU IRQ 62                       <td>R  <td>GPU IRQ 62 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>19   <td>GPU IRQ 57                       <td>R  <td>GPU IRQ 57 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>18   <td>GPU IRQ 56                       <td>R  <td>GPU IRQ 56 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>17   <td>GPU IRQ 55                       <td>R  <td>GPU IRQ 55 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>16   <td>GPU IRQ 54                       <td>R  <td>GPU IRQ 54 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>15   <td>GPU IRQ 53                       <td>R  <td>GPU IRQ 53 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>14   <td>GPU IRQ 19                       <td>R  <td>GPU IRQ 19 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>13   <td>GPU IRQ 18                       <td>R  <td>GPU IRQ 18 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>12   <td>GPU IRQ 10                       <td>R  <td>GPU IRQ 10 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>11   <td>GPU IRQ 9                        <td>R  <td>GPU IRQ 9 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>10   <td>GPU IRQ 7                        <td>R  <td>GPU IRQ 7 pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>9    <td>IRQ pending register 2           <td>R  <td>IRQ pending 2<br>
1 = one or more interrupts pending<br>
0 = no interrupts pending<br>
<tr><td>                  <td>               <td>8    <td>IRQ pending register 1           <td>R  <td>IRQ pending 1<br>
1 = one or more interrupts pending<br>
0 = no interrupts pending<br>
<tr><td>                  <td>               <td>7    <td>Illegal access type 0 IRQ pending<td>R  <td>Illegal access type 0 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>6    <td>Illegal access type 1 IRQ pending<td>R  <td>Illegal access type 1 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>5    <td>GPU1 halted IRQ pending          <td>R  <td>GPU 1 halted interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>4    <td>GPU0 halted IRQ pending          <td>R  <td>GPU 0 halted interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>3    <td>ARM Doorbell 1 IRQ pending       <td>R  <td>ARM Doorbell 1 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>2    <td>ARM Doorbell 0 IRQ pending       <td>R  <td>ARM Doorbell 0 interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>1    <td>ARM Mailbox IRQ pending          <td>R  <td>ARM Mailbox interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>                  <td>               <td>0    <td>ARM Timer IRQ pending            <td>R  <td>ARM Timer interrupt pending<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>IRQ_PENDING_1     <td>Base+0x0000B204<td>31-0 <td>GPU IRQ 0..31 pending            <td>R  <td>GPU IRQ pending 0..31<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>IRQ_PENDING_2     <td>Base+0x0000B208<td>31-0 <td>GPU IRQ 32..63 pending           <td>R  <td>GPU IRQ pending 32..63<br>
1 = interrupt pending<br>
0 = no interrupt pending</tr>
<tr><td>FIQ_CONTROL       <td>Base+0x0000B20C<td>31-8 <td>Unused                           <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>7    <td>FIQ enable                       <td>R/W<td>Enable FIQ. Only 1 FIQ can be enabled at any time<br>
1 = FIQ enabled<br>
0 = FIQ disabled)</tr>
<tr><td>                  <td>               <td>6:0  <td>Select FIQ source                <td>R/W<td>Select FIQ source<br>
0-63: GPU Interrupts (See GPU IRQ table)<br>
64: ARM Timer interrupt<br>
65: ARM Mailbox interrupt<br>
66: ARM Doorbell 0 interrupt<br>
67: ARM Doorbell 1 interrupt<br>
68: GPU0 Halted interrupt (Or GPU1)<br>
69: GPU1 Halted interrupt<br>
70: Illegal access type-1 interrupt<br>
71: Illegal access type-0 interrupt<br>
72-127: Do Not Use</tr>
<tr><td>ENABLE_IRQS_1     <td>Base+0x0000B210<td>31:0 <td>GPU IRQ 31:0 enable              <td>R/W<td>GPU IRQ 0..31 enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>ENABLE_IRQS_2     <td>Base+0x0000B214<td>31:0 <td>GPU IRQ 63:32 enable              <td>R/W<td>GPU IRQ 32:63 enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>ENABLE_BASIC_IRQS <td>Base+0x0000B218<td>31:8 <td>Unused                           <td>-  <td>Unused</tr>
<tr><td>                  <td>               <td>7    <td>Illegal access type 0 IRQ enable <td>R/W<td>Illegal access type 0 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>6    <td>Illegal access type 1 IRQ enable <td>R/W<td>Illegal access type 1 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>5    <td>GPU 1 Halted IRQ enable          <td>R/W<td>GPU 1 Halted IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>4    <td>GPU 0 Halted IRQ enable          <td>R/W<td>GPU 0 Halted IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>3    <td>ARM Doorbell 1 IRQ enable        <td>R/W<td>ARM Doorbell 1 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>2    <td>ARM Doorbell 0 IRQ enable        <td>R/W<td>ARM Doorbell 0 IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>1    <td>ARM Mailbox IRQ enable            <td>R/W<td>ARM Mailbox IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>                  <td>               <td>0    <td>ARM Timer IRQ enable              <td>R/W<td>ARM Timer IRQ enable<br>
1 = interrupt enabled<br>
0 = no change</tr>
<tr><td>DISABLE_IRQS_1    <td>Base+0x0000B21C<td>31:0 <td>GPU IRQ 31:0 disable             <td>R/W<td>GPU IRQ 0..31 disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>DISABLE_IRQS_2    <td>Base+0x0000B220<td>31:0 <td>GPU IRQ 63:32 disable             <td>R/W<td>GPU IRQ 32:63 disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>DISABLE_BASIC_IRQS<td>Base+0x0000B224<td>31:8 <td>Unused                           <td>-  <td>Unused</tr>
<tr><td>                 <td>               <td>7    <td>Illegal access type 0 IRQ disable<td>R/W<td>Illegal access type 0 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>6    <td>Illegal access type 1 IRQ disable<td>R/W<td>Illegal access type 1 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>5    <td>GPU 1 Halted IRQ disable         <td>R/W<td>GPU 1 Halted IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>4    <td>GPU 0 Halted IRQ disable         <td>R/W<td>GPU 0 Halted IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>3    <td>ARM Doorbell 1 IRQ disable       <td>R/W<td>ARM Doorbell 1 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>2    <td>ARM Doorbell 0 IRQ disable       <td>R/W<td>ARM Doorbell 0 IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>1    <td>ARM Mailbox IRQ disable           <td>R/W<td>ARM Mailbox IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
<tr><td>                 <td>               <td>0    <td>ARM Timer IRQ disable             <td>R/W<td>ARM Timer IRQ disable<br>
1 = interrupt disabled<br>
0 = no change</tr>
</table>
