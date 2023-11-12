//Author: ackerman
//Convert IMA disks to .h
//        roms to .h
//        com to .h
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define maxObjects 255
#define max_cad_title 12


unsigned char gb_bufferFile[524288]; //512 KB archivo maximo no deberia pasar de 384 KB
unsigned char gb_global_tipo=0; //0 COM   1 DSK

void listFilesRecursively(char *basePath);
void ShowHelp(void);
void WriteHeadCOM_H(char *cadDefine);
void WriteHeadDSK_H(char *cadDefine);
void WriteHeadFONT_H(char *cadDefine);
void WriteHeadBIOS_H(char *cadDefine);
void WriteHeadROMBASIC_H(char *cadDefine);
void WriteHeadVIDEOROM_H(char *cadDefine);
void WriteHeadSNARARE_H(char *cadDefine);
void WriteFileHEX(unsigned char num,char *cadPath,char *cadFile, char *cadFileSource,char *tipoPath,char *tipo,char *defTipo);
void WriteSizeCOM(void);
int GetSizeFile(char *cadFile);
void InitTitles(void);
void RemoveExt(char *cad);
void ProcesaFicherosCOM(void);
void ProcesaFicherosDSK(void);
void ProcesaFont(void);
void ProcesaBIOS(void);
void ProcesaROMBASIC(void);
void ProcesaVIDEOROM(void);
void ProcesaSNARARE(void);
void WriteArrayHexFile(char *cadFileOrigen, FILE *fileDestino);

FILE *gb_fileWrite = NULL;
char gb_titles[maxObjects][32];
char gb_nameFiles[maxObjects][64];
char gb_nameDir[maxObjects][64];
unsigned char gb_contRom=0;

//***********************************
void RemoveExt(char *cad)
{
 int total= strlen(cad);
 if (total > max_cad_title)
 {
  total= max_cad_title;
  cad[max_cad_title]='\0';
 }
 for (int i=0;i<total;i++)
 {
  if (cad[i]==' ')   
   cad[i] = '_';
  else
  {
   if (cad[i]=='.')
   {
    cad[i]='\0';
    return;
   }
  }
 }
}

//**********************************************
void InitTitles()
{
 for (unsigned char i=0;i<maxObjects;i++)
  gb_titles[i][0]='\0';
}

//**********************************************
void ShowHelp()
{
 printf("TinyFake86 IMA tool\n");
 printf("Author: ackerman\n\n");
}

//**********************************************
int GetSizeFile(char *cadFile)
{
 long aReturn=0;
 FILE* fp = fopen(cadFile,"rb");
 if(fp) 
 {
  fseek(fp, 0 , SEEK_END);
  aReturn = ftell(fp);
  fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file
  fclose(fp);
 }
 return aReturn;
}

//**********************************************
void WriteArrayHexFile(char *cadFileOrigen, FILE *fileDestino)
{
 unsigned char contLine=0;
 FILE *auxWrite = fileDestino;
 FILE *auxRead = NULL;
 long auxSize=0;
 
 auxRead = fopen(cadFileOrigen,"rb");
 printf("Read:%s\n",cadFileOrigen);
 if ((auxRead!=NULL)&&(auxWrite!=NULL))
 {
  auxSize = GetSizeFile(cadFileOrigen);
  printf("Size %d\n",auxSize);
  fread(gb_bufferFile,1,auxSize,auxRead);
  for (long i=0;i<auxSize;i++)
  {
   fprintf(auxWrite,"0x%02X",gb_bufferFile[i]);
   if (i<(auxSize-1)){ fprintf(auxWrite,","); }
   contLine++;
   if (contLine>15)
   {
    contLine=0;
    fprintf(auxWrite,"\n");
   }
  }
  fclose(auxRead);
 }
}

