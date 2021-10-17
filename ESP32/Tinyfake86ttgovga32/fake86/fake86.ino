//Port Fake86 to TTGO VGA32 by ackerman
//Load COM, DSK
// MODE320x200
// Single core and dual core
// Only SRAM
// Tiny mod VGA library bitluni
// gbConfig options configuration compile

#include "PS2Kbd.h"
#include <Arduino.h>
#ifndef use_lib_speaker_cpu
 #include <Ticker.h>
#endif 
#include "gbConfig.h"
#include "gbGlobals.h"
#include "fake86.h"
#include "hardware.h"
#ifdef use_lib_bitluni_fast
#else
 #include "MartianVGA.h"
 //#include "def/Font.h"
#endif 
#include "hardware.h"
#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include "input.h"
#include "i8253.h"
#include "i8259.h"
#include "i8237.h"
#include "cpu.h"
#include "render.h"
#include "video.h"
#include "timing.h"
#include "disk.h"
#include "PS2KeyCode.h"
#include "dataFlash/com/comdigger.h"
#include "dataFlash/gbcom.h"
#include "gb_sdl_font8x8.h"
#include "osd.h"

#ifndef use_lib_singlecore
//Video Task Core BEGIN
 void videoTask(void *unused);
 QueueHandle_t vidQueue;
 TaskHandle_t videoTaskHandle;
 volatile bool videoTaskIsRunning = false;
 uint16_t *param;
//Video Task Core END
#endif

#ifndef use_lib_speaker_cpu
 Ticker gb_ticker_callback;
#endif 

unsigned char gb_invert_color=0;
unsigned char gb_silence=0;

unsigned char gb_delay_tick_cpu_milis= use_lib_delay_tick_cpu_milis;
unsigned char gb_auto_delay_cpu= use_lib_delay_tick_cpu_auto;
unsigned char gb_vga_poll_milis= use_lib_vga_poll_milis;
unsigned char gb_keyboard_poll_milis= use_lib_keyboard_poll_milis;
unsigned char gb_timers_poll_milis= use_lib_timers_poll_milis;

unsigned char gb_frec_speaker_low=0;
unsigned char gb_frec_speaker_high=0;
unsigned char gb_cont_frec_speaker=0;
volatile int gb_frecuencia01=0;
volatile int gb_volumen01=0;

unsigned char gb_use_remap_cartdridge=0;
unsigned char gb_use_snarare_madmix=0;

unsigned char gb_font_8x8=1;

unsigned int lasttick; //16 bits timer
unsigned char gb_reset=0;
unsigned char gb_id_cur_com=0;

unsigned char tiempo_vga= 0;
unsigned int jj_ini_cpu,jj_end_cpu,jj_ini_vga,jj_end_vga;
unsigned int gb_cpu_timer_before,gb_cpu_timer_cur;
unsigned int gb_max_cpu_ticks,gb_min_cpu_ticks,gb_cur_cpu_ticks;
unsigned int gb_max_vga_ticks,gb_min_vga_ticks,gb_cur_vga_ticks;
unsigned long tickgap=0;
unsigned char port3da=0;

unsigned char **gb_buffer_vga;
unsigned char gb_key_cur[45];
unsigned char gb_key_before[45];

//static unsigned char gb_color_vga[16]={
////Escala Gris
//unsigned char gb_color_vga[16]={
// //0x00,0x3F,0x01,0x29,0x21,0x19,0x10,0x1E,
// 0x00,0x15,0x2A,0x3F,0x21,0x19,0x10,0x1E,
// 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
//};

////CGA 2
//unsigned char gb_color_vga[16]={ 
// 0x00,0x08,0x02,0x06,
// 0x21,0x19,0x10,0x1E,
// 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
//};

//CGA 2 brillo
//unsigned char gb_color_vga[16]={ 
// 0x00,0x0C,0x03,0x06,
// 0x21,0x19,0x10,0x1E,
// 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
//};

////CGA 1
//unsigned char gb_color_vga[16]={ 
// 0x00,0x28,0x22,0x2A,
// 0x21,0x19,0x10,0x1E,
// 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
//};

//#include "dataFlash/gbrom.h"
//#include "PS2Boot/PS2KeyCode.h"
#ifdef use_lib_bitluni_fast
 #include "vga_6bit.h"
 // VGA output pins
 #define PIN_RED_LOW    21
 #define PIN_RED_HIGH   22
 #define PIN_GREEN_LOW  18
 #define PIN_GREEN_HIGH 19
 #define PIN_BLUE_LOW   4
 #define PIN_BLUE_HIGH  5
 #define PIN_HSYNC      23
 #define PIN_VSYNC      15

 static const unsigned char pin_config[] = {
  PIN_RED_LOW,
  PIN_RED_HIGH,
  PIN_GREEN_LOW,
  PIN_GREEN_HIGH,
  PIN_BLUE_LOW,
  PIN_BLUE_HIGH,
  PIN_HSYNC,
  PIN_VSYNC
 };

