#include "gbConfig.h"
#include "fake86.h"
#include "osd.h"
#include "dataFlash/gbcom.h"
#include "gbGlobals.h"
#include "PS2Kbd.h"
#include "gb_sdl_font8x8.h"
#include "render.h"
#include "dataFlash/gbsnarare.h"
#include "dataFlash/gbdsk.h"
#include "cpu.h"
#include "ports.h"


//#define BLACK   0
//#define BLUE    4
//#define RED     1
//#define MAGENTA 5
//#define GREEN   2
//#define CYAN    6
//#define YELLOW  3
//#define WHITE   15

#ifdef COLOR_3B           //       BGR 
 #define BLACK   0x08      // 0000 1000
 #define BLUE    0x0C      // 0000 1100
 #define RED     0x09      // 0000 1001
 #define MAGENTA 0x0D      // 0000 1101
 #define GREEN   0x0A      // 0000 1010
 #define CYAN    0x0E      // 0000 1110
 #define YELLOW  0x0B      // 0000 1011
 #define WHITE   0x0F      // 0000 1111
#endif


//extern int gb_screen_xIni;
//extern int gb_screen_yIni;
//extern unsigned char gb_cache_zxcolor[8];


unsigned char gb_show_osd_main_menu=0;

//extern SDL_Surface * gb_screen;
//extern SDL_Event gb_event;



#define max_gb_delay_cpu_menu 50
const char * gb_delay_cpu_menu[max_gb_delay_cpu_menu]={ 
 "0 (fast)","1","2","3","4","5","6","7","8","9",
 "10","11","12","13","14","15","16","17","18","19",
 "20","21","22","23","24","25","26","27","28","29",
 "30","31","32","33","34","35","36","37","38","39",
 "40","41","42","43","44","45","46","47","48","49"
};

#define max_gb_sound_menu 2
const char * gb_sound_menu[max_gb_sound_menu]={
 "Sound ON"
 ,"Sound OFF"
};

//#define max_gb_osd_screen 1
//const char * gb_osd_screen[max_gb_osd_screen]={
// "Pixels Left"//,
// //"Pixels Top",
// //"Color 8",
// //"Mono Blue 8",
// //"Mono Green 8",
// //"Mono Red 8",
// //"Mono Grey 8"
//};

//#define max_gb_osd_screen_values 5
//const char * gb_osd_screen_values[max_gb_osd_screen_values]={
// "0",
// "2",
// "4", 
// "8", 
// "16"
//};


#define max_gb_main_menu 8
const char * gb_main_menu[max_gb_main_menu]={
 "Load COM",
 "Select DSK",
 "Reset",
 "LOAD SNA RARE",
 "Speed",
 "Video",
 "Sound",
 "Return"
};

#define max_gb_video_menu 7
const char * gb_video_menu[max_gb_video_menu]={
 "Font 4x8",
 "Font 8x8",  
 "Invert Color",
 "Colour CGA1",
 "Colour CGA2",
 "Gray CGA",
 "Colour PCJR"
};

#define max_gb_speed_menu 4
const char * gb_speed_menu[max_gb_speed_menu]={
 "CPU delay",
 "Timer poll",
 "VGA poll",
 "Keyboard poll"
};


#define max_gb_vga_poll_menu 4
const char * gb_vga_poll_menu[max_gb_vga_poll_menu]={
 "20",
 "30",
 "40",
 "50"
};

#define max_gb_keyboard_poll_menu 5
const char * gb_keyboard_poll_menu[max_gb_keyboard_poll_menu]={
 "10",
 "20",
 "30",
 "40",
 "50"
};

#define max_gb_timers_poll_menu 7
const char * gb_timers_poll_menu[max_gb_timers_poll_menu]={
 "216 (4.62)",
 "108 (9.2)",
 "54 (18.5)",
 "27 (37.03)",
 "13 (76.92)",
 "6  (166.66)",
 "1  (fast)"
};

//#define max_gb_machine_menu 3
//const char * gb_machine_menu[max_gb_machine_menu]={
// "464",
// "664",
// "6128"
//};

