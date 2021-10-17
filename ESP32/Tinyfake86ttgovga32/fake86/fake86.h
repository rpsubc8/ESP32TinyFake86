#ifndef _FAKE86_H
 #define _FAKE86_H

 #include "gbConfig.h"
 #include "hardware.h"
 #ifndef use_lib_bitluni_fast 
  #include "MartianVGA.h"
 #endif 

 // Declared vars
 #ifdef use_lib_bitluni_fast
 #else
  #ifdef use_lib_bitluni_3bpp
   #ifdef use_lib_bitluni_low_memory
    extern VGA3BitI vga;
   #else
    extern VGA3Bit vga;
   #endif 
  #else
   extern VGA6Bit vga;
  #endif 
 #endif 

 // Declared methods
 void do_keyboard();
#endif