#endif

#ifdef use_lib_bitluni_fast 
 //unsigned char **framebuffer = vga_get_framebuffer();
 unsigned char gb_sync_bits;
#else 
 #ifdef use_lib_bitluni_3bpp
  #ifdef use_lib_bitluni_low_memory
   VGA3BitI vga;
  #else
   VGA3Bit vga;
  #endif 
 #else 
  VGA6Bit vga;
 #endif 
#endif


//const unsigned char gb_reserved_memory[16]={
// 0xEA,0x5B,0xE0,0x00,0xF0,0x30,0x35,0x2F,0x30,0x32,0x2F,0x31,0x32,0xFF,0xFE,0xE9
//};

unsigned char *gb_ram_00=NULL; //32 KB
unsigned char *gb_ram_01=NULL;
unsigned char *gb_ram_02=NULL;
unsigned char *gb_ram_03=NULL;
unsigned char *gb_ram_04=NULL;
unsigned char *gb_ram_bank[5];
unsigned char gb_video_cga[16384];
//unsigned char gb_video_hercules[16384];
unsigned char slowsystem=0;
unsigned short int constantw = 0, constanth = 0;
unsigned char bootdrive=0;
unsigned char speakerenabled=0;
unsigned char renderbenchmark= 0;
unsigned int gb_keyboard_time_before, gb_keyboard_time_cur;
unsigned int gb_ini_vga;
unsigned int gb_cur_vga;
unsigned char scrmodechange = 0;

//unsigned char updatedscreen=0;
unsigned int x,y;
unsigned short int segregs[4];
//const unsigned int palettecga[16]={
// 0x000000,0x0000AA,0x00AA00,0x00AAAA,
// 0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
// 0x555555,0x5555FF,0x55FF55,0x55FFFF,
// 0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF
//};

const unsigned char palettecga[16]={
 0x00,0xAA,0x00,0xAA,
 0x00,0xAA,0x00,0xAA,
 0x55,0xFF,0x55,0xFF,
 0x55,0xFF,0x55,0xFF
};

unsigned char gbKeepAlive=0;

#define uint8_t unsigned char

unsigned char vidmode=5;
unsigned char gb_force_set_cga=0; //fuerzo modo cga Digger
unsigned char gb_force_load_com=0;
unsigned char gb_force_boot=0;
unsigned char gb_force_load_dsk=0;
unsigned char gb_id_cur_dsk=0;

//JJ unsigned char portram[gb_max_portram];
unsigned char gb_portramTiny[51]; //Solo 51 puertos
void * gb_portTiny_write_callback[5]; //Solo 5
void * gb_portTiny_read_callback[5];
//unsigned char cf; //Lo dejo static en cpu optimizado
unsigned char cf;
unsigned char hdcount=0;
unsigned char running=0;

//extern void reset86();
//extern void exec86 (uint32_t execloops);

extern void VideoThreadPoll(void);
extern void draw(void);
extern void doscrmodechange();
//extern void handleinput();

extern uint64_t totalexec, totalframes;
uint64_t starttick, endtick;

uint32_t speed = 0;

//void KeepAlive()
//{           
// unsigned char auxColor;
// gbKeepAlive = ~gbKeepAlive;
// auxColor = (gbKeepAlive==0)?0:15;
// for (int y=0;y<4;y++)
// {
//  for (int x=0;x<4;x++)
//  {
//   jj_fast_putpixel(x,y,auxColor);
//  }
// }
//}


//extern uint8_t insertdisk (uint8_t drivenum, char *filename);
//extern void ejectdisk (uint8_t drivenum);
//extern void doirq (uint8_t irqnum);
//void inittiming();
//extern void init8253();
//extern void init8259();
//extern void init8237();
//extern void initVideoPorts();

void LoadCOMFlash(const unsigned char *ptr,int auxSize,int seg_load)
{
 int dir_load= seg_load*16;
 for (int i=0;i<auxSize;i++)
 {
  write86((dir_load+0x100+i),ptr[i]);
 }
 SetRegCS(seg_load);
 SetRegDS(seg_load);
 SetRegES(seg_load);
 SetRegSS(seg_load);
 SetRegIP(0x100);//0x100; 

 SetRegSP(0);
 SetRegBP(0);
 SetRegSI(0);
 SetRegDI(0);
 SetCF(0);     
}