//#define max_gb_speed_sound_menu 7
//const char * gb_speed_sound_menu[max_gb_speed_sound_menu]={
// "0",
// "1",
// "2",
// "4",
// "8",
// "16",
// "32"
//};

//#define max_gb_value_binary_menu 2
//const char * gb_value_binary_menu[max_gb_value_binary_menu]={
// "0",
// "1"
//};


//#define max_gb_speed_videoaudio_options_menu 5
//const char * gb_speed_videoaudio_options_menu[max_gb_speed_videoaudio_options_menu]={
// "Audio poll",
// "Video delay",
// "Skip Frame",
// "Keyboard poll",
// "Mouse poll"
//};


//#define max_gb_speed_menu 5
//const char * gb_speed_menu[max_gb_speed_menu]={
// "Normal",
// "2x",
// "4x",
// "8x",
// "16x"
//};


//#define max_gb_osd_mouse_menu 2
//const char * gb_osd_mouse_menu[max_gb_osd_mouse_menu]={
// "right handed",
// "left handed"
//};

#define max_gb_reset_menu 2
const char * gb_reset_menu[max_gb_reset_menu]={
 "Soft",
 "Hard"
};


#define gb_pos_x_menu 50
#define gb_pos_y_menu 20
#define gb_osd_max_rows 10

void SDLClear()
{
 //for (int y=0; y<(auxSurface->w); y++)
 // for (int x=0; x<(auxSurface->h); x++)
 //  SDLputpixel(auxSurface,x,y,1);
 for (int y=0; y<200; y++)
  for (int x=0; x<320; x++)
   jj_fast_putpixel(x,y,0);
   //SDLputpixel(auxSurface,x,y,3); 
}

//*************************************************************************************
void SDLprintCharOSD(char car,int x,int y,unsigned char color,unsigned char backcolor)
{ 
// unsigned char aux = gb_sdl_font_6x8[(car-64)];
 int auxId = car << 3; //*8
 unsigned char aux;
 unsigned char auxBit,auxColor;
 for (unsigned char j=0;j<8;j++)
 {
  aux = gb_sdl_font_8x8[auxId + j];
  for (int i=0;i<8;i++)
  {
   auxColor= ((aux>>i) & 0x01);
   //SDLputpixel(surface,x+(6-i),y+j,(auxColor==1)?color:backcolor);
   jj_fast_putpixel(x+(6-i),y+j,(auxColor==1)?color:backcolor);
  }
 }
}

void SDLprintText(const char *cad,int x, int y, unsigned char color,unsigned char backcolor)
{
//SDL_Surface *surface,
// gb_sdl_font_6x8
 int auxLen= strlen(cad);
 if (auxLen>50)
  auxLen=50;
 for (int i=0;i<auxLen;i++)
 {
  SDLprintCharOSD(cad[i],x,y,color,backcolor);
  x+=7;
 }
}

void OSDMenuRowsDisplayScroll(const char **ptrValue,unsigned char currentId,unsigned char aMax)
{//Dibuja varias lineas
 for (int i=0;i<gb_osd_max_rows;i++)
  SDLprintText("                    ",gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),0,0);
 
 for (int i=0;i<gb_osd_max_rows;i++)
 {
  if (currentId >= aMax)
   break;
  //SDLprintText(gb_osd_sdl_surface,ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?CYAN:WHITE),((i==0)?BLUE:BLACK),1);
  SDLprintText(ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?0:WHITE),((i==0)?WHITE:0));  
  currentId++;
 }     
}

