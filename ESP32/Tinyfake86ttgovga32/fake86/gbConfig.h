#ifndef _GB_CONFIG_H
 #define _GB_CONFIG_H
 
 #define true 1
 #define false 0

 //Cuando uso capturadora usb
 //#define use_lib_capture_usb
 
 //Corrige teclados que no se inicializan(solucion dcrespo3D)
 //#define FIX_PERIBOARD_NOT_INITING

 //Usa la libreria bitluni modificada en C tiny fast
 #define use_lib_bitluni_fast

 //#define use_lib_sna_rare
 //#define use_lib_speaker_cpu

 //Section Optimice
 #define use_lib_fast_push
 #define use_lib_fast_pop
 #define use_lib_fast_op_add16
 #define use_lib_fast_op_sub16
 #define use_lib_fast_op_and8
 #define use_lib_fast_op_add8
 #define use_lib_fast_op_writew86
 #define use_lib_fast_op_xor8
 #define use_lib_fast_op_or16
 #define use_lib_fast_op_or8
 #define use_lib_fast_op_and16
 #define use_lib_fast_flag_log8
 #define use_lib_fast_flag_log16
 //#define use_lib_fast_flag_adc8
 //#define use_lib_fast_flag_adc16
 //#define use_lib_fast_flag_add8
 //#define use_lib_fast_flag_add16
 //#define use_lib_fast_flag_sbb8
 //#define use_lib_fast_flag_sbb16
 #define use_lib_fast_op_xor16
 //#define use_lib_fast_flag_sub8
 //#define use_lib_fast_flag_sub16
 #define use_lib_fast_op_sub8

 #define use_lib_fast_readw86
 #define use_lib_fast_flag_szp8
 #define use_lib_fast_flag_szp16

 #define use_lib_fast_op_adc8
 #define use_lib_fast_op_adc16
 #define use_lib_fast_op_sbb8
 #define use_lib_fast_op_sbb16

 #define use_lib_fast_modregrm
 #define use_lib_fast_readrm16
 #define use_lib_fast_readrm8
 #define use_lib_fast_writerm16
 #define use_lib_fast_writerm8
 #define use_lib_fast_op_div8
 

 #define use_lib_fast_decodeflagsword
 #define use_lib_fast_makeflagsword

 //#define use_lib_fast_doirq


 //#define use_lib_adlib
 //#define use_lib_disneysound
 //#define use_lib_mouse
 //#define use_lib_net
 
 //Usar 1 solo nucleo
 #define use_lib_singlecore
 
 //#define use_lib_snapshot
 //#define use_lib_limit_256KB
 
 #define gb_max_portram 0x3FF
 #define use_lib_limit_portram
 
 //#define gb_max_ram 0x100000
 //1024 KB
 //#define gb_max_ram 0x100000
 //96 KB 
 //#define gb_max_ram 98304
 //98 KB
 //#define gb_max_ram 100352
 //100 KB
 //#define gb_max_ram 102400
 //128 KB arranca alley cat boot
 //#define gb_max_ram 131072
 //160 KB Para MADMIX
 #define gb_max_ram 163840
 //192 KB
 //#define gb_max_ram 196608
 //224 KB Para MADMIX2
 //#define gb_max_ram 229376 
 //256 KB
 //#define gb_max_ram 262144
 //640 KB 
 //#define gb_max_ram 655360 
 
 //#define use_lib_not_use_callback_port
 #define use_lib_fast_boot
 
 //#define use_lib_not_use_stretchblit
 //#define use_lib_not_use_roughblit
 //#define use_lib_not_use_doubleblit
 
 #define use_lib_force_sdl_blit
 #define use_lib_force_sdl_direct_vga
 #define use_lib_force_sdl_8bpp
 
 
 //options debug
 //#define use_lib_debug_interrupt
 




 
 //milisegundos espera en cada frame
 #define use_lib_delay_tick_cpu_auto 0
 #define use_lib_delay_tick_cpu_milis 0
 #define use_lib_vga_poll_milis 41
 #define use_lib_keyboard_poll_milis 20
 #define use_lib_timers_poll_milis 54






 //Video mode
 //#define use_lib_200x150
 #define use_lib_320x200

 //Logs
 //#define use_lib_log_serial
 
 //#define use_lib_bitluni_fast

 //#define use_lib_bitluni_3bpp
 
 //#define use_lib_bitluni_low_memory
 



 //#ifdef define use_lib_200x150
 // #define gb_add_offset_x 0
 // #define gb_add_offset_y 0
 //#else
 // #define gb_add_offset_x 60 
 // #define gb_add_offset_y 40
 //#endif
#endif