void inithardware()
{
#ifdef NETWORKING_ENABLED
	//JJ if (ethif != 254) initpcap();
#endif
   #ifdef use_lib_log_serial
	 Serial.printf ("Initializing emulated hardware:\n");
   #endif
   #ifndef use_lib_not_use_callback_port
   memset(gb_portramTiny,0,sizeof(gb_portramTiny));
   memset(gb_portTiny_write_callback,0,sizeof(gb_portTiny_write_callback));
   memset(gb_portTiny_read_callback, 0, sizeof (gb_portTiny_read_callback));
	//JJ memset (port_write_callback, 0, sizeof (port_write_callback) );
	//JJ memset (port_read_callback, 0, sizeof (port_read_callback) );
	//JJ memset (port_write_callback16, 0, sizeof (port_write_callback16) ); //No se usa
	//JJ memset (port_read_callback16, 0, sizeof (port_read_callback16) ); //No se usa
   #endif
   #ifdef use_lib_log_serial	
	 Serial.printf ("  - Intel 8253 timer: ");
   #endif 
	init8253();
   #ifdef use_lib_log_serial
	 Serial.printf ("OK\n");
	 Serial.printf ("  - Intel 8259 interrupt controller: ");
   #endif 
	init8259();
   #ifdef use_lib_log_serial
	 Serial.printf ("OK\n");
	 Serial.printf ("  - Intel 8237 DMA controller: ");
   #endif 
	init8237();
   #ifdef use_lib_log_serial
	 Serial.printf ("OK\n");
   #endif 
	initVideoPorts();
	#ifdef use_lib_disneysound
	 //JJ if (usessource)
     //JJ {
	 //JJ 	printf ("  - Disney Sound Source: ");
	 //JJ 	initsoundsource();
	 //JJ 	printf ("OK\n");
	 //JJ }
	#endif
#ifndef NETWORKING_OLDCARD
	//JJ printf ("  - Novell NE2000 ethernet adapter: ");
	//JJ isa_ne2000_init (0x300, 6); //Red
	//JJ printf ("OK\n");
#endif
   #ifdef use_lib_adlib
	//JJ printf ("  - Adlib FM music card: ");
	//JJ initadlib (0x388); //Adlib
	//JJ printf ("OK\n");
   #endif	
	//JJ printf ("  - Creative Labs Sound Blaster Pro: ");
	//JJ initBlaster (0x220, 7); //Sound Blaster
	//JJ printf ("OK\n");
   #ifdef use_lib_mouse
	//JJ printf ("  - Serial mouse (Microsoft compatible): ");
	//JJ initsermouse (0x3F8, 4); //Raton
	//JJ printf ("OK\n");
   #endif	
	//JJ if (doaudio) initaudio();
	inittiming();
	//initscreen ( (uint8_t *) build);
	initscreen();
}


void ProcesarAcciones()
{
 if (gb_reset == 1)
 {
  gb_reset=0;
  ClearRAM();
  memset(gb_video_cga,0,16384);
  memset(gb_key_cur,1,45);
  memset(gb_key_before,1,45);
  running = 1;
  reset86();	
  inithardware();
  return;
 }
 if (gb_force_load_com == 1)
 {
  gb_force_load_com=0;
  int auxOffs= 0;
  if (gb_list_seg_load[gb_id_cur_com] == 0)
   auxOffs= 0x07C0;
  else
   auxOffs= 0x0051;       
  LoadCOMFlash(gb_list_com_data[gb_id_cur_com],gb_list_com_size[gb_id_cur_com],auxOffs);
  return;
 }
}

//****************************
void ClearRAM()
{
 int i;
 for (i=0;i<gb_max_ram;i++)
 {
  write86(i,0);
 }
}

//****************************
void CreateRAM()
{
 gb_ram_00 = (unsigned char *)malloc(32768);
 gb_ram_01 = (unsigned char *)malloc(32768);
 gb_ram_02 = (unsigned char *)malloc(32768);
 gb_ram_03 = (unsigned char *)malloc(32768);
 if (gb_max_ram >= 163840)
 {
  gb_ram_04 = (unsigned char *)malloc(32768);
  memset(gb_ram_04,0,32768);
  gb_ram_bank[4]= gb_ram_04;
 }

 memset(gb_ram_00,0,32768);
 memset(gb_ram_01,0,32768);
 memset(gb_ram_02,0,32768);
 memset(gb_ram_03,0,32768);

 gb_ram_bank[0]= gb_ram_00;
 gb_ram_bank[1]= gb_ram_01;
 gb_ram_bank[2]= gb_ram_02;
 gb_ram_bank[3]= gb_ram_03;
}