//**********************************************
void WriteFileHEX(unsigned char num,char *cadPath,char *cadFile, char *cadFileSource,char *tipoPath,char *tipo,char *defTipo)
{
 unsigned char contLine=0;
 FILE *auxWrite = NULL;
 FILE *auxRead = NULL;
 long auxSize=0;
 char cadDest[2048];
 char cadOri[2048];
 sprintf(cadOri,"%s\\%s",cadPath,cadFileSource);
 sprintf(cadDest,"output/dataFlash/%s/%s%s.h",tipoPath,tipo,cadFile);
 auxWrite = fopen(cadDest,"w+");
 if (auxWrite!= NULL)
 {
  fprintf(auxWrite,"#ifndef _%s_%s_H\n",defTipo,cadFile);
  fprintf(auxWrite," #define _%s_%s_H\n",defTipo,cadFile);
  //WriteHexData(auxWrite,cadFile);  

  auxRead = fopen(cadOri,"rb");
  if (auxRead!=NULL)
  {
   auxSize = GetSizeFile(cadOri);
   
   fprintf(auxWrite," //%s %s %d bytes\n\n",tipo,cadFile,auxSize);
   fprintf(auxWrite,"const unsigned char gb_%s_%s[]={\n",tipo,cadFile);
      
   //printf ("Tam %d",auxSize);
   fread(gb_bufferFile,1,auxSize,auxRead);
   for (long i=0;i<auxSize;i++)
   {
    fprintf(auxWrite,"0x%02X",gb_bufferFile[i]);
    if (i<(auxSize-1))
     fprintf(auxWrite,",");
    contLine++;
    if (contLine>15)
    {
     contLine=0;
     fprintf(auxWrite,"\n");
    }
   }
   fclose(auxRead);
  }
  
  
  fprintf(auxWrite,"\n};\n");
  fprintf(auxWrite,"#endif\n");
  fclose(auxWrite);
 }
}


//**********************************************
void WriteHeadCOM_H(char *cadDefine)
{//Los 64k
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #include <stddef.h>\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) != 0)
   fprintf(gb_fileWrite," #include \"com/com%s.h\"\n",gb_titles[i]);
 }
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define max_list_com %d\n\n\n",gb_contRom);
 fprintf(gb_fileWrite," //coms\n //Titulos\n");
 fprintf(gb_fileWrite," static const char * gb_list_com_title[max_list_com]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
   fprintf(gb_fileWrite,"  \"\"");
  else 
   fprintf(gb_fileWrite,"  \"%s\"",gb_titles[i]);
  if (i<(gb_contRom-1))
   fprintf(gb_fileWrite,",\n");
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");
 
 fprintf(gb_fileWrite," //SEGMEN 0  1\n");
 fprintf(gb_fileWrite," static const unsigned char gb_list_seg_load[max_list_com]={\n");  
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  fprintf(gb_fileWrite,"  1");
  if (i<(gb_contRom-1)) { fprintf(gb_fileWrite,",\n"); }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n"); 
 
  
 fprintf(gb_fileWrite," //Datos com\n");
 fprintf(gb_fileWrite," static const unsigned char * gb_list_com_data[max_list_com]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
   fprintf(gb_fileWrite,"  NULL");
  else 
   fprintf(gb_fileWrite,"  gb_com_%s",gb_titles[i]);
  if (i<(gb_contRom-1))
   fprintf(gb_fileWrite,",\n");   
 }  
 fprintf(gb_fileWrite,"\n };\n");
 
 fprintf(gb_fileWrite,"\n");
 //fprintf(gb_fileWrite,"#endif\n");
}


//********************************************
void WriteSizeCOM()
{
 char cadDestino[1024];
 int auxSize;
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite," //Tamanio en bytes\n");
 fprintf(gb_fileWrite," static const unsigned short int gb_list_com_size[max_list_com]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
   fprintf(gb_fileWrite,"  \"\"");
  else
  {
   sprintf(cadDestino,"input\\com\\%s",gb_nameFiles[i]);   
   auxSize = GetSizeFile(cadDestino);
   fprintf(gb_fileWrite,"  %d",auxSize);
  }
  if (i<(gb_contRom-1))
   fprintf(gb_fileWrite,",\n");
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");  
}



//**********************************************
void listFilesRecursively(char *basePath)
{     
    char cadFileSource[1000];
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return; // Unable to open directory stream
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            printf("%s\n", dp->d_name);
            
            strcpy(cadFileSource,dp->d_name);
            strcpy(gb_nameFiles[gb_contRom],dp->d_name); //Nombre completo
            RemoveExt(dp->d_name);
            strcpy(gb_titles[gb_contRom],dp->d_name);

            switch (gb_global_tipo)
            {
             case 0: WriteFileHEX(gb_contRom,basePath,dp->d_name,cadFileSource,"com","com","COM"); break;
             case 1: WriteFileHEX(gb_contRom,basePath,dp->d_name,cadFileSource,"dsk","dsk","DSK"); break;
            }
            
                        
            gb_contRom++;
            if (gb_contRom > (maxObjects-1))
             return;
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            listFilesRecursively(path);            
        }
    }
    closedir(dir);
}

