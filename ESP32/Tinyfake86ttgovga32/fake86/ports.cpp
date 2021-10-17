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
// ports.c: functions to handle port I/O from the CPU module, as well
//   as functions for emulated hardware components to register their
//   read/write callback functions across the port address range.

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include "gbConfig.h"
#include "gbGlobals.h"
#include "cpu.h"
#include "ports.h"

//Lista puertos
//0x60 teclado
//0x64
//0x61 speaker
//0x3D8 video
//0x3D4
//0x3D8
//0x3D9
//0x3C0
//0x3C4
//0x3CE

//JJ extern uint8_t portram[0x10000];
//JJ extern uint8_t speakerenabled;


#ifndef use_lib_not_use_callback_port
 void (*do_callback_write) (uint16_t portnum, uint8_t value) = NULL;
 uint8_t (*do_callback_read) (uint16_t portnum) = NULL;
 //JJ void (*do_callback_write16) (uint16_t portnum, uint16_t value) = NULL;
 //JJ uint16_t (*do_callback_read16) (uint16_t portnum) = NULL;
#endif 
//JJ void * (port_write_callback[gb_max_portram]);
//JJ void * (port_read_callback[gb_max_portram]);
//JJ void * (port_write_callback16[gb_max_portram]);
//JJ void * (port_read_callback16[gb_max_portram]); 


void CalculaPulsosSonido(int frec)
{ 
 //freq= (gb_volumen01==0)?0:gb_frecuencia01;
 //int auxContOnda;
 if (frec!=0) gb_pulsos_onda= (SAMPLE_RATE/frec)>>1; 
 else gb_pulsos_onda=0;
 //unsigned char estadoOnda=0;
}

void portout (uint16_t portnum, uint8_t value)
{
 unsigned char auxIdport;        
 if (portnum >= (gb_max_portram-1))
  return;     
    #ifdef use_lib_limit_portram 
     if (portnum<gb_max_portram)
     {
      //JJ puerto portram[portnum] = value;
      WriteTinyPortRAM(portnum, value);
     }
    #else 
     //JJ puerto portram[portnum] = value;
     WriteTinyPortRAM(portnum, value);
    #endif
	
	//printf("portout(0x%X, 0x%02X);\n", portnum, value);
	switch (portnum) {
			case 0x61:
				//if ( (value & 3) == 3) speakerenabled = 1;
				//else speakerenabled = 0;
            speakerenabled= ( (value & 3) == 3)?1:0;
            unsigned int aData= (gb_frec_speaker_high<<8)|gb_frec_speaker_low;
            aData= (aData!=0)?(1193180/aData):0;            
            //aData = aData >>1;            
            //Serial.printf("speakerenabled %d Frec:0x%02X%02X %d Hz\n",speakerenabled,gb_frec_speaker_high,gb_frec_speaker_low,aData);
            CalculaPulsosSonido(aData);
            if (speakerenabled)             
            {             
             gb_volumen01=128;
             gb_frecuencia01= aData;             
            }
            else
            {
             gb_volumen01=0;
             gb_frecuencia01= 0;
             //if (speaker_pin_estado != LOW)
             //{
             // //digitalWrite(25, LOW);             
             // REG_WRITE(GPIO_OUT_W1TC_REG , BIT25); //LOW clear
             // speaker_pin_estado=0;
             //}
             //gb_pulsos_onda=0;
            }            
				return;
		}

   #ifndef use_lib_not_use_callback_port
    auxIdport= GetId_port_write_callback(portnum);
    if (auxIdport != 0xFF)
    {
     do_callback_write = (void (*) (uint16_t portnum, uint8_t value) ) gb_portTiny_write_callback[auxIdport];
	  if (do_callback_write != (void *) 0)
     {     
      (*do_callback_write) (portnum, value);
     }   	 
    }                     

	 //JJ do_callback_write = (void (*) (uint16_t portnum, uint8_t value) ) port_write_callback[portnum];
	 //JJ if (do_callback_write != (void *) 0) (*do_callback_write) (portnum, value);
   #endif	
}

