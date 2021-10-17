#ifndef _PORTS_H
 #define _PORTS_H
 
 //JJ extern void set_port_write_redirector_16 (uint16_t startport, uint16_t endport, void *callback);
 extern void set_port_write_redirector (unsigned short int startport, unsigned short int endport, void *callback);
 extern void set_port_read_redirector (unsigned short int startport, unsigned short int endport, void *callback);

 void WriteTinyPortRAM(unsigned short int numPort, unsigned char aValue);
 unsigned char ReadTinyPortRAM(unsigned short int numPort);

 unsigned char GetId_port_write_callback(unsigned short int numPort);
 unsigned char GetId_port_read_callback(unsigned short int numPort);
        
 //JJ extern void set_port_write_redirector_16 (unsigned short int startport, unsigned short int endport, void *callback);
 //JJ extern void set_port_read_redirector_16 (unsigned short int startport, unsigned short int endport, void *callback);
 
#endif