//*************************
//void ClearSDL()
//{//No hago nada
// for (int y=0;y<200;y++)
// {
//  for (int x=0;x<320;x++)
//  {
//   jj_fast_putpixel(x,y,0);
//  }
// }
//}



//Funciones
void Setup(void);
void SDL_DumpVGA(void);


//****************************
//void SDL_DumpVGA(void)
//{
// #ifdef use_lib_bitluni_fast        
//  //framebuffer = vga_get_framebuffer();  
//  for (int y=0;y<10;y++)
//  {
//   for (int x=0;x<10;x++)
//   {
//    //framebuffer[y][x^2] = vga_get_sync_bits() | 255;
//    framebuffer[y][x^2] = gb_sync_bits | 0x3F;
//   }
//  }
//  //vga_swap_buffers();
// #else
//  for (int y=0;y<10;y++)
//  {
//   for (int x=0;x<10;x++)
//   {     
//    #ifdef use_lib_bitluni_3bpp
//     vga.dotFast(x,y,0x07);
//    #else
//     vga.dotFast(x,y,0x3F);
//    #endif 
//   }
//  }
// #endif 
//}



//******************************************
//Setup principal
void setup()
{ 
 pinMode(SPEAKER_PIN, OUTPUT);
 //REG_WRITE(GPIO_OUT_W1TC_REG , BIT25); //LOW clear
 digitalWrite(SPEAKER_PIN, LOW);  

 #ifdef use_lib_log_serial
  Serial.begin(115200);         
  Serial.printf("HEAP BEGIN %d\n", ESP.getFreeHeap()); 
 #endif
 CreateRAM();
 ClearRAM();
 SetRAMTruco();//Primero pongo RAM 
 bootstrapPoll(); //Al inicio
 //insertdisk (0, "COMPAQDOS211cat.img");
 memset(gb_key_cur,1,45);
 memset(gb_key_before,1,45);
 FuerzoParityRAM(); //Fuerzo que Parity sea en RAM
 
 #ifdef use_lib_bitluni_fast  
  //void vga_init(const int *pin_map, const VgaMode &mode, bool double_buffered)
  //vga_init(pin_config,vga_mode_320x240,false);
  vga_init(pin_config,VgaMode_vga_mode_320x200,false);  
  gb_sync_bits= vga_get_sync_bits();
  gb_buffer_vga = vga_get_framebuffer();  
  #ifdef use_lib_log_serial
   Serial.printf("vsync_inv_bit:0x%02X hsync_inv_bit:0x%02X\n",vga_get_vsync_inv_bit(),vga_get_hsync_inv_bit());
   Serial.printf("Sync bits:0x%02X\n",gb_sync_bits);
  #endif 
  //En 320x200 sale bits:0x80 y deberia ser 0x40
  //En 320x240 sale bits:0xC0
 #else
  #ifdef use_lib_bitluni_3bpp   
   vga.init(vga.MODE320x200, RED_PIN_3B, GRE_PIN_3B, BLU_PIN_3B, HSYNC_PIN, VSYNC_PIN);        
   vga.fillRect(0,0,320,180,BLACK);
   vga.fillRect(0,0,320,180,BLACK);//fix mode fast video
   #ifdef use_lib_bitluni_low_memory
   #else
    Serial.printf("RGBMask:%02X Sbits:%2X\n",vga.RGBAXMask,vga.SBits);   
   #endif 
  #else
   #define RED_PINS_6B 21, 22
   #define GRE_PINS_6B 18, 19
   #define BLU_PINS_6B  4,  5
   const int redPins[] = {RED_PINS_6B};
   const int grePins[] = {GRE_PINS_6B};
   const int bluPins[] = {BLU_PINS_6B}; 
   //vga.init(vga.MODE320x240, redPins, grePins, bluPins, HSYNC_PIN, VSYNC_PIN);
   vga.init(vga.MODE320x200, redPins, grePins, bluPins, PIN_HSYNC, PIN_VSYNC);
   //vga.setFont(Font6x8);
   vga.clear(BLACK);
  
   vga.fillRect(0,0,320,180,BLACK);
   vga.fillRect(0,0,320,180,BLACK);//fix mode fast video  
   //vga.RGBAXMask,vga.SBits  
   Serial.printf("RGBMask:%02X Sbits:%2X\n",vga.RGBAXMask,vga.SBits);
   //RGBMask:3F Sbits:C0 320x240
   //RGBMask:3F Sbits:40 320x200
  #endif 
 #endif 
 
 #ifdef use_lib_bitluni_fast
 #else
  gb_buffer_vga= vga.backBuffer;
 #endif 
 
 #ifdef use_lib_log_serial
  Serial.printf("VGA %d\n", ESP.getFreeHeap()); 
 #endif
 PreparaColorVGA();

 kb_begin();


	running = 1;
	reset86();
   #ifdef use_lib_log_serial
	 Serial.printf ("OK!\n");    
   #endif 

	inithardware();

	lasttick = starttick = 0; //JJ millis();
	gb_ini_vga= lasttick;	
    //Serial.printf("Memoria:0x%02X 0x%02X\n",read86(0x413),read86(0x414));
    gb_keyboard_time_before= gb_keyboard_time_cur = gb_cpu_timer_before= gb_cpu_timer_cur= millis();

 
 #ifndef use_lib_singlecore
 //BEGIN TASK video
  vidQueue = xQueueCreate(1, sizeof(uint16_t *));
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0); 
 //END Task video 
 #endif

 #ifndef use_lib_speaker_cpu
  float auxTimer= (float)1.0/(float)SAMPLE_RATE;
  gb_ticker_callback.attach(auxTimer,my_callback_speaker_func); 
 #endif 

 #ifdef use_lib_log_serial  
  Serial.printf("END SETUP %d\n", ESP.getFreeHeap()); 
 #endif 
}