uint8_t portin (uint16_t portnum)
{
 unsigned char auxIdport;           
 if (portnum >= (gb_max_portram-1))
  return 0;
	//printf("portin(0x%X);\n", portnum);
	switch (portnum) {
			case 0x62:
				return (0x00);
			case 0x60:
			case 0x61:
			case 0x63:
			case 0x64:
                #ifdef use_lib_limit_portram 
                 if (portnum<gb_max_portram)
                 {
                  //JJ puerto return portram[portnum];
                  return ReadTinyPortRAM(portnum);
                 }
                #else                  
 				     //JJ puerto return (portram[portnum]);
                 return ReadTinyPortRAM(portnum);
				    #endif
		}
   #ifndef use_lib_not_use_callback_port
    auxIdport= GetId_port_read_callback(portnum);
    if (auxIdport != 0xFF)
    {
     do_callback_read = (uint8_t (*) (uint16_t portnum) ) gb_portTiny_read_callback[auxIdport];                  
	  if (do_callback_read != (void *) 0)
     {    
      return ( (*do_callback_read) (portnum) );
     }
    }

	 //JJ do_callback_read = (uint8_t (*) (uint16_t portnum) ) port_read_callback[portnum];
	 //JJ if (do_callback_read != (void *) 0) return ( (*do_callback_read) (portnum) );
   #endif	
	return (0xFF);
}

void portout16 (uint16_t portnum, uint16_t value)
{
 if (portnum >= (gb_max_portram-1)){
  //printf("portout16 %d\n",portnum);
  //fflush(stdout);      
  return;
 }  
 //No se usa 
 //JJ  #ifndef use_lib_not_use_callback_port     
 //JJ	do_callback_write16 = (void (*) (uint16_t portnum, uint16_t value) ) port_write_callback16[portnum];
 //JJ	if (do_callback_write16 != (void *) 0) {
 //JJ			(*do_callback_write16) (portnum, value);
 //JJ			return;
 //JJ		}
 //JJ #endif	

	portout (portnum, (uint8_t) value);
	portout (portnum + 1, (uint8_t) (value >> 8) );
}

uint16_t portin16 (uint16_t portnum)
{
 uint16_t ret;
 if (portnum >= (gb_max_portram-1))
 {
  //printf("portin16 %d\n",portnum); 
  //fflush(stdout);
  return 0;
 }
 
 //No se usa
 //JJ  #ifndef use_lib_not_use_callback_port
 //JJ	do_callback_read16 = (uint16_t (*) (uint16_t portnum) ) port_read_callback16[portnum];
 //JJ	if (do_callback_read16 != (void *) 0) return ( (*do_callback_read16) (portnum) );
 //JJ  #endif	

	ret = (uint16_t) portin (portnum);
	ret |= (uint16_t) portin (portnum+1) << 8;
	return (ret);
}

//JJ extern void set_port_write_redirector (uint16_t startport, uint16_t endport, void *callback) 
extern void set_port_write_redirector (unsigned short int startport, unsigned short int endport, void *callback)
{       
   #ifndef use_lib_not_use_callback_port       
	//uint16_t i;
	int i;
   unsigned char auxIdport;
	for (i=startport; i<=endport; i++)
    {
     if ((i>=0) && (i<gb_max_portram))
     {
 	   //JJ port_write_callback[i] = callback;
 	   auxIdport= GetId_port_write_callback(i);
 	   if (auxIdport != 0xFF)
 	   {
        gb_portTiny_write_callback[auxIdport]= callback;
      }

     }
	}
   #endif		
}