//Maximo 256 elementos
unsigned char ShowTinyMenu(const char *cadTitle,const char **ptrValue,unsigned char aMax)
{
 unsigned char aReturn=0;
 unsigned char salir=0;
 SDLClear();
 SDLprintText("Port Fake86 by Ackerman",gb_pos_x_menu-(4<<3),gb_pos_y_menu-16,WHITE,0);
 for (int i=0;i<20;i++)
  SDLprintCharOSD(' ',gb_pos_x_menu+(i<<3),gb_pos_y_menu,0,WHITE);
 SDLprintText(cadTitle,gb_pos_x_menu,gb_pos_y_menu,0,WHITE);

 OSDMenuRowsDisplayScroll(ptrValue,0,aMax);
 
 while (salir == 0)
 {
  //SDL_PollEvent(gb_osd_sdl_event);
  //if(SDL_WaitEvent(gb_osd_sdl_event))
  {
   //if(gb_osd_sdl_event->type == SDL_KEYDOWN)
   {
    //switch(gb_osd_sdl_event->key.keysym.sym)
    {
     //case SDLK_UP:
     if (checkAndCleanKey(KEY_CURSOR_LEFT))
     {
      if (aReturn>10) aReturn-=10;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
     }
     if (checkAndCleanKey(KEY_CURSOR_RIGHT))
     {
      if (aReturn<(aMax-10)) aReturn+=10;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
     }     

     if (checkAndCleanKey(KEY_CURSOR_UP))
     {
      //vga.setTextColor(WHITE,BLACK);
      //vga.setCursor(gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3));
      //vga.print(ptrValue[aReturn]);
      ////SDLprintText(gb_osd_sdl_surface,ptrValue[aReturn],gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3),WHITE,BLACK,1);
      if (aReturn>0) aReturn--;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
      //vga.setTextColor(CYAN,BLUE);
      //vga.setCursor(gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3));
      //vga.print(ptrValue[aReturn]);
      ////SDLprintText(gb_osd_sdl_surface,ptrValue[aReturn],gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3),CYAN,BLUE,1);
      ////break;
     }
     if (checkAndCleanKey(KEY_CURSOR_DOWN))
     {
     //case SDLK_DOWN: 
      //SDLprintText(gb_osd_sdl_surface,ptrValue[aReturn],gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3),WHITE,BLACK,1);
      //vga.setTextColor(WHITE,BLACK);
      //vga.setCursor(gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3));
      //vga.print(ptrValue[aReturn]);
      if (aReturn < (aMax-1)) aReturn++;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
      //vga.setTextColor(CYAN,BLUE);
      //vga.setCursor(gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3));
      //vga.print(ptrValue[aReturn]);
      ////SDLprintText(gb_osd_sdl_surface,ptrValue[aReturn],gb_pos_x_menu,gb_pos_y_menu+((aReturn+1)<<3),CYAN,BLUE,1);
      ////break;
     }
     if (checkAndCleanKey(KEY_ENTER))
     {
      salir= 1;
     }
     //case SDLK_KP_ENTER: case SDLK_RETURN: salir= 1;break;
     if (checkAndCleanKey(KEY_ESC))
     {
      salir=1; aReturn= 255;    
     }
     //case SDLK_ESCAPE: salir=1; aReturn= 255; break;
     //default: break;
    }
    //SDL_Flip(gb_osd_sdl_surface);
    //SDL_PollEvent(gb_osd_sdl_event);
   }  
  }  
 } 
 gb_show_osd_main_menu= 0;
 return aReturn;
}

//Menu DSK
void ShowTinyDSKMenu()
{
 unsigned char aSelNum;     
 aSelNum = ShowTinyMenu("DSK",gb_list_dsk_title,max_list_dsk);

 //gb_cartfilename= (char *)gb_list_rom_title[aSelNum];
 gb_force_load_dsk= 1;
 if (aSelNum > (max_list_dsk-1))
  aSelNum= max_list_dsk-1;
 gb_id_cur_dsk= aSelNum;
 //running= 0;
}


void ShowTinyCPUDelayMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Delay CPU ms",gb_delay_cpu_menu,max_gb_delay_cpu_menu);
 if (aSelNum == 255)
  return;
 gb_auto_delay_cpu=1;
 gb_delay_tick_cpu_milis = aSelNum;  
}

void ShowTinyTimerDelayMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Timers poll ms",gb_timers_poll_menu,max_gb_timers_poll_menu);
 switch (aSelNum)
 {
  case 0: gb_timers_poll_milis= 216; break;
  case 1: gb_timers_poll_milis= 108; break;
  case 2: gb_timers_poll_milis= 54; break;
  case 3: gb_timers_poll_milis= 27; break;
  case 4: gb_timers_poll_milis= 13; break;
  case 5: gb_timers_poll_milis= 6; break;
  case 6: gb_timers_poll_milis= 1; break;
 }
}

void ShowTinyVGApollMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("VGA poll ms",gb_vga_poll_menu,max_gb_vga_poll_menu);
 switch (aSelNum)
 {
  case 0: gb_vga_poll_milis= 20; break;
  case 1: gb_vga_poll_milis=30; break;
  case 2: gb_vga_poll_milis=40; break;
  case 3: gb_vga_poll_milis=50; break;
 }
}

void ShowTinyKeyboardPollMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Keyboard poll ms",gb_keyboard_poll_menu,max_gb_keyboard_poll_menu);
 switch (aSelNum)
 {
  case 0: gb_keyboard_poll_milis= 10; break;
  case 1: gb_keyboard_poll_milis= 20; break;
  case 2: gb_keyboard_poll_milis= 30; break;
  case 3: gb_keyboard_poll_milis= 40; break;
  case 4: gb_keyboard_poll_milis= 50; break;
 } 
}

//Menu velocidad emulador
void ShowTinySpeedMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Speed",gb_speed_menu,max_gb_speed_menu);
 switch (aSelNum)
 {
  case 0: ShowTinyCPUDelayMenu(); break;
  case 1: ShowTinyTimerDelayMenu(); break;
  case 2: ShowTinyVGApollMenu(); break;
  case 3: ShowTinyKeyboardPollMenu(); break;
 } 
}

//Menu sonido
void ShowTinySoundMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Sound",gb_sound_menu,max_gb_sound_menu);
 gb_silence= (aSelNum==0)?0:1;
}



//Menu ROM
//void ShowTinyMachineMenu()
//{
 //unsigned char aSelNum;
 //model = ShowTinyMenu("Machine",gb_machine_menu,max_gb_machine_menu);
 //loadroms2FlashModel();
 //switch (aSelNum)
 //{
 // case 0: model= 0; loadroms2FlashModel(0); break;
 // case 1: model= 1; loadroms2FlashModel(1); break;
 // case 2: model= 2; loadroms2FlashModel(2); break;
 // default: model= 0; loadroms2FlashModel(0); break;
 //}
 //vTaskDelay(2);
 //resetz80();
 //resetcrtc();
 //SDL_Delay(2);
//}


//Menu resetear
void ShowTinyResetMenu()
{
 unsigned char aSelNum;
 aSelNum= ShowTinyMenu("Reset",gb_reset_menu,max_gb_reset_menu);
 if (aSelNum == 1)
 {
  ESP.restart();
 }
 else
 {
   gb_reset= 1;
 } 
}

//Menu TAPE
/*void ShowTinyTAPEMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("48K TAPE",gb_list_tapes_48k_title,max_list_tape_48);
 load_tape2Flash(aSelNum);
}

//Play sound tape
void ShowTinySelectTAPEMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("48K TAPE",gb_list_tapes_48k_title,max_list_tape_48);
 Z80EmuSelectTape(aSelNum);
// gb_play_tape_current = aSelNum;
// gb_play_tape_on_now = 1;
}

//Menu SCREEN
void ShowTinySCRMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("48K SCREEN",gb_list_scr_48k_title,max_list_scr_48);
 load_scr2Flash(aSelNum);     
}
*/