//***************************************************************
unsigned char TraduceTecla(unsigned char aux)
{
 unsigned char aRet=0;
 switch (aux)
 {
  case PS2_KC_A: aRet= 30; break; //a
  case PS2_KC_B: aRet= 48; break; //b
  case PS2_KC_C: aRet= 46; break; //c
  case PS2_KC_D: aRet= 32; break; //d
  case PS2_KC_E: aRet= 18; break; //e
  case PS2_KC_F: aRet= 33; break; //f
  case PS2_KC_G: aRet= 34; break; //g
  case PS2_KC_H: aRet= 35; break; //h
  case PS2_KC_I: aRet= 23; break; //i
  case PS2_KC_J: aRet= 36; break; //j
  case PS2_KC_K: aRet= 37; break; //k
  case PS2_KC_L: aRet= 38; break; //l
  case PS2_KC_M: aRet= 50; break; //m
  case PS2_KC_N: aRet= 49; break; //n
  //case PS2_KC_: aRet= 39; break; //ñ
  case PS2_KC_O: aRet= 24; break; //o
  case PS2_KC_P: aRet= 25; break; //p
  case PS2_KC_Q: aRet= 16; break; //q
  case PS2_KC_R: aRet= 19; break; //r
  case PS2_KC_S: aRet= 32; break; //s
  case PS2_KC_T: aRet= 20; break; //t
  case PS2_KC_U: aRet= 22; break; //u
  case PS2_KC_V: aRet= 47; break; //v
  case PS2_KC_W: aRet= 17; break; //w
  case PS2_KC_X: aRet= 45; break; //x
  case PS2_KC_Y: aRet= 21; break; //y
  case PS2_KC_Z: aRet= 44; break; //z
  case PS2_KC_0: aRet= 11; break; //0
  case PS2_KC_1: aRet= 2; break; //1
  case PS2_KC_2: aRet= 3; break; //2
  case PS2_KC_3: aRet= 4; break; //3
  case PS2_KC_4: aRet= 5; break; //4
  case PS2_KC_5: aRet= 6; break; //5
  case PS2_KC_6: aRet= 7; break; //6
  case PS2_KC_7: aRet= 8; break; //7
  case PS2_KC_8: aRet= 9; break; //8
  case PS2_KC_9: aRet= 10; break; //9
  case PS2_KC_ENTER: aRet= 28; break; //ENTER
  case PS2_KC_SPACE: aRet= 57; break; //Barra espaciadora
  case KEY_BACKSPACE: aRet= 14; break; //Back space 
  //case PS2_KC_BACK: aRet= 14; break; //Back space
  case PS2_KC_ESC: aRet= 1; break; //ESC
  case PS2_KC_F8: aRet= 66; break; //F8
  case KEY_CURSOR_LEFT: aRet= 75; break; //izquierda
  case KEY_CURSOR_RIGHT: aRet= 77; break; //derecha
  case KEY_CURSOR_UP: aRet= 72; break; //arriba 
  case KEY_CURSOR_DOWN: aRet= 80; break; //abajo  
  case PS2_KC_F1: aRet= 59; break; //F1  
  default: aRet=0; break;
 }
 return aRet;
}