//JJextern void set_port_read_redirector (uint16_t startport, uint16_t endport, void *callback) 
extern void set_port_read_redirector (unsigned short int startport, unsigned short int endport, void *callback) 
{
  #ifndef use_lib_not_use_callback_port
  //uint16_t i;
  int i;
  unsigned char auxIdport;
  for (i=startport; i<=endport; i++)
  {
   if ((i>=0) && (i<gb_max_portram))
   {      
    //JJ port_read_callback[i] = callback;
    auxIdport= GetId_port_read_callback(i);
 	 if (auxIdport != 0xFF)
 	 {
     gb_portTiny_read_callback[auxIdport]= callback;
    }    

   }
  }
  #endif		
}

//No se usa
//JJextern void set_port_write_redirector_16 (uint16_t startport, uint16_t endport, void *callback) 
//JJ extern void set_port_write_redirector_16 (unsigned short int startport, unsigned short int endport, void *callback)
//JJ {
//JJ   #ifndef use_lib_not_use_callback_port
//JJ 	//uint16_t i;
//JJ 	int i;
//JJ 	for (i=startport; i<=endport; i++)
//JJ     {
//JJ      if ((i>=0) && (i<gb_max_portram))
//JJ      {
//JJ    	  port_write_callback16[i] = callback;
//JJ      }
//JJ 	}
//JJ   #endif		
//JJ }

//No se usa
//JJ extern void set_port_read_redirector_16 (uint16_t startport, uint16_t endport, void *callback) 
//JJ extern void set_port_read_redirector_16 (unsigned short int startport, unsigned short int endport, void *callback)
//JJ {
//JJ   #ifndef use_lib_not_use_callback_port       
//JJ 	//uint16_t i;
//JJ 	int i;
//JJ 	for (i=startport; i<=endport; i++)
//JJ     {
//JJ      if ((i>=0) && (i<gb_max_portram))
//JJ      {        
//JJ 	  port_read_callback16[i] = callback;
//JJ      }
//JJ 	}
//JJ   #endif		
//JJ }