//Ajustar pantalla
//void ShowTinyScreenAdjustMenu()
//{
// unsigned char aSelNum, auxCol; 
// aSelNum= ShowTinyMenu("Screen Adjust",gb_osd_screen,max_gb_osd_screen);
// auxCol= ShowTinyMenu("Pixels",gb_osd_screen_values,max_gb_osd_screen_values);
// auxCol = auxCol<<1; //x2
// //gb_screen_xOffset = auxCol; 
// /*switch (aSelNum)
// {
//  case 2:
//   //OSD_PreparaPaleta8Colores();
//   OSD_PreparaPaleta64para8Colores();
//   memcpy(gb_cache_zxcolor,gb_const_colornormal8,8);
//   #ifdef use_lib_use_bright
//    memcpy(gb_cache_zxcolor_bright,gb_const_colornormal8_bright,8);
//   #endif
//   SDL_SetPalette(gb_osd_sdl_surface, SDL_LOGPAL|SDL_PHYSPAL, gb_osd_sdl_palette, 0, 256);
//   return;
//   break;
//  case 3:
//   OSD_PreparaPaleta64para8Colores();
//   memcpy(gb_cache_zxcolor,gb_const_monoBlue8,8);
//   #ifdef use_lib_use_bright
//    memcpy(gb_cache_zxcolor_bright,gb_const_monoBlue8_bright,8);
//   #endif 
//   SDL_SetPalette(gb_osd_sdl_surface, SDL_LOGPAL|SDL_PHYSPAL, gb_osd_sdl_palette, 0, 256);
//   //for (int k=0;k<256;k++)
//   // printf("%d %d %d\n",gb_osd_sdl_palette[k].r,gb_osd_sdl_palette[k].g,gb_osd_sdl_palette[k].b);
//   //for (int k=0;k<8;k++)
//   // printf("%d ",gb_cache_zxcolor[k]);
//   //printf("\n");    
//   return;
//   break;
//  case 4:
//   OSD_PreparaPaleta64para8Colores();   
//   memcpy(gb_cache_zxcolor,gb_const_monoGreen8,8);
//   #ifdef use_lib_use_bright
//    memcpy(gb_cache_zxcolor_bright,gb_const_monoGreen8_bright,8);
//   #endif 
//   SDL_SetPalette(gb_osd_sdl_surface, SDL_LOGPAL|SDL_PHYSPAL, gb_osd_sdl_palette, 0, 256);
//   return;
//   break;
//  case 5:
//   OSD_PreparaPaleta64para8Colores();   
//   memcpy(gb_cache_zxcolor,gb_const_monoRed8,8);
//   #ifdef use_lib_use_bright
//    memcpy(gb_cache_zxcolor_bright,gb_const_monoRed8_bright,8);
//   #endif 
//   SDL_SetPalette(gb_osd_sdl_surface, SDL_LOGPAL|SDL_PHYSPAL, gb_osd_sdl_palette, 0, 256);
//   return;
//  case 6:
//   OSD_PreparaPaleta64para8Colores();   
//   memcpy(gb_cache_zxcolor,gb_const_grey8,8);
//   #ifdef use_lib_use_bright
//    memcpy(gb_cache_zxcolor_bright,gb_const_grey8_bright,8);
//   #endif 
//   SDL_SetPalette(gb_osd_sdl_surface, SDL_LOGPAL|SDL_PHYSPAL, gb_osd_sdl_palette, 0, 256);
//   return;
//   break;          
// }
// auxCol= ShowTinyMenu("Cols or pixels",gb_osd_screen_values,max_gb_osd_screen_values);
// switch (aSelNum)
// {
//  case 0: gb_screen_xIni = -(auxCol<<3); break;
//  case 1: gb_screen_yIni = (auxCol<=3)? (-auxCol):(-3); break;  
//  default: break;
// }
// */
//}

void ShowTinyCOMMenu()
{
 unsigned char aSelNum;     
 aSelNum = ShowTinyMenu("COM",gb_list_com_title,max_list_com);

 //gb_cartfilename= (char *)gb_list_rom_title[aSelNum];
 gb_force_load_com= 1;
 gb_id_cur_com= aSelNum;
 //running= 0;
}

//void ShowTinyMouseMenu()
//{
// #ifdef use_lib_amx_mouse    
//  unsigned char aSelNum;
//  aSelNum = ShowTinyMenu("Mouse Buttons",gb_osd_mouse_menu,max_gb_osd_mouse_menu); 
//  switch (aSelNum)
//  {
//   case 0: gb_force_left_handed= 0; break; //diestro
 //  case 1: gb_force_left_handed= 1; break; //zurdo
 //  default: break;
 // }
 //#endif
 //vTaskDelay(2);
