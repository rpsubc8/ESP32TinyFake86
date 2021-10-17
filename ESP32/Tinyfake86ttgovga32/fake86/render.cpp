//  Fake86: A portable, open-source 8086 PC emulator.
//  Copyright (C)2010-2012 Mike Chambers
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// render.c: functions for SDL initialization, as well as video scaling/rendering.
//   it is a bit messy. i plan to rework much of this in the future. i am also
//   going to add hardware accelerated scaling soon.

#include "gbConfig.h"
//#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>
//JJ #include "mutex.h"
#include "gbGlobals.h"
#include "cpu.h"
#include "render.h"
#include "fake86.h"
#include "gb_sdl_font8x8.h"
#include "gb_sdl_font4x8.h"
#include "render.h"

#define uint32_t int

//Escala Gris static Rapido
static unsigned char gb_color_vga[16]={
 //0x00,0x3F,0x01,0x29,0x21,0x19,0x10,0x1E,
 0x00,0x15,0x2A,0x3F,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};

//Color Modo Texto Rapido
static unsigned char gb_color_text_cga[16]={ 
 0,32,8,40,2,34,6,42,
 21,53,29,61,23,55,31,63
};

//

//unsigned int prestretch[1024][1024]; //no necesito escalar
//unsigned char jj[2138576]; //no necesito escalar

//JJ #ifdef _WIN32
//JJ  CRITICAL_SECTION screenmutex;
//JJ #else
//JJ pthread_t vidthread;
//JJ pthread_mutex_t screenmutex = PTHREAD_MUTEX_INITIALIZER;
//JJ #endif

//JJ SDL_Surface *screen = NULL;
//uint32_t *scalemap = NULL;
//uint8_t regenscalemap = 1;

//JJ extern uint8_t RAM[0x100000]
//JJ extern uint8_t portram[0x10000];
extern uint8_t cgabg, blankattr, vidgfxmode, vidcolor;
//VRAM[262144]
//vidmode 
//JJ running;
extern uint16_t cursx, cursy, cols, rows, vgapage, cursorposition, cursorvisible;
extern uint8_t clocksafe, port6, portout16;
//port3da
extern uint32_t videobase, textbase;
// x, y;
//JJ extern uint8_t fontcga[32768];
//extern uint32_t palettecga[16];
//extern uint32_t palettevga[256]; //no necesito vga por ahora
extern uint32_t usefullscreen;
//, usegrabmode;

uint64_t totalframes = 0;
//JJ uint32_t framedelay = 20;
//uint8_t scrmodechange = 0, noscale = 0, nosmooth = 1, doaudio = 0;
//uint8_t nosmooth = 1;
//noscale = 0, renderbenchmark = 0,doaudio = 0;
char windowtitle[128];

//void initcga(); //No necesito incializar cga
//JJ #ifdef _WIN32
//JJ void VideoThread (void *dummy);
//JJ #else
//JJ void *VideoThread (void *dummy);
//JJ #endif

//JJ no SDL SDL_Color gb_colors[256]; //paleta sdl 256 colores
int gb_cont_rgb=0;



/*JJ no SDL
void SetRGBSDL(unsigned char r,unsigned char g,unsigned char b)
{
 gb_colors[gb_cont_rgb].r= r;
 gb_colors[gb_cont_rgb].g= g;
 gb_colors[gb_cont_rgb].b= b;
 gb_cont_rgb++;
 if (gb_cont_rgb>255)
  gb_cont_rgb=255;  
}
*/

//*************************
//void InitPaletaCGA()
//{//4 colores 320x200
/*JJ no sdl
 gb_cont_rgb=0;
 SetRGBSDL(0,0,0);
 SetRGBSDL(170,0,170);
 SetRGBSDL(0,170,170);
 SetRGBSDL(170,170,170);
 
 SetRGBSDL(170,170,170);
 SetRGBSDL(170,170,170);
 SetRGBSDL(170,170,170);
 SetRGBSDL(170,170,170);
 
 for (int i=0;i<8;i++)
 {
  gb_colors[i].r= (i & 1)<<7;
  gb_colors[i].g= ((i>>1) & 1)<<7;
  gb_colors[i].b= ((i>>2)&1)<<7;
 }
 
 SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, gb_colors, 0, 256);
 */
//}

//*************************
//void InitPaletaCGAgray()
//{//2 colores blanco y negro
/*JJ no SDL
 gb_cont_rgb=0;
 unsigned char auxColor;
 for (int j=0;j<64;j++)
 {
  for (int i=0;i<4;i++)
  {
   auxColor= (i<<6);
   SetRGBSDL(auxColor,auxColor,auxColor);
  }
 }
 SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, gb_colors, 0, 256);
 */
//}

//*************************
//void InitPaletaCGAbw()
//{//2 colores blanco y negro
/*JJ no sdl
 gb_cont_rgb=0;
 SetRGBSDL(0,0,0);
 for (int i=0;i<254;i++){
  SetRGBSDL(255,255,255);
 }
 SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, gb_colors, 0, 256);
 */
//}

//*************************
//void InitPaletaPCJR()
//{//16 colores
/*JJ no sdl
 gb_cont_rgb=0;
 SetRGBSDL(0,0,0);
 SetRGBSDL(0,0,170);
 SetRGBSDL(0,170,0);
 SetRGBSDL(0,170,170);
 SetRGBSDL(170,0,0);
 SetRGBSDL(170,0,170);
 SetRGBSDL(170,85,0);
 SetRGBSDL(170,170,170);
 SetRGBSDL(85,85,85);
 SetRGBSDL(85,85,255);
 SetRGBSDL(85,255,85);
 SetRGBSDL(85,255,255);
 SetRGBSDL(255,85,85); 
 SetRGBSDL(255,85,255);
 SetRGBSDL(255,255,85);
 SetRGBSDL(255,255,255);
 for (int i=0;i<240;i++){
  SetRGBSDL(255,255,255);
 }
 SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, gb_colors, 0, 256); 
 */
//}

