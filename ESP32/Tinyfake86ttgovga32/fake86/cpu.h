//
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
#ifndef _CPU_H
 #define _CPU_H

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define regax 0
#define regcx 1
#define regdx 2
#define regbx 3
#define regsp 4
#define regbp 5
#define regsi 6
#define regdi 7
#define reges 0
#define regcs 1
#define regss 2
#define regds 3

//JJ #ifdef __BIG_ENDIAN__
//JJ #define regal 1
//JJ #define regah 0
//JJ #define regcl 3
//JJ #define regch 2
//JJ #define regdl 5
//JJ #define regdh 4
//JJ #define regbl 7
//JJ #define regbh 6
//JJ #else
#define regal 0
#define regah 1
#define regcl 2
#define regch 3
#define regdl 4
#define regdh 5
#define regbl 6
#define regbh 7
//JJ #endif

union _bytewordregs_ {
	uint16_t wordregs[8];
	uint8_t byteregs[8];
};

#define StepIP(x)	ip += x
#define getmem8(x, y)	read86(segbase(x) + y)
#define getmem16(x, y)	readw86(segbase(x) + y)
#define putmem8(x, y, z)	write86(segbase(x) + y, z)
#define putmem16(x, y, z)	writew86(segbase(x) + y, z)
#define signext(value)	(int16_t)(int8_t)(value)
#define signext32(value)	(int32_t)(int16_t)(value)
#define getreg16(regid)	regs.wordregs[regid]
#define getreg8(regid)	regs.byteregs[byteregtable[regid]]
#define putreg16(regid, writeval)	regs.wordregs[regid] = writeval
#define putreg8(regid, writeval)	regs.byteregs[byteregtable[regid]] = writeval
#define getsegreg(regid)	segregs[regid]
#define putsegreg(regid, writeval)	segregs[regid] = writeval
#define segbase(x)	((uint32_t) x << 4)


unsigned char GetRegAL(void);
unsigned char GetRegAH(void);
unsigned char GetRegBL(void);
unsigned char GetRegBH(void);
unsigned char GetRegCL(void);
unsigned char GetRegCH(void);
unsigned char GetRegDL(void);
unsigned char GetRegDH(void);

unsigned short int GetRegCS(void);
unsigned short int GetRegIP(void);
unsigned short int GetRegDS(void);
unsigned short int GetRegSS(void);
unsigned short int GetRegES(void);

unsigned short int GetRegSP(void);
unsigned short int GetRegBP(void);
unsigned short int GetRegSI(void);
unsigned short int GetRegDI(void);

unsigned short int GetCF(void);

void intcall86 (uint8_t intnum);



void SetRegAL(unsigned char a);
void SetRegAH(unsigned char a);
void SetRegBL(unsigned char a);
void SetRegBH(unsigned char a);
void SetRegCL(unsigned char a);
void SetRegCH(unsigned char a);
void SetRegDL(unsigned char a);
void SetRegDH(unsigned char a);

void SetRegCS(unsigned short int a);
void SetRegIP(unsigned short int a);
void SetRegDS(unsigned short int a);
void SetRegSS(unsigned short int a);
void SetRegES(unsigned short int a);

void SetRegSP(unsigned short int a);
void SetRegBP(unsigned short int a);
void SetRegSI(unsigned short int a);
void SetRegDI(unsigned short int a);

void SetCF(unsigned short int a);

unsigned char read86 (unsigned int addr32);
void write86 (unsigned int addr32, unsigned char value);
void reset86(void);
void exec86 (uint32_t execloops);
void FuerzoParityRAM(void);

void bootstrapPoll(void);
void SetRAMTruco(void);
void ExternalSetCF(unsigned char valor);

void my_callback_speaker_func(void);

#ifdef use_lib_sna_rare
 void jj_write86_remap(unsigned int addr32, unsigned char value);
 unsigned char jj_read86_remap(unsigned int addr32);
#endif 

#ifndef use_lib_fast_push
 void push (unsigned short int pushval);
#endif

#ifndef use_lib_fast_pop
 unsigned short int pop(void);
#endif

#ifndef use_lib_fast_op_add16
 void op_add16(void);
#endif

#ifndef use_lib_fast_op_sub16
 void op_sub16(void);
#endif

#ifndef use_lib_fast_op_and8
 void op_and8(void);
#endif

#ifndef use_lib_fast_op_add8
 void op_add8(void);
#endif

#ifndef use_lib_fast_op_writew86
 void writew86 (unsigned int addr32, unsigned short int value);
#endif

#ifndef use_lib_fast_op_xor8
 void op_xor8(void);
#endif

#ifndef use_lib_fast_op_or16
 void op_or16(void);
#endif

#ifndef use_lib_fast_op_or8
 void op_or8(void);
#endif

#ifndef use_lib_fast_op_and16
 void op_and16(void);
#endif

#ifndef use_lib_fast_flag_log8
 void flag_log8 (unsigned char value);
#endif

#ifndef use_lib_fast_flag_log16
 void flag_log16 (unsigned short int value);
#endif

#ifndef use_lib_fast_flag_adc8
 void flag_adc8 (unsigned char v1, unsigned char v2, unsigned char v3);
#endif 

#ifndef use_lib_fast_flag_adc16
 void flag_adc16 (uint16_t v1, uint16_t v2, uint16_t v3);
#endif

#ifndef use_lib_fast_op_xor16
 void op_xor16(void);
#endif

#ifndef use_lib_fast_readw86
 unsigned short int readw86 (unsigned int addr32);
#endif

#ifndef use_lib_fast_flag_szp8
 void flag_szp8(unsigned char value);
#endif

#ifndef use_lib_fast_flag_szp16
 void flag_szp16(unsigned short int value);
#endif

#ifndef use_lib_fast_op_adc8
 void op_adc8(void);
#endif

#ifndef use_lib_fast_op_adc16
 void op_adc16(void);
#endif

#ifndef use_lib_fast_op_sbb8
 void op_sbb8(void)
#endif

#ifndef use_lib_fast_op_sbb16
 void op_sbb16(void);
#endif 

#ifndef use_lib_fast_modregrm
 void modregrm(void);
#endif

#ifndef use_lib_fast_readrm16
 unsigned short int readrm16 (unsigned char rmval);
#endif

#ifndef use_lib_fast_readrm8
 unsigned char readrm8 (unsigned char rmval);
#endif

#ifndef use_lib_fast_writerm16
 void writerm16 (unsigned char rmval, unsigned short int value);
#endif

#ifndef use_lib_fast_writerm8
 void writerm8 (unsigned char rmval, unsigned char value);
#endif 

#ifndef use_lib_fast_op_div8
 void op_div8 (unsigned short int valdiv, unsigned char divisor);
#endif 

#endif

