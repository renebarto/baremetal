# Raspberry Pi Random number generator {#RASPBERRY_PI_RANDOM_NUMBER_GENERATOR}

<u>__Undocumented__</u>

<table>
<caption id="RNG_registers">RNG registers</caption>
<tr><th>Register       <th>Address        <th>Bits <th>Name<th>Access<td>Meaning</tr>
<tr><td>RNG_CTRL       <td>Base+0x00104000<td>31:1 <td>-   <td>?     <td>Unused?</tr>
<tr><td>               <td>               <td>0    <td>-   <td>?     <td>Enable generator? (1 = enable, 0 = disable)</tr>
<tr><td>RNG_STATUS     <td>Base+0x00104004<td>31:24<td>-   <td>?     <td>Number of words ready to read</tr>
<tr><td>               <td>               <td>23:20<td>-   <td>?     <td>Unused?</tr>
<tr><td>               <td>               <td>19   <td>-   <td>?     <td>Enable generator? (1 = enable, 0 = disable)</tr>
<tr><td>               <td>               <td>18:0 <td>-   <td>?     <td>Unused?</tr>
<tr><td>RNG_DATA       <td>Base+0x00104008<td>31:0 <td>-   <td>?     <td>Free running timer high 32 bits</tr>
<tr><td>RNG_INT_MASK   <td>Base+0x0010400C<td>31:1 <td>-   <td>?     <td>Unused?</tr>
<tr><td>               <td>               <td>0    <td>-   <td>?     <td>Mask interrupt (0 = enable, 1 = mask)</tr>
</table>