//************************
//void InitPaletaVGA()
//{
/*JJ no SDL
gb_cont_rgb=0;
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,169);
SetRGBSDL(0,169,0);
SetRGBSDL(0,169,169);
SetRGBSDL(169,0,0);
SetRGBSDL(169,0,169);
SetRGBSDL(169,169,0);
SetRGBSDL(169,169,169);
SetRGBSDL(0,0,84);
SetRGBSDL(0,0,255);
SetRGBSDL(0,169,84);
SetRGBSDL(0,169,255);
SetRGBSDL(169,0,84);
SetRGBSDL(169,0,255);
SetRGBSDL(169,169,84);
SetRGBSDL(169,169,255);
SetRGBSDL(0,84,0);
SetRGBSDL(0,84,169);
SetRGBSDL(0,255,0);
SetRGBSDL(0,255,169);
SetRGBSDL(169,84,0);
SetRGBSDL(169,84,169);
SetRGBSDL(169,255,0);
SetRGBSDL(169,255,169);
SetRGBSDL(0,84,84);
SetRGBSDL(0,84,255);
SetRGBSDL(0,255,84);
SetRGBSDL(0,255,255);
SetRGBSDL(169,84,84);
SetRGBSDL(169,84,255);
SetRGBSDL(169,255,84);
SetRGBSDL(169,255,255);
SetRGBSDL(84,0,0);
SetRGBSDL(84,0,169);
SetRGBSDL(84,169,0);
SetRGBSDL(84,169,169);
SetRGBSDL(255,0,0);
SetRGBSDL(255,0,169);
SetRGBSDL(255,169,0);
SetRGBSDL(255,169,169);
SetRGBSDL(84,0,84);
SetRGBSDL(84,0,255);
SetRGBSDL(84,169,84);
SetRGBSDL(84,169,255);
SetRGBSDL(255,0,84);
SetRGBSDL(255,0,255);
SetRGBSDL(255,169,84);
SetRGBSDL(255,169,255);
SetRGBSDL(84,84,0);
SetRGBSDL(84,84,169);
SetRGBSDL(84,255,0);
SetRGBSDL(84,255,169);
SetRGBSDL(255,84,0);
SetRGBSDL(255,84,169);
SetRGBSDL(255,255,0);
SetRGBSDL(255,255,169);
SetRGBSDL(84,84,84);
SetRGBSDL(84,84,255);
SetRGBSDL(84,255,84);
SetRGBSDL(84,255,255);
SetRGBSDL(255,84,84);
SetRGBSDL(255,84,255);
SetRGBSDL(255,255,84);
SetRGBSDL(255,255,255);
SetRGBSDL(255,125,125);
SetRGBSDL(255,157,125);
SetRGBSDL(255,190,125);
SetRGBSDL(255,222,125);
SetRGBSDL(255,255,125);
SetRGBSDL(222,255,125);
SetRGBSDL(190,255,125);
SetRGBSDL(157,255,125);
SetRGBSDL(125,255,125);
SetRGBSDL(125,255,157);
SetRGBSDL(125,255,190);
SetRGBSDL(125,255,222);
SetRGBSDL(125,255,255);
SetRGBSDL(125,222,255);
SetRGBSDL(125,190,255);
SetRGBSDL(125,157,255);
SetRGBSDL(182,182,255);
SetRGBSDL(198,182,255);
SetRGBSDL(218,182,255);
SetRGBSDL(234,182,255);
SetRGBSDL(255,182,255);
SetRGBSDL(255,182,234);
SetRGBSDL(255,182,218);
SetRGBSDL(255,182,198);
SetRGBSDL(255,182,182);
SetRGBSDL(255,198,182);
SetRGBSDL(255,218,182);
SetRGBSDL(255,234,182);
SetRGBSDL(255,255,182);
SetRGBSDL(234,255,182);
SetRGBSDL(218,255,182);
SetRGBSDL(198,255,182);
SetRGBSDL(182,255,182);
SetRGBSDL(182,255,198);
SetRGBSDL(182,255,218);
SetRGBSDL(182,255,234);
SetRGBSDL(182,255,255);
SetRGBSDL(182,234,255);
SetRGBSDL(182,218,255);
SetRGBSDL(182,198,255);
SetRGBSDL(0,0,113);
SetRGBSDL(28,0,113);
SetRGBSDL(56,0,113);
SetRGBSDL(84,0,113);
SetRGBSDL(113,0,113);
SetRGBSDL(113,0,84);
SetRGBSDL(113,0,56);
SetRGBSDL(113,0,28);
SetRGBSDL(113,0,0);
SetRGBSDL(113,28,0);
SetRGBSDL(113,56,0);
SetRGBSDL(113,84,0);
SetRGBSDL(113,113,0);
SetRGBSDL(84,113,0);
SetRGBSDL(56,113,0);
SetRGBSDL(28,113,0);
SetRGBSDL(0,113,0);
SetRGBSDL(0,113,28);
SetRGBSDL(0,113,56);
SetRGBSDL(0,113,84);
SetRGBSDL(0,113,113);
SetRGBSDL(0,84,113);
SetRGBSDL(0,56,113);
SetRGBSDL(0,28,113);
SetRGBSDL(56,56,113);
SetRGBSDL(68,56,113);
SetRGBSDL(84,56,113);
SetRGBSDL(97,56,113);
SetRGBSDL(113,56,113);
SetRGBSDL(113,56,97);
SetRGBSDL(113,56,84);
SetRGBSDL(113,56,68);
SetRGBSDL(113,56,56);
SetRGBSDL(113,68,56);
SetRGBSDL(113,84,56);
SetRGBSDL(113,97,56);
SetRGBSDL(113,113,56);
SetRGBSDL(97,113,56);
SetRGBSDL(84,113,56);
SetRGBSDL(68,113,56);
SetRGBSDL(56,113,56);
SetRGBSDL(56,113,68);
SetRGBSDL(56,113,84);
SetRGBSDL(56,113,97);
SetRGBSDL(56,113,113);
SetRGBSDL(56,97,113);
SetRGBSDL(56,84,113);
SetRGBSDL(56,68,113);
SetRGBSDL(80,80,113);
SetRGBSDL(89,80,113);
SetRGBSDL(97,80,113);
SetRGBSDL(105,80,113);
SetRGBSDL(113,80,113);
SetRGBSDL(113,80,105);
SetRGBSDL(113,80,97);
SetRGBSDL(113,80,89);
SetRGBSDL(113,80,80);
SetRGBSDL(113,89,80);
SetRGBSDL(113,97,80);
SetRGBSDL(113,105,80);
SetRGBSDL(113,113,80);
SetRGBSDL(105,113,80);
SetRGBSDL(97,113,80);
SetRGBSDL(89,113,80);
SetRGBSDL(80,113,80);
SetRGBSDL(80,113,89);
SetRGBSDL(80,113,97);
SetRGBSDL(80,113,105);
SetRGBSDL(80,113,113);
SetRGBSDL(80,105,113);
SetRGBSDL(80,97,113);
SetRGBSDL(80,89,113);
SetRGBSDL(0,0,64);
SetRGBSDL(16,0,64);
SetRGBSDL(32,0,64);
SetRGBSDL(48,0,64);
SetRGBSDL(64,0,64);
SetRGBSDL(64,0,48);
SetRGBSDL(64,0,32);
SetRGBSDL(64,0,16);
SetRGBSDL(64,0,0);
SetRGBSDL(64,16,0);
SetRGBSDL(64,32,0);
SetRGBSDL(64,48,0);
SetRGBSDL(64,64,0);
SetRGBSDL(48,64,0);
SetRGBSDL(32,64,0);
SetRGBSDL(16,64,0);
SetRGBSDL(0,64,0);
SetRGBSDL(0,64,16);
SetRGBSDL(0,64,32);
SetRGBSDL(0,64,48);
SetRGBSDL(0,64,64);
SetRGBSDL(0,48,64);
SetRGBSDL(0,32,64);
SetRGBSDL(0,16,64);
SetRGBSDL(32,32,64);
SetRGBSDL(40,32,64);
SetRGBSDL(48,32,64);
SetRGBSDL(56,32,64);
SetRGBSDL(64,32,64);
SetRGBSDL(64,32,56);
SetRGBSDL(64,32,48);
SetRGBSDL(64,32,40);
SetRGBSDL(64,32,32);
SetRGBSDL(64,40,32);
SetRGBSDL(64,48,32);
SetRGBSDL(64,56,32);
SetRGBSDL(64,64,32);
SetRGBSDL(56,64,32);
SetRGBSDL(48,64,32);
SetRGBSDL(40,64,32);
SetRGBSDL(32,64,32);
SetRGBSDL(32,64,40);
SetRGBSDL(32,64,48);
SetRGBSDL(32,64,56);
SetRGBSDL(32,64,64);
SetRGBSDL(32,56,64);
SetRGBSDL(32,48,64);
SetRGBSDL(32,40,64);
SetRGBSDL(44,44,64);
SetRGBSDL(48,44,64);
SetRGBSDL(52,44,64);
SetRGBSDL(60,44,64);
SetRGBSDL(64,44,64);
SetRGBSDL(64,44,60);
SetRGBSDL(64,44,52);
SetRGBSDL(64,44,48);
SetRGBSDL(64,44,44);
SetRGBSDL(64,48,44);
SetRGBSDL(64,52,44);
SetRGBSDL(64,60,44);
SetRGBSDL(64,64,44);
SetRGBSDL(60,64,44);
SetRGBSDL(52,64,44);
SetRGBSDL(48,64,44);
SetRGBSDL(44,64,44);
SetRGBSDL(44,64,48);
SetRGBSDL(44,64,52);
SetRGBSDL(44,64,60);
SetRGBSDL(44,64,64);
SetRGBSDL(44,60,64);
SetRGBSDL(44,52,64);
SetRGBSDL(44,48,64);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SetRGBSDL(0,0,0);
SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, gb_colors, 0, 256); 
*/
//}

//**************************
void PreparaPaleta()
{
 //InitPaletaCGA();
 //InitPaletaVGA();
 //InitPaletaPCJR();
 //InitPaletaCGAbw();
 InitPaletaCGAgray();
}

void setwindowtitle (uint8_t *extra) {
	//char temptext[128];
	//sprintf (temptext, "%s%s", windowtitle, extra);
	//JJ no SDL SDL_WM_SetCaption ( (const char *) temptext, NULL);
}

