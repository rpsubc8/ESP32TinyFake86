//Convertir
//if (dst & 0xFF00) {
//cf = 1;
//}
//else {
//cf = 0;
//}
//
//En
//cf = (dst & 0xFF00) != 0;

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
//
//
// cpu.c: functions to emulate the 8086/V20 CPU in software. the heart of Fake86.

#include "gbConfig.h"
#include "hardware.h"
#include "dataFlash/bios/biospcxt.h"
#include "dataFlash/rom/rombasic.h"
#include "dataFlash/rom/videorom.h"
#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include "cpu.h"
#include "i8259.h"
#include "i8253.h"
#include "gbGlobals.h"
#include <Arduino.h>
#include "dataFlash/gbsnarare.h"

extern struct i8253_s i8253;

extern struct structpic i8259;
uint64_t curtimer, lasttimer, timerfreq;

static unsigned char byteregtable[8] = { regal, regcl, regdl, regbl, regah, regch, regdh, regbh };

//static const uint8_t parity[0x100] = {
//	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
//	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
//	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
//	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
//	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
//	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
//	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
//	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
//};

static unsigned char parity[0x100] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

void FuerzoParityRAM()
{
 parity[0]=1; //Fuerzo a que se cree en RAM
}


volatile unsigned int gb_pulsos_onda=0;
volatile unsigned int gb_cont_my_callbackfunc=0;
volatile unsigned char gb_estado_sonido=0;
volatile unsigned char speaker_pin_estado=LOW;

void my_callback_speaker_func()
{ 
 gb_cont_my_callbackfunc++;
 if (gb_cont_my_callbackfunc>=gb_pulsos_onda)
 {
  gb_cont_my_callbackfunc=0;
  gb_estado_sonido = (~gb_estado_sonido)& 0x1;
  if ((gb_volumen01 == 0)||(gb_frecuencia01==0)||(gb_silence==1))
  {     
   //  digitalWrite(SPEAKER_PIN, LOW);
   digitalWrite(SPEAKER_PIN, LOW);
  }  
  else
  {
   if (speaker_pin_estado != gb_estado_sonido){
     //digitalWrite(SPEAKER_PIN,gb_estado_sonido);
	 //digitalWrite(25,gb_estado_sonido);
	 if (gb_estado_sonido==LOW)
	 {
	  //REG_WRITE(GPIO_OUT_W1TC_REG , BIT25); //LOW clear
	  digitalWrite(SPEAKER_PIN,LOW);
	 }
	 else
	 {
	  //REG_WRITE(GPIO_OUT_W1TS_REG , BIT25); //High Set
	  digitalWrite(SPEAKER_PIN,HIGH);
	 }
     speaker_pin_estado= gb_estado_sonido;
   }
  }
 } 
   
 //if ((gb_volumen01 == 0)||(gb_frecuencia01==0))
 // digitalWrite(SPEAKER_PIN, LOW);
}



static unsigned char opcode, segoverride, reptype;
//JJ bootdrive = 0;
//unsigned char hdcount = 0;
//uint16_t segregs[4], 
static unsigned short int savecs, saveip, ip, useseg, oldsp;
static unsigned char tempcf, oldcf, pf, af, zf, sf, tf, ifl, df, of, mode, reg, rm;
//unsigned char cf;
//static unsigned char cf; //Optimizado
static unsigned short int oper1, oper2, res16, disp16, temp16, dummy, stacksize, frametemp;
static unsigned char oper1b, oper2b, res8, disp8, temp8, nestlev, addrbyte;
static unsigned int temp1, temp2, temp3, temp4, temp5, temp32, tempaddr32, ea;
//static int result;
uint64_t totalexec;

//extern uint16_t VGA_SC[0x100], VGA_CRTC[0x100], VGA_ATTR[0x100], VGA_GC[0x100]; //no necesito VGA
//extern uint8_t updatedscreen;
union _bytewordregs_ regs;

//JJuint8_t	portram[0x10000];
//JJ uint8_t	running = 0
//unsigned char verbose,showcsip,debugmode,mouseemu;
unsigned char didbootstrap = 0;
//uint8_t	ethif;

//extern uint8_t	vidmode;
//JJ extern uint8_t verbose;

extern void vidinterrupt();

extern uint8_t readVGA (uint32_t addr32);

void ExternalSetCF(unsigned char valor)
{
 cf= valor;
}

void intcall86 (unsigned char intnum);

unsigned char GetRegAL(){ return regs.byteregs[regal]; }
unsigned char GetRegAH(){ return regs.byteregs[regah]; }
unsigned char GetRegBL(){ return regs.byteregs[regbl]; }
unsigned char GetRegBH(){ return regs.byteregs[regbh]; }
unsigned char GetRegCL(){ return regs.byteregs[regcl]; }
unsigned char GetRegCH(){ return regs.byteregs[regch]; }
unsigned char GetRegDL(){ return regs.byteregs[regdl]; }
unsigned char GetRegDH(){ return regs.byteregs[regdh]; }

unsigned short int GetRegCS(){ return segregs[regcs]; }
unsigned short int GetRegDS(){ return segregs[regds]; }
unsigned short int GetRegSS(){ return segregs[regss]; }
unsigned short int GetRegES(){ return segregs[reges]; }

unsigned short int GetRegIP(){ return ip; }
unsigned short int GetRegSP(){ return regs.wordregs[regsp]; }
unsigned short int GetRegBP(){ return regs.wordregs[regbp]; }
unsigned short int GetRegSI(){ return regs.wordregs[regsi]; }
unsigned short int GetRegDI(){ return regs.wordregs[regdi]; }

unsigned short int GetCF(){ return cf; }




void SetRegAL(unsigned char a){ regs.byteregs[regal]= a ; }
void SetRegAH(unsigned char a){ regs.byteregs[regah]= a; }
void SetRegBL(unsigned char a){ regs.byteregs[regbl]= a; }
void SetRegBH(unsigned char a){ regs.byteregs[regbh]= a; }
void SetRegCL(unsigned char a){ regs.byteregs[regcl]= a; }
void SetRegCH(unsigned char a){ regs.byteregs[regch]= a; }
void SetRegDL(unsigned char a){ regs.byteregs[regdl]= a; }
void SetRegDH(unsigned char a){ regs.byteregs[regdh]= a; }

void SetRegCS(unsigned short int a){ segregs[regcs]= a; }
void SetRegDS(unsigned short int a){ segregs[regds]= a; }
void SetRegSS(unsigned short int a){ segregs[regss]= a; }
void SetRegES(unsigned short int a){ segregs[reges]= a; }

void SetRegIP(unsigned short int a){ ip= a; }
void SetRegSP(unsigned short int a){ regs.wordregs[regsp]= a; }
void SetRegBP(unsigned short int a){ regs.wordregs[regbp]= a; }
void SetRegSI(unsigned short int a){ regs.wordregs[regsi]= a; }
void SetRegDI(unsigned short int a){ regs.wordregs[regdi]= a; }

void SetCF(unsigned short int a){ cf= a; }


unsigned char gb_check_memory_before;

#ifdef use_lib_fast_makeflagsword
 #define makeflagsword() \
	( \
	2 | (uint16_t) cf | ((uint16_t) pf << 2) | ((uint16_t) af << 4) | ((uint16_t) zf << 6) | ((uint16_t) sf << 7) | \
	((uint16_t) tf << 8) | ((uint16_t) ifl << 9) | ((uint16_t) df << 10) | ((uint16_t) of << 11) \
	)
#else
 static inline unsigned short int makeflagsword()
 {
  return (
	2 | (unsigned short int) cf | ((unsigned short int) pf << 2) | ((unsigned short int) af << 4) | ((unsigned short int) zf << 6) | ((unsigned short int) sf << 7) |
	((unsigned short int) tf << 8) | ((unsigned short int) ifl << 9) | ((unsigned short int) df << 10) | ((unsigned short int) of << 11)
	);
 }
#endif

#ifdef use_lib_fast_decodeflagsword
 static inline void decodeflagsword(unsigned short int x)
 {//Es mas rapido metodo inline que define original
  temp16 = x;
  cf = temp16 & 1;
  pf = (temp16 >> 2) & 1;
  af = (temp16 >> 4) & 1;
  zf = (temp16 >> 6) & 1;
  sf = (temp16 >> 7) & 1;
  tf = (temp16 >> 8) & 1;
  ifl = (temp16 >> 9) & 1;
  df = (temp16 >> 10) & 1;
  of = (temp16 >> 11) & 1;
 }
#else
 #define decodeflagsword(x) { \
	temp16 = x; \
	cf = temp16 & 1; \
	pf = (temp16 >> 2) & 1; \
	af = (temp16 >> 4) & 1; \
	zf = (temp16 >> 6) & 1; \
	sf = (temp16 >> 7) & 1; \
	tf = (temp16 >> 8) & 1; \
	ifl = (temp16 >> 9) & 1; \
	df = (temp16 >> 10) & 1; \
	of = (temp16 >> 11) & 1; \
	}
#endif



extern void	writeVGA (uint32_t addr32, uint8_t value);
extern void	portout (uint16_t portnum, uint8_t value);
extern void	portout16 (uint16_t portnum, uint16_t value);
extern uint8_t	portin (uint16_t portnum);
extern uint16_t portin16 (uint16_t portnum);

unsigned char gb_ram_truco_low=0x80; //128 KB
unsigned char gb_ram_truco_high=0x00;

//Fuerzo RAM
void SetRAMTruco()
{
 switch (gb_max_ram)
 {
  case 131072: gb_ram_truco_low= 0x80; gb_ram_truco_high= 0x00; break; //128 KB
  case 163840: gb_ram_truco_low= 0xA0; gb_ram_truco_high= 0x00; break; //160 KB
  case 196608: gb_ram_truco_low= 0xC0; gb_ram_truco_high= 0x00; break; //192 KB
  case 229376: gb_ram_truco_low= 0xE0; gb_ram_truco_high= 0x00; break; //224 KB
  case 262144: gb_ram_truco_low= 0x00; gb_ram_truco_high= 0x01; break; //256 KB
  case 655360: gb_ram_truco_low= 0x80; gb_ram_truco_high= 0x02; break; //640 KB
 } 
}

//Lo saco fuera de Read86. Se ejecuta al inicio y en timer 54 ms.
void bootstrapPoll()
{ 
 if (!didbootstrap)
 {
  #ifdef use_lib_sna_rare	 
   jj_write86_remap(0x410,0x41);
   jj_write86_remap(0x410,hdcount);

   jj_write86_remap(0x413,gb_ram_truco_low);
   jj_write86_remap(0x414,gb_ram_truco_high);
  #else 
   gb_ram_00[0x410]= 0x41;
   gb_ram_00[0x475]= 0;

   //gb_ram_00[0x413]= 0x80; gb_ram_00[0x414]= 0x00; //128 KB
   gb_ram_bank[0][0x413]= gb_ram_truco_low;
   gb_ram_bank[0][0x414]= gb_ram_truco_high;    
  #endif
//  if (gb_use_remap_cartdridge==1)
//  {
//   jj_write86_remap(0x410,0x41);
//   jj_write86_remap(0x410,hdcount);
//
//   jj_write86_remap(0x413,gb_ram_truco_low);
//   jj_write86_remap(0x414,gb_ram_truco_high);   	  
//  }
//  else
//  {
//   gb_ram_00[0x410]= 0x41;
//   gb_ram_00[0x475]= 0;
//
//   //gb_ram_00[0x413]= 0x80; gb_ram_00[0x414]= 0x00; //128 KB
//   gb_ram_bank[0][0x413]= gb_ram_truco_low;
//   gb_ram_bank[0][0x414]= gb_ram_truco_high;
//  }
 }
}

#ifdef use_lib_sna_rare
unsigned char jj_read86_remap(unsigned int addr32)
{
 unsigned int idRAM;
 unsigned int auxOffs;
 unsigned int addrDest;
 //idRAM= (addr32/1024); 
 idRAM= (addr32>>10); 
 if (idRAM>=140)
 {
  return 0;
 }
 if (gb_use_snarare_madmix == 1)
 {
  if (gb_madmix_memory_rare_1KB[idRAM] == 0)
  {
   return gb_madmix_flash_rare_sna_rare[addr32];                           
  }
  else
  {
   auxOffs = (addr32 & 1023);      
   addrDest = (unsigned int)((gb_madmix_memory_rare_remap_1KB[idRAM]<<10));   
   addrDest = addrDest+ auxOffs;   
   //if (addrDest>64000)
   if (addrDest>32000)
   {
    return 0;
   }
   //return gb_use_minimal_ram[addrDest];
   return gb_ram_bank[0][addrDest];
  }
 }
}
#endif

