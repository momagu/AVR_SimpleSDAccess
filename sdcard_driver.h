

#ifndef sdcard_driver_h
#define sdcard_driver_h 

#include <stdint.h>


void sd_card_open(void) ;
void sd_card_close(void);

void sd_file_open(const char* filename);
void sd_file_new(const char* filename);
void sd_file_close(void);
void sd_file_write(const char* line);

void sd_get_Directory(void);
uint8_t sd_get_disk_info(void);


uint16_t 	file_getAnzLine(void);
uint8_t 	file_getLine(uint16_t lineNr,char* zielbuffer);// anzahl Zeilen aktuelles file


#endif