//uint8_t initscreen (uint8_t *ver) 
unsigned char initscreen() 
{
	//if (doaudio) {
	//		if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) ) return (0);
	//	}
	//else {
	//		if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) ) return (0);
	//	}
	//if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) ) return (0);
	//JJ no SDL if (SDL_Init (SDL_INIT_VIDEO)) return (0);
	//JJ screen = SDL_SetVideoMode (640, 400, 32, SDL_HWSURFACE);	
	#ifdef use_lib_force_sdl_8bpp
 	 //JJ no SDL screen = SDL_SetVideoMode (640, 400, 8, SDL_SWSURFACE | SDL_DOUBLEBUF); 	 
     PreparaPaleta();     
 	#else 
 	 screen = SDL_SetVideoMode (640, 400, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
 	#endif
 	//JJ no SDL SDL_WM_SetCaption ("ESP32 Fake86", NULL);
	#ifdef use_lib_log_serial
	 Serial.printf("initscreen SDL_SetVideoMode\n");
	#endif 
	//fflush(stdout);
	//if (screen == NULL) return (0);
	//sprintf (windowtitle, "%s", ver);
	//JJ setwindowtitle ("");
	//initcga(); //No necesito inicializar cga
	//JJ no SDL if (screen == NULL) return (0);
//JJ #ifdef _WIN32
//JJ 	InitializeCriticalSection (&screenmutex);
//JJ 	#ifndef use_lib_nothread
//JJ  	 _beginthread (VideoThread, 0, NULL);
//JJ  	#endif
//JJ #else
//JJ 	#ifndef use_lib_nothread
//JJ  	 pthread_create (&vidthread, NULL, (void *) VideoThread, NULL);
//JJ  	#endif 
//JJ #endif

	return (1);
}

//uint32_t prestretch[1024][1024]; //no necesito escalar
uint32_t nw, nh; //native width and height, pre-stretching (i.e. 320x200 for mode 13h)
//JJ no necesito escalar
/*
void createscalemap() {
	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	double xscale, yscale;

	xscale = (double) nw / (double) screen->w;
	yscale = (double) nh / (double) screen->h;
	if (scalemap != NULL) free(scalemap);
	//scalemap = (void *)malloc( ((uint32_t)screen->w + 1) * (uint32_t)screen->h * 4);
	scalemap = (int *)malloc( ((uint32_t)screen->w + 1) * (uint32_t)screen->h * 4);
	if (scalemap == NULL) {
			printf("\nFATAL: Unable to allocate memory for scalemap!\n");
			exit(1);
		}
	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)screen->h; dsty++) {
			srcy = (uint32_t) ( (double) dsty * yscale);
			scalemap[scalemapptr++] = srcy;
			for (dstx=0; dstx<(uint32_t)screen->w; dstx++) {
					srcx = (uint32_t) ( (double) dstx * xscale);
					scalemap[scalemapptr++] = srcx;
				}
		}

	regenscalemap = 0;
}
*/

extern uint16_t oldw, oldh;
void draw();
extern void handleinput();
//JJ #ifdef _WIN32
//JJ void VideoThread (void *dummy) {
//JJ #else
//JJ void *VideoThread (void *dummy) {
//JJ #endif
//JJ  printf("VideoThread\n");
//JJ  fflush(stdout);
//JJ 	uint32_t cursorprevtick, cursorcurtick, delaycalc;
//JJ 	cursorprevtick = SDL_GetTicks();
//JJ 	cursorvisible = 0;
//JJ 	while (running) {
//JJ 			cursorcurtick = SDL_GetTicks();
//JJ 			if ( (cursorcurtick - cursorprevtick) >= 250) {
//JJ 					updatedscreen = 1;
//JJ 					cursorvisible = ~cursorvisible & 1;
//JJ 					cursorprevtick = cursorcurtick;
//JJ 				}
//JJ 
//JJ 			if (updatedscreen || renderbenchmark) {
//JJ 					updatedscreen = 0;
//JJ 					if (screen != NULL) {
//JJ 							if (regenscalemap) createscalemap();
//JJ 							//JJ MutexLock (screenmutex);
//JJ 							draw();
//JJ 							//JJ MutexUnlock (screenmutex);
//JJ 						}
//JJ 					totalframes++;
//JJ 				}
//JJ 			if (!renderbenchmark) {
//JJ 					delaycalc = framedelay - (SDL_GetTicks() - cursorcurtick);
//JJ 					if (delaycalc > framedelay) delaycalc = framedelay;
//JJ 					//JJ SDL_Delay (delaycalc);
//JJ 				}
//JJ 		}
//JJ }



void VideoThreadPoll()
{
 draw();
/*
 //printf("VideoThreadPoll\n");
 //fflush(stdout);

//JJ	uint32_t cursorprevtick, cursorcurtick, delaycalc;
//JJ	cursorprevtick = millis();
//JJ	cursorvisible = 0;
	if (running) 
    {
//JJ			cursorcurtick = millis();
//JJ			if ( (cursorcurtick - cursorprevtick) >= 250) {
//JJ					//updatedscreen = 1;
//JJ					cursorvisible = ~cursorvisible & 1;
//JJ					cursorprevtick = cursorcurtick;
//JJ				}

			//if (updatedscreen || renderbenchmark) 
			//{
			//		updatedscreen = 0;
					//JJ no SDL if (screen != NULL) {
							//JJ if (regenscalemap) createscalemap(); //no necesito escalar
							//MutexLock (screenmutex);
							draw();
							//MutexUnlock (screenmutex);
					//JJ no SDL	}
					totalframes++;
			//}
//JJ			if (!renderbenchmark) {
//JJ					delaycalc = framedelay - (millis() - cursorcurtick);
//JJ					if (delaycalc > framedelay) delaycalc = framedelay;
//JJ					//JJ SDL_Delay (delaycalc);
//JJ				}
		}     
		
*/  
}

//********************************************************
/*void jj_doscrmodechange()
{
	//MutexLock (screenmutex);
	if (scrmodechange) 
    {
			if (screen != NULL)
			{
             //SDL_FreeSurface (screen);
            }
			if (constantw && constanth)
			{
             //JJ screen = SDL_SetVideoMode (constantw, constanth, 32, SDL_HWSURFACE | usefullscreen);
             //screen = SDL_SetVideoMode (constantw, constanth, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
             printf("SDL_SetVideoMode constantw:%d,constanth\n",constantw,constanth);
             fflush(stdout);
            }
			else
            { 
            if (noscale) 
            {
             //JJ screen = SDL_SetVideoMode (nw, nh, 32, SDL_HWSURFACE | usefullscreen);
             //screen = SDL_SetVideoMode (nw, nh, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
             printf("initscreen doscrmodechange nw:%d nh:%d rarito\n",nw,nh);
            }
			else 
            {
					if ( (nw >= 640) || (nh >= 400) )
					{
                     //JJ screen = SDL_SetVideoMode (nw, nh, 32, SDL_HWSURFACE | usefullscreen);
                     //screen = SDL_SetVideoMode (nw, nh, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
                     printf("initscreen doscrmodechange nw:%d nh:%d\n",nw,nh);
                     fflush(stdout);
                    }
					else
                    {
                     //JJ screen = SDL_SetVideoMode (640, 400, 32, SDL_HWSURFACE | usefullscreen);
                     //screen = SDL_SetVideoMode (640, 400, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
                     printf("initscreen raro jj_doscrmodechange\n");
                     fflush(stdout);
                    }
				}
            }
			regenscalemap = 1;
			createscalemap();
		}
	//MutexUnlock (screenmutex);
	scrmodechange = 0;     
}*/

void doscrmodechange() 
{
//jj_doscrmodechange();
     
//JJ	MutexLock (screenmutex);

   //printf("vidmode:%d scrmodechange:%d\n",vidmode,scrmodechange);
   //fflush(stdout);
	if (scrmodechange) 
    {
			//JJ if (screen != NULL)
			//JJ{
            //JJ SDL_FreeSurface (screen);
            //JJ //SDL_Delay(500);
            //JJ}
			if (constantw && constanth)
			{
             //JJ screen = SDL_SetVideoMode (constantw, constanth, 32, SDL_HWSURFACE | usefullscreen);
             //screen = SDL_SetVideoMode (constantw, constanth, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
             //printf("1)SDL_SetVideoMode constantw:%d,constanth\n",constantw,constanth);
             //fflush(stdout);
            }
			else
            { 
            //if (noscale) 
            //{
            // //JJ screen = SDL_SetVideoMode (nw, nh, 32, SDL_HWSURFACE | usefullscreen);
            // //screen = SDL_SetVideoMode (nw, nh, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
            // printf("2)initscreen doscrmodechange nw:%d nh:%d rarito\n",nw,nh);
            // fflush(stdout);
            //}
			//else 
            {
					if ( (nw >= 640) || (nh >= 400) )
					{
                     //JJ screen = SDL_SetVideoMode (nw, nh, 32, SDL_HWSURFACE | usefullscreen);
                     //screen = SDL_SetVideoMode (nw, nh, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
                     //printf("3)initscreen doscrmodechange nw:%d nh:%d\n",nw,nh);
                     //fflush(stdout);
                    }
					else
                    {
                     //JJ screen = SDL_SetVideoMode (640, 400, 32, SDL_HWSURFACE | usefullscreen);
                     //screen = SDL_SetVideoMode (640, 400, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
                     //printf("4)initscreen raro\n");
                     //fflush(stdout);
                    }
				}
            }
//JJ    	    if (usefullscreen) SDL_WM_GrabInput (SDL_GRAB_ON); //always have mouse grab turned on for full screen mode
//JJ			else SDL_WM_GrabInput (usegrabmode);
//JJ SDL_WM_GrabInput (SDL_GRAB_QUERY);
//JJ			SDL_ShowCursor (SDL_DISABLE);			
			if (!usefullscreen) {
//JJ					if (usegrabmode == SDL_GRAB_ON) setwindowtitle (" (press Ctrl + Alt to release mouse)");
//JJ					else setwindowtitle ("");
				}
//			regenscalemap = 1;
			//JJ createscalemap(); //no necesito escalar
		}
//JJ	MutexUnlock (screenmutex);
	scrmodechange = 0;

 //printf("FIN doscrmodechange\n");
 //fflush(stdout);
}

//JJ No necesito escalar
/*
void stretchblit (SDL_Surface *target) 
{
	uint32_t srcx, srcy, dstx, dsty, lastx, lasty, r, g, b;
	uint32_t consecutivex, consecutivey = 0, limitx, limity, scalemapptr;
	uint32_t ofs;
	uint8_t *pixelrgb;


#ifdef use_lib_force_sdl_direct_vga
 SDL_Flip(target);
 return;
#endif

	limitx = (uint32_t)((double) nw / (double) target->w);
	limity = (uint32_t)((double) nh / (double) target->h);

	if (SDL_MUSTLOCK (target) )
		if (SDL_LockSurface (target) < 0)
			return;

	lasty = 0;
	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)target->h; dsty++) {
			srcy = scalemap[scalemapptr++];
			ofs = dsty*target->w;
			consecutivex = 0;
			lastx = 0;
			if (srcy == lasty) consecutivey++;
			else consecutivey = 0;
			for (dstx=0; dstx<(uint32_t)target->w; dstx++) {
					srcx = scalemap[scalemapptr++];
					pixelrgb = (uint8_t *) &prestretch[srcy][srcx];
					r = pixelrgb[0];
					g = pixelrgb[1];
					b = pixelrgb[2];
					if (srcx == lastx) consecutivex++;
					else consecutivex = 0;
					if ( (consecutivex > limitx) && (consecutivey > limity) ) {
							pixelrgb = (uint8_t *) &prestretch[srcy][srcx+1];
							r += pixelrgb[0];
							g += pixelrgb[1];
							b += pixelrgb[2];
							pixelrgb = (uint8_t *) &prestretch[srcy+1][srcx];
							r += pixelrgb[0];
							g += pixelrgb[1];
							b += pixelrgb[2];
							pixelrgb = (uint8_t *) &prestretch[srcy+1][srcx+1];
							r += pixelrgb[0];
							g += pixelrgb[1];
							b += pixelrgb[2];
							r = r >> 2;
							g = g >> 2;
							b = b >> 2;
							//r = 255; g = 0; b = 0;
						}
					else if (consecutivex > limitx) {
							pixelrgb = (uint8_t *) &prestretch[srcy][srcx+1];
							r += pixelrgb[0];
							r = r >> 1;
							g += pixelrgb[1];
							g = g >> 1;
							b += pixelrgb[2];
							b = b >> 1;
							//r = 0; g = 255; b = 0;
						}
					else if (consecutivey > limity) {
							pixelrgb = (uint8_t *) &prestretch[srcy+1][srcx];
							r += pixelrgb[0];
							r = r >> 1;
							g += pixelrgb[1];
							g = g >> 1;
							b += pixelrgb[2];
							b = b >> 1;
							//r = 0; g = 0; b = 255;
						}
					( (uint32_t *) target->pixels) [ofs++] = SDL_MapRGB (target->format, (uint8_t) r, (uint8_t) g, (uint8_t) b);
					lastx = srcx;
				}
			lasty = srcy;
		}

	if (SDL_MUSTLOCK (target) )
		SDL_UnlockSurface (target);
	SDL_UpdateRect (target, 0, 0, target->w, target->h);
}
*/

//JJ no neceisto escalado
/*
void roughblit (SDL_Surface *target)
{
	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	int32_t ofs;
	uint8_t *pixelrgb;

	if (SDL_MUSTLOCK (target) )
		if (SDL_LockSurface (target) < 0)
			return;

	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)target->h; dsty++) {
			srcy = scalemap[scalemapptr++];
			ofs = dsty*target->w;
			for (dstx=0; dstx<(uint32_t)target->w; dstx++) {
					srcx = scalemap[scalemapptr++];
					pixelrgb = (uint8_t *) &prestretch[srcy][srcx];
					( (uint32_t *) target->pixels) [ofs++] = SDL_MapRGB (target->format, pixelrgb[0], pixelrgb[1], pixelrgb[2]);
				}
		}

	if (SDL_MUSTLOCK (target) )
		SDL_UnlockSurface (target);
	SDL_UpdateRect (target, 0, 0, target->w, target->h);
}
*/

/* NOTE: doubleblit is only used when smoothing is not enabled, and the SDL window size
         is exactly double of native resolution for the current video mode. we can take
         advantage of the fact that every pixel is simply doubled both horizontally and
         vertically. this way, we do not need to waste mountains of CPU time doing
         floating point multiplication for each and every on-screen pixel. it makes the
         difference between games being smooth and playable, and being jerky on my old
         400 MHz PowerPC G3 iMac.
*/
//JJ No necesito escalado
/*
void doubleblit (SDL_Surface *target) {
	uint32_t srcx, srcy, dstx, dsty, curcolor;
	int32_t ofs, startofs;
	uint8_t *pixelrgb;

	if (SDL_MUSTLOCK (target) )
		if (SDL_LockSurface (target) < 0)
			return;

	for (dsty=0; dsty<(uint32_t)target->h; dsty += 2) {
			srcy = (uint32_t) (dsty >> 1);
			startofs = ofs = dsty*target->w;
			for (dstx=0; dstx<(uint32_t)target->w; dstx += 2) {
					srcx = (uint32_t) (dstx >> 1);
					pixelrgb = (uint8_t *) &prestretch[srcy][srcx];
					curcolor = SDL_MapRGB (target->format, pixelrgb[0], pixelrgb[1], pixelrgb[2]);
					( (uint32_t *) target->pixels) [ofs+target->w] = curcolor;
					( (uint32_t *) target->pixels) [ofs++] = curcolor;
					( (uint32_t *) target->pixels) [ofs+target->w] = curcolor;
					( (uint32_t *) target->pixels) [ofs++] = curcolor;
				}
		}

	if (SDL_MUSTLOCK (target) )
		SDL_UnlockSurface (target);
	SDL_UpdateRect (target, 0, 0, target->w, target->h);
}
*/

inline void jj_fast_putpixel(int x,int y,unsigned char c)
{
 //if (x<0 || x>319 || y<0 || y>199)
 // return;
 //vga.dotFast(x,y,(c & 0x3F));
 gb_buffer_vga[y][x^2]= gb_color_vga[c];
}

/*JJ no SDL
void jj_fast_putpixel(SDL_Surface *auxScreen, int x,int y,unsigned char c)
{
 if (x > 639|| x<0 || y<0 || y > 399)
 {
  printf("x:%d y:%d\n",x,y);
  fflush(stdout);
  return;
 }
 if (auxScreen == NULL)
 {
  printf("screen null\n");
  fflush(stdout);
  return;
 }
 //Uint8* p = (Uint8*)auxScreen->pixels + (y * auxScreen->pitch) + x;
 //*p= c; 
 #ifdef use_lib_force_sdl_8bpp
  Uint8* p = (Uint8*)auxScreen->pixels + (y * auxScreen->pitch) + x;
  *p= c;   
 #else
  Uint32* p = (Uint32*)auxScreen->pixels + (y * auxScreen->pitch) + x;
  *p= c;  
 #endif
}
*/

extern uint16_t vtotal;



//*************************************************************************************
void SDLprintChar(char car,int x,int y,unsigned char color,unsigned char backcolor)
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
   //jj_fast_putpixel(x+(7-i),y+j,(auxColor==1)?color:backcolor);
   //gb_buffer_vga[(y+j)][(x+(7-i))^2]= gb_color_vga[(auxColor==1)?color:backcolor];
   gb_buffer_vga[(y+j)][(x+(7-i))^2]= gb_color_text_cga[(auxColor==1)?color:backcolor];
  }
 }
}