//*********************************************
//unsigned char GetPilaTeclaPulsada()
//{
//
//}

//*********************************************
void CheckTeclaSDL(int aux, unsigned char pulsado)
{
 //unsigned char aRet=0;
 //unsigned char codigo = TraduceTecla(aux);
 if (pulsado == 0)
 {
  //JJ puerto portram[0x60] = aux;
  //JJ puerto portram[0x64] |= 2; 
  gb_portramTiny[fast_tiny_port_0x60]= aux;
  gb_portramTiny[fast_tiny_port_0x64] |= 2;  
  doirq(1);
  //keydown[aux] = 1;  
 } 
 else
 {
  //JJ puerto portram[0x60] = aux | 0x80;  
  //JJ puerto portram[0x64] |= 2;   
  gb_portramTiny[fast_tiny_port_0x60]=  aux | 0x80;
  gb_portramTiny[fast_tiny_port_0x64] |= 2;  
  doirq(1);
  //keydown[aux] = 0;        
 }  
}


//**********************************************
unsigned char TraduceCodigoTecla(int aux)
{
 unsigned char aRet=0;
 switch (aux)
 {
  case 0: aRet= 30; break; //a
  case 1: aRet= 48; break; //b
  case 2: aRet= 46; break; //c
  case 3: aRet= 32; break; //d
  case 4: aRet= 18; break; //e
  case 5: aRet= 33; break; //f
  case 6: aRet= 34; break; //g
  case 7: aRet= 35; break; //h
  case 8: aRet= 23; break; //i
  case 9: aRet= 36; break; //j
  case 10: aRet= 37; break; //k
  case 11: aRet= 38; break; //l
  case 12: aRet= 50; break; //m
  case 13: aRet= 49; break; //n
  case 14: aRet= 24; break; //o
  case 15: aRet= 25; break; //p
  case 16: aRet= 16; break; //q
  case 17: aRet= 19; break; //r
  case 18: aRet= 31; break; //s
  case 19: aRet= 20; break; //t
  case 20: aRet= 22; break; //u
  case 21: aRet= 47; break; //v
  case 22: aRet= 17; break; //w
  case 23: aRet= 45; break; //x
  case 24: aRet= 21; break; //y
  case 25: aRet= 44; break; //z
  case 26: aRet= 11; break; //0
  case 27: aRet= 2; break; //1
  case 28: aRet= 3; break; //2
  case 29: aRet= 4; break; //3
  case 30: aRet= 5; break; //4
  case 31: aRet= 6; break; //5
  case 32: aRet= 7; break; //6
  case 33: aRet= 8; break; //7
  case 34: aRet= 9; break; //8
  case 35: aRet= 10; break; //9
  case 36: aRet= 28; break; //ENTER
  case 37: aRet= 57; break; //Barra espaciadora
  case 38: aRet= 14; break; //Back space
  case 39: aRet= 1; break; //ESC
  case 40: aRet= 75; break; //izquierda
  case 41: aRet= 77; break; //derecha
  case 42: aRet= 72; break; //arriba 
  case 43: aRet= 80; break; //abajo  
  case 44: aRet= 59; break; //F1
  default: aRet=0; break;
 }
 return aRet;
}

