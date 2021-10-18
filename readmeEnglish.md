# Fake86
Port of the Fake86 emulator (Mike Chambers) to the TTGO VGA32 v1.0, v1.1, v1.2 and v1.4 board with ESP32.
<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/pakupaku.gif'></center>
I have made several modifications:
<ul>
 <li>Ported from x86 PC to ESP32</li>
 <li>No PSRAM used, running on ESP32 with 520 KB RAM (TTGO VGA32 v1.x)</li> 
 <li>Single core usage (possibility of 2)</li>
 <li>Low-resource OSD</li>
 <li>Created project compatible with Arduino IDE and Platform IO</li>
 <li>Any digital pin can be used for the audio (SPEAKER_PIN in hardware.h)</li>
 <li>It can use any digital pin for the audio (SPEAKER_PIN in hardware.h)</li>
 <li>Any digital pin can be used for the video (hardware.h)</li> 
 <li>Emulation video speed menu, VGA, keyboard</li>
 <li>Menu of emulation video speed, VGA, keyboard</li>
 <li>Support for 64 color mode</li>.    
 <li>VGA 320x200</li>
 <li>The full bitluni library is not required. I have reduced to the minimum, saving RAM and FLASH, based on Ricardo Massaro's library</li>.
 <li>Text mode support 80x25</li>
 <li>Text mode 40x25</li>
 <li>Support text mode 40x25</li>.
 <li>Support video mode 4 and 5, 320x200 2bpp</li>
 <li>Mode video support 4 and 5, 320x200 2bpp</li>
 <li>Video mode 6, 640x200 1bpp</li> <li>Support video mode 6, 640x200 1bpp</li>
 <li>Soporte modo texto 160x100 (Paku Paku)</li>
 <li>Al usar sólo SRAM, para esta versión de test se ha dejado sólo emular 160 KB (163840 bytes)</li>
 <li>El OSD se saca con la tecla <b>F12</b></li>
</ul> 


<br><br>
<h1>Requerimientos</h1>
Se requiere:
 <ul>
  <li>TTGO VGA32 v1.x (1.0, 1.1, 1.2, 1.4) o Single core ESP32</li>
  <li>Visual Studio 1.48.1 PLATFORMIO 2.2.0</li>
  <li>Espressif 32 v3.3.2</li>
  <li>Arduino IDE 1.8.11</li>
 </ul>
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/ttgovga32v12.jpg'></center>
La imagen adjunta es de mi placa TTGO VGA32 v1.2 sin psram ni módulo de SD.
<br>


<br><br>
<h1>PlatformIO</h1>
Se debe instalar el PLATFORMIO 2.2.0 desde las extensiones del Visual Studio.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFairChild/main/preview/previewPlatformIOinstall.gif'></center>
Luego se seleccionará el directorio de trabajo <b>Tinyfake86ttgovga32</b>.
Debemos modificar el fichero <b>platformio.ini</b> la opción <b>upload_port</b> para seleccionar el puerto COM donde tenemos nuestra placa TTGO VGA32.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewPlatformIO.gif'></center>
Then we will proceed to compile and upload to the board. No partitions are used, so we must upload the entire compiled binary.
It's all set up so we don't have to install any libraries.


<br><br>
<h1>Arduino IDE</h1>
The whole project is compatible with the Arduino 1.8.11 framework.
We just need to open the <b>fairChild.ino</b> in the <b>fairChild</b> directory.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewArduinoIDEpreferences.gif'></center>
For normal mode, the project is already prepared, so no library is needed. 
We must disable the PSRAM option, and in case of exceeding 1 MB of binary, select 4 MB partition at upload time. Although the code does not use PSRAM, if the option is active and our ESP32 does not have it, an exception will be generated and it will restart in loop mode.



<br><br>
<h1>Paku Paku</h1>.
The game uses CGA 160x100 video mode, which is a CGA text mode, where only 2 lines of character 221 and 222 are drawn, skipping the special background bit 7 (flicker), to achieve the 16 background colors, as well as foreground.
When using the 320x200 fixed video mode, only 40x25 will be displayed, i.e. 80x100. Therefore from the OSD menu, we must choose font 4x8, to show us the full screen 80x25, i.e. 160x100, in scaling reduction.


<br><br>
<h1>COM</h1>
A .COM set, 64 KB in size, can be loaded at any time. Since the data is copied to RAM and a jump is made to the code, ideally, a COM should be loaded from the OSD when the emulator is starting up, i.e. during the memory test.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewBoot.gif'></center>
If this is done later, e.g. when the OS is already up, the COM will use all interrupts and timers changed by the software itself.


<br><br>
<h1>BASIC ROM</h1>.
After the RAM test, the ROM BASIC can be started by pressing the specifier bar.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewBASIC.gif'></center>
If it is not pressed, the BOOTSTRAP, i.e. the floppy disk, will be booted.



<br><br>
<h1>Timers</h1>.
When a game is running too fast, we can reduce the speed from the OSD by changing the poll timer in milliseconds and adding a CPU delay in milliseconds.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewDigger.gif'></center>
