#ifndef _GB_GLOBALS_H
 #define _GB_GLOBALS_H
 #include "gbConfig.h"
 #include <stdio.h>



 //#include <SDL/SDL.h>
//JJ #include "mutex.h"
 #include "gbConfig.h"
 

 #define fast_tiny_port_0x60 11
 #define fast_tiny_port_0x61 12
 #define fast_tiny_port_0x64 14
 #define fast_tiny_port_0x3C0 37
 #define fast_tiny_port_0x3C4 40
 #define fast_tiny_port_0x3D4 44
 #define fast_tiny_port_0x3D8 46
 #define fast_tiny_port_0x3D9 47
 #define fast_tiny_port_0x3B9 31

 //extern unsigned char verbose; //No lo necesito
 extern unsigned int lasttick;
 // extern unsigned long tickgap; //no lo necesito fuerzo 54 ms
 //JJ extern CRITICAL_SECTION screenmutex;
 //extern SDL_Surface *screen;
 
 extern unsigned char bootdrive,renderbenchmark;
 //noscale, usessource, useconsole,doaudio,cgaonly,nosmooth,ethif
 
 extern unsigned char gb_force_set_cga;
 extern unsigned char gb_force_load_com;
 extern unsigned char gb_id_cur_com;
 extern unsigned char gb_force_boot;
 extern unsigned char gb_force_load_dsk;
 extern unsigned char gb_id_cur_dsk; 
 #ifdef use_lib_snapshot
  //extern unsigned char gb_memory_write[1048576];
  extern unsigned char gb_force_snapshot_begin;
  extern unsigned char gb_force_snapshot_end;
 #endif 
 
 #ifdef use_lib_limit_256KB
  extern unsigned char gb_check_memory_before;
 #endif 
 
 //extern unsigned char RAM[0x100000];
 //extern unsigned char RAM[gb_max_ram];  //Ya no uso RAM sino bloques de RAM
 //JJ extern unsigned char VRAM[262144]; //quito vga por ahora
 //extern unsigned char cf; //Lo dejo static optimizado
 extern unsigned char cf;
 extern unsigned char hdcount;
 
 //#ifdef use_lib_limit_portram
 // extern unsigned char portram[gb_max_portram]; //limito a 1023 bytes de puertos
 //#else
 // extern unsigned char portram[0x10000];
 //#endif 
 //JJ extern unsigned char portram[gb_max_portram]; //limito a 1023 bytes de puertos
 extern unsigned char gb_portramTiny[51]; //Solo 51 puertos
 extern void * gb_portTiny_write_callback[5];
 extern void * gb_portTiny_read_callback[5]; 
 
 extern unsigned char running;
 
 extern unsigned char vidmode;
  
 #ifdef use_lib_debug_interrupt
  extern unsigned char gb_interrupt_before;
 #endif

 extern unsigned char didbootstrap;



 //Parte parsecl
 extern unsigned char slowsystem;
 extern unsigned short int constantw;
 extern unsigned short int constanth; 
 
 extern unsigned char speakerenabled;
 
 //JJ extern void *port_write_callback[gb_max_portram];
 //JJ extern void *port_read_callback[gb_max_portram];
 //JJ extern void *port_write_callback16[gb_max_portram];
 //JJ extern void *port_read_callback16[gb_max_portram]; 
 
 extern const unsigned char fontcga[];
 extern unsigned long int gb_jj_cont_timer;
 //JJVGA extern unsigned short int VGA_SC[0x100], VGA_CRTC[0x100], VGA_ATTR[0x100], VGA_GC[0x100]; //no necesito VGA
 extern unsigned char scrmodechange; 
 
// extern unsigned int prestretch[1024][1024];


 //extern SDL_Event event;
 
 
 //extern const unsigned int palettecga[16];
 extern const unsigned char palettecga[16];
 
 extern unsigned short int segregs[4];
 //extern unsigned char VGA_latch[4]; //Solo CGA

 //extern unsigned char updatedscreen;
 
 extern unsigned int usegrabmode;
 
 extern unsigned int x,y;
 
 extern unsigned char gb_video_cga[16384];
 //extern unsigned char gb_video_hercules[16384];
 
 extern unsigned char *gb_ram_00; //32 KB
 extern unsigned char *gb_ram_01;
 extern unsigned char *gb_ram_02;
 extern unsigned char *gb_ram_03;
 extern unsigned char *gb_ram_04;
 extern unsigned char *gb_ram_bank[5];
 
// extern const unsigned char gb_reserved_memory[16];













 extern volatile unsigned char keymap[256];
 extern volatile unsigned char oldKeymap[256];

 //extern unsigned char keydown[0x100];

/* extern unsigned char gb_current_ms_poll_sound; //milisegundos muestreo
 extern unsigned char gb_current_ms_poll_keyboard; //milisegundos muestreo teclado
 extern unsigned char gb_current_frame_crt_skip; //el actual salto de frame
 extern unsigned char gb_current_delay_emulate_ms; //la espera en cada iteracion
 extern unsigned char gb_sdl_blit;
 extern unsigned char gb_screen_xOffset;

 extern unsigned char gb_show_osd_main_menu;*/


 extern unsigned char gb_key_cur[45];
 extern unsigned char gb_key_before[45];

 extern unsigned char **gb_buffer_vga; 
 //extern unsigned char gb_color_vga[16];

 extern unsigned char gb_font_8x8;

 //retrazo
 extern unsigned char port3da;
 
 extern unsigned char gb_reset;
 

 //Medicion tiempos
 extern unsigned int jj_ini_cpu,jj_end_cpu,jj_ini_vga,jj_end_vga;
 extern unsigned int gb_max_cpu_ticks,gb_min_cpu_ticks,gb_cur_cpu_ticks;
 extern unsigned int gb_max_vga_ticks,gb_min_vga_ticks,gb_cur_vga_ticks;
 extern unsigned char tiempo_vga;

 extern unsigned char keyboardwaitack;

 extern unsigned char gb_ram_truco_low;
 extern unsigned char gb_ram_truco_high;

 extern unsigned char gb_use_snarare_madmix;
 extern unsigned char gb_use_remap_cartdridge;


 extern unsigned char gb_frec_speaker_low;
 extern unsigned char gb_frec_speaker_high;
 extern unsigned char gb_cont_frec_speaker;
 extern volatile int gb_frecuencia01;
 extern volatile int gb_volumen01;
 extern volatile unsigned int gb_pulsos_onda;
 extern volatile unsigned int gb_cont_my_callbackfunc; 
 extern volatile unsigned char speaker_pin_estado;

 extern unsigned char gb_delay_tick_cpu_milis;
 extern unsigned char gb_auto_delay_cpu;
 extern unsigned char gb_vga_poll_milis;
 extern unsigned char gb_keyboard_poll_milis;
 extern unsigned char gb_timers_poll_milis;

 //fast bitluni
 extern unsigned char gb_sync_bits;

 extern unsigned char gb_invert_color;
 extern unsigned char gb_silence;
  



 #ifdef use_lib_speaker_cpu
  #define SAMPLE_RATE 10000
 #else
  #define SAMPLE_RATE 16000
  //#define SAMPLE_RATE 11025
 #endif 

#endif
 
