#ifndef _GB_DSK_H
 #define _GB_DSK_H

 #include <stddef.h>
 #include "dsk/dskpackgames00.h"
 //#include "dsk/dskcompaqdos211madmix.h"
 //#include "dsk/dskcompaqdos211pakupaku.h"
 //#include "dsk/dskdos32.h"
 //#include "dsk/dskcalgames.h"
 //#include "dsk/dskcompaqdos211streetfightingman1de2.h"
 //#include "dsk/dskcompaqdos211solnegro.h"
 //#include "dsk/dsktestdrive.h" //Va con msdos 3.2
 //#include "dsk/dskmsdos300gameover.h"
 //#include "dsk/dskcompaqdos211madmix2.h"
 //#include "dsk/dsk007killcga.h" //No va
 //#include "dsk/dskfacetris.h"
 //#include "dsk/dskcompaqdos211cat.h"
 //#include "dsk/dskcompaqdos211lastmission.h"
 //#include "dsk/dskmonkey0108.h"

 
 //#define max_list_dsk 3
 //#define max_list_dsk 2
 #define max_list_dsk 1
 //#define max_list_dsk 2
 //#define max_list_dsk 0

 //Titulos
 static const char * gb_list_dsk_title[max_list_dsk]={
  "Pack00"
  //"SOL NEGRO"
  //,"Paku Paku"
  //"STREET FIGHTING MAN"
  //,
  //"MADMIX"
  //,"GAMEVOVER"
  //,"DOS 3.2"
  //,"CALL GAMES"
  //,"TEST DRIVE"
  //,
  //"MADMIX2"
  //,"007"
  //,"FACETRIS"
  //,
  //"CAT"
  //,"LAST MISSION"
  //"MONKEY"
 };
   
 //Datos
 static const unsigned char * gb_list_dsk_data[max_list_dsk]={
  gb_dsk_packgames00
  //gb_dsk_solnegro
  //,gb_dsk_pakupaku
  //gb_dsk_streetfightingman1de2
  //gb_dsk_compaq211madmix
  //gb_dsk_msdos300gameover
  //,gb_dsk_dos32
  //,gb_dsk_calgames
  //,gb_dsk_testdrive
  //,
  //gb_dsk_compaq211madmix2
  //,gb_dsk_007killcga
  //,gb_dsk_facetris
  //,
  //gb_dsk_compaq211cat
  //,gb_dsk_compaq211lastmission
  //gb_dsk_monkey0108
 };
 
 static const unsigned int gb_list_dsk_filesize[max_list_dsk]={
  368640
  //368640
  //,368640
  //,368640
  //,368640
 };
 
 static const unsigned char gb_list_dsk_cyls[max_list_dsk]={
  40
  //40
  //,40
  //,40
  //,40
 };
 
 static const unsigned char gb_list_dsk_sects[max_list_dsk]={
  9
  //9
  //,9
  //,9
  //,9
 };
 
 static const unsigned char gb_list_dsk_heads[max_list_dsk]={
  2
  //2
  //,2
  //,2
  //,2
 };
 

 
#endif