//********************************************************************
void WriteTinyPortRAM(unsigned short int numPort, unsigned char aValue)
{
 switch (numPort)
 {
  case 0x001:  gb_portramTiny[0]=aValue; break;
  case 0x008:  gb_portramTiny[1]=aValue; break;
  case 0x00A:  gb_portramTiny[2]=aValue; break;
  case 0x00B:  gb_portramTiny[3]=aValue; break;
  case 0x00D:  gb_portramTiny[4]=aValue; break;    
  case 0x020:  gb_portramTiny[5]=aValue; break;
  case 0x021:  gb_portramTiny[6]=aValue; break;          
  case 0x040:  gb_portramTiny[7]=aValue; break;
  case 0x041:  gb_portramTiny[8]=aValue; break;
  case 0x042:
   gb_portramTiny[9]=aValue;
   if ((gb_cont_frec_speaker & 1) == 0)
    gb_frec_speaker_low= aValue;
   else 
    gb_frec_speaker_high= aValue;
   gb_cont_frec_speaker++;
   //Frecuencia speaker
   break;
  case 0x043:  gb_portramTiny[10]=aValue; break;          
  case 0x060:  gb_portramTiny[fast_tiny_port_0x60]=aValue; break; //teclado
  case 0x061:  gb_portramTiny[fast_tiny_port_0x61]=aValue; break; //speaker
  case 0x063:  gb_portramTiny[13]=aValue; break;
  case 0x064:  gb_portramTiny[fast_tiny_port_0x64]=aValue; break; //teclado
  case 0x081:  gb_portramTiny[15]=aValue; break;
  case 0x082:  gb_portramTiny[16]=aValue; break;
  case 0x083:  gb_portramTiny[17]=aValue; break;
  case 0x0A0:  gb_portramTiny[18]=aValue; break;
  case 0x0C0:  gb_portramTiny[19]=aValue; break;
  case 0x200:  gb_portramTiny[20]=aValue; break;    
  case 0x201:  gb_portramTiny[21]=aValue; break;  
  case 0x213:  gb_portramTiny[22]=aValue; break;
  case 0x278:  gb_portramTiny[23]=aValue; break;
  case 0x2C8:  gb_portramTiny[24]=aValue; break;
  case 0x2CB:  gb_portramTiny[25]=aValue; break;  
  case 0x2FB:  gb_portramTiny[26]=aValue; break;
  case 0x378:  gb_portramTiny[27]=aValue; break;  
  case 0x3B4:  gb_portramTiny[28]=aValue; break;
  case 0x3B5:  gb_portramTiny[29]=aValue; break;
  case 0x3B8:  gb_portramTiny[30]=aValue; break;
  case 0x3B9:  gb_portramTiny[fast_tiny_port_0x3B9]=aValue; break;  
  case 0x3BA:  gb_portramTiny[32]=aValue; break; //CGA    
  case 0x3BC:  gb_portramTiny[33]=aValue; break;
  case 0x3BD:  gb_portramTiny[34]=aValue; break;  
  case 0x3BE:  gb_portramTiny[35]=aValue; break;  
  case 0x3BF:  gb_portramTiny[36]=aValue; break;  
  case 0x3C0:  gb_portramTiny[fast_tiny_port_0x3C0]=aValue; break; //CGA
  case 0x3C2:  gb_portramTiny[38]=aValue; break;
  case 0x3C3:  gb_portramTiny[39]=aValue; break;
  case 0x3C4:  gb_portramTiny[fast_tiny_port_0x3C4]=aValue; break; //CGA
  case 0x3C5:  gb_portramTiny[41]=aValue; break;
  case 0x3CB:  gb_portramTiny[42]=aValue; break;  
  case 0x3CC:  gb_portramTiny[43]=aValue; break;
  case 0x3D4:  gb_portramTiny[fast_tiny_port_0x3D4]=aValue; break; //CGA
  case 0x3D5:  gb_portramTiny[45]=aValue; break;
  case 0x3D8:  gb_portramTiny[fast_tiny_port_0x3D8]=aValue; break; //video CGA
  case 0x3D9:  gb_portramTiny[fast_tiny_port_0x3D9]=aValue; break; //CGA  
  case 0x3DA:  gb_portramTiny[48]=aValue; break;
  case 0x3F2:  gb_portramTiny[49]=aValue; break;
  case 0x3FB:  gb_portramTiny[50]=aValue; break;
  //case 0x3CE: gb_portramTiny[0]; break; //solo VGA
  default:
   //printf("WriteTinyPortRAM 0x%04X 0x%02X\n",numPort,aValue);
   //fflush(stdout);
   break;
 }
}