//*****************************************
void SDLprintChar160x100_font4x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
{
 unsigned char aux;
 unsigned char auxBit,auxColor;
 unsigned char aColor;
 unsigned char nibble0;
 unsigned char nibble1;
 switch (car)
 {
  case 221: nibble0=color; nibble1=backcolor; break;
  case 222: nibble0=backcolor; nibble1=color; break;
  default: nibble0=0; nibble1=0; break;  
 }
 for (unsigned char j=0;j<2;j++)
 {
  //aux = gb_sdl_font_8x8[auxId + j];  
  for (int i=0;i<2;i++)
  {//4 primeros pixels      
   gb_buffer_vga[(y+j)][(x+i)^2]= gb_color_text_cga[nibble0];
  }
  for (int i=2;i<4;i++)
  {//4 segundos pixels   
   gb_buffer_vga[(y+j)][(x+i)^2]= gb_color_text_cga[nibble1];
  }  
 }
}     

//*****************************************
void SDLdump160x100_font4x8()
{
 unsigned char aColor,aBgColor,aChar;
 unsigned int auxOffset=0;     
 for (int y=0;y<100;y++)
 {  
  for (unsigned char x=0;x<80;x++) //Modo 80x25
  {
   aChar= gb_video_cga[auxOffset];
   auxOffset++;
   aColor = gb_video_cga[auxOffset]&0x0F;
   aBgColor = ((gb_video_cga[auxOffset]>>4)&0x0F);   
   SDLprintChar160x100_font4x8(aChar,(x<<2),(y*2),aColor,aBgColor);  //40x25
   auxOffset++;
  } 
 }
}