//*************************************************
void WriteHeadDSK_H(char *cadDefine)
{
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #include <stddef.h>\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) != 0)
   fprintf(gb_fileWrite," #include \"dsk/dsk%s.h\"\n",gb_titles[i]);
 }
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define max_list_dsk %d\n\n\n",gb_contRom);
 fprintf(gb_fileWrite," //disk\n //Title\n");
 fprintf(gb_fileWrite," static const char * gb_list_dsk_title[max_list_dsk]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
   fprintf(gb_fileWrite,"  \"\"");
  else 
   fprintf(gb_fileWrite,"  \"%s\"",gb_titles[i]);
  if (i<(gb_contRom-1))
   fprintf(gb_fileWrite,",\n");
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");
 
 fprintf(gb_fileWrite," //SIZE DISK FORCE 368640 bytes\n");
 fprintf(gb_fileWrite," static const unsigned int gb_list_dsk_filesize[max_list_dsk]={\n");  
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  fprintf(gb_fileWrite,"  368640");
  if (i<(gb_contRom-1)) { fprintf(gb_fileWrite,",\n"); }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");
 
 fprintf(gb_fileWrite," //DISK CYL FORCE 40\n");
 fprintf(gb_fileWrite," static const unsigned char gb_list_dsk_cyls[max_list_dsk]={\n");  
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  fprintf(gb_fileWrite,"  40");
  if (i<(gb_contRom-1)) { fprintf(gb_fileWrite,",\n"); }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n"); 
 
 fprintf(gb_fileWrite," //DISK SECTOR FORCE 9\n");
 fprintf(gb_fileWrite," static const unsigned char gb_list_dsk_sects[max_list_dsk]={\n");  
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  fprintf(gb_fileWrite,"  9");
  if (i<(gb_contRom-1)) { fprintf(gb_fileWrite,",\n"); }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");
 
 fprintf(gb_fileWrite," //DISK HEAD FORCE 2\n");
 fprintf(gb_fileWrite," static const unsigned char gb_list_dsk_heads[max_list_dsk]={\n");  
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  fprintf(gb_fileWrite,"  2");
  if (i<(gb_contRom-1)) { fprintf(gb_fileWrite,",\n"); }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n"); 
 
  
 fprintf(gb_fileWrite," //Data disk\n");
 fprintf(gb_fileWrite," static const unsigned char * gb_list_dsk_data[max_list_dsk]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
   fprintf(gb_fileWrite,"  NULL");
  else 
   fprintf(gb_fileWrite,"  gb_dsk_%s",gb_titles[i]);
  if (i<(gb_contRom-1))
   fprintf(gb_fileWrite,",\n");   
 }  
 fprintf(gb_fileWrite,"\n };\n");
 
 fprintf(gb_fileWrite,"\n");
 //fprintf(gb_fileWrite,"#endif\n");     
}


//*************************************************
void WriteHeadFONT_H(char *cadDefine)
{
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," //asciivga 32768 bytes asciivga.dat\n");
 fprintf(gb_fileWrite," #include \"../../gbGlobals.h\"\n");

 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define gb_size_fon_asciivga 32768\n\n");
 fprintf(gb_fileWrite," const unsigned char fontcga[gb_size_fon_asciivga]={\n");
}

//*************************************************
void WriteHeadBIOS_H(char *cadDefine)
{
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," //BIOS 8192 bytes pcxtbios.bin\n");

 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define gb_size_rom_bios_pcxt 8192\n\n");
 fprintf(gb_fileWrite," const unsigned char gb_bios_pcxt[]={\n");
}

//*************************************************
void WriteHeadROMBASIC_H(char *cadDefine)
{
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," //BASIC 32768 bytes rombasic.bin\n");

 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define gb_size_rom_basic 32768\n\n");
 fprintf(gb_fileWrite," const unsigned char gb_rom_basic[]={\n");     
}

//*************************************************
void WriteHeadVIDEOROM_H(char *cadDefine)
{
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," //VIDEO ROM 32768 bytes videorom.bin\n");

 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define gb_size_rom_videorom 32768\n\n");
 fprintf(gb_fileWrite," const unsigned char gb_rom_videorom[]={\n");     
}