void handleinput()
{
 if (keymap[PS2_KC_F8] == 0)
 { 
  LoadCOMFlash(gb_com_digger,57856,0x0051);
  return;
 }   
gb_key_cur[0]= keymap[PS2_KC_A];
gb_key_cur[1]= keymap[PS2_KC_B];
gb_key_cur[2]= keymap[PS2_KC_C];
gb_key_cur[3]= keymap[PS2_KC_D];
gb_key_cur[4]= keymap[PS2_KC_E];
gb_key_cur[5]= keymap[PS2_KC_F];
gb_key_cur[6]= keymap[PS2_KC_G];
gb_key_cur[7]= keymap[PS2_KC_H];
gb_key_cur[8]= keymap[PS2_KC_I];
gb_key_cur[9]= keymap[PS2_KC_J];
gb_key_cur[10]= keymap[PS2_KC_K];
gb_key_cur[11]= keymap[PS2_KC_L];
gb_key_cur[12]= keymap[PS2_KC_M];
gb_key_cur[13]= keymap[PS2_KC_N];  
gb_key_cur[14]= keymap[PS2_KC_O];
gb_key_cur[15]= keymap[PS2_KC_P];
gb_key_cur[16]= keymap[PS2_KC_Q];
gb_key_cur[17]= keymap[PS2_KC_R];
gb_key_cur[18]= keymap[PS2_KC_S];
gb_key_cur[19]= keymap[PS2_KC_T];
gb_key_cur[20]= keymap[PS2_KC_U];
gb_key_cur[21]= keymap[PS2_KC_V];
gb_key_cur[22]= keymap[PS2_KC_W];
gb_key_cur[23]= keymap[PS2_KC_X];
gb_key_cur[24]= keymap[PS2_KC_Y];
gb_key_cur[25]= keymap[PS2_KC_Z];
gb_key_cur[26]= keymap[PS2_KC_0];
gb_key_cur[27]= keymap[PS2_KC_1];
gb_key_cur[28]= keymap[PS2_KC_2];
gb_key_cur[29]= keymap[PS2_KC_3];
gb_key_cur[30]= keymap[PS2_KC_4];
gb_key_cur[31]= keymap[PS2_KC_5];
gb_key_cur[32]= keymap[PS2_KC_6];
gb_key_cur[33]= keymap[PS2_KC_7];
gb_key_cur[34]= keymap[PS2_KC_8];
gb_key_cur[35]= keymap[PS2_KC_9];
gb_key_cur[36]= keymap[PS2_KC_ENTER];
gb_key_cur[37]= keymap[PS2_KC_SPACE];
//gb_key_cur[38]= keymap[PS2_KC_BACK];
gb_key_cur[38]= keymap[KEY_BACKSPACE]; //Back space
gb_key_cur[39]= keymap[PS2_KC_ESC];
gb_key_cur[40]= keymap[KEY_CURSOR_LEFT];
gb_key_cur[41]= keymap[KEY_CURSOR_RIGHT];
gb_key_cur[42]= keymap[KEY_CURSOR_UP];
gb_key_cur[43]= keymap[KEY_CURSOR_DOWN];
gb_key_cur[44]= keymap[PS2_KC_F1];


 unsigned char igual= 1;
 unsigned char mi_tecla=0;
 for (unsigned char i=0;i<45;i++)
 {
  if (gb_key_cur[i] != gb_key_before[i])
  {
   igual= 0;
   mi_tecla=i;
   break;
  }  
 }

 memcpy(gb_key_before,gb_key_cur,45); 

 unsigned char codigo_key=0;
 if (igual == 0)
 {//Hay un cambio en el teclado
  if (gb_key_cur[mi_tecla] == 0)
  {//Paso de 1 a 0. Se pulsa una tecla
   //printf("Mi tecla %d\n",mi_tecla);
   //fflush(stdout);  
   codigo_key = TraduceCodigoTecla(mi_tecla);
   CheckTeclaSDL(codigo_key,0);
  }
  else
  {//Paso de 0 a 1. Se libera una tecla
   codigo_key = TraduceCodigoTecla(mi_tecla);
   CheckTeclaSDL(codigo_key,1);
  }  
 } 
}

#ifndef use_lib_singlecore
//******************************
void videoTask(void *unused)
{
 videoTaskIsRunning = true;   
 uint16_t *param;
 while (1) 
 {        
  xQueuePeek(vidQueue, &param, portMAX_DELAY);
  if ((int)param == 1)
   break; 

  draw();

  xQueueReceive(vidQueue, &param, portMAX_DELAY);
  videoTaskIsRunning = false;   
 } 
 videoTaskIsRunning = false;
 vTaskDelete(NULL);

 while (1) {
 }     
}
#endif








unsigned char gb_cpunoexe=0;
unsigned int gb_cpunoexe_timer_ini;
unsigned int tiempo_ini_cpu,tiempo_fin_cpu;
unsigned int total_tiempo_ms_cpu;
unsigned int tiene_que_tardar=0;