//*****************************************
void SDLdump80x25_font4x8()
{//Muestro 80 columnas
 unsigned char aColor,aBgColor,aChar,swapColor;;
 unsigned int auxOffset=0;
 //if ( (portram[0x3D8]==9) && (portram[0x3D4]==9) )
 //{
 // SDLdump160x100_font4x8(screen);
 // return;
 //}

 if ( (gb_portramTiny[fast_tiny_port_0x3D8]==9) && (gb_portramTiny[fast_tiny_port_0x3D4]==9) )
 {
  SDLdump160x100_font4x8();
  return;
 }
  
 for (int y=0;y<25;y++)
 {  
  for (int x=0;x<80;x++) //Modo 80x25
  {
   aChar= gb_video_cga[auxOffset];
   auxOffset++;
   //aColor = (((gb_video_cga[auxOffset]&0x0F)+1)<<2)-1;
   //aBgColor = ((gb_video_cga[auxOffset]>>4)&0x07)<<1;

   //aColor=0;
   //aBgColor=3;

   aColor = gb_video_cga[auxOffset]&0x0F;
   aBgColor = ((gb_video_cga[auxOffset]>>4)&0x07);

   if (gb_invert_color == 1)
   {
    swapColor= aColor;
    aColor= aBgColor;
    aBgColor= swapColor;
   }   

   #ifdef use_lib_capture_usb
    if (x<79){//Para verlo en capturadora
     SDLprintChar4x8(aChar,((x+1)<<2),(y<<3),aColor,aBgColor);  //80x25	
    }
   #else
    SDLprintChar4x8(aChar,(x<<2),(y<<3),aColor,aBgColor);//Sin capturadora
   #endif   
   auxOffset++;
  }
 }
}

//*****************************************
void SDLprintChar4x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
{ 
// unsigned char aux = gb_sdl_font_6x8[(car-64)];
 int auxId = car << 3; //*8
 unsigned char aux;
 unsigned char auxBit,auxColor;
 for (unsigned char j=0;j<8;j++)
 {  
  aux = gb_sdl_font_4x8[auxId + j];  
  for (int i=4;i<8;i++)
  {
   auxColor= ((aux>>i) & 0x01);
   //jj_fast_putpixel(x+(7-i),y+j,(auxColor==1)?color:backcolor);
   gb_buffer_vga[(y+j)][(x+(7-i))^2]= gb_color_text_cga[((auxColor==1)?color:backcolor)];
  }
 }
}


//*****************************************************
void SDLprintChar160x100_font8x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
{
 unsigned char aux;
 unsigned char auxBit,auxColor;
 unsigned char aColor;
 unsigned char nibble0;
 unsigned char nibble1; 
 switch (car)
 {
  case 221: nibble0=color; nibble1=backcolor; break;
  case 222: nibble0=backcolor; nibble1=color; break;
  default: nibble0=0; nibble1=0; break;  
 }
 for (unsigned char j=0;j<2;j++)
 {
  //aux = gb_sdl_font_8x8[auxId + j];  
  for (int i=0;i<4;i++)
  {//4 primeros pixels      
   gb_buffer_vga[(y+j)][(x+i)^2]= gb_color_text_cga[nibble0];
  }
  for (int i=4;i<8;i++)
  {//4 segundos pixels
   gb_buffer_vga[(y+j)][(x+i)^2]= gb_color_text_cga[nibble1];
  }  
 }     
}

//*****************************************************
void SDLdump160x100_font8x8()
{
 unsigned char aColor,aBgColor,aChar;
 unsigned int auxOffset=0;     
 for (int y=0;y<100;y++)
 {  
  for (unsigned char x=0;x<40;x++) //Modo 40x25
  {
   aChar= gb_video_cga[auxOffset];
   auxOffset++;
   aColor = gb_video_cga[auxOffset]&0x0F;   
   aBgColor = ((gb_video_cga[auxOffset]>>4)&0x0F);   
   SDLprintChar160x100_font8x8(aChar,(x<<3),(y*2),aColor,aBgColor);  //40x25
   auxOffset++;
  }
  auxOffset+= 80; //40x25  
 }
}

//*****************************************
void SDLdump80x25_font8x8()
{//Muestro solo 40 columnas
 unsigned char aColor,aBgColor,aChar;
 unsigned int auxOffset=0;
 if ( (gb_portramTiny[fast_tiny_port_0x3D8]==9) && (gb_portramTiny[fast_tiny_port_0x3D4]==9) ) 
 {
  //printf("Modo PAKUPAKU\n");
  //fflush(stdout);
  SDLdump160x100_font8x8();
  return;
 }

 for (unsigned char y=0;y<25;y++)
 {
  //for (unsigned char x=0;x<80;x++)
  for (unsigned char x=0;x<40;x++)
  {
   aChar= gb_video_cga[auxOffset];
   auxOffset++;
   //aColor = (((gb_video_cga[auxOffset]&0x0F)+1)<<2)-1;
   //aBgColor = ((gb_video_cga[auxOffset]>>4)&0x07)<<1;
   //aColor=3;
   //aBgColor=0;
   aColor = gb_video_cga[auxOffset]&0x0F;
   aBgColor = ((gb_video_cga[auxOffset]>>4)&0x07);
   #ifdef use_lib_capture_usb
    if (x<79){//Para verlo en capturadora    
     SDLprintChar(aChar,((x+1)<<3),(y<<3),aColor,aBgColor);  //Capturadora usb
    }
   #else
    SDLprintChar(aChar,(x<<3),(y<<3),aColor,aBgColor); //Sin capturadora
   #endif
   auxOffset++;
  }
  auxOffset+= 80;
 } 
}

static unsigned int gb_local_scanline[80];

//cga5 rapido
void jj_sdl_dump_cga5()
{//Bajado de 4000 micros a 3495
 //Puntero 32 bits 2310 
 //Puntero 32 bits scanline 2435 es mas lento
 unsigned short int cont=0;
 unsigned int a0,a1,a2,a3;
 unsigned char aux;
 unsigned int yDest; 
 unsigned int x;
 unsigned char y;
 unsigned int *ptr32;
 unsigned int a32;
  

 //jj_ini_vga = micros();

  for (y=0;y<100;y++)
  {        
   yDest= (y<<1);
   ptr32= (unsigned int *)gb_buffer_vga[yDest];
   for (x=0;x<80;x++)   
   {//Lineas impares
    aux = gb_video_cga[cont];   
    a3= (aux & 0x03); //empieza izquierda derecha pixel
    a2= ((aux>>2)& 0x03);
    a1= ((aux>>4)& 0x03);
    a0= ((aux>>6)& 0x03);
      		
	a32= (gb_color_vga[a2]) | (gb_color_vga[a3]<<8) | (gb_color_vga[a0]<<16) | (gb_color_vga[a1]<<24);
	//ptr32[x]= a32;
	gb_local_scanline[x]= a32;

    cont++;
   }
   memcpy(ptr32,gb_local_scanline,320);
  } 

  cont = 0x2000;   
  for (y=0;y<100;y++)
  {      
   yDest= (y<<1)+1;
   ptr32= (unsigned int *)gb_buffer_vga[yDest];  
   for (x=0;x<80;x++)
   {//Lineas impares
    aux = gb_video_cga[cont];   
    a3= (aux & 0x03); //empieza izquierda derecha pixel
    a2= ((aux>>2)& 0x03);
    a1= ((aux>>4)& 0x03);
    a0= ((aux>>6)& 0x03);
   	
	a32= (gb_color_vga[a2]) | (gb_color_vga[a3]<<8) | (gb_color_vga[a0]<<16) | (gb_color_vga[a1]<<24);
	//ptr32[x]= a32;
	gb_local_scanline[x]= a32;

    cont++;
   }
   memcpy(ptr32,gb_local_scanline,320);
  } 

 //jj_end_vga = micros();
 //gb_cur_vga_ticks= (jj_end_vga-jj_ini_vga);
 //if (gb_cur_vga_ticks>gb_max_vga_ticks)
 // gb_max_vga_ticks= gb_cur_vga_ticks;
 //if (gb_cur_vga_ticks<gb_min_vga_ticks)   
 // gb_min_vga_ticks= gb_cur_vga_ticks;
 //tiempo_vga=1;  

}


