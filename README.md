<h1><u><b>Micro Keyer</b></u></h1>

<div style="display: flex; gap: 10px; justify-content: space-between;">

  <img src="main.jpg" alt="Alt text" height="400">
  <img src="main2.jpg" alt="Alt text" height="400">
</div>
<div style="display: flex; gap: 10px; justify-content: space-between;">
  <img src="open screen.jpg" alt="Alt text" height="150">
  <img src="open front.jpg" alt="Alt text" height="150">
  <img src="open back.jpg" alt="Alt text" height="150">
  <img src="pcb front.jpg" alt="Alt text" height="150">
  <img src="pcb back.jpg" alt="Alt text" height="150">
</div>

<br>
<br>
The <b>Micro Keyer</b> is a small morse code keyer that can be used as a practice oscillator. It supports Iambic A, Iambic B, bug emulation, and straight key keying modes.




<h2><u><b>Version notes</b></u></h2>
<ul>
  <li>Board version V1.0: There is a problem with the power switch in the V1.0 board where the display does not turn off when the device is turned off via the power switch. To fix the problem, do the following:
    <br><br>
      <ol type="1">
        <li>Do not solder the jumper pad that connects the display to power.</li> 
        <li>Connect a jumper wire between top leg of Q1 (3V) to the left side of the power switch.</li>  
        <li>Solder a wire from the center pin of the switch to the power pin on the display.</li> 
      </ol>
      <br>
      (Note: the two side pins on the switch are not connected to anything so it is okay if the jumper wire is making contact with them. 
      <br>
      <img src="open back.jpg" alt="Alt text" height="150">
  </li>
</ul>