//Loop main
void loop() 
{ 
 //SDL_DumpVGA();
 //if(running)
 //{
  jj_ini_cpu = micros();
  #ifdef use_lib_singlecore
   //exec86 (5000); //Rapido usar un solo CORE
   if (gb_cpunoexe == 0)
   {
    //tiempo_ini_cpu= millis(); 
    exec86 (10000);
    //tiempo_fin_cpu= millis();
    //total_tiempo_ms_cpu= tiempo_fin_cpu-tiempo_ini_cpu;    
   }
  #else 
   exec86 (10000); //Tarda 22 milis usar 2 cores
  #endif 
  
  jj_end_cpu = micros();
  gb_cur_cpu_ticks= (jj_end_cpu-jj_ini_cpu);
  total_tiempo_ms_cpu= gb_cur_cpu_ticks/1000;
  if (gb_cur_cpu_ticks>gb_max_cpu_ticks)
   gb_max_cpu_ticks= gb_cur_cpu_ticks;
  if (gb_cur_cpu_ticks<gb_min_cpu_ticks)   
   gb_min_cpu_ticks= gb_cur_cpu_ticks;



  gb_keyboard_time_cur= millis();
  if ((gb_keyboard_time_cur- gb_keyboard_time_before) > gb_keyboard_poll_milis)
  {
    gb_keyboard_time_before= gb_keyboard_time_cur;
	 handleinput();
    ProcesarAcciones();    
    do_tinyOSD();    
  }

  if (scrmodechange)
  {
   //ClearSDL(); //Para borra modo texto grande
   doscrmodechange();
  }

  #ifdef use_lib_singlecore
  //Un solo CORE BEGIN
  gb_cur_vga= millis();
  //if ((gb_cur_vga - gb_ini_vga)>=41)
  if ((gb_cur_vga - gb_ini_vga) >= gb_vga_poll_milis)
  {           
//   jj_ini_vga = micros();
   //VideoThreadPoll();
   draw();
//   jj_end_vga = micros();
//   gb_cur_vga_ticks= (jj_end_vga-jj_ini_vga);
//   if (gb_cur_vga_ticks>gb_max_vga_ticks)
//    gb_max_vga_ticks= gb_cur_vga_ticks;
//   if (gb_cur_vga_ticks<gb_min_vga_ticks)   
//    gb_min_vga_ticks= gb_cur_vga_ticks;
//   tiempo_vga=1;  
   //draw(); 
   //KeepAlive();
   //if (screen != NULL){
   // SDL_Flip(screen);//Tengo que mirar si lo quito
   //}          
   gb_ini_vga = gb_cur_vga;   
   //SDL_Delay(18);
  }
//  else 
//   tiempo_vga=0;
  //port3da= 8; //retrazo
  //totalframes++;
  //Un solo CORE END
  #endif
  
  #ifndef use_lib_singlecore
  //Task video Begin
  xQueueSend(vidQueue, &param, portMAX_DELAY);
  //while (videoTaskIsRunning)
  //{
  //}
  //Task video END
  #endif
 
  //delay(gb_delay_tick_cpu_milis);
  //gb_volumen01=0;
  //gb_frecuencia01=0; 


     
     if (gb_cpunoexe == 0)
     {
      gb_cpunoexe=1;
      gb_cpunoexe_timer_ini= millis();
      //tiene_que_tardar= 20-total_tiempo_ms_cpu; //20+10-total_tiempo_ms_cpu; //10 ms video
      //if (tiene_que_tardar>=20){
      // tiene_que_tardar=0;
      //}
      tiene_que_tardar= gb_delay_tick_cpu_milis;
     }
     else
     {
      if ((millis()-gb_cpunoexe_timer_ini) >= tiene_que_tardar)
      {
       gb_cpunoexe=0;
      }
     }
     


 gb_cpu_timer_cur= millis();
 if ((gb_cpu_timer_cur-gb_cpu_timer_before)>1000)
 {
  gb_cpu_timer_before= gb_cpu_timer_cur;
  if (tiempo_vga == 1)
  {
   #ifdef use_lib_log_serial
    Serial.printf("c:%u m:%u mx:%u vc:%u m:%u mx:%u\n",gb_cur_cpu_ticks,gb_min_cpu_ticks,gb_max_cpu_ticks, gb_cur_vga_ticks,gb_min_vga_ticks,gb_max_vga_ticks);
   #endif 
   //Reseteo VGA
   gb_min_vga_ticks= 1000000;
   gb_max_vga_ticks= 0;
   gb_cur_vga_ticks= 0;   
   tiempo_vga=0;
  }
  else
  {//Imprimo CPU
   #ifdef use_lib_log_serial
    Serial.printf("c:%u m:%u mx:%u\n",gb_cur_cpu_ticks,gb_min_cpu_ticks,gb_max_cpu_ticks);
   #endif 
  }

  //Reseteo CPU a 1 segundo
  gb_min_cpu_ticks= 1000000;
  gb_max_cpu_ticks= 0;
  gb_cur_cpu_ticks= 0;   
 }

 #ifndef use_lib_singlecore
 //TASK video BEGIN
 TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
 TIMERG0.wdt_feed = 1;
 TIMERG0.wdt_wprotect = 0;
 vTaskDelay(0); // important to avoid task watchdog timeouts - change this to slow down emu   
 //TASK video END
 #endif

 //}//fin if running

}