//cga6 rapido
void jj_sdl_dump_cga6()
{//640x200 1 bit Escalado a la mitad
 unsigned short int cont=0;
 unsigned char a0,a2,a4,a6;
 unsigned char aux;
 unsigned int yDest; 
 unsigned int x;
 unsigned char y;
 unsigned int *ptr32;
 unsigned int a32;
   

  for (y=0;y<100;y++)
  {        
   yDest= (y<<1);
   ptr32= (unsigned int *)gb_buffer_vga[yDest];
   for (x=0;x<80;x++)   
   {//Lineas impares
    aux = gb_video_cga[cont];   
    a6= ((aux>>1)& 0x01); //empieza izquierda derecha pixel
    a4= ((aux>>3)& 0x01);
    a2= ((aux>>5)& 0x01);
    a0= ((aux>>7)& 0x01);

    a0= (a0==0?0:3); //Deberia ser 15, por ahora 3 de 4 colores
    a2= (a2==0?0:3);
    a4= (a4==0?0:3);
    a6= (a6==0?0:3);

	a32= (gb_color_vga[a4]) | (gb_color_vga[a6]<<8) | (gb_color_vga[a0]<<16) | (gb_color_vga[a2]<<24);
	//ptr32[x]= a32;
	gb_local_scanline[x]= a32;

    cont++;
   }
   memcpy(ptr32,gb_local_scanline,320);
  } 

  cont = 0x2000;   
  for (y=0;y<100;y++)
  {      
   yDest= (y<<1)+1;
   ptr32= (unsigned int *)gb_buffer_vga[yDest];  
   for (x=0;x<80;x++)
   {//Lineas impares
    aux = gb_video_cga[cont];   
    a6= ((aux>>1)& 0x01); //empieza izquierda derecha pixel
    a4= ((aux>>3)& 0x01);
    a2= ((aux>>5)& 0x01);
    a0= ((aux>>7)& 0x01);

    a0= (a0==0?0:3);//Deberia ser 15, por ahora 3 de 4 colores
    a2= (a2==0?0:3);
    a4= (a4==0?0:3);
    a6= (a6==0?0:3);
   	
	a32= (gb_color_vga[a4]) | (gb_color_vga[a6]<<8) | (gb_color_vga[a0]<<16) | (gb_color_vga[a2]<<24);
	//ptr32[x]= a32;
	gb_local_scanline[x]= a32;

    cont++;
   }
   memcpy(ptr32,gb_local_scanline,320);
  } 

 //jj_end_vga = micros();
 //gb_cur_vga_ticks= (jj_end_vga-jj_ini_vga);
 //if (gb_cur_vga_ticks>gb_max_vga_ticks)
 // gb_max_vga_ticks= gb_cur_vga_ticks;
 //if (gb_cur_vga_ticks<gb_min_vga_ticks)   
 // gb_min_vga_ticks= gb_cur_vga_ticks;
 //tiempo_vga=1;  

}



/*
void jj_sdl_dump_cga5()
{//320x200 4 colores
 int cont=0;
 unsigned char a0,a1,a2,a3;
 unsigned char aux;
 int xDest,yDest;

 jj_ini_vga = micros();

 //B800:0000 even(0,2,4,...)  B800:2000 odd(1,3,5...)
 //100 lineas pares, 100 impares
 for (int i=0;i<2;i++)
 {//lineas pares impares  
  if (i==0)
  {   
   cont=0;
  }
  else
  {   
   cont = 0x2000;    
  }
  for (int y=0;y<100;y++)
  {  
   //for (int x=0;x<640;x+=8)
   for (int x=0;x<80;x++)
   {//Lineas impares
    aux = gb_video_cga[cont];   
    a3= (aux & 0x03); //empieza izquierda derecha pixel
    a2= ((aux>>2)& 0x03);
    a1= ((aux>>4)& 0x03);
    a0= ((aux>>6)& 0x03);
   
    //xDest= x>>1;
	xDest= x<<2;
    yDest= (y<<1)+i;
    //jj_fast_putpixel(xDest,yDest,a0);
    //jj_fast_putpixel(xDest+1,yDest,a1);
    //jj_fast_putpixel(xDest+2,yDest,a2);
    //jj_fast_putpixel(xDest+3,yDest,a3);    

	gb_buffer_vga[yDest][xDest^2]= gb_color_vga[a0];
	gb_buffer_vga[yDest][(xDest+1)^2]= gb_color_vga[a1];
	gb_buffer_vga[yDest][(xDest+2)^2]= gb_color_vga[a2];
	gb_buffer_vga[yDest][(xDest+3)^2]= gb_color_vga[a3];
    cont++;
   }
  }
 }

 jj_end_vga = micros();
 gb_cur_vga_ticks= (jj_end_vga-jj_ini_vga);
 if (gb_cur_vga_ticks>gb_max_vga_ticks)
  gb_max_vga_ticks= gb_cur_vga_ticks;
 if (gb_cur_vga_ticks<gb_min_vga_ticks)   
  gb_min_vga_ticks= gb_cur_vga_ticks;
 tiempo_vga=1;
}
*/


//void jj_video_0x82(void)
//{
// SDLdump80x25();
// return;
 /*

 int charx,divx,chary,vgapage,vidptr,auxOffs;
 int x,y;
 unsigned char curchar,auxColor8;
 int color;
 videobase = 0xB8000;
 nw = 640;
 nh = 400;
 
 //for (y=0; y<400; y++)
 for (y=0; y<400; y+=2)
 {
  //for (x=0; x<640; x++)
  for (x=0; x<640; x+=2)
  {      
   if (cols==80){
	charx = x/8;
	divx = 1;
   }
   else{
	charx = x/16;
	divx = 2;
   }
   if ( (portram[0x3D8]==9) && (portram[0x3D4]==9) )
   {
	chary = y/4;
	vidptr = vgapage + videobase + chary*cols*2 + charx*2;
	//JJ curchar = RAM[vidptr];
    curchar = read86(vidptr);
	auxOffs= curchar*128 + (y%4) *8 + ( (x/divx) %8);
	if (auxOffs>32767)
	 auxOffs=32767;
	color = fontcga[auxOffs];
	//color = fontcga[curchar*128 + (y%4) *8 + ( (x/divx) %8) ];									
	auxColor8 = curchar*128 + (y%4) *8 + ( (x/divx) %8);
   }
   else 
   {
	chary = y/16;
	vidptr = videobase + chary*cols*2 + charx*2;
    //JJ curchar = RAM[vidptr];
	curchar = read86(vidptr);
	//color = fontcga[curchar*128 + (y%16) *8 + ( (x/divx) %8) ];
	auxOffs = curchar*128 + (y%16) *8 + ( (x/divx) %8);
	if (auxOffs>32767)
	 auxOffs=32767;
	color = fontcga[auxOffs];
	auxColor8=curchar*128 + (y%16) *8 + ( (x/divx) %8);
   }
   if (vidcolor)
   {
    if (!color)
    {
                              if (portram[0x3D8]&128)
                              {
                               //JJ color = palettecga[ (RAM[vidptr+1]/16) &7];
                               //JJ auxColor8 = (RAM[vidptr+1]/16) &7;
                               //color = palettecga[ (read86(vidptr+1)/16) &7];
   							   auxOffs = (read86(vidptr+1)/16) &7;
							   if (auxOffs>=15)
						        auxOffs=15;									
   							   color = palettecga[auxOffs];                               
                               auxColor8 = (read86(vidptr+1)/16) &7;
                              }
							  else
							  {
                               //JJ color = palettecga[RAM[vidptr+1]/16]; //high intensity background
                               //JJ auxColor8= RAM[vidptr+1]/16;
                               //Mascara paleta limite &7
                               //color = palettecga[(read86(vidptr+1)/16) & 7]; //high intensity background
                               auxOffs= (read86(vidptr+1)/16) & 7;
							   if (auxOffs>=15)
						        auxOffs=15;									
   							   color = palettecga[auxOffs];                                                              
                               auxColor8= read86(vidptr+1)/16;
                              }
                             }
							 else
							 {
                              //JJ color = palettecga[RAM[vidptr+1]&15];
                              //JJ auxColor8 = RAM[vidptr+1]&15;
                              //color = palettecga[read86(vidptr+1)&15];
                              auxOffs= (vidptr+1)&15;
                              if (auxOffs>=15)
						        auxOffs=15;
						      color = palettecga[auxOffs];  
                              auxColor8 = read86(vidptr+1)&15;                              
                             }
                            }
							else 
                            {
							 //JJ if ( (RAM[vidptr+1] & 0x70) ) 
							 if ( (read86(vidptr+1) & 0x70) ) 
                             {
							  if (!color)
                              {
                               color = palettecga[7];
                              }
							  else
                              {
                               color = palettecga[0];
                              }
							 }
							 else 
                             {
							  if (!color)
                              {
                               color = palettecga[0];
                              }
							  else
                              {
                               color = palettecga[7];
                              }
							 }
							}

                          //jj_fast_putpixel(x,y,auxColor8);						  
                          //if (((y & 1)==1) && ((x & 1) == 1)){ //linea impar
                          // jj_fast_putpixel(screen,(x>>1),(y>>1),auxColor8);
                          //}
                          jj_fast_putpixel(x>>1,y>>1,auxColor8);
						}   
                    }  
  */                  