#ifdef use_lib_sna_rare
//********************************************************
void jj_write86_remap(unsigned int addr32, unsigned char value)
{
 unsigned int idRAM;
 unsigned int auxOffs;
 unsigned int addrDest;
 //idRAM= (addr32/1024);
 idRAM= (addr32>>10);
 if (idRAM>=140)
 {
  return;
 }
 if (gb_use_snarare_madmix == 1)
 {
  if (gb_madmix_memory_rare_1KB[idRAM]==0){
   return;
  }
  else
  {
   auxOffs = (addr32 & 1023);      
   addrDest = (unsigned int)((gb_madmix_memory_rare_remap_1KB[idRAM]<<10));
   addrDest = addrDest + auxOffs;   
   //if (addrDest>64000)
   if (addrDest>32000)
    return;
   //gb_use_minimal_ram[addrDest]= value; 
   gb_ram_bank[0][addrDest]= value;
  }
 }
}
#endif

//********************************************************
void write86 (unsigned int addr32, unsigned char value)
{
 unsigned char idRAM;
 unsigned short int auxOffs;

 //Primero video
 //Primero CGA
 //if ((addr32 >= 0xB8000) && (addr32 < (0xB8000+16384)))
 if ((addr32 >= 0xB8000) && (addr32 < 0xBC000))
 {  
  gb_video_cga[(addr32-0xB8000)]= value;
  //updatedscreen = 1;
  return;
 }

 //Segundo memoria
 if ((addr32>=0) && (addr32<gb_max_ram))
 {
  //JJ RAM[addr32] = value;
  //JJ return;
  #ifdef use_lib_sna_rare  
   if (gb_use_remap_cartdridge==1)
   {
    jj_write86_remap(addr32,value);
    return;
   }
  #endif

  idRAM= (addr32>>15);
  auxOffs = (addr32 & 32767);
  gb_ram_bank[idRAM][auxOffs]= value;
  //switch (idRAM)
  //{
  // case 0: gb_ram_00[auxOffs]= value; break;
  // case 1: gb_ram_01[auxOffs]= value; break;
  // case 2: gb_ram_02[auxOffs]= value; break;
  // case 3: gb_ram_03[auxOffs]= value; break;
  // default: return;   
  //}     
 }
 
 //if (!didbootstrap)
 //{
 // gb_check_memory_before= value;  
 //}
 //printf("write86 over 0x%08X\n",addr32);
 //fflush(stdout);
 

 //if ( (addr32 >= 0xA0000) && (addr32 <= 0xBFFFF) )
 //{
 // //RAM[addr32] = value;
 // //updatedscreen = 1;  
 //}
  


 //BIOS ADDR NOT WRITE
 //if ((addr32 >= 0xFE000) && (addr32 < (0xFE000 + gb_size_rom_bios_pcxt)))
 if ((addr32 >= 0xFE000) && (addr32 < 0x100000))
 {
  return;
 }
 //if ((addr32 >= 0xF6000) && (addr32 < (0xF6000 + gb_size_rom_basic)))
 if ((addr32 >= 0xF6000) && (addr32 < 0xFE000))
 {
  return;
 }
 //Video ROM y demas
 if (addr32 >= 0xC0000)
 {
  return;
 } 

 //Ultimo Hercules
 //if ((addr32 >= 0xB0000) && (addr32 < (0xB0000+16384)))
 if ((addr32 >= 0xB0000) && (addr32 < 0xB4000))
 {//FIX Error 04 en el Post Boot
  //gb_video_hercules[auxOffs]= value;
  gb_video_cga[(addr32-0xB0000)]= value; //Mezclo CGA y hercules para ahorrar memoria
  //updatedscreen = 1;
  return;
 }

 if (addr32 > 1048575)
 {
  //printf("WRITE86 FIX EXPAND 0x%08X\n",addr32);
  //fflush(stdout);
  addr32 = addr32 & 0xFFFFF; //FIX EXPAND MICROSOFT ERROR MADMIX GAME

  #ifdef use_lib_sna_rare
   if (gb_use_remap_cartdridge==1)
   {
    jj_write86_remap(addr32,value);
    return;
   }
  #endif

  idRAM= (addr32>>15);
  auxOffs = (addr32 & 32767);
  gb_ram_bank[idRAM][auxOffs]= value;  
 }

}

/*
//write86 original
void write86 (uint32_t addr32, uint8_t value)
{
 tempaddr32 = addr32 & 0xFFFFF;

 //BIOS ADDR NOT WRITE
 if ((addr32 >= 0xFE000) && (addr32 < (0xFE000 + gb_size_rom_bios_pcxt)))
 {
  return;
 }
 if ((addr32 >= 0xF6000) && (addr32 < (0xF6000 + gb_size_rom_basic)))
 {
  return;
 }
 //Video ROM y demas
 if (tempaddr32 >= 0xC0000)
 {
  return;
 } 

    #ifdef use_lib_limit_256KB
//     if ((addr32 >= 128000) && (addr32 <= 260000))
//     {
//      printf("W %d %d\n",addr32,value);
//      fflush(stdout);
//      gb_check_memory_before = value;
//      return;
//     }
    #endif

	if ( (tempaddr32 >= 0xA0000) && (tempaddr32 <= 0xBFFFF) ) {
			if ( (vidmode != 0x13) && (vidmode != 0x12) && (vidmode != 0xD) && (vidmode != 0x10) ) 
            {
             #ifdef use_lib_snapshot
              //gb_memory_write[tempaddr32]= 1;
             #endif
			 RAM[tempaddr32] = value;
			 updatedscreen = 1;
			}
			else
			{             
              if ( ( (VGA_SC[4] & 6) == 0) && (vidmode != 0xD) && (vidmode != 0x10) && (vidmode != 0x12) )
              {
               #ifdef use_lib_snapshot
                //gb_memory_write[tempaddr32]= 1;
               #endif                 
			   RAM[tempaddr32] = value;
			   updatedscreen = 1;
			  }
			  else
              {
			   writeVGA (tempaddr32 - 0xA0000, value);
			  }
            }

			updatedscreen = 1;
		}
	else
    {
     #ifdef use_lib_snapshot
      //gb_memory_write[tempaddr32]= 1;
     #endif                         
	 RAM[tempaddr32] = value;
	}
}
*/

#ifdef use_lib_fast_op_writew86
 static inline void writew86 (unsigned int addr32, unsigned short int value)
 {
  write86 (addr32, (unsigned char) value);
  write86 (addr32 + 1, (unsigned char) (value >> 8) );
 } 
#else
 void writew86 (unsigned int addr32, unsigned short int value)
 {
  write86 (addr32, (unsigned char) value);
  write86 (addr32 + 1, (unsigned char) (value >> 8) );
 }
#endif 


unsigned char read86 (unsigned int addr32) 
{
 //BIOS ADDR
 unsigned short int auxOffs;
 unsigned char idRAM;

 //Primero video
 //Primero CGA
 //if ((addr32 >= 0xB8000) && (addr32 < (0xB8000+16384)))
 if ((addr32 >= 0xB8000) && (addr32 < 0xBC000))
 {   
  return (gb_video_cga[(addr32-0xB8000)]);
 } 

 //Segundo memoria RAM
 if ((addr32>=0) && (addr32<gb_max_ram))
 {
  #ifdef use_lib_sna_rare
   if (gb_use_remap_cartdridge==1)
   {
    return (jj_read86_remap(addr32));
   }  
  #endif 

  idRAM= (addr32>>15);
  auxOffs = (addr32 & 32767);
  return (gb_ram_bank[idRAM][auxOffs]);
  //switch (idRAM)
  //{
  // case 0: return gb_ram_00[auxOffs]; break;
  // case 1: return gb_ram_01[auxOffs]; break;
  // case 2: return gb_ram_02[auxOffs]; break;
  // case 3: return gb_ram_03[auxOffs]; break;
  // default: return 0;   
  //}  
  ////JJ return (RAM[addr32]);
 }
 //else
 //{
  //if ((addr32 == 0xC9FEA) || (addr32 == 0x000C9FEB))
  //{
  // printf("addr 0x%08X\n",addr32);
  // fflush(stdout);
  // return 1;
  //}
 //}

  ////System Identification
  //if (addr32==0xFFFFE)
  //{
  ////Serial.printf("System Id 0x%08X\n",addr32);
  ////fflush(stdout);  
  //return 0xFE;
  //}
 
  //if(addr32==0xFFFFF)
  //{
  // //printf("System Id 0x%08X\n",addr32);
  // //fflush(stdout);                      
  // return 0xE9;
  //}
 
 
 
 //if ((addr32 >= 0xFE000) && (addr32 < (0xFE000 + gb_size_rom_bios_pcxt)))
 if ((addr32 >= 0xFE000) && (addr32 < 0x100000))
 {
  return gb_bios_pcxt[(addr32-0xFE000)];
 }
 //if ((addr32 >= 0xF6000) && (addr32 < (0xF6000 + gb_size_rom_basic)))
 if ((addr32 >= 0xF6000) && (addr32 < 0xFE000))
 {
  return gb_rom_basic[(addr32-0xF6000)];
 }
 //if ((addr32 >= 0xC0000) && (addr32 < (0xC0000 + gb_size_rom_videorom)))
 if ((addr32 >= 0xC0000) && (addr32 < 0xC8000))
 {
  return gb_rom_videorom[(addr32-0xC0000)];
 }


// if (!didbootstrap)
// {
//  //JJ RAM[0x410] = 0x41; //ugly hack to make BIOS always believe we have an EGA/VGA card installed
//  //JJ RAM[0x475] = hdcount; //the BIOS doesn't have any concept of hard drives, so here's another hack
//  gb_ram_00[0x410]= 0x41;
//  gb_ram_00[0x475]= 0x41;
//			
//  //RAM[0x413]= 0xFF;//Hack MEMSIZE RAM 0xFF 256 KB RAM
//  //RAM[0x413]= 0xA0;//Hack 95 KB 0x5F    0xA0 160 KB
//  //RAM[0x414]= 0x00;
//  
//  //JJ RAM[0x413]= 0x5F;//Hack 95 KB 0x5F    0xA0 160 KB
//  //JJ RAM[0x414]= 0x00;
//  //gb_ram_00[0x413]= 0x5F;  gb_ram_00[0x414]= 0x00;
//  //gb_ram_00[0x413]= 0x60; gb_ram_00[0x414]= 0x00; //96 KB
//  gb_ram_00[0x413]= 0x80; gb_ram_00[0x414]= 0x00; //128 KB
// }
 
 ////extended BIOS data area (EBDA) No se usa
 //if ((addr32>=0x9FC00) && (addr32<=0x9FFFF))
 //{
 // //printf("extended BIOS 0x%08X\n",addr32);
 // //fflush(stdout);  
 // return 0;
 //}
  
 //Motherboard bios
 //if ((addr32>=0xF0000) && (addr32<=0xFFFFF))
 //{
 // //printf("motherboard 0x%08X\n",addr32);
 // //fflush(stdout);  
 // return 0;                       
 //}
  
 //reserved memory no se usa
 //if ((addr32>=0xFFFF0) && (addr32<=0xFFFFF))
 //{
 // //printf("reserved 0x%08X\n",addr32);
 // //fflush(stdout);  
 // return gb_reserved_memory[(addr32-0xFFFF0)];
 //}


 //Ultimo Hercules B0000
 //if ((addr32 >= 0xB0000) && (addr32 < (0xB0000+16384)))
 if ((addr32 >= 0xB0000) && (addr32 < 0xB4000))
 {//FIX Error 04 en el Post Boot
  //auxOffs= addr32-0xB0000;   
  //return (gb_video_hercules[auxOffs]);
  return (gb_video_cga[(addr32-0xB0000)]); //Mezclo cga y hercules para ahorrar memoria
 }

 if (addr32 > 1048575)
 {
  //printf("READ86 FIX EXPAND 0x%08X\n",addr32);
  //fflush(stdout);
  addr32 = addr32 & 0xFFFFF; //FIX EXPAND MICROSOFT ERROR MADMIX GAME  

  #ifdef use_lib_sna_rare
   if (gb_use_remap_cartdridge==1)
   {
    return (jj_read86_remap(addr32));
   }  
  #endif 

  idRAM= (addr32>>15);
  auxOffs = (addr32 & 32767);  
  return (gb_ram_bank[idRAM][auxOffs]);  
 }

 //if (!didbootstrap)
 //{
 // return (gb_check_memory_before);
 //} 
 //printf("read86 over 0x%08X\n",addr32);
 //fflush(stdout);                
 return 0; 
}

