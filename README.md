<h1><u><b>Micro Keyer</b></u></h1>

<div style="display: flex; gap: 10px; justify-content: space-between;">

  <img src="images/main.jpg" alt="Alt text" height="400">
  <img src="images/main2.jpg" alt="Alt text" height="400">
</div>
<div style="display: flex; gap: 10px; justify-content: space-between;">
  <img src="images/open screen.jpg" alt="Alt text" height="150">
  <img src="images/open front.jpg" alt="Alt text" height="150">
  <img src="images/open back.jpg" alt="Alt text" height="150">
  <img src="images/pcb front.jpg" alt="Alt text" height="150">
  <img src="images/pcb back.jpg" alt="Alt text" height="150">
</div>

<br>
<br>
The <b>Micro Keyer</b> is a small morse code keyer that can be used as a practice oscillator and computer keyer. It supports Iambic A, Iambic B, bug emulation, and straight key keying modes.
<br>
<br>
<h1><u><b><a href="https://grilled-cheese-engineering.github.io/micro-keyer/" target="_blank">View bill of materials</a></b></u></h1>
<br>
<br>
<br>

<h2><u><b>Version Notes:</b></u></h2>
<ul>
  <li><h4><u><b>V1.0 Board:</b></u></h4> There is a problem with the V1.0 board where the display does not turn off when the device is turned off with the power switch. <h4><u><b>Fix:</b></u></h4>
      <ol type="1">
        <li>Do not solder the jumper pad that connects the display to power.</li> 
        <li>Connect a jumper wire between top leg of Q1 (3V) to the left side of the power switch.</li>  
        <li>Solder a wire from the center pin of the switch to the power pin on the display.</li> 
      </ol>
      <h5>(Note: the two side pins on the switch are not connected to anything so it is okay if the jumper wire is making contact with them.)</h5>
      <br>
      <img src="images/open back.jpg" alt="Alt text" height="150">
  </li>
</ul>

<h2><u><b>Menu Usage:</b></u></h2>
<ul>
<br>

You can clear the decoded text by holding the knob down for 2 seconds.
<br>
  
  <li>Speed: Sending speed in words per minute.</li>
  <li>Tone: Audio tone for sending in hetrz. (It may not sound exactly right on the built in buzzer compared to a real speaker.)</li>
  <li>Mode:</li> <ol type="1">
   <li>Iambic A: This is for a morse code paddle and it sends exactly what you are holding once the current element finishes being sent.</li>
   <li>Iambic B: This is for a morse code paddle and it has a memory for what has been pressed so for if you press the dit side during a dah and let go of the paddle then it will send a dit once it finishes the dah.</li>
   <li>Stright Key: This can be used with a straight key/bug or it can be used with a paddle to emulate a cootie. It sends only when the key is being pressed.</li>
   <li>Bug Emu: This is for using a paddle as a bug. It will automaticaly send dits at the set speed but the dah side will act as a straight key</li>
  </ol>
  <li>Reverse: This reverses which side of the paddle is dit and which is dah.</li>
  <li>Mute: This will mute the audio from both the built in buzzer and the head phone jack.</li>
  <li>Serial: This will enable the ability to do computer keying over serial.</li>
  <li>HID: This will enable the ability to use it as a USB keyboard.</li>
  <li>MIDI: This will enable the ability to use it as a USB MIDI device.</li>
  <li>Dumb MIDI: This disables some features and only puts out the paddle state over midi. (Experimental)</li>
  <li>Flash Firmware: This puts the device into boot mode to allow for firmware flashing. It will appear on a computer as a USB mass storage device. To flash the firmware just copy the .UF2 firmware file onto it.</li>
</ul>