//}

void ShowTinyVideoMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Video",gb_video_menu,max_gb_video_menu);
 switch (aSelNum)
 {
   case 0: gb_font_8x8= 0; break; //font 4x8
   case 1: gb_font_8x8= 1; break; //font 8x8 
   case 2: gb_invert_color= ((~gb_invert_color)&0x01); break; //Invertir color
   case 3: InitPaletaCGA(); break;
   case 4: InitPaletaCGA2(); break;    
   case 5: InitPaletaCGAgray(); break;    
   case 6: InitPaletaPCJR(); break;   
 }
}


//*******************************************
void SDLActivarOSDMainMenu()
{     
 gb_show_osd_main_menu= 1;   
}


 //**************************************************
 void LoadSNArareMADMIX()
 {
/*   
  gb_use_snarare_madmix= 1;
  gb_use_remap_cartdridge=1;
  memcpy(gb_video_cga,gb_madmix_video_rare_cga,16384);
  for (int i=0;i<0x3FF;i++){
   //WriteTinyPortRAM(i,gb_madmix_ports_rare[i]);
   //portram[i]= gb_madmix_ports_rare[i];
   WriteTinyPortRAM(i,gb_madmix_ports_rare[i]);
  }
  
  #ifdef use_lib_sna_rare
   for (int i=0;i<gb_max_ram;i++){   
    jj_write86_remap(i,gb_madmix_flash_rare_sna_rare[i]);    
   }  
  #endif 
  
  vidmode=4;
  
SetRegAL(0x5F); SetRegAH(0xF5);
SetRegBL(0xF4); SetRegBH(0x17);
SetRegCL(0x06); SetRegCH(0x00);
SetRegDL(0x7D); SetRegDH(0x7D);
SetRegCS(0x20B3);
SetRegIP(0x1913);
SetRegSS(0x2093);
SetRegDS(0x1394);
SetRegES(0x0610);
SetRegSP(0x01E6);
SetRegBP(0x0000);
SetRegSI(0x0990);
SetRegDI(0x0F9C);
SetCF(0x0000);
*/
}


//Very small tiny osd
void do_tinyOSD() 
{
 int auxVol;
 int auxFrec;  
 unsigned char aSelNum;
 if (checkAndCleanKey(KEY_F12))
 {
  gb_show_osd_main_menu= 1;
  #ifdef use_lib_sound_ay8912  
   gb_silence_all_channels = 1;
  #endif    
  return;
 }

 if (gb_show_osd_main_menu == 1)
 {
  auxVol= gb_volumen01;
  auxFrec= gb_frecuencia01;
  gb_volumen01= gb_frecuencia01=0;

  aSelNum = ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu);
  switch (aSelNum)
  {
   case 0:
    ShowTinyCOMMenu();
    gb_show_osd_main_menu=0;
    break;
   case 1:
    ShowTinyDSKMenu();
    gb_show_osd_main_menu=0;
    break;
   case 2:     
    ShowTinyResetMenu();
    gb_show_osd_main_menu=0;    
    break;
   case 3: 
/*    //ShowTinySpeedMenu();
    didbootstrap= 1;
    gb_use_remap_cartdridge=0;
    for (int i=0;i<5;i++){
     memset(gb_ram_bank[i],0,32768);
    }
    gb_use_remap_cartdridge=1;
    //memset(gb_use_minimal_ram,0,sizeof(gb_use_minimal_ram));    
    LoadSNArareMADMIX();    
*/    
    gb_show_osd_main_menu=0;    
    break;
   case 4: ShowTinySpeedMenu(); 
    gb_show_osd_main_menu=0;   
    break;
   case 5: ShowTinyVideoMenu(); 
    gb_show_osd_main_menu=0;
    break;   
   case 6:
    ShowTinySoundMenu();        
    gb_show_osd_main_menu=0; 
    break;
   default: break;
  }

  gb_volumen01= auxVol;
  gb_frecuencia01= auxFrec;
  
 }
 #ifdef use_lib_sound_ay8912
  gb_silence_all_channels = 0;
 #endif 
}