/*
//read86 original
uint8_t read86 (uint32_t addr32) 
{
	addr32 &= 0xFFFFF;

 //BIOS ADDR
 if ((addr32 >= 0xFE000) && (addr32 < (0xFE000 + gb_size_rom_bios_pcxt)))
 {
  return gb_bios_pcxt[(addr32-0xFE000)];
 }
 if ((addr32 >= 0xF6000) && (addr32 < (0xF6000 + gb_size_rom_basic)))
 {
  return gb_rom_basic[(addr32-0xF6000)];
 }
 if ((addr32 >= 0xC0000) && (addr32 < (0xC0000 + gb_size_rom_videorom)))
 {
  return gb_rom_videorom[(addr32-0xC0000)];
 }

    #ifdef use_lib_limit_256KB
//     if ((addr32 >= 128000) && (addr32 <= 260000))
//     {
//      printf("R %d %d\n",addr32,RAM[addr32]);
//      fflush(stdout);                 
//      return gb_check_memory_before;
//     }
    #endif	
		
	
	if ( (addr32 >= 0xA0000) && (addr32 <= 0xBFFFF) ) {
			if ( (vidmode == 0xD) || (vidmode == 0xE) || (vidmode == 0x10) ) return (readVGA (addr32 - 0xA0000) );
			if ( (vidmode != 0x13) && (vidmode != 0x12) && (vidmode != 0xD) ) return (RAM[addr32]);			
			if ( (VGA_SC[4] & 6) == 0)
			 	return (RAM[addr32]);
			 else
			 	return (readVGA (addr32 - 0xA0000) );
		}

	if (!didbootstrap)
    {
	 RAM[0x410] = 0x41; //ugly hack to make BIOS always believe we have an EGA/VGA card installed
	 RAM[0x475] = hdcount; //the BIOS doesn't have any concept of hard drives, so here's another hack
			
   	 //RAM[0x413]= 0xFF;//Hack MEMSIZE RAM 0xFF 256 KB RAM
   	 RAM[0x413]= 0xA0;//Hack 95 KB 0x5F    0xA0 160 KB
	 RAM[0x414]= 0x00;
	}

	return (RAM[addr32]);
}
*/


#ifdef use_lib_fast_readw86
 static inline unsigned short int readw86 (unsigned int addr32)
#else
 unsigned short int readw86 (unsigned int addr32)
#endif 
 {
  return ( (unsigned short int) read86 (addr32) | (unsigned short int) (read86 (addr32 + 1) << 8) );
 }



#ifdef use_lib_fast_flag_szp8
 static inline void flag_szp8(unsigned char value)
#else
 void flag_szp8(unsigned char value)
#endif 
 {
  zf= (!value)?1:0; //set or clear zero flag  
  sf= (value & 0x80)?1:0; //set or clear sign flag
  pf = parity[value]; //retrieve parity state from lookup table
 }


#ifdef use_lib_fast_flag_szp16
 static inline void flag_szp16(unsigned short int value)
#else
 void flag_szp16(unsigned short int value)
#endif 
 {	 
  zf= (!value)?1:0; //set or clear zero flag
  sf= (value & 0x8000)?1:0; //set or clear sign flag
  pf = parity[value & 255];	//retrieve parity state from lookup table
 }

#ifdef use_lib_fast_flag_log8
 static inline void flag_log8(unsigned char value)
#else
 void flag_log8(unsigned char value)
#endif 
 {
  flag_szp8(value);
  cf=0;
  of=0; //bitwise logic ops always clear carry and overflow
 }


#ifdef use_lib_fast_flag_log16
 static inline void flag_log16(unsigned short int value)
#else
 void flag_log16(unsigned short int value)
#endif 
 {
  flag_szp16(value);
  cf=0;
  of=0; //bitwise logic ops always clear carry and overflow
 }

#ifdef use_lib_fast_flag_adc8
 static inline void flag_adc8 (unsigned char v1, unsigned char v2, unsigned char v3)
#else
 void flag_adc8 (unsigned char v1, unsigned char v2, unsigned char v3)
#endif 
{//v1 = destination operand, v2 = source operand, v3 = carry flag 
 unsigned short int dst;
 dst = (unsigned short int) v1 + (unsigned short int) v2 + (unsigned short int) v3;
 flag_szp8 ( (unsigned char) dst);
 of= ( ( (dst ^ v1) & (dst ^ v2) & 0x80) == 0x80)?1:0; //set or clear overflow flag
 cf= (dst & 0xFF00)?1:0; // set or clear carry flag 
 af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0; // set or clear auxilliary flag
}

#ifdef use_lib_fast_flag_adc16
 static inline void flag_adc16 (unsigned short int v1, unsigned short int v2, unsigned short int v3)
#else
 void flag_adc16 (unsigned short int v1, unsigned short int v2, unsigned short int v3)
#endif 
 {
  unsigned int dst;
  dst = (unsigned int) v1 + (unsigned int) v2 + (unsigned int) v3;
  flag_szp16 ( (unsigned short int) dst);
  of= ( ( ( (dst ^ v1) & (dst ^ v2) ) & 0x8000) == 0x8000)?1:0;
  cf= (dst & 0xFFFF0000)?1:0;
  af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0;
 }


#ifdef use_lib_fast_flag_add8
 static inline void flag_add8 (unsigned char v1, unsigned char v2)
#else
 void flag_add8 (unsigned char v1, unsigned char v2)
#endif 
 {
  //v1 = destination operand, v2 = source operand
  unsigned short int dst;
  dst = (unsigned short int) v1 + (unsigned short int) v2;
  flag_szp8 ( (unsigned char) dst);
  cf= (dst & 0xFF00)?1:0;
  of= ( ( (dst ^ v1) & (dst ^ v2) & 0x80) == 0x80)?1:0;
  af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0;
 }


#ifdef use_lib_fast_flag_add16
 static inline void flag_add16 (uint16_t v1, uint16_t v2)
#else
 void flag_add16 (uint16_t v1, uint16_t v2)
#endif 
 {
  //v1 = destination operand, v2 = source operand
  unsigned int dst;
  dst = (unsigned int) v1 + (unsigned int) v2;
  flag_szp16 ( (unsigned short int) dst);  
  cf= (dst & 0xFFFF0000)?1:0;
  of= ( ( (dst ^ v1) & (dst ^ v2) & 0x8000) == 0x8000)?1:0;
  af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0;
 }


#ifdef use_lib_fast_flag_sbb8
 static inline void flag_sbb8 (unsigned char v1, unsigned char v2, unsigned char v3)
#else
 void flag_sbb8 (unsigned char v1, unsigned char v2, unsigned char v3)
#endif 
 {
  //v1 = destination operand, v2 = source operand, v3 = carry flag
  unsigned short int dst;
  v2 += v3;
  dst = (unsigned short int) v1 - (unsigned short int) v2;
  flag_szp8 ( (unsigned short int) dst);
  cf= (dst & 0xFF00)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x80)?1:0;
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }


#ifdef use_lib_fast_flag_sbb16 
 static inline void flag_sbb16 (unsigned short int v1, unsigned short int v2, unsigned short int v3)
#else 
 void flag_sbb16 (unsigned short int v1, unsigned short int v2, unsigned short int v3)
#endif 
 {
  //v1 = destination operand, v2 = source operand, v3 = carry flag
  unsigned int dst;
  v2 += v3;
  dst = (unsigned int) v1 - (unsigned int) v2;
  flag_szp16 ( (uint16_t) dst);
  cf= (dst & 0xFFFF0000)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x8000)?1:0;
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }
 

#ifdef use_lib_fast_flag_sub8
 static inline void flag_sub8 (unsigned char v1, unsigned char v2)
#else
 void flag_sub8 (unsigned char v1, unsigned char v2)
#endif 
 {//v1 = destination operand, v2 = source operand
  unsigned short int dst;
  dst = (unsigned short int) v1 - (unsigned short int) v2;
  flag_szp8 ( (unsigned char) dst);
  cf= (dst & 0xFF00)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x80)?1:0; 
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }

#ifdef use_lib_fast_flag_sub16
 static inline void flag_sub16 (unsigned short int v1, unsigned short int v2)
#else
 void flag_sub16 (unsigned short int v1, unsigned short int v2)
#endif 
 {//v1 = destination operand, v2 = source operand
  unsigned int dst;
  dst = (unsigned int) v1 - (unsigned int) v2;
  flag_szp16 ( (unsigned short int) dst);
  cf= (dst & 0xFFFF0000)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x8000)?1:0;
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }

#ifdef use_lib_fast_op_adc8
 static inline void op_adc8()
#else
 void op_adc8()
#endif 
 {
  res8 = oper1b + oper2b + cf;
  flag_adc8 (oper1b, oper2b, cf);
 }

#ifdef use_lib_fast_op_adc16
 static inline void op_adc16()
#else
 void op_adc16()
#endif 
 {
  res16 = oper1 + oper2 + cf;
  flag_adc16 (oper1, oper2, cf);
 }

#ifdef use_lib_fast_op_add8
 static inline void op_add8()
#else
 void op_add8()
#endif 
 {
  res8 = oper1b + oper2b;
  flag_add8 (oper1b, oper2b);
 }

#ifdef use_lib_fast_op_add16
 static inline void op_add16()
#else
 void op_add16()
#endif  
 {
  res16 = oper1 + oper2;
  flag_add16 (oper1, oper2);
 }


#ifdef use_lib_fast_op_and8
 static inline void op_and8()
#else
 void op_and8()
#endif
 {
  res8 = oper1b & oper2b;
  flag_log8 (res8);
 }


#ifdef use_lib_fast_op_and16
 static inline void op_and16()
#else
 void op_and16()
#endif 
 {
  res16 = oper1 & oper2;
  flag_log16 (res16);
 }



#ifdef use_lib_fast_op_or8
 static inline void op_or8()
#else
 void op_or8()
#endif 
 {
  res8 = oper1b | oper2b;
  flag_log8 (res8);
 } 


#ifdef use_lib_fast_op_or16
 static inline void op_or16()
#else
 void op_or16()
#endif 
 {
  res16 = oper1 | oper2;
  flag_log16 (res16);
 }


#ifdef use_lib_fast_op_xor8
 static inline void op_xor8()
#else
 void op_xor8()
#endif 
 {
  res8 = oper1b ^ oper2b;
  flag_log8 (res8);
 }


#ifdef use_lib_fast_op_xor16
 static inline void op_xor16()
#else
 void op_xor16()
#endif  
 {
  res16 = oper1 ^ oper2;
  flag_log16((unsigned short int)res16);
 }

#ifdef use_lib_fast_op_sub8
 static inline void op_sub8()
#else
 void op_sub8()
#endif
 {
  res8 = oper1b - oper2b;
  flag_sub8 (oper1b, oper2b);
 }

#ifdef use_lib_fast_op_sub16
 static inline void op_sub16()
#else
 void op_sub16()
#endif  
 {
  res16 = oper1 - oper2;
  flag_sub16 (oper1, oper2);
 }


#ifdef use_lib_fast_op_sbb8
 static inline void op_sbb8()
#else
 void op_sbb8()
#endif 
 {
  res8 = oper1b - (oper2b + cf);
  flag_sbb8 (oper1b, oper2b, cf);
 }

#ifdef use_lib_fast_op_sbb16
 static inline void op_sbb16()
#else
 void op_sbb16()
#endif 
 {
  res16 = oper1 - (oper2 + cf);
  flag_sbb16 (oper1, oper2, cf);
 }