//******************************************************
unsigned char ReadTinyPortRAM(unsigned short int numPort)
{
 unsigned char aReturn=0;
 switch (numPort)
 {
  case 0x001: aReturn= gb_portramTiny[0]; break;
  case 0x008: aReturn= gb_portramTiny[1]; break;
  case 0x00A: aReturn= gb_portramTiny[2]; break;
  case 0x00B: aReturn= gb_portramTiny[3]; break;
  case 0x00D: aReturn= gb_portramTiny[4]; break;    
  case 0x020: aReturn= gb_portramTiny[5]; break;
  case 0x021: aReturn= gb_portramTiny[6]; break;          
  case 0x040: aReturn= gb_portramTiny[7]; break;
  case 0x041: aReturn= gb_portramTiny[8]; break;
  case 0x042: aReturn= gb_portramTiny[9]; break;
  case 0x043: aReturn= gb_portramTiny[10]; break;          
  case 0x060: aReturn= gb_portramTiny[fast_tiny_port_0x60]; break; //teclado
  case 0x061: aReturn= gb_portramTiny[fast_tiny_port_0x61]; break; //speaker
  case 0x063: aReturn= gb_portramTiny[13]; break;
  case 0x064: aReturn= gb_portramTiny[fast_tiny_port_0x64]; break; //teclado
  case 0x081: aReturn= gb_portramTiny[15]; break;
  case 0x082: aReturn= gb_portramTiny[16]; break;
  case 0x083: aReturn= gb_portramTiny[17]; break;
  case 0x0A0: aReturn= gb_portramTiny[18]; break;
  case 0x0C0: aReturn= gb_portramTiny[19]; break;
  case 0x200: aReturn= gb_portramTiny[20]; break;    
  case 0x201: aReturn= gb_portramTiny[21]; break;  
  case 0x213: aReturn= gb_portramTiny[22]; break;
  case 0x278: aReturn= gb_portramTiny[23]; break;
  case 0x2C8: aReturn= gb_portramTiny[24]; break;
  case 0x2CB: aReturn= gb_portramTiny[25]; break;  
  case 0x2FB: aReturn= gb_portramTiny[26]; break;
  case 0x378: aReturn= gb_portramTiny[27]; break;  
  case 0x3B4: aReturn= gb_portramTiny[28]; break;
  case 0x3B5: aReturn= gb_portramTiny[29]; break;
  case 0x3B8: aReturn= gb_portramTiny[30]; break;
  case 0x3B9: aReturn= gb_portramTiny[fast_tiny_port_0x3B9]; break;  
  case 0x3BA: aReturn= gb_portramTiny[32]; break; //CGA    
  case 0x3BC: aReturn= gb_portramTiny[33]; break;
  case 0x3BD: aReturn= gb_portramTiny[34]; break;  
  case 0x3BE: aReturn= gb_portramTiny[35]; break;  
  case 0x3BF: aReturn= gb_portramTiny[36]; break;  
  case 0x3C0: aReturn= gb_portramTiny[fast_tiny_port_0x3C0]; break; //CGA
  case 0x3C2: aReturn= gb_portramTiny[38]; break;
  case 0x3C3: aReturn= gb_portramTiny[39]; break;
  case 0x3C4: aReturn= gb_portramTiny[fast_tiny_port_0x3C4]; break; //CGA
  case 0x3C5: aReturn= gb_portramTiny[41]; break;
  case 0x3CB: aReturn= gb_portramTiny[42]; break;  
  case 0x3CC: aReturn= gb_portramTiny[43]; break;
  case 0x3D4: aReturn= gb_portramTiny[fast_tiny_port_0x3D4]; break; //CGA
  case 0x3D5: aReturn= gb_portramTiny[45]; break;
  case 0x3D8: aReturn= gb_portramTiny[fast_tiny_port_0x3D8]; break; //video CGA
  case 0x3D9: aReturn= gb_portramTiny[fast_tiny_port_0x3D9]; break; //CGA  
  case 0x3DA: aReturn= gb_portramTiny[48]; break;
  case 0x3F2: aReturn= gb_portramTiny[49]; break;
  case 0x3FB: aReturn= gb_portramTiny[50]; break;
  //case 0x3CE: gb_portramTiny[0]; break; //solo VGA
  default:
   //printf("ReadTinyPortRAM 0x%04X\n",numPort);
   //fflush(stdout);
   break;
 }
 return aReturn;
}



//********************************************************************
unsigned char GetId_port_write_callback(unsigned short int numPort)
{
 if (numPort<=0x0F) 
  return 0; //out8237            
 if (numPort>=0x20 && numPort<=0x21)
  return 1; //out8259  
 if (numPort>=0x40 && numPort<=0x43)    
  return 2; //out8253
 if (numPort>=0x80 && numPort<=0x8F)
  return 3; //out8237
 if (numPort>=0x3B0 && numPort<=0x3DA)
  return 4; //outVGA
  
 return 0xFF;          
}

//********************************************************************
unsigned char GetId_port_read_callback(unsigned short int numPort)
{         
 if (numPort<=0x0F) 
  return 0; //in8237            
 if (numPort>=0x20 && numPort<=0x21)
  return 1; //in8259  
 if (numPort>=0x40 && numPort<=0x43)    
  return 2; //in8253
 if (numPort>=0x80 && numPort<=0x8F)
  return 3; //in8237
 if (numPort>=0x3B0 && numPort<=0x3DA)
  return 4; //inVGA
  
 return 0xFF; 
}