//}

void draw () 
{
// jj_video_0x82();
// return;
     
 //#ifdef use_lib_force_sdl_direct_vga     
 // unsigned char auxColor8;
 //#endif
 //printf("Video:0x%02X\n",vidmode);
 //fflush(stdout);
 int x,y;
 
	uint32_t planemode, vgapage, color, chary, charx, vidptr, divx, divy, curchar, curpixel, usepal, intensity, blockw, curheight, x1, y1;
//	if (gb_force_set_cga == 1)
//    {
//     gb_force_set_cga=0;
//     vidmode= 5;
//    }
	switch (vidmode) 
    {
			case 0:
			case 1:
			case 2: //text modes
			case 3:
			case 7:
       		case 0x82:
             //jj_video_0x82(); //Modo especial texto sin registros VGA
			 if (gb_font_8x8 == 1)
			  SDLdump80x25_font8x8();
			 else 
			  SDLdump80x25_font4x8();
             //if (gb_font_8x8 == 1)
             // SDLdump80x25_font8x8();
             //else 
             // SDLdump80x25_font4x8();
             //jj_video_0x82(); //Modo especial texto sin registros VGA             			 
             break;
            /*                 
			case 0x82:
				nw = 640;
				nh = 400;
				vgapage = ( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD];
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							if (cols==80) {
									charx = x/8;
									divx = 1;
								}
							else {
									charx = x/16;
									divx = 2;
								}
							if ( (portram[0x3D8]==9) && (portram[0x3D4]==9) ) {
									chary = y/4;
									vidptr = vgapage + videobase + chary*cols*2 + charx*2;
									//JJ curchar = RAM[vidptr];
									curchar = read86(vidptr);
									color = fontcga[curchar*128 + (y%4) *8 + ( (x/divx) %8) ];
									auxColor8 = curchar*128 + (y%4) *8 + ( (x/divx) %8);
								}
							else {
									chary = y/16;
									vidptr = videobase + chary*cols*2 + charx*2;
									//JJ curchar = RAM[vidptr];
									curchar = read86(vidptr);
									color = fontcga[curchar*128 + (y%16) *8 + ( (x/divx) %8) ];
									auxColor8=curchar*128 + (y%16) *8 + ( (x/divx) %8);
								}
							if (vidcolor)
                            {
							 if (!color)
							 {
                              if (portram[0x3D8]&128)
                              {
                               //JJ color = palettecga[ (RAM[vidptr+1]/16) &7];
                               //JJ auxColor8 = (RAM[vidptr+1]/16) &7;
                               color = palettecga[ (read86(vidptr+1)/16) &7];
                               auxColor8 = (read86(vidptr+1)/16) &7;
                              }
							  else
							  {
                               //JJ color = palettecga[RAM[vidptr+1]/16]; //high intensity background
                               //JJ auxColor8= RAM[vidptr+1]/16;
                               
                               color = palettecga[read86(vidptr+1)/16]; //high intensity background
                               auxColor8= read86(vidptr+1)/16;
                              }
                             }
							 else
							 {
                              //JJ color = palettecga[RAM[vidptr+1]&15];
                              //JJ auxColor8 = RAM[vidptr+1]&15;
                              color = palettecga[read86(vidptr+1)&15];
                              auxColor8 = read86(vidptr+1)&15;                              
                             }
                            }
							else 
                            {
							 //JJ if ( (RAM[vidptr+1] & 0x70) ) 
							 if ( (read86(vidptr+1) & 0x70) ) 
                             {
							  if (!color)
                              {
                               color = palettecga[7];
                              }
							  else
                              {
                               color = palettecga[0];
                              }
							 }
							 else 
                             {
							  if (!color)
                              {
                               color = palettecga[0];
                              }
							  else
                              {
                               color = palettecga[7];
                              }
							 }
							}
                         #ifdef use_lib_force_sdl_direct_vga
                          if ((y & 1)==0){ //linea impar
                           //prestretch[(y>>1)][(x>>1)] = color;
                           jj_fast_putpixel(screen,(x>>1),(y>>1),auxColor8);
                           //jj_fast_putpixel(screen,(x>>1),(y>>1),color);                           
                          }
                         #else
						  //JJ prestretch[y][x] = color; //No escalar
					     #endif
						}
				break;
			 */



			case 4:
			case 5:
			    jj_sdl_dump_cga5();
				break;
				/*JJ no lo necesito
				nw = 320;
				nh = 200;
				usepal = (portram[0x3D9]>>5) & 1;
				intensity = ( (portram[0x3D9]>>4) & 1) << 3;
				for (y=0; y<200; y++) {
						for (x=0; x<320; x++) {
								charx = x;
								chary = y;
								//vidptr = videobase + ( (chary>>1) * 80) + ( (chary & 1) * 8192) + (charx >> 2);
								////curpixel = RAM[vidptr];
								//curpixel = read86(vidptr);
								vidptr = ( (chary>>1) * 80) + ( (chary & 1) * 8192) + (charx >> 2);
								curpixel= gb_video_cga[vidptr];								
								switch (charx & 3) {
										case 3:
											curpixel = curpixel & 3;
											break;
										case 2:
											curpixel = (curpixel>>2) & 3;
											break;
										case 1:
											curpixel = (curpixel>>4) & 3;
											break;
										case 0:
											curpixel = (curpixel>>6) & 3;
											break;
									}
								#ifdef use_lib_force_sdl_direct_vga
								 auxColor8= curpixel;
								#endif
                                	
								if (vidmode==4) {
										curpixel = curpixel * 2 + usepal + intensity;
										if (curpixel == (usepal + intensity) )  curpixel = cgabg;
										color = palettecga[curpixel];
                                        #ifdef use_lib_force_sdl_direct_vga
                                         jj_fast_putpixel(x,y,auxColor8);
                                        #else
 										 //JJ prestretch[y][x] = color; //no escalar
 										#endif
									}
								else {
										curpixel = curpixel * 63;
										color = palettecga[curpixel];
										#ifdef use_lib_force_sdl_direct_vga
 										 jj_fast_putpixel(x,y,auxColor8);
										#else
										 //JJ prestretch[y][x] = color; //No escalar
										#endif 
									}
							}
					}
				break;
				*/
			case 6:
			    jj_sdl_dump_cga6();  
				/*nw = 640;
				nh = 200;
				for (y=0; y<200; y++) {
						for (x=0; x<640; x++) {
								charx = x;
								chary = y;
								vidptr = videobase + ( (chary>>1) * 80) + ( (chary&1) * 8192) + (charx>>3);
								//JJ curpixel = (RAM[vidptr]>> (7- (charx&7) ) ) &1;
								curpixel = (read86(vidptr)>> (7- (charx&7) ) ) &1;
								color = palettecga[curpixel*15];
								//JJ prestretch[y][x] = color; //no necesito escalar
								jj_fast_putpixel((x>>1),y,color);
							}
					}
					*/
				break;
			case 127:
				nw = 720;
				nh = 348;
				for (y=0; y<348; y++) {
						for (x=0; x<720; x++) {
								charx = x;
								chary = y>>1;
								vidptr = videobase + ( (y & 3) << 13) + (y >> 2) *90 + (x >> 3);
								//JJ curpixel = (RAM[vidptr]>> (7- (charx&7) ) ) &1;
								curpixel = (read86(vidptr)>> (7- (charx&7) ) ) &1;
//JJ #ifdef __BIG_ENDIAN__
//JJ 								if (curpixel) color = 0xFFFFFF00;
//JJ #else
								if (curpixel) color = 0x00FFFFFF;
//JJ #endif
								else color = 0x00000000;
								//JJ prestretch[y][x] = color; //no necesito escalar
								jj_fast_putpixel((x>>2),(y>>1),color);
							}
					}
				break;
			case 0x8: //160x200 16-color (PCjr)
				nw = 640; //fix this
				nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							vidptr = 0xB8000 + (y>>2) *80 + (x>>3) + ( (y>>1) &1) *8192;
							if ( ( (x>>1) &1) ==0)
                            {
                             //color = palettecga[RAM[vidptr] >> 4];
                             color = palettecga[read86(vidptr) >> 4];
                            }
							else
                            {
                             //color = palettecga[RAM[vidptr] & 15];
                             color = palettecga[read86(vidptr) & 15];
                            }
							//JJ prestretch[y][x] = color; //no necesito escalar
							jj_fast_putpixel((x>>1),(y>>1),color);
						}
				break;
			case 0x9: //320x200 16-color (Tandy/PCjr)
				nw = 640; //fix this
				nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							vidptr = 0xB8000 + (y>>3) *160 + (x>>2) + ( (y>>1) &3) *8192;
							if ( ( (x>>1) &1) ==0)
                            {
                             //color = palettecga[RAM[vidptr] >> 4];
                             color = palettecga[read86(vidptr) >> 4];
                            }
							else
                            {
                             //color = palettecga[RAM[vidptr] & 15];
                             color = palettecga[read86(vidptr) & 15];
                            }
							//JJ prestretch[y][x] = color; //no necesito escalar
							jj_fast_putpixel((x>>1),(y>>1),color);
						}
				break;
			case 0xD:
			case 0xE:
				nw = 640; //fix this
				nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							divx = x>>1;
							divy = y>>1;
							vidptr = divy*40 + (divx>>3);
							x1 = 7 - (divx & 7);
							//JJ color = (VRAM[vidptr] >> x1) & 1;
							//JJ color += ( ( (VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
							//JJ color += ( ( (VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
							//JJ color += ( ( (VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
							//JJ color = palettevga[color];
							//JJ prestretch[y][x] = color; //no necesito escalar
							jj_fast_putpixel((x>>1),(y>>1),color);
						}
				break;
			case 0x10:
				nw = 640;
				nh = 350;
				for (y=0; y<350; y++)
					for (x=0; x<640; x++) {
							vidptr = y*80 + (x>>3);
							x1 = 7 - (x & 7);
							//JJ color = (VRAM[vidptr] >> x1) & 1;
							//JJ color += ( ( (VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
							//JJ color += ( ( (VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
							//JJ color += ( ( (VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
							//JJ color = palettevga[color];
							//JJ prestretch[y][x] = color; //no necesito escalar
							jj_fast_putpixel((x>>1),(y>>1),color);
						}
				break;