#ifdef use_lib_fast_modregrm
 void modregrm()
 {//Es mas rapido la funcion que la macro original
  addrbyte = getmem8(segregs[regcs], ip);
  StepIP(1);
  mode = addrbyte >> 6;
  reg = (addrbyte >> 3) & 7;
  rm = addrbyte & 7;
  switch(mode)
  {
   case 0:
	if(rm == 6)
	{
	 disp16 = getmem16(segregs[regcs], ip);
	 StepIP(2);
	}
	if(((rm == 2) || (rm == 3)) && !segoverride)
	{
	 useseg = segregs[regss];
	}
	break; 
   case 1:
	disp16 = signext(getmem8(segregs[regcs], ip));
	StepIP(1);
	if(((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride)
	{
	 useseg = segregs[regss];
	}
	break; 
   case 2:
	disp16 = getmem16(segregs[regcs], ip);
	StepIP(2);
	if(((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride)
	{
	 useseg = segregs[regss];
	}
	break;
   default:
	disp8 = 0;
	disp16 = 0;
  }
 }
#else
 #define modregrm() { \
	addrbyte = getmem8(segregs[regcs], ip); \
	StepIP(1); \
	mode = addrbyte >> 6; \
	reg = (addrbyte >> 3) & 7; \
	rm = addrbyte & 7; \
	switch(mode) \
	{ \
	case 0: \
	if(rm == 6) { \
	disp16 = getmem16(segregs[regcs], ip); \
	StepIP(2); \
	} \
	if(((rm == 2) || (rm == 3)) && !segoverride) { \
	useseg = segregs[regss]; \
	} \
	break; \
 \
	case 1: \
	disp16 = signext(getmem8(segregs[regcs], ip)); \
	StepIP(1); \
	if(((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride) { \
	useseg = segregs[regss]; \
	} \
	break; \
 \
	case 2: \
	disp16 = getmem16(segregs[regcs], ip); \
	StepIP(2); \
	if(((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride) { \
	useseg = segregs[regss]; \
	} \
	break; \
 \
	default: \
	disp8 = 0; \
	disp16 = 0; \
	} \
	}
#endif	

void getea (uint8_t rmval)
{
	uint32_t	tempea;

	tempea = 0;
	switch (mode) {
			case 0:
				switch (rmval) {
						case 0:
							tempea = regs.wordregs[regbx] + regs.wordregs[regsi];
							break;
						case 1:
							tempea = regs.wordregs[regbx] + regs.wordregs[regdi];
							break;
						case 2:
							tempea = regs.wordregs[regbp] + regs.wordregs[regsi];
							break;
						case 3:
							tempea = regs.wordregs[regbp] + regs.wordregs[regdi];
							break;
						case 4:
							tempea = regs.wordregs[regsi];
							break;
						case 5:
							tempea = regs.wordregs[regdi];
							break;
						case 6:
							tempea = disp16;
							break;
						case 7:
							tempea = regs.wordregs[regbx];
							break;
					}
				break;

			case 1:
			case 2:
				switch (rmval) {
						case 0:
							tempea = regs.wordregs[regbx] + regs.wordregs[regsi] + disp16;
							break;
						case 1:
							tempea = regs.wordregs[regbx] + regs.wordregs[regdi] + disp16;
							break;
						case 2:
							tempea = regs.wordregs[regbp] + regs.wordregs[regsi] + disp16;
							break;
						case 3:
							tempea = regs.wordregs[regbp] + regs.wordregs[regdi] + disp16;
							break;
						case 4:
							tempea = regs.wordregs[regsi] + disp16;
							break;
						case 5:
							tempea = regs.wordregs[regdi] + disp16;
							break;
						case 6:
							tempea = regs.wordregs[regbp] + disp16;
							break;
						case 7:
							tempea = regs.wordregs[regbx] + disp16;
							break;
					}
				break;
		}

	ea = (tempea & 0xFFFF) + (useseg << 4);
}

#ifdef use_lib_fast_push 
 #define push(x) \ 
  putreg16(regsp,getreg16(regsp)-2); \
  putmem16(segregs[regss],getreg16(regsp),x); \
   
#else
 void push (unsigned short int pushval)
 {
  putreg16 (regsp, getreg16 (regsp) - 2);
  putmem16 (segregs[regss], getreg16 (regsp), pushval);
 }
#endif

#ifdef use_lib_fast_pop
 //En gcc se puede usar una macro statement expressions
 static inline unsigned short int pop()
 {
  unsigned short int tempval;
  tempval = getmem16 (segregs[regss], getreg16 (regsp) );
  putreg16 (regsp, getreg16 (regsp) + 2);
  return tempval;
 }  
#else
 unsigned short int pop()
 {
  unsigned short int tempval;
  tempval = getmem16 (segregs[regss], getreg16 (regsp) );
  putreg16 (regsp, getreg16 (regsp) + 2);
  return tempval;
 }
#endif 


//void SetCSLoadCom()
//{
// ///Memoria 0x1F400 128000  
// //segregs[regcs] = 0x1040;
// segregs[regcs] = 0x07C0;
// ip=0x100;//0x100;
// //ip = 0x0C00;
// segregs[regds] = 0x07C0;
// segregs[reges] = 0x07C0;
// segregs[regss] = 0x07C0;
// //ip = 0x0100;
// //segregs[regcs] = 0x7000;
// //ip = 0x0100;
//}

void reset86() {
	segregs[regcs] = 0xFFFF;
	ip = 0x0000;
	//regs.wordregs[regsp] = 0xFFFE;
}

#ifdef use_lib_fast_readrm16
 static inline unsigned short int readrm16 (unsigned char rmval)
#else
 unsigned short int readrm16 (unsigned char rmval)
#endif 
 {
  if (mode < 3)
  {
   getea (rmval);
   return read86 (ea) | ( (unsigned short int) read86 (ea + 1) << 8);
  }
  else
  {
   return getreg16 (rmval);
  }
}

#ifdef use_lib_fast_readrm8
 static inline unsigned char readrm8 (unsigned char rmval) 
#else
 unsigned char readrm8 (unsigned char rmval) 
#endif 
 {
  if (mode < 3)
  {
   getea (rmval);
   return read86 (ea);
  }
  else
  {
   return getreg8 (rmval);
  }
}

#ifdef use_lib_fast_writerm16
 static inline void writerm16 (unsigned char rmval, unsigned short int value)
#else
 void writerm16 (unsigned char rmval, unsigned short int value)
#endif 
 {
  if (mode < 3)
  {
   getea (rmval);
   write86 (ea, value & 0xFF);
   write86 (ea + 1, value >> 8);
  }
  else
  {
   putreg16 (rmval, value);
  }
}

#ifdef use_lib_fast_writerm8
 static inline void writerm8 (unsigned char rmval, unsigned char value)
#else
 void writerm8 (unsigned char rmval, unsigned char value) 
#endif 
 {
  if (mode < 3)
  {
   getea (rmval);
   write86 (ea, value);
  }
  else
  {
   putreg8 (rmval, value);
  }
}

uint8_t op_grp2_8 (uint8_t cnt)
{
	uint16_t	s;
	uint16_t	shift;
	uint16_t	oldcf;
	uint16_t	msb;

	s = oper1b;
	oldcf = cf;
#ifdef CPU_V20 //80186/V20 class CPUs limit shift count to 31
	cnt &= 0x1F;
#endif
	switch (reg) {
			case 0: // ROL r/m8
				for (shift = 1; shift <= cnt; shift++)
				{
				 cf= (s & 0x80)?1:0;
				 s = s << 1;
				 s = s | cf;
				}

				if (cnt == 1)
				{
				 of = cf ^ ( (s >> 7) & 1);
				}
				break;

			case 1: // ROR r/m8
				for (shift = 1; shift <= cnt; shift++) {
						cf = s & 1;
						s = (s >> 1) | (cf << 7);
					}

				if (cnt == 1) {
						of = (s >> 7) ^ ( (s >> 6) & 1);
					}
				break;

			case 2: // RCL r/m8
				for (shift = 1; shift <= cnt; shift++)
				{
				 oldcf = cf;
				 cf= (s & 0x80)?1:0;
				 s = s << 1;
				 s = s | oldcf;
				}

				if (cnt == 1)
				{
				 of = cf ^ ( (s >> 7) & 1);
				}
				break;

			case 3: // RCR r/m8
				for (shift = 1; shift <= cnt; shift++) {
						oldcf = cf;
						cf = s & 1;
						s = (s >> 1) | (oldcf << 7);
					}

				if (cnt == 1) {
						of = (s >> 7) ^ ( (s >> 6) & 1);
					}
				break;

			case 4:
			case 6: // SHL r/m8
				for (shift = 1; shift <= cnt; shift++) 
				{
                 cf= (s & 0x80)?1:0;
  				 s = (s << 1) & 0xFF;
				}
                of= ( (cnt == 1) && (cf == (s >> 7) ) )?0:1;
				flag_szp8 ( (unsigned char) s);
				break;

			case 5: // SHR r/m8
			    of= ( (cnt == 1) && (s & 0x80) )?1:0; 
				for (shift = 1; shift <= cnt; shift++) 
				{
				 cf = s & 1;
				 s = s >> 1;
				}
				flag_szp8 ( (unsigned char) s);
				break;

			case 7: // SAR r/m8
				for (shift = 1; shift <= cnt; shift++) {
						msb = s & 0x80;
						cf = s & 1;
						s = (s >> 1) | msb;
					}

				of = 0;
				flag_szp8 ( (unsigned char) s);
				break;
		}

	return s & 0xFF;
}

uint16_t op_grp2_16 (uint8_t cnt)
{
	uint32_t	s;
	uint32_t	shift;
	uint32_t	oldcf;
	uint32_t	msb;

	s = oper1;
	oldcf = cf;
#ifdef CPU_V20 //80186/V20 class CPUs limit shift count to 31
	cnt &= 0x1F;
#endif
	switch (reg) {
			case 0: //ROL r/m8
				for (shift = 1; shift <= cnt; shift++) 
				{
  				 cf= (s & 0x8000)?1:0;
 				 s = s << 1;
				 s = s | cf;
				}
				if (cnt == 1){
				 of = cf ^ ( (s >> 15) & 1);
				}
				break;

			case 1: //ROR r/m8
				for (shift = 1; shift <= cnt; shift++) {
						cf = s & 1;
						s = (s >> 1) | (cf << 15);
					}

				if (cnt == 1) {
						of = (s >> 15) ^ ( (s >> 14) & 1);
					}
				break;

			case 2: // RCL r/m8 
				for (shift = 1; shift <= cnt; shift++)
				{
				 oldcf = cf;
				 cf= (s & 0x8000)?1:0;
				 s = s << 1;
				 s = s | oldcf;
				}

				if (cnt == 1) {
				 of = cf ^ ( (s >> 15) & 1);
				}
				break;

			case 3: //RCR r/m8
				for (shift = 1; shift <= cnt; shift++)
				{
				 oldcf = cf;
				 cf = s & 1;
				 s = (s >> 1) | (oldcf << 15);
				}

				if (cnt == 1) {
				 of = (s >> 15) ^ ( (s >> 14) & 1);
				}
				break;

			case 4:
			case 6: //SHL r/m8
				for (shift = 1; shift <= cnt; shift++)
				{
				 cf= (s & 0x8000)?1:0;
				 s = (s << 1) & 0xFFFF;
				}
                of= ( (cnt == 1) && (cf == (s >> 15) ) )?0:1;
				flag_szp16 ( (unsigned short int) s);
				break;

			case 5: // SHR r/m8
			    of= ( (cnt == 1) && (s & 0x8000) )?1:0; 
				for (shift = 1; shift <= cnt; shift++)
				{
				 cf = s & 1;
				 s = s >> 1;
				}
				flag_szp16 ( (unsigned short int) s);
				break;

			case 7: // SAR r/m8
				for (shift = 1; shift <= cnt; shift++) {
						msb = s & 0x8000;
						cf = s & 1;
						s = (s >> 1) | msb;
					}

				of = 0;
				flag_szp16 ( (unsigned short int) s);
				break;
		}

	return (unsigned short int) s & 0xFFFF;
}


#ifdef use_lib_fast_op_div8
 static inline void op_div8 (unsigned short int valdiv, unsigned char divisor)
#else
 void op_div8 (unsigned short int valdiv, unsigned char divisor)
#endif 
 {
  if (divisor == 0)
  {
   intcall86 (0);
   return;
  }

  if ( (valdiv / (unsigned short int) divisor) > 0xFF)
  {
   intcall86 (0);
   return;
  }

  regs.byteregs[regah] = valdiv % (unsigned short int) divisor;
  regs.byteregs[regal] = valdiv / (unsigned short int) divisor;
}

void op_idiv8 (uint16_t valdiv, uint8_t divisor) {

	uint16_t	s1;
	uint16_t	s2;
	uint16_t	d1;
	uint16_t	d2;
	int	sign;

	if (divisor == 0) {
			intcall86 (0);
			return;
		}

	s1 = valdiv;
	s2 = divisor;
	sign = ( ( (s1 ^ s2) & 0x8000) != 0);
	s1 = (s1 < 0x8000) ? s1 : ( (~s1 + 1) & 0xffff);
	s2 = (s2 < 0x8000) ? s2 : ( (~s2 + 1) & 0xffff);
	d1 = s1 / s2;
	d2 = s1 % s2;
	if (d1 & 0xFF00) {
			intcall86 (0);
			return;
		}

	if (sign) {
			d1 = (~d1 + 1) & 0xff;
			d2 = (~d2 + 1) & 0xff;
		}

	regs.byteregs[regah] = (uint8_t) d2;
	regs.byteregs[regal] = (uint8_t) d1;
}

void op_grp3_8() {
	oper1 = signext (oper1b);
	oper2 = signext (oper2b);
	switch (reg) {
			case 0:
			case 1: //TEST
				flag_log8 (oper1b & getmem8 (segregs[regcs], ip) );
				StepIP (1);
				break;

			case 2: //NOT
				res8 = ~oper1b;
				break;

			case 3: //NEG
				res8 = (~oper1b) + 1;
				flag_sub8 (0, oper1b);
				cf= (res8 == 0)?0:1;
				break;

			case 4: /* MUL */
				temp1 = (uint32_t) oper1b * (uint32_t) regs.byteregs[regal];
				putreg16 (regax, temp1 & 0xFFFF);
				flag_szp8 ( (uint8_t) temp1);
				if (regs.byteregs[regah]) {
						cf = 1;
						of = 1;
					}
				else {
						cf = 0;
						of = 0;
					}
#ifndef CPU_V20
				zf = 0;
#endif
				break;

			case 5: /* IMUL */
				oper1 = signext (oper1b);
				temp1 = signext (regs.byteregs[regal]);
				temp2 = oper1;
				if ( (temp1 & 0x80) == 0x80) {
						temp1 = temp1 | 0xFFFFFF00;
					}

				if ( (temp2 & 0x80) == 0x80) {
						temp2 = temp2 | 0xFFFFFF00;
					}

				temp3 = (temp1 * temp2) & 0xFFFF;
				putreg16 (regax, temp3 & 0xFFFF);
				if (regs.byteregs[regah]) {
						cf = 1;
						of = 1;
					}
				else {
						cf = 0;
						of = 0;
					}
#ifndef CPU_V20
				zf = 0;
#endif
				break;

			case 6: /* DIV */
				op_div8 (getreg16 (regax), oper1b);
				break;

			case 7: /* IDIV */
				op_idiv8 (getreg16 (regax), oper1b);
				break;
		}
}

void op_div16 (uint32_t valdiv, uint16_t divisor) {
	if (divisor == 0) {
			intcall86 (0);
			return;
		}

	if ( (valdiv / (uint32_t) divisor) > 0xFFFF) {
			intcall86 (0);
			return;
		}

	putreg16 (regdx, valdiv % (uint32_t) divisor);
	putreg16 (regax, valdiv / (uint32_t) divisor);
}

void op_idiv16 (uint32_t valdiv, uint16_t divisor) {

	uint32_t	d1;
	uint32_t	d2;
	uint32_t	s1;
	uint32_t	s2;
	int	sign;

	if (divisor == 0) {
			intcall86 (0);
			return;
		}

	s1 = valdiv;
	s2 = divisor;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	sign = ( ( (s1 ^ s2) & 0x80000000) != 0);
	s1 = (s1 < 0x80000000) ? s1 : ( (~s1 + 1) & 0xffffffff);
	s2 = (s2 < 0x80000000) ? s2 : ( (~s2 + 1) & 0xffffffff);
	d1 = s1 / s2;
	d2 = s1 % s2;
	if (d1 & 0xFFFF0000) {
			intcall86 (0);
			return;
		}

	if (sign) {
			d1 = (~d1 + 1) & 0xffff;
			d2 = (~d2 + 1) & 0xffff;
		}

	putreg16 (regax, d1);
	putreg16 (regdx, d2);
}

void op_grp3_16() {
	switch (reg) {
			case 0:
			case 1: /* TEST */
				flag_log16 (oper1 & getmem16 (segregs[regcs], ip) );
				StepIP (2);
				break;

			case 2: /* NOT */
				res16 = ~oper1;
				break;

			case 3: //NEG
				res16 = (~oper1) + 1;
				flag_sub16 (0, oper1);
                cf= (res16)?1:0;
				break;

			case 4: /* MUL */
				temp1 = (uint32_t) oper1 * (uint32_t) getreg16 (regax);
				putreg16 (regax, temp1 & 0xFFFF);
				putreg16 (regdx, temp1 >> 16);
				flag_szp16 ( (uint16_t) temp1);
				if (getreg16 (regdx) ) {
						cf = 1;
						of = 1;
					}
				else {
						cf = 0;
						of = 0;
					}
#ifndef CPU_V20
				zf = 0;
#endif
				break;

			case 5: /* IMUL */
				temp1 = getreg16 (regax);
				temp2 = oper1;
				if (temp1 & 0x8000) {
						temp1 |= 0xFFFF0000;
					}

				if (temp2 & 0x8000) {
						temp2 |= 0xFFFF0000;
					}

				temp3 = temp1 * temp2;
				putreg16 (regax, temp3 & 0xFFFF);	/* into register ax */
				putreg16 (regdx, temp3 >> 16);	/* into register dx */
				if (getreg16 (regdx) ) {
						cf = 1;
						of = 1;
					}
				else {
						cf = 0;
						of = 0;
					}
#ifndef CPU_V20
				zf = 0;
#endif
				break;

			case 6: /* DIV */
				op_div16 ( ( (uint32_t) getreg16 (regdx) << 16) + getreg16 (regax), oper1);
				break;

			case 7: /* DIV */
				op_idiv16 ( ( (uint32_t) getreg16 (regdx) << 16) + getreg16 (regax), oper1);
				break;
		}
}

void op_grp5() {
	switch (reg) {
			case 0: /* INC Ev */
				oper2 = 1;
				tempcf = cf;
				op_add16();
				cf = tempcf;
				writerm16 (rm, res16);
				break;

			case 1: /* DEC Ev */
				oper2 = 1;
				tempcf = cf;
				op_sub16();
				cf = tempcf;
				writerm16 (rm, res16);
				break;

			case 2: /* CALL Ev */
				push (ip);
				ip = oper1;
				break;

			case 3: /* CALL Mp */
				push (segregs[regcs]);
				push (ip);
				getea (rm);
				ip = (uint16_t) read86 (ea) + (uint16_t) read86 (ea + 1) * 256;
				segregs[regcs] = (uint16_t) read86 (ea + 2) + (uint16_t) read86 (ea + 3) * 256;
				break;

			case 4: /* JMP Ev */
				ip = oper1;
				break;

			case 5: /* JMP Mp */
				getea (rm);
				ip = (uint16_t) read86 (ea) + (uint16_t) read86 (ea + 1) * 256;
				segregs[regcs] = (uint16_t) read86 (ea + 2) + (uint16_t) read86 (ea + 3) * 256;
				break;

			case 6: /* PUSH Ev */
				push (oper1);
				break;
		}
}

//JJ uint8_t dolog = 0, didintr = 0;
//JJ FILE	*logout;//No necesito log
//uint8_t printops = 0;

#ifdef NETWORKING_ENABLED
extern void nethandler();
#endif
extern void diskhandler();
extern void readdisk (uint8_t drivenum, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount);

void intcall86 (unsigned char intnum)
{
	static uint16_t lastint10ax;
	uint16_t oldregax;
	//JJ didintr = 1;

	if (intnum == 0x19) didbootstrap = 1;

 #ifdef use_lib_debug_interrupt
  if (gb_interrupt_before != intnum)
  {
   gb_interrupt_before= intnum;
   //printf("Int:%02X AL:%02X AH:%02X \n",intnum,GetRegAL(),GetRegAH());
   //fflush(stdout);
  }
 #endif


	switch (intnum)
    {
			case 0x10:
				//updatedscreen = 1; //no lo necesito
				if ( (regs.byteregs[regah]==0x00) || (regs.byteregs[regah]==0x10) ) {
						oldregax = regs.wordregs[regax];
						vidinterrupt();
						regs.wordregs[regax] = oldregax;
						if (regs.byteregs[regah]==0x10) return;
						if (vidmode==9) return;
					}
				if ( (regs.byteregs[regah]==0x1A) && (lastint10ax!=0x0100) ) { //the 0x0100 is a cheap hack to make it not do this if DOS EDIT/QBASIC
						regs.byteregs[regal] = 0x1A;
						regs.byteregs[regbl] = 0x8;
						return;
					}
				lastint10ax = regs.wordregs[regax];
				break;

#ifndef DISK_CONTROLLER_ATA
			case 0x19: //bootstrap
				if (bootdrive<255) { //read first sector of boot drive into 07C0:0000 and execute it
						regs.byteregs[regdl] = bootdrive;
						readdisk (regs.byteregs[regdl], 0x07C0, 0x0000, 0, 1, 0, 1);
						segregs[regcs] = 0x0000;
						ip = 0x7C00;
					}
				else {
						segregs[regcs] = 0xF600;	//start ROM BASIC at bootstrap if requested
						ip = 0x0000;
					}
				return;

			case 0x13:
			case 0xFD:
				diskhandler();
				return;
#endif
#ifdef NETWORKING_OLDCARD
			case 0xFC:
#ifdef NETWORKING_ENABLED
				nethandler();
#endif
				return;
#endif
		}

	push (makeflagsword() );
	push (segregs[regcs]);
	push (ip);
	segregs[regcs] = getmem16 (0, (uint16_t) intnum * 4 + 2);
	ip = getmem16 (0, (uint16_t) intnum * 4);
	ifl = 0;
	tf = 0;
}

//JJ #if defined(NETWORKING_ENABLED)
//JJ extern struct netstruct {
//JJ 	uint8_t	enabled;
//JJ 	uint8_t	canrecv;
//JJ 	uint16_t	pktlen;
//JJ } net;
//JJ #endif
uint64_t	frametimer = 0, didwhen = 0, didticks = 0;
uint32_t	makeupticks = 0;
extern float	timercomp;
uint64_t	timerticks = 0, realticks = 0;
uint64_t	lastcountertimer = 0, counterticks = 10000;
extern uint8_t	nextintr();
extern void	timing();

void exec86 (uint32_t execloops) {

	uint32_t	loopcount;
	uint8_t	docontinue;
	static uint16_t firstip;
	static uint16_t trap_toggle = 0;

	counterticks = (uint64_t) ( (double) timerfreq / (double) 65536.0);

	for (loopcount = 0; loopcount < execloops; loopcount++)
	{

     #ifdef use_lib_speaker_cpu
      my_callback_speaker_func();
	 #endif 

			if ( (totalexec & 31) == 0) timing();

            //if ( (totalexec & 0x07) == 0)
			//{
            // delayMicroseconds(gb_delay_tick_cpu_micros);
            //}

			if (trap_toggle) {
					intcall86 (1);
				}
            trap_toggle=  (tf)?1:0;
			if (!trap_toggle && (ifl && (i8259.irr & (~i8259.imr) ) ) ) {
					intcall86 (nextintr() );	/* get next interrupt from the i8259, if any */
				}

			reptype = 0;
			segoverride = 0;
			useseg = segregs[regds];
			docontinue = 0;
			firstip = ip;

			if ( (segregs[regcs] == 0xF000) && (ip == 0xE066) ) didbootstrap = 0; //detect if we hit the BIOS entry point to clear didbootstrap because we've rebooted

			while (!docontinue) {
					segregs[regcs] = segregs[regcs] & 0xFFFF;
					ip = ip & 0xFFFF;
					savecs = segregs[regcs];
					saveip = ip;
					opcode = getmem8 (segregs[regcs], ip);
					StepIP (1);

					switch (opcode) {
								/* segment prefix check */
							case 0x2E:	/* segment segregs[regcs] */
								useseg = segregs[regcs];
								segoverride = 1;
								break;

							case 0x3E:	/* segment segregs[regds] */
								useseg = segregs[regds];
								segoverride = 1;
								break;

							case 0x26:	/* segment segregs[reges] */
								useseg = segregs[reges];
								segoverride = 1;
								break;

							case 0x36:	/* segment segregs[regss] */
								useseg = segregs[regss];
								segoverride = 1;
								break;

								/* repetition prefix check */
							case 0xF3:	/* REP/REPE/REPZ */
								reptype = 1;
								break;

							case 0xF2:	/* REPNE/REPNZ */
								reptype = 2;
								break;

							default:
								docontinue = 1;
								break;
						}
				}

			totalexec++;

			/*
			 * if (printops == 1) { printf("%04X:%04X - %s\n", savecs, saveip, oplist[opcode]);
			 * }
			 */
			switch (opcode) {
					case 0x0:	/* 00 ADD Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_add8();
						writerm8 (rm, res8);
						break;

					case 0x1:	/* 01 ADD Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_add16();
						writerm16 (rm, res16);
						break;

					case 0x2:	/* 02 ADD Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_add8();
						putreg8 (reg, res8);
						break;

					case 0x3:	/* 03 ADD Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_add16();
						putreg16 (reg, res16);
						break;

					case 0x4:	/* 04 ADD regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_add8();
						regs.byteregs[regal] = res8;
						break;

					case 0x5:	/* 05 ADD eAX Iv */
						oper1 = (getreg16 (regax) );
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_add16();
						putreg16 (regax, res16);
						break;

					case 0x6:	/* 06 PUSH segregs[reges] */
						push (segregs[reges]);
						break;

					case 0x7:	/* 07 POP segregs[reges] */
						segregs[reges] = pop();
						break;

					case 0x8:	/* 08 OR Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_or8();
						writerm8 (rm, res8);
						break;

					case 0x9:	/* 09 OR Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_or16();
						writerm16 (rm, res16);
						break;

					case 0xA:	/* 0A OR Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_or8();
						putreg8 (reg, res8);
						break;

					case 0xB:	/* 0B OR Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_or16();
						if ( (oper1 == 0xF802) && (oper2 == 0xF802) ) {
								sf = 0;	/* cheap hack to make Wolf 3D think we're a 286 so it plays */
							}

						putreg16 (reg, res16);
						break;

					case 0xC:	/* 0C OR regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_or8();
						regs.byteregs[regal] = res8;
						break;

					case 0xD:	/* 0D OR eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_or16();
						putreg16 (regax, res16);
						break;

					case 0xE:	/* 0E PUSH segregs[regcs] */
						push (segregs[regcs]);
						break;

					case 0xF: //0F POP CS
#ifndef CPU_V20
						segregs[regcs] = pop();
#endif
						break;

					case 0x10:	/* 10 ADC Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_adc8();
						writerm8 (rm, res8);
						break;

					case 0x11:	/* 11 ADC Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_adc16();
						writerm16 (rm, res16);
						break;

					case 0x12:	/* 12 ADC Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_adc8();
						putreg8 (reg, res8);
						break;

					case 0x13:	/* 13 ADC Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_adc16();
						putreg16 (reg, res16);
						break;

					case 0x14:	/* 14 ADC regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_adc8();
						regs.byteregs[regal] = res8;
						break;

					case 0x15:	/* 15 ADC eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_adc16();
						putreg16 (regax, res16);
						break;

					case 0x16:	/* 16 PUSH segregs[regss] */
						push (segregs[regss]);
						break;

					case 0x17:	/* 17 POP segregs[regss] */
						segregs[regss] = pop();
						break;

					case 0x18:	/* 18 SBB Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_sbb8();
						writerm8 (rm, res8);
						break;

					case 0x19:	/* 19 SBB Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_sbb16();
						writerm16 (rm, res16);
						break;

					case 0x1A:	/* 1A SBB Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_sbb8();
						putreg8 (reg, res8);
						break;

					case 0x1B:	/* 1B SBB Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_sbb16();
						putreg16 (reg, res16);
						break;

					case 0x1C:	/* 1C SBB regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_sbb8();
						regs.byteregs[regal] = res8;
						break;

					case 0x1D:	/* 1D SBB eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_sbb16();
						putreg16 (regax, res16);
						break;

					case 0x1E:	/* 1E PUSH segregs[regds] */
						push (segregs[regds]);
						break;

					case 0x1F:	/* 1F POP segregs[regds] */
						segregs[regds] = pop();
						break;

					case 0x20:	/* 20 AND Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_and8();
						writerm8 (rm, res8);
						break;

					case 0x21:	/* 21 AND Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_and16();
						writerm16 (rm, res16);
						break;

					case 0x22:	/* 22 AND Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_and8();
						putreg8 (reg, res8);
						break;

					case 0x23:	/* 23 AND Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_and16();
						putreg16 (reg, res16);
						break;

					case 0x24:	/* 24 AND regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_and8();
						regs.byteregs[regal] = res8;
						break;

					case 0x25:	/* 25 AND eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_and16();
						putreg16 (regax, res16);
						break;

					case 0x27:	/* 27 DAA */
						if ( ( (regs.byteregs[regal] & 0xF) > 9) || (af == 1) ) {
								oper1 = regs.byteregs[regal] + 6;
								regs.byteregs[regal] = oper1 & 255;
								cf= (oper1 & 0xFF00)?1:0; 
								af = 1;
							}
						else {
								af = 0;
							}

						if ( ( (regs.byteregs[regal] & 0xF0) > 0x90) || (cf == 1) ) {
								regs.byteregs[regal] = regs.byteregs[regal] + 0x60;
								cf = 1;
							}
						else {
								cf = 0;
							}

						regs.byteregs[regal] = regs.byteregs[regal] & 255;
						flag_szp8 (regs.byteregs[regal]);
						break;

					case 0x28:	/* 28 SUB Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_sub8();
						writerm8 (rm, res8);
						break;

					case 0x29:	/* 29 SUB Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_sub16();
						writerm16 (rm, res16);
						break;

					case 0x2A:	/* 2A SUB Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_sub8();
						putreg8 (reg, res8);
						break;

					case 0x2B:	/* 2B SUB Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_sub16();
						putreg16 (reg, res16);
						break;

					case 0x2C:	/* 2C SUB regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_sub8();
						regs.byteregs[regal] = res8;
						break;

					case 0x2D:	/* 2D SUB eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_sub16();
						putreg16 (regax, res16);
						break;

					case 0x2F:	//2F DAS
						if ( ( (regs.byteregs[regal] & 15) > 9) || (af == 1) ) {
								oper1 = regs.byteregs[regal] - 6;
								regs.byteregs[regal] = oper1 & 255;
								cf= (oper1 & 0xFF00)?1:0; 
								af = 1;
							}
						else {
								af = 0;
							}

						if ( ( (regs.byteregs[regal] & 0xF0) > 0x90) || (cf == 1) ) {
								regs.byteregs[regal] = regs.byteregs[regal] - 0x60;
								cf = 1;
							}
						else {
								cf = 0;
							}

						flag_szp8 (regs.byteregs[regal]);
						break;

					case 0x30:	/* 30 XOR Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						op_xor8();
						writerm8 (rm, res8);
						break;

					case 0x31:	/* 31 XOR Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						op_xor16();
						writerm16 (rm, res16);
						break;

					case 0x32:	/* 32 XOR Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						op_xor8();
						putreg8 (reg, res8);
						break;

					case 0x33:	/* 33 XOR Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						op_xor16();
						putreg16 (reg, res16);
						break;

					case 0x34:	/* 34 XOR regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						op_xor8();
						regs.byteregs[regal] = res8;
						break;

					case 0x35:	/* 35 XOR eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						op_xor16();
						putreg16 (regax, res16);
						break;

					case 0x37:	/* 37 AAA ASCII */
						if ( ( (regs.byteregs[regal] & 0xF) > 9) || (af == 1) ) {
								regs.byteregs[regal] = regs.byteregs[regal] + 6;
								regs.byteregs[regah] = regs.byteregs[regah] + 1;
								af = 1;
								cf = 1;
							}
						else {
								af = 0;
								cf = 0;
							}

						regs.byteregs[regal] = regs.byteregs[regal] & 0xF;
						break;

					case 0x38:	/* 38 CMP Eb Gb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getreg8 (reg);
						flag_sub8 (oper1b, oper2b);
						break;

					case 0x39:	/* 39 CMP Ev Gv */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getreg16 (reg);
						flag_sub16 (oper1, oper2);
						break;

					case 0x3A:	/* 3A CMP Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						flag_sub8 (oper1b, oper2b);
						break;

					case 0x3B:	/* 3B CMP Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						flag_sub16 (oper1, oper2);
						break;

					case 0x3C:	/* 3C CMP regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						flag_sub8 (oper1b, oper2b);
						break;

					case 0x3D:	/* 3D CMP eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						flag_sub16 (oper1, oper2);
						break;

					case 0x3F:	/* 3F AAS ASCII */
						if ( ( (regs.byteregs[regal] & 0xF) > 9) || (af == 1) ) {
								regs.byteregs[regal] = regs.byteregs[regal] - 6;
								regs.byteregs[regah] = regs.byteregs[regah] - 1;
								af = 1;
								cf = 1;
							}
						else {
								af = 0;
								cf = 0;
							}

						regs.byteregs[regal] = regs.byteregs[regal] & 0xF;
						break;

					case 0x40:	/* 40 INC eAX */
						oldcf = cf;
						oper1 = getreg16 (regax);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regax, res16);
						break;

					case 0x41:	/* 41 INC eCX */
						oldcf = cf;
						oper1 = getreg16 (regcx);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regcx, res16);
						break;

					case 0x42:	/* 42 INC eDX */
						oldcf = cf;
						oper1 = getreg16 (regdx);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regdx, res16);
						break;

					case 0x43:	/* 43 INC eBX */
						oldcf = cf;
						oper1 = getreg16 (regbx);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regbx, res16);
						break;

					case 0x44:	/* 44 INC eSP */
						oldcf = cf;
						oper1 = getreg16 (regsp);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regsp, res16);
						break;

					case 0x45:	/* 45 INC eBP */
						oldcf = cf;
						oper1 = getreg16 (regbp);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regbp, res16);
						break;

					case 0x46:	/* 46 INC eSI */
						oldcf = cf;
						oper1 = getreg16 (regsi);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regsi, res16);
						break;

					case 0x47:	/* 47 INC eDI */
						oldcf = cf;
						oper1 = getreg16 (regdi);
						oper2 = 1;
						op_add16();
						cf = oldcf;
						putreg16 (regdi, res16);
						break;

					case 0x48:	/* 48 DEC eAX */
						oldcf = cf;
						oper1 = getreg16 (regax);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regax, res16);
						break;

					case 0x49:	/* 49 DEC eCX */
						oldcf = cf;
						oper1 = getreg16 (regcx);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regcx, res16);
						break;

					case 0x4A:	/* 4A DEC eDX */
						oldcf = cf;
						oper1 = getreg16 (regdx);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regdx, res16);
						break;

					case 0x4B:	/* 4B DEC eBX */
						oldcf = cf;
						oper1 = getreg16 (regbx);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regbx, res16);
						break;

					case 0x4C:	/* 4C DEC eSP */
						oldcf = cf;
						oper1 = getreg16 (regsp);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regsp, res16);
						break;

					case 0x4D:	/* 4D DEC eBP */
						oldcf = cf;
						oper1 = getreg16 (regbp);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regbp, res16);
						break;

					case 0x4E:	/* 4E DEC eSI */
						oldcf = cf;
						oper1 = getreg16 (regsi);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regsi, res16);
						break;

					case 0x4F:	/* 4F DEC eDI */
						oldcf = cf;
						oper1 = getreg16 (regdi);
						oper2 = 1;
						op_sub16();
						cf = oldcf;
						putreg16 (regdi, res16);
						break;

					case 0x50:	/* 50 PUSH eAX */
						push (getreg16 (regax) );
						break;

					case 0x51:	/* 51 PUSH eCX */
						push (getreg16 (regcx) );
						break;

					case 0x52:	/* 52 PUSH eDX */
						push (getreg16 (regdx) );
						break;

					case 0x53:	/* 53 PUSH eBX */
						push (getreg16 (regbx) );
						break;

					case 0x54:	/* 54 PUSH eSP */
						push (getreg16 (regsp) - 2);
						break;

					case 0x55:	/* 55 PUSH eBP */
						push (getreg16 (regbp) );
						break;

					case 0x56:	/* 56 PUSH eSI */
						push (getreg16 (regsi) );
						break;

					case 0x57:	/* 57 PUSH eDI */
						push (getreg16 (regdi) );
						break;

					case 0x58:	/* 58 POP eAX */
						putreg16 (regax, pop() );
						break;

					case 0x59:	/* 59 POP eCX */
						putreg16 (regcx, pop() );
						break;

					case 0x5A:	/* 5A POP eDX */
						putreg16 (regdx, pop() );
						break;

					case 0x5B:	/* 5B POP eBX */
						putreg16 (regbx, pop() );
						break;

					case 0x5C:	/* 5C POP eSP */
						putreg16 (regsp, pop() );
						break;

					case 0x5D:	/* 5D POP eBP */
						putreg16 (regbp, pop() );
						break;

					case 0x5E:	/* 5E POP eSI */
						putreg16 (regsi, pop() );
						break;

					case 0x5F:	/* 5F POP eDI */
						putreg16 (regdi, pop() );
						break;

