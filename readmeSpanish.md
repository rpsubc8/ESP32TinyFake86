
# Fake86
Port del emulador de Fake86 (Mike Chambers) a la placa TTGO VGA32 v1.0, v1.1, v1.2 y v1.4 con ESP32.
<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/pakupaku.gif'></center>
He realizado varias modificaciones:
<ul>
 <li>Portado de x86 PC a ESP32</li>
 <li>No se usa PSRAM, funcionando en ESP32 de 520 KB de RAM (TTGO VGA32 v1.x)</li> 
 <li>Uso de un sólo core (posibilidad de 2)</li>
 <li>OSD de bajos recursos</li>
 <li>Creado proyecto compatible con Arduino IDE y Platform IO</li>
 <li>Se puede usar cualquier pin digital para el audio (SPEAKER_PIN en hardware.h)</li>
 <li>Se puede usar cualquier pin digital para el video (hardware.h)</li> 
 <li>Menú de velocidad de video de emulación, VGA, teclado</li>
 <li>Soporte para modo 64 colores.</li>    
 <li>VGA 320x200</li>
 <li>No se requiere la libreria de bitluni completa. He reducido a lo mínimo, ahorrando RAM y FLASH, basado en la librería de Ricardo Massaro</li>
 <li>Soporte modo texto 80x25</li>
 <li>Soporte modo texto 40x25</li>
 <li>Soporte modo video 4 y 5, 320x200 2bpp</li>
 <li>Soporte modo video 6, 640x200 1bpp</li>
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
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewPlatformIOinstall.gif'></center>
Luego se seleccionará el directorio de trabajo <b>Tinyfake86ttgovga32</b>.
Debemos modificar el fichero <b>platformio.ini</b> la opción <b>upload_port</b> para seleccionar el puerto COM donde tenemos nuestra placa TTGO VGA32.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewPlatformIO.gif'></center>
Luego procederemos a compilar y subir a la placa. No se usa particiones, así que debemos subir todo el binario compilado.
Está todo preparado para no tener que instalar ninguna librería.


<br><br>
<h1>Arduino IDE</h1>
Todo el proyecto es compatible con la estructura de Arduino 1.8.11.
Tan sólo tenemos que abrir el <b>fake86.ino</b> del directorio <b>fake86</b>.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewArduinoIDEpreferences.gif'></center>
Para el modo normal, ya está preparado el proyecto, de forma que no se necesita ninguna librería. 
Debemos desactivar la opción de PSRAM, y en caso de superar 1 MB de binario, seleccionar 4 MB de partición a la hora de subir. Aunque el código no use PSRAM, si la opción está activa y nuestro ESP32 no dispone de ella, se generará una excepción y reinicio del mismo en modo bucle.



<br><br>
<h1>Paku Paku</h1>
El juego usa el modo de video CGA 160x100, que es un modo de texto CGA, donde sólo se dibujan 2 lineas del caracter 221 y 222, saltándose el bit especial 7 de background (parpadeo), para lograr los 16 colores de fondo, así como primer plano.
Al usar el modo de video fijo de 320x200, se mostrará sólo 40x25, es decir, 80x100. Por tanto desde el menú del OSD, debemos elegir font 4x8, para que nos muestre la pantalla completa 80x25, es decir, 160x100, en reducción de escalado.


<br><br>
<h1>COM</h1>
En cualquier momento se puede cargar un juego .COM, de 64 KB. Dado que se copian los datos a RAM y se hace un salto al código, lo ideal, es cargar desde el OSD un COM cuando está arrancando el emulador, es decir, durante el test de memoria.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewBoot.gif'></center>
Si se hace después, por ejemplo, cuando ya está el Sistema Operativo, el COM usará todas las interrupciones y los timers que haya cambiado el propio software.


<br><br>
<h1>BASIC ROM</h1>
Después del test de RAM, se puede arrancar el BASIC de ROM, pulsando la barra especiadora.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewBASIC.gif'></center>
Si no se pulsa, se procederá al arranque del BOOTSTRAP, es decir, del disquete.



<br><br>
<h1>Timers</h1>
Cuando un juego nos vaya muy rápido, podemos reducir la velocidad desde el OSD, cambiando el timer poll en milisegundos y añadiendo un delay CPU en milisegundos.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewDigger.gif'></center>


<br><br>
<h1>DIY circuito</h1>
Si no queremos usar una placa TTGO VGA32 v1.x, podemos construirla siguiendo el esquema de <b>fabgl</b>:
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/fabglcircuit.gif'></center>