//*************************************************
void WriteHeadSNARARE_H(char *cadDefine)
{
 if (gb_fileWrite == NULL)
  return;
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine);
 fprintf(gb_fileWrite,"\n\n");
 fprintf(gb_fileWrite," #include <stddef.h>\n\n");
 fprintf(gb_fileWrite," #define max_list_snarare 0\n\n");
 fprintf(gb_fileWrite," //Titulos\n");
 fprintf(gb_fileWrite," static const char * gb_list_snarare_title[max_list_snarare]={\n");
 fprintf(gb_fileWrite," };\n\n");
 fprintf(gb_fileWrite," //Datos\n");
 fprintf(gb_fileWrite," static const unsigned char * gb_list_snarare_data[max_list_snarare]={\n");
 fprintf(gb_fileWrite," };\n\n");  
}

//*************************************************
void ProcesaFicherosCOM()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/gbcom.h","w+");
 if (gb_fileWrite!= NULL)
 {
  listFilesRecursively("input/com");
  WriteHeadCOM_H("_GB_COM_H");
  WriteSizeCOM();
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite);
 }     
}

//*************************************************
void ProcesaFicherosDSK()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/gbdsk.h","w+");
 if (gb_fileWrite!= NULL)
 {
  listFilesRecursively("input/dsk");
  WriteHeadDSK_H("_GB_DSK_H"); //Fuerza a 368640 bytes 40 9 2
  //WriteSizeDSK(); //Fuerza a 368640 bytes 40 9 2
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite);
 }     
}

//*************************************************
void ProcesaFont()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/font/fontasciivga.h","w+");     
 if (gb_fileWrite!= NULL)
 {
  WriteHeadFONT_H("_GB_FONT_ASCIIVGA_H");  
  WriteArrayHexFile("input/font/fontasciivga.dat",gb_fileWrite);
  fprintf(gb_fileWrite,"\n };\n\n");
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite); 
 }
}

//*************************************************
void ProcesaBIOS()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/bios/biospcxt.h","w+");     
 if (gb_fileWrite!= NULL)
 {
  WriteHeadBIOS_H("_GB_BIOS_PCXTBIOS_H");  
  WriteArrayHexFile("input/bios/biospcxt.bin",gb_fileWrite);
  fprintf(gb_fileWrite,"\n };\n\n");
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite); 
 }     
}

//*************************************************
void ProcesaROMBASIC()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/rom/rombasic.h","w+");     
 if (gb_fileWrite!= NULL)
 {
  WriteHeadROMBASIC_H("_GB_ROM_BASIC_H");  
  WriteArrayHexFile("input/rom/rombasic.bin",gb_fileWrite);
  fprintf(gb_fileWrite,"\n };\n\n");
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite); 
 }          
}

//*************************************************
void ProcesaVIDEOROM()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/rom/videorom.h","w+");     
 if (gb_fileWrite!= NULL)
 {
  WriteHeadVIDEOROM_H("_GB_ROM_VIDEOROM_H");  
  WriteArrayHexFile("input/rom/videorom.bin",gb_fileWrite);
  fprintf(gb_fileWrite,"\n };\n\n");
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite); 
 }               
}

//*************************************************
void ProcesaSNARARE()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/gbsnarare.h","w+");     
 if (gb_fileWrite!= NULL)
 {
  WriteHeadSNARARE_H("_GB_SNARARE_H");
  fprintf(gb_fileWrite,"\n\n");
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite); 
 }                    
}





//********
//* MAIN *
//********
int main(int argc, char**argv)
{
 char path[100];    //Directory path to list files
 ShowHelp();
  
 printf("COM FILE PROCESS:\n"); //Procesar fichero COM
 gb_global_tipo= 0; //COM tipo
 InitTitles(); 
 ProcesaFicherosCOM();
  
 printf("\nDSK FILE PROCESS:\n"); //Procesar discos
 gb_global_tipo= 1; //DSK tipo
 InitTitles(); 
 ProcesaFicherosDSK();
  
 printf("\nFONT fontasciivga FILE PROCESS:\n"); //Procesar font
 ProcesaFont();
 
 printf("\nBIOS biospcxt FILE PROCESS:\n"); //Procesar bios
 ProcesaBIOS();

 printf("\nBIOS rombasic FILE PROCESS:\n"); //Procesar rombasic
 ProcesaROMBASIC();
 
 printf("\nBIOS videorom FILE PROCESS:\n"); //Procesar videorom
 ProcesaVIDEOROM();
 
 printf("\nSNARARE FILE PROCESS:\n"); //Procesar SNA por encima de RAM requerido
 ProcesaSNARARE();
 
 
 //printf("Enter path to list files: ");    // Input path from user
 //scanf("%s", path);
 //listFilesRecursively(path);
 
 return 0;
}