#ifdef CPU_V20
					case 0x60:	/* 60 PUSHA (80186+) */
						oldsp = getreg16 (regsp);
						push (getreg16 (regax) );
						push (getreg16 (regcx) );
						push (getreg16 (regdx) );
						push (getreg16 (regbx) );
						push (oldsp);
						push (getreg16 (regbp) );
						push (getreg16 (regsi) );
						push (getreg16 (regdi) );
						break;

					case 0x61:	/* 61 POPA (80186+) */
						putreg16 (regdi, pop() );
						putreg16 (regsi, pop() );
						putreg16 (regbp, pop() );
						dummy = pop();
						putreg16 (regbx, pop() );
						putreg16 (regdx, pop() );
						putreg16 (regcx, pop() );
						putreg16 (regax, pop() );
						break;

					case 0x62: /* 62 BOUND Gv, Ev (80186+) */
						modregrm();
						getea (rm);
						if (signext32 (getreg16 (reg) ) < signext32 ( getmem16 (ea >> 4, ea & 15) ) ) {
								intcall86 (5); //bounds check exception
							}
						else {
								ea += 2;
								if (signext32 (getreg16 (reg) ) > signext32 ( getmem16 (ea >> 4, ea & 15) ) ) {
										intcall86(5); //bounds check exception
									}
							}
						break;

					case 0x68:	/* 68 PUSH Iv (80186+) */
						push (getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0x69:	/* 69 IMUL Gv Ev Iv (80186+) */
						modregrm();
						temp1 = readrm16 (rm);
						temp2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						if ( (temp1 & 0x8000L) == 0x8000L) {
								temp1 = temp1 | 0xFFFF0000L;
							}

						if ( (temp2 & 0x8000L) == 0x8000L) {
								temp2 = temp2 | 0xFFFF0000L;
							}

						temp3 = temp1 * temp2;
						putreg16 (reg, temp3 & 0xFFFFL);
						if (temp3 & 0xFFFF0000L) {
								cf = 1;
								of = 1;
							}
						else {
								cf = 0;
								of = 0;
							}
						break;

					case 0x6A:	/* 6A PUSH Ib (80186+) */
						push (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						break;

					case 0x6B:	/* 6B IMUL Gv Eb Ib (80186+) */
						modregrm();
						temp1 = readrm16 (rm);
						temp2 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if ( (temp1 & 0x8000L) == 0x8000L) {
								temp1 = temp1 | 0xFFFF0000L;
							}

						if ( (temp2 & 0x8000L) == 0x8000L) {
								temp2 = temp2 | 0xFFFF0000L;
							}

						temp3 = temp1 * temp2;
						putreg16 (reg, temp3 & 0xFFFFL);
						if (temp3 & 0xFFFF0000L) {
								cf = 1;
								of = 1;
							}
						else {
								cf = 0;
								of = 0;
							}
						break;

					case 0x6C:	/* 6E INSB */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						putmem8 (useseg, getreg16 (regsi) , portin (regs.wordregs[regdx]) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 1);
								putreg16 (regdi, getreg16 (regdi) - 1);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 1);
								putreg16 (regdi, getreg16 (regdi) + 1);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0x6D:	/* 6F INSW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						putmem16 (useseg, getreg16 (regsi) , portin16 (regs.wordregs[regdx]) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 2);
								putreg16 (regdi, getreg16 (regdi) - 2);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 2);
								putreg16 (regdi, getreg16 (regdi) + 2);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0x6E:	/* 6E OUTSB */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						portout (regs.wordregs[regdx], getmem8 (useseg, getreg16 (regsi) ) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 1);
								putreg16 (regdi, getreg16 (regdi) - 1);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 1);
								putreg16 (regdi, getreg16 (regdi) + 1);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0x6F:	/* 6F OUTSW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						portout16 (regs.wordregs[regdx], getmem16 (useseg, getreg16 (regsi) ) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 2);
								putreg16 (regdi, getreg16 (regdi) - 2);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 2);
								putreg16 (regdi, getreg16 (regdi) + 2);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;
#endif

					case 0x70:	/* 70 JO Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (of) {
								ip = ip + temp16;
							}
						break;

					case 0x71:	/* 71 JNO Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!of) {
								ip = ip + temp16;
							}
						break;

					case 0x72:	/* 72 JB Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (cf) {
								ip = ip + temp16;
							}
						break;

					case 0x73:	/* 73 JNB Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!cf) {
								ip = ip + temp16;
							}
						break;

					case 0x74:	/* 74 JZ Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (zf) {
								ip = ip + temp16;
							}
						break;

					case 0x75:	/* 75 JNZ Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!zf) {
								ip = ip + temp16;
							}
						break;

					case 0x76:	/* 76 JBE Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (cf || zf) {
								ip = ip + temp16;
							}
						break;

					case 0x77:	/* 77 JA Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!cf && !zf) {
								ip = ip + temp16;
							}
						break;

					case 0x78:	/* 78 JS Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (sf) {
								ip = ip + temp16;
							}
						break;

					case 0x79:	/* 79 JNS Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!sf) {
								ip = ip + temp16;
							}
						break;

					case 0x7A:	/* 7A JPE Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (pf) {
								ip = ip + temp16;
							}
						break;

					case 0x7B:	/* 7B JPO Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!pf) {
								ip = ip + temp16;
							}
						break;

					case 0x7C:	/* 7C JL Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (sf != of) {
								ip = ip + temp16;
							}
						break;

					case 0x7D:	/* 7D JGE Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (sf == of) {
								ip = ip + temp16;
							}
						break;

					case 0x7E:	/* 7E JLE Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if ( (sf != of) || zf) {
								ip = ip + temp16;
							}
						break;

					case 0x7F:	/* 7F JG Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (!zf && (sf == of) ) {
								ip = ip + temp16;
							}
						break;

					case 0x80:
					case 0x82:	/* 80/82 GRP1 Eb Ib */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						switch (reg) {
								case 0:
									op_add8();
									break;
								case 1:
									op_or8();
									break;
								case 2:
									op_adc8();
									break;
								case 3:
									op_sbb8();
									break;
								case 4:
									op_and8();
									break;
								case 5:
									op_sub8();
									break;
								case 6:
									op_xor8();
									break;
								case 7:
									flag_sub8 (oper1b, oper2b);
									break;
								default:
									break;	/* to avoid compiler warnings */
							}

						if (reg < 7) {
								writerm8 (rm, res8);
							}
						break;

					case 0x81:	/* 81 GRP1 Ev Iv */
					case 0x83:	/* 83 GRP1 Ev Ib */
						modregrm();
						oper1 = readrm16 (rm);
						if (opcode == 0x81) {
								oper2 = getmem16 (segregs[regcs], ip);
								StepIP (2);
							}
						else {
								oper2 = signext (getmem8 (segregs[regcs], ip) );
								StepIP (1);
							}

						switch (reg) {
								case 0:
									op_add16();
									break;
								case 1:
									op_or16();
									break;
								case 2:
									op_adc16();
									break;
								case 3:
									op_sbb16();
									break;
								case 4:
									op_and16();
									break;
								case 5:
									op_sub16();
									break;
								case 6:
									op_xor16();
									break;
								case 7:
									flag_sub16 (oper1, oper2);
									break;
								default:
									break;	/* to avoid compiler warnings */
							}

						if (reg < 7) {
								writerm16 (rm, res16);
							}
						break;

					case 0x84:	/* 84 TEST Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						oper2b = readrm8 (rm);
						flag_log8 (oper1b & oper2b);
						break;

					case 0x85:	/* 85 TEST Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						oper2 = readrm16 (rm);
						flag_log16 (oper1 & oper2);
						break;

					case 0x86:	/* 86 XCHG Gb Eb */
						modregrm();
						oper1b = getreg8 (reg);
						putreg8 (reg, readrm8 (rm) );
						writerm8 (rm, oper1b);
						break;

					case 0x87:	/* 87 XCHG Gv Ev */
						modregrm();
						oper1 = getreg16 (reg);
						putreg16 (reg, readrm16 (rm) );
						writerm16 (rm, oper1);
						break;

					case 0x88:	/* 88 MOV Eb Gb */
						modregrm();
						writerm8 (rm, getreg8 (reg) );
						break;

					case 0x89:	/* 89 MOV Ev Gv */
						modregrm();
						writerm16 (rm, getreg16 (reg) );
						break;

					case 0x8A:	/* 8A MOV Gb Eb */
						modregrm();
						putreg8 (reg, readrm8 (rm) );
						break;

					case 0x8B:	/* 8B MOV Gv Ev */
						modregrm();
						putreg16 (reg, readrm16 (rm) );
						break;

					case 0x8C:	/* 8C MOV Ew Sw */
						modregrm();
						writerm16 (rm, getsegreg (reg) );
						break;

					case 0x8D:	/* 8D LEA Gv M */
						modregrm();
						getea (rm);
						putreg16 (reg, ea - segbase (useseg) );
						break;

					case 0x8E:	/* 8E MOV Sw Ew */
						modregrm();
						putsegreg (reg, readrm16 (rm) );
						break;

					case 0x8F:	/* 8F POP Ev */
						modregrm();
						writerm16 (rm, pop() );
						break;

					case 0x90:	/* 90 NOP */
						break;

					case 0x91:	/* 91 XCHG eCX eAX */
						oper1 = getreg16 (regcx);
						putreg16 (regcx, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x92:	/* 92 XCHG eDX eAX */
						oper1 = getreg16 (regdx);
						putreg16 (regdx, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x93:	/* 93 XCHG eBX eAX */
						oper1 = getreg16 (regbx);
						putreg16 (regbx, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x94:	/* 94 XCHG eSP eAX */
						oper1 = getreg16 (regsp);
						putreg16 (regsp, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x95:	/* 95 XCHG eBP eAX */
						oper1 = getreg16 (regbp);
						putreg16 (regbp, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x96:	/* 96 XCHG eSI eAX */
						oper1 = getreg16 (regsi);
						putreg16 (regsi, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x97:	/* 97 XCHG eDI eAX */
						oper1 = getreg16 (regdi);
						putreg16 (regdi, getreg16 (regax) );
						putreg16 (regax, oper1);
						break;

					case 0x98:	//98 CBW
					    regs.byteregs[regah]= ( (regs.byteregs[regal] & 0x80) == 0x80)?0xFF:0;						
						break;

					case 0x99:	//99 CWD
					    regs.wordregs[regdx] = ( (regs.byteregs[regah] & 0x80) == 0x80)?0xFFFF:0;
						break;

					case 0x9A:	/* 9A CALL Ap */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						push (segregs[regcs]);
						push (ip);
						ip = oper1;
						segregs[regcs] = oper2;
						break;

					case 0x9B:	/* 9B WAIT */
						break;

					case 0x9C:	/* 9C PUSHF */
						push (makeflagsword() | 0xF800);
						break;

					case 0x9D:	/* 9D POPF */
						temp16 = pop();
						decodeflagsword (temp16);
						break;

					case 0x9E:	/* 9E SAHF */
						decodeflagsword ( (makeflagsword() & 0xFF00) | regs.byteregs[regah]);
						break;

					case 0x9F:	/* 9F LAHF */
						regs.byteregs[regah] = makeflagsword() & 0xFF;
						break;

					case 0xA0:	/* A0 MOV regs.byteregs[regal] Ob */
						regs.byteregs[regal] = getmem8 (useseg, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0xA1:	/* A1 MOV eAX Ov */
						oper1 = getmem16 (useseg, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						putreg16 (regax, oper1);
						break;

					case 0xA2:	/* A2 MOV Ob regs.byteregs[regal] */
						putmem8 (useseg, getmem16 (segregs[regcs], ip), regs.byteregs[regal]);
						StepIP (2);
						break;

					case 0xA3:	/* A3 MOV Ov eAX */
						putmem16 (useseg, getmem16 (segregs[regcs], ip), getreg16 (regax) );
						StepIP (2);
						break;

					case 0xA4:	/* A4 MOVSB */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						putmem8 (segregs[reges], getreg16 (regdi), getmem8 (useseg, getreg16 (regsi) ) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 1);
								putreg16 (regdi, getreg16 (regdi) - 1);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 1);
								putreg16 (regdi, getreg16 (regdi) + 1);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xA5:	/* A5 MOVSW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						putmem16 (segregs[reges], getreg16 (regdi), getmem16 (useseg, getreg16 (regsi) ) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 2);
								putreg16 (regdi, getreg16 (regdi) - 2);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 2);
								putreg16 (regdi, getreg16 (regdi) + 2);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xA6:	/* A6 CMPSB */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						oper1b = getmem8 (useseg, getreg16 (regsi) );
						oper2b = getmem8 (segregs[reges], getreg16 (regdi) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 1);
								putreg16 (regdi, getreg16 (regdi) - 1);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 1);
								putreg16 (regdi, getreg16 (regdi) + 1);
							}

						flag_sub8 (oper1b, oper2b);
						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						if ( (reptype == 1) && !zf) {
								break;
							}
						else if ( (reptype == 2) && (zf == 1) ) {
								break;
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xA7:	/* A7 CMPSW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						oper1 = getmem16 (useseg, getreg16 (regsi) );
						oper2 = getmem16 (segregs[reges], getreg16 (regdi) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 2);
								putreg16 (regdi, getreg16 (regdi) - 2);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 2);
								putreg16 (regdi, getreg16 (regdi) + 2);
							}

						flag_sub16 (oper1, oper2);
						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						if ( (reptype == 1) && !zf) {
								break;
							}

						if ( (reptype == 2) && (zf == 1) ) {
								break;
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xA8:	/* A8 TEST regs.byteregs[regal] Ib */
						oper1b = regs.byteregs[regal];
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						flag_log8 (oper1b & oper2b);
						break;

					case 0xA9:	/* A9 TEST eAX Iv */
						oper1 = getreg16 (regax);
						oper2 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						flag_log16 (oper1 & oper2);
						break;

					case 0xAA:	//AA STOSB
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						putmem8 (segregs[reges], getreg16 (regdi), regs.byteregs[regal]);
						if (df) {
								putreg16 (regdi, getreg16 (regdi) - 1);
							}
						else {
								putreg16 (regdi, getreg16 (regdi) + 1);
							}						

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xAB:	/* AB STOSW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						putmem16 (segregs[reges], getreg16 (regdi), getreg16 (regax) );
						if (df) {
								putreg16 (regdi, getreg16 (regdi) - 2);
							}
						else {
								putreg16 (regdi, getreg16 (regdi) + 2);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xAC:	/* AC LODSB */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						regs.byteregs[regal] = getmem8 (useseg, getreg16 (regsi) );
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 1);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 1);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xAD:	/* AD LODSW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						oper1 = getmem16 (useseg, getreg16 (regsi) );
						putreg16 (regax, oper1);
						if (df) {
								putreg16 (regsi, getreg16 (regsi) - 2);
							}
						else {
								putreg16 (regsi, getreg16 (regsi) + 2);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xAE:	/* AE SCASB */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						oper1b = getmem8 (segregs[reges], getreg16 (regdi) );
						oper2b = regs.byteregs[regal];
						flag_sub8 (oper1b, oper2b);
						if (df) {
								putreg16 (regdi, getreg16 (regdi) - 1);
							}
						else {
								putreg16 (regdi, getreg16 (regdi) + 1);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						if ( (reptype == 1) && !zf) {
								break;
							}
						else if ( (reptype == 2) && (zf == 1) ) {
								break;
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xAF:	/* AF SCASW */
						if (reptype && (getreg16 (regcx) == 0) ) {
								break;
							}

						oper1 = getmem16 (segregs[reges], getreg16 (regdi) );
						oper2 = getreg16 (regax);
						flag_sub16 (oper1, oper2);
						if (df) {
								putreg16 (regdi, getreg16 (regdi) - 2);
							}
						else {
								putreg16 (regdi, getreg16 (regdi) + 2);
							}

						if (reptype) {
								putreg16 (regcx, getreg16 (regcx) - 1);
							}

						if ( (reptype == 1) && !zf) {
								break;
							}
						else if ( (reptype == 2) & (zf == 1) ) {
								break;
							}

						totalexec++;
						loopcount++;
						if (!reptype) {
								break;
							}

						ip = firstip;
						break;

					case 0xB0:	/* B0 MOV regs.byteregs[regal] Ib */
						regs.byteregs[regal] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB1:	/* B1 MOV regs.byteregs[regcl] Ib */
						regs.byteregs[regcl] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB2:	/* B2 MOV regs.byteregs[regdl] Ib */
						regs.byteregs[regdl] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB3:	/* B3 MOV regs.byteregs[regbl] Ib */
						regs.byteregs[regbl] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB4:	/* B4 MOV regs.byteregs[regah] Ib */
						regs.byteregs[regah] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB5:	/* B5 MOV regs.byteregs[regch] Ib */
						regs.byteregs[regch] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB6:	/* B6 MOV regs.byteregs[regdh] Ib */
						regs.byteregs[regdh] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB7:	/* B7 MOV regs.byteregs[regbh] Ib */
						regs.byteregs[regbh] = getmem8 (segregs[regcs], ip);
						StepIP (1);
						break;

					case 0xB8:	/* B8 MOV eAX Iv */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						putreg16 (regax, oper1);
						break;

					case 0xB9:	/* B9 MOV eCX Iv */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						putreg16 (regcx, oper1);
						break;

					case 0xBA:	/* BA MOV eDX Iv */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						putreg16 (regdx, oper1);
						break;

					case 0xBB:	/* BB MOV eBX Iv */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						putreg16 (regbx, oper1);
						break;

					case 0xBC:	/* BC MOV eSP Iv */
						putreg16 (regsp, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0xBD:	/* BD MOV eBP Iv */
						putreg16 (regbp, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0xBE:	/* BE MOV eSI Iv */
						putreg16 (regsi, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0xBF:	/* BF MOV eDI Iv */
						putreg16 (regdi, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0xC0:	/* C0 GRP2 byte imm8 (80186+) */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						writerm8 (rm, op_grp2_8 (oper2b) );
						break;

					case 0xC1:	/* C1 GRP2 word imm8 (80186+) */
						modregrm();
						oper1 = readrm16 (rm);
						oper2 = getmem8 (segregs[regcs], ip);
						StepIP (1);
						writerm16 (rm, op_grp2_16 ( (uint8_t) oper2) );
						break;

					case 0xC2:	/* C2 RET Iw */
						oper1 = getmem16 (segregs[regcs], ip);
						ip = pop();
						putreg16 (regsp, getreg16 (regsp) + oper1);
						break;

					case 0xC3:	/* C3 RET */
						ip = pop();
						break;

					case 0xC4:	/* C4 LES Gv Mp */
						modregrm();
						getea (rm);
						putreg16 (reg, read86 (ea) + read86 (ea + 1) * 256);
						segregs[reges] = read86 (ea + 2) + read86 (ea + 3) * 256;
						break;

					case 0xC5:	/* C5 LDS Gv Mp */
						modregrm();
						getea (rm);
						putreg16 (reg, read86 (ea) + read86 (ea + 1) * 256);
						segregs[regds] = read86 (ea + 2) + read86 (ea + 3) * 256;
						break;

					case 0xC6:	/* C6 MOV Eb Ib */
						modregrm();
						writerm8 (rm, getmem8 (segregs[regcs], ip) );
						StepIP (1);
						break;

					case 0xC7:	/* C7 MOV Ev Iv */
						modregrm();
						writerm16 (rm, getmem16 (segregs[regcs], ip) );
						StepIP (2);
						break;

					case 0xC8:	/* C8 ENTER (80186+) */
						stacksize = getmem16 (segregs[regcs], ip);
						StepIP (2);
						nestlev = getmem8 (segregs[regcs], ip);
						StepIP (1);
						push (getreg16 (regbp) );
						frametemp = getreg16 (regsp);
						if (nestlev) {
								for (temp16 = 1; temp16 < nestlev; temp16++) {
										putreg16 (regbp, getreg16 (regbp) - 2);
										push (getreg16 (regbp) );
									}

								push (getreg16 (regsp) );
							}

						putreg16 (regbp, frametemp);
						putreg16 (regsp, getreg16 (regbp) - stacksize);

						break;

					case 0xC9:	/* C9 LEAVE (80186+) */
						putreg16 (regsp, getreg16 (regbp) );
						putreg16 (regbp, pop() );

						break;

					case 0xCA:	/* CA RETF Iw */
						oper1 = getmem16 (segregs[regcs], ip);
						ip = pop();
						segregs[regcs] = pop();
						putreg16 (regsp, getreg16 (regsp) + oper1);
						break;

					case 0xCB:	/* CB RETF */
						ip = pop();;
						segregs[regcs] = pop();
						break;

					case 0xCC:	/* CC INT 3 */
						intcall86 (3);
						break;

					case 0xCD:	/* CD INT Ib */
						oper1b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						intcall86 (oper1b);
						break;

					case 0xCE:	/* CE INTO */
						if (of) {
								intcall86 (4);
							}
						break;

					case 0xCF:	/* CF IRET */
						ip = pop();
						segregs[regcs] = pop();
						decodeflagsword (pop() );

						/*
						 * if (net.enabled) net.canrecv = 1;
						 */
						break;

					case 0xD0:	/* D0 GRP2 Eb 1 */
						modregrm();
						oper1b = readrm8 (rm);
						writerm8 (rm, op_grp2_8 (1) );
						break;

					case 0xD1:	/* D1 GRP2 Ev 1 */
						modregrm();
						oper1 = readrm16 (rm);
						writerm16 (rm, op_grp2_16 (1) );
						break;

					case 0xD2:	/* D2 GRP2 Eb regs.byteregs[regcl] */
						modregrm();
						oper1b = readrm8 (rm);
						writerm8 (rm, op_grp2_8 (regs.byteregs[regcl]) );
						break;

					case 0xD3:	/* D3 GRP2 Ev regs.byteregs[regcl] */
						modregrm();
						oper1 = readrm16 (rm);
						writerm16 (rm, op_grp2_16 (regs.byteregs[regcl]) );
						break;

					case 0xD4:	/* D4 AAM I0 */
						oper1 = getmem8 (segregs[regcs], ip);
						StepIP (1);
						if (!oper1) {
								intcall86 (0);
								break;
							}	/* division by zero */

						regs.byteregs[regah] = (regs.byteregs[regal] / oper1) & 255;
						regs.byteregs[regal] = (regs.byteregs[regal] % oper1) & 255;
						flag_szp16 (getreg16 (regax) );
						break;

					case 0xD5:	/* D5 AAD I0 */
						oper1 = getmem8 (segregs[regcs], ip);
						StepIP (1);
						regs.byteregs[regal] = (regs.byteregs[regah] * oper1 + regs.byteregs[regal]) & 255;
						regs.byteregs[regah] = 0;
						flag_szp16 (regs.byteregs[regah] * oper1 + regs.byteregs[regal]);
						sf = 0;
						break;

					case 0xD6:	/* D6 XLAT on V20/V30, SALC on 8086/8088 */
#ifndef CPU_V20
						regs.byteregs[regal] = cf ? 0xFF : 0x00;
						break;
#endif

					case 0xD7:	/* D7 XLAT */
						regs.byteregs[regal] = read86(useseg * 16 + (regs.wordregs[regbx]) + regs.byteregs[regal]);
						break;

					case 0xD8:
					case 0xD9:
					case 0xDA:
					case 0xDB:
					case 0xDC:
					case 0xDE:
					case 0xDD:
					case 0xDF:	/* escape to x87 FPU (unsupported) */
						modregrm();
						break;

					case 0xE0:	/* E0 LOOPNZ Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						putreg16 (regcx, getreg16 (regcx) - 1);
						if ( (getreg16 (regcx) ) && !zf) {
								ip = ip + temp16;
							}
						break;

					case 0xE1:	/* E1 LOOPZ Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						putreg16 (regcx, (getreg16 (regcx) ) - 1);
						if ( (getreg16 (regcx) ) && (zf == 1) ) {
								ip = ip + temp16;
							}
						break;

					case 0xE2:	/* E2 LOOP Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						putreg16 (regcx, (getreg16 (regcx) ) - 1);
						if (getreg16 (regcx) ) {
								ip = ip + temp16;
							}
						break;

					case 0xE3:	/* E3 JCXZ Jb */
						temp16 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						if (! (getreg16 (regcx) ) ) {
								ip = ip + temp16;
							}
						break;

					case 0xE4:	/* E4 IN regs.byteregs[regal] Ib */
						oper1b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						regs.byteregs[regal] = (uint8_t) portin (oper1b);
						break;

					case 0xE5:	/* E5 IN eAX Ib */
						oper1b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						putreg16 (regax, portin16 (oper1b) );
						break;

					case 0xE6:	/* E6 OUT Ib regs.byteregs[regal] */
						oper1b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						portout (oper1b, regs.byteregs[regal]);
						break;

					case 0xE7:	/* E7 OUT Ib eAX */
						oper1b = getmem8 (segregs[regcs], ip);
						StepIP (1);
						portout16 (oper1b, (getreg16 (regax) ) );
						break;

					case 0xE8:	/* E8 CALL Jv */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						push (ip);
						ip = ip + oper1;
						break;

					case 0xE9:	/* E9 JMP Jv */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						ip = ip + oper1;
						break;

					case 0xEA:	/* EA JMP Ap */
						oper1 = getmem16 (segregs[regcs], ip);
						StepIP (2);
						oper2 = getmem16 (segregs[regcs], ip);
						ip = oper1;
						segregs[regcs] = oper2;
						break;

					case 0xEB:	/* EB JMP Jb */
						oper1 = signext (getmem8 (segregs[regcs], ip) );
						StepIP (1);
						ip = ip + oper1;
						break;

					case 0xEC:	/* EC IN regs.byteregs[regal] regdx */
						oper1 = (getreg16 (regdx) );
						regs.byteregs[regal] = (uint8_t) portin (oper1);
						break;

					case 0xED:	/* ED IN eAX regdx */
						oper1 = (getreg16 (regdx) );
						putreg16 (regax, portin16 (oper1) );
						break;

					case 0xEE:	/* EE OUT regdx regs.byteregs[regal] */
						oper1 = (getreg16 (regdx) );
						portout (oper1, regs.byteregs[regal]);
						break;

					case 0xEF:	/* EF OUT regdx eAX */
						oper1 = (getreg16 (regdx) );
						portout16 (oper1, (getreg16 (regax) ) );
						break;

					case 0xF0:	/* F0 LOCK */
						break;

					case 0xF4:	/* F4 HLT */
						ip--;
						break;

					case 0xF5:	//F5 CMC
					    cf= (!cf)?1:0;
						break;

					case 0xF6:	/* F6 GRP3a Eb */
						modregrm();
						oper1b = readrm8 (rm);
						op_grp3_8();
						if ( (reg > 1) && (reg < 4) ) {
								writerm8 (rm, res8);
							}
						break;

					case 0xF7:	/* F7 GRP3b Ev */
						modregrm();
						oper1 = readrm16 (rm);
						op_grp3_16();
						if ( (reg > 1) && (reg < 4) ) {
								writerm16 (rm, res16);
							}
						break;

					case 0xF8:	/* F8 CLC */
						cf = 0;
						break;

					case 0xF9:	/* F9 STC */
						cf = 1;
						break;

					case 0xFA:	/* FA CLI */
						ifl = 0;
						break;

					case 0xFB:	/* FB STI */
						ifl = 1;
						break;

					case 0xFC:	/* FC CLD */
						df = 0;
						break;

					case 0xFD:	/* FD STD */
						df = 1;
						break;

					case 0xFE:	/* FE GRP4 Eb */
						modregrm();
						oper1b = readrm8 (rm);
						oper2b = 1;
						if (!reg) {
								tempcf = cf;
								res8 = oper1b + oper2b;
								flag_add8 (oper1b, oper2b);
								cf = tempcf;
								writerm8 (rm, res8);
							}
						else {
								tempcf = cf;
								res8 = oper1b - oper2b;
								flag_sub8 (oper1b, oper2b);
								cf = tempcf;
								writerm8 (rm, res8);
							}
						break;

					case 0xFF:	/* FF GRP5 Ev */
						modregrm();
						oper1 = readrm16 (rm);
						op_grp5();
						break;

					default:
#ifdef CPU_V20
						intcall86 (6); /* trip invalid opcode exception (this occurs on the 80186+, 8086/8088 CPUs treat them as NOPs. */
						               /* technically they aren't exactly like NOPs in most cases, but for our pursoses, that's accurate enough. */
#endif
						//if (verbose) {
						//		Serial.printf ("Illegal opcode: %02X @ %04X:%04X\n", opcode, savecs, saveip);
						//	}
						break;
				}

			if (!running) {
					return;
				}
		}
}