/* JJ Quito los modos de video VGA, solo dejo CGA
			case 0x12:
				nw = 640;
				nh = 480;
				vgapage = ( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD];
				for (y=0; y<nh; y++)
					for (x=0; x<nw; x++) {
							vidptr = y*80 + (x/8);
							//JJ color  = (VRAM[vidptr] >> (~x & 7) ) & 1;
							//JJ color |= ( (VRAM[vidptr+0x10000] >> (~x & 7) ) & 1) << 1;
							//JJ color |= ( (VRAM[vidptr+0x20000] >> (~x & 7) ) & 1) << 2;
							//JJ color |= ( (VRAM[vidptr+0x30000] >> (~x & 7) ) & 1) << 3;
							//JJ prestretch[y][x] = palettevga[color]; //no necesito VGA
						}
				break;
			case 0x13:
				if (vtotal == 11) { //ugly hack to show Flashback at the proper resolution
						nw = 256;
						nh = 224;
					}
				else {
						nw = 320;
						nh = 200;
					}
				if (VGA_SC[4] & 6) planemode = 1;
				else planemode = 0;
				vgapage = ( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD];
				for (y=0; y<nh; y++)
					for (x=0; x<nw; x++) {
							if (!planemode)
                            {
                             //JJ color = palettevga[RAM[videobase + y*nw + x]];
                            }
							else {
									vidptr = y*nw + x;
									vidptr = vidptr/4 + (x & 3) *0x10000;
									vidptr = vidptr + vgapage - (VGA_ATTR[0x13] & 15);
									//JJ color = palettevga[VRAM[vidptr]];
								}
							//JJ prestretch[y][x] = color; //no necesito escalar
							jj_fast_putpixel(screen,(x>>1),(y>>1),color);
						}
//Fin quitar modos VGA, solo dejo CGA
*/
						
      default:
       //printf("Video default:0x%02X\n",vidmode);
       //fflush(stdout);
       break;
	} //Fin switch vidmode

    //printf("vidgfxmode:%d\n",vidgfxmode);
    //fflush(stdout);
	/* No lo necesito
	if (vidgfxmode==0) 
    {     
	 if (cursorvisible) 
     {
	  curheight = 2;
	  if (cols==80){
       blockw = 8;
      }
	  else{
       blockw = 16;
      }
	  x1 = cursx * blockw;
	  y1 = cursy * 8 + 8 - curheight;
	  for (y=y1*2; y<=y1*2+curheight-1; y++)
	  {
	   for (x=x1; x<=x1+blockw-1; x++)
       {
		//JJ color = palettecga[RAM[videobase+cursy*cols*2+cursx*2+1]&15];
		//JJ prestretch[y&1023][x&1023] = color; // no necesito escalar
		color = palettecga[read86(videobase+cursy*cols*2+cursx*2+1)&15];
		jj_fast_putpixel((x>>1),(y>>1),color);
	   }
	  }
     }
	}//fin vidgfxmode
	*/
 
 #ifdef use_lib_force_sdl_blit		
  //JJ no necesito escalar
  //JJ stretchblit(screen); //Fuerzo a volcar SDL modo JJ
  //Al no necesitar escalar, le fuerzo un SDL_Flip
  //if (screen != NULL){
  // SDL_Flip(screen);//Tengo que mirar si lo quito
  //}
 #else 
//JJ no necesito escalar 
//JJ	if (nosmooth) 
//JJ    {
//JJ			if ( ((nw << 1) == screen->w) && ((nh << 1) == screen->h) )
//JJ            {
//JJ             #ifndef use_lib_not_use_doubleblit
//JJ              doubleblit (screen);
//JJ             #endif 
//JJ            }
//JJ			else
//JJ            {
//JJ             #ifndef use_lib_not_use_roughblit
//JJ              roughblit (screen);
//JJ             #endif 
//JJ            }
//JJ	}
//JJ	else
//JJ    {
//JJ      #ifndef use_lib_not_use_stretchblit        
//JJ       stretchblit (screen);
//JJ      #endif  
//JJ    }
 #endif     
}



//******************************************
void PreparaColorVGA()
{
 #ifdef use_lib_bitluni_fast
  //Modo grafico CGA
  for (unsigned char i=0;i<16;i++)
  {
   gb_color_vga[i] = gb_color_vga[i] | gb_sync_bits;
  }

  //Modo texto cga
  for (unsigned char i=0;i<16;i++)
  {
   gb_color_text_cga[i] = gb_color_text_cga[i] | gb_sync_bits; 
  } 
 #else
  //Modo grafico CGA
  for (unsigned char i=0;i<16;i++)
  {
   gb_color_vga[i] = (gb_color_vga[i] & vga.RGBAXMask) | vga.SBits; 
  }

  //Modo texto cga
  for (unsigned char i=0;i<16;i++)
  {
   gb_color_text_cga[i] = (gb_color_text_cga[i] & vga.RGBAXMask) | vga.SBits; 
  }
 #endif 
}


//cga 1
const unsigned char gb_color_cga[16]={ 
 0x00,0x28,0x22,0x2A,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};

//cga 2
const unsigned char gb_color_cga2[16]={ 
 0x00,0x08,0x02,0x0A,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};

//PCJR
const unsigned char gb_color_pcjr[16]={ 
 0x00,0x15,0x2A,0x3F,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};


//Escala grises cambiado orden 1 2 por 2 1
const unsigned char gb_color_cgagray[16]={ 
 0x00,0x2A,0x15,0x3F,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};

//const unsigned char gb_color_cgagray[16]={ 
// 0x00,0x15,0x2A,0x3F,0x21,0x19,0x10,0x1E,
// 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
//};


void InitPaletaCGA()
{
 memcpy(gb_color_vga,gb_color_cga,16);
 PreparaColorVGA();
}

void InitPaletaCGA2()
{
 memcpy(gb_color_vga,gb_color_cga2,16);
 PreparaColorVGA();
}

void InitPaletaCGAgray()
{
 memcpy(gb_color_vga,gb_color_cgagray,16);
 PreparaColorVGA();
}

void InitPaletaPCJR()
{
 memcpy(gb_color_vga,gb_color_pcjr,16);
 PreparaColorVGA();
}