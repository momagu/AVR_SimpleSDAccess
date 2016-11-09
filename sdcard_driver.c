
#include "sdcard_driver.h"

#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>




// SD Card
#include "sdcard/fat.h"
#include "sdcard/fat_config.h"
#include "sdcard/partition.h"
#include "sdcard/sd_raw.h"
#include "sdcard/sd_raw_config.h"

#define DEBUG 0

struct partition_struct* partition;
struct fat_fs_struct* fs;
struct fat_dir_struct* dd;
struct fat_dir_entry_struct directory;
struct fat_file_struct* fd;
struct fat_dir_entry_struct dir_entry;
struct fat_dir_entry_struct file_entry;

uint8_t	file_open = 0,
		card_open = 0;



uint16_t file_getAnzLine(void)	// anzahl Zeilen aktuelles file
{
	uint16_t counterLines=0;
	if(fd)
	{
		uint8_t buffer[30];
		intptr_t count;
		while((count = fat_read_file(fd, buffer, sizeof(buffer))) > 0)
		{
			for(intptr_t i = 0; i < count; ++i)
			{
				if(buffer[i] == '\r')	{ counterLines++;}
			}
		}
	}
	return counterLines+1;
}

uint8_t file_getLine(uint16_t lineNr,char* zielbuffer)// Anzahl Zeilen aktuelles file
{
	if(fd)
	{
		if (lineNr < 1 ) {return 0;}
		uint8_t     bufferlen			= 30;// maximale Zeilenlaenge von 30 Zeichen
		uint8_t 	arbeitsbuffer[bufferlen];
		uint16_t 	counterLines		= 0;
		int32_t 	pos_after_last_enter= 0;
		uint8_t		count;

		// Anfang der n-ten Zeile suchen und position in "pos_after_last_enter" zurueckgeben
		// es werden alle '\r' gezaehlt bis die Anfangsposition der gesuchten Zeile gefunden ist


		while(counterLines < lineNr-1)
		{
			if(!fat_seek_file(fd, &pos_after_last_enter, FAT_SEEK_SET)) {/* error*/}

			count = fat_read_file(fd, arbeitsbuffer, bufferlen);
			for(intptr_t i = 0; i < count; ++i)
			{
				if(arbeitsbuffer[i] == '\r')
				{
					counterLines++;
					pos_after_last_enter+=i+1;// Zeiger auf Zeichen nach dem letzten Enter
					break;
				}
			}
		}

		// Daten auslesen und in Zielbuffer schreiben
		if(!fat_seek_file(fd, &pos_after_last_enter, FAT_SEEK_SET)) {/* error*/}
		count = fat_read_file(fd, arbeitsbuffer, bufferlen);

		for(intptr_t i=0; i<count; ++i)
		{
			zielbuffer[i] = arbeitsbuffer[i];			
			if(arbeitsbuffer[i] == '\r')	
			{
				zielbuffer[i+1] = 0;
				return i; // return Zeilenl�nge
			}
		}
	}
	return 0;
}

uint32_t strtolong(const char* str)
{
	uint32_t l = 0;
	while(*str >= '0' && *str <= '9')
		l = l * 10 + (*str++ - '0');

	return l;
}

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
	while(fat_read_dir(dd, dir_entry))
	{
		if(strcmp(dir_entry->long_name, name) == 0)
		{
			fat_reset_dir(dd);
			return 1;
		}
	}

	return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
	struct fat_dir_entry_struct file_entry;
	if(!find_file_in_dir(fs, dd, name, &file_entry))
		return 0;

	return fat_open_file(fs, &file_entry);
}

uint8_t print_disk_info(const struct fat_fs_struct* fs)
{

	if(!fs)			return 0;
	struct sd_raw_info disk_info;
	if(!sd_raw_get_info(&disk_info))	return 0;
	//	usart_puts(&USARTD0_data, "#........\r");
	//    usart_puts(&USARTD0_data,"#manuf : 0x"); 	usart_putc_hex(&USARTD0_data,disk_info.manufacturer); 				usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#oem   : "); 		usart_puts(&USARTD0_data,(char*) disk_info.oem); 					usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#prod  : "); 		usart_puts(&USARTD0_data,(char*) disk_info.product); 				usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#rev   : "); 		usart_putc_hex(&USARTD0_data,disk_info.revision); 					usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#serial: 0x"); 	usart_putdw_hex(&USARTD0_data,disk_info.serial); 					usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#date  : "); 		usart_putw_dec(&USARTD0_data,disk_info.manufacturing_month); 		usart_putc(&USARTD0_data,'/');
	//												usart_putw_dec(&USARTD0_data,disk_info.manufacturing_year); 		usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#size  : "); 		usart_putdw_dec(&USARTD0_data,disk_info.capacity / 1024 / 1024);	usart_puts(&USARTD0_data,"MB\r");
	//    usart_puts(&USARTD0_data,"#copy  : "); 		usart_putw_dec(&USARTD0_data,disk_info.flag_copy); 					usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#wr.pr.: "); 		usart_putw_dec(&USARTD0_data,disk_info.flag_write_protect_temp); 	usart_putc(&USARTD0_data,'/');
	//												usart_putw_dec(&USARTD0_data,disk_info.flag_write_protect); 		usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#format: ");		usart_putw_dec(&USARTD0_data,disk_info.format); 					usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#free  : "); 		usart_putdw_dec(&USARTD0_data,fat_get_fs_free(fs)); 				usart_putc(&USARTD0_data,'/');
	//												usart_putdw_dec(&USARTD0_data,fat_get_fs_size(fs)); 				usart_putc(&USARTD0_data,'\r');
	//    usart_puts(&USARTD0_data,"#........\r");

	return 1;	
}


void sd_get_Directory(void)
{	
	//usart_puts(&USARTD0_data,"#........Directory:\r");
	while(fat_read_dir(dd, &dir_entry))
	{
		uint8_t spaces = 20 - strlen(dir_entry.long_name);

		//usart_puts(&USARTD0_data,"#");
		//usart_puts(&USARTD0_data,dir_entry.long_name);
		//usart_putc(&USARTD0_data,dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ');

		//		for (uint8_t i=0; i<spaces; i++)	{usart_putc(&USARTD0_data,' ');}

		//usart_putdw_dec(&USARTD0_data,dir_entry.file_size);
		//usart_puts(&USARTD0_data,"\r");
	}
}	

void sd_card_open(void) //Logging File auf SD-Karte oeffnen und auf das Ende positionieren. 
{
	if (card_open == 1) return;

	//############# setup sd card slot
	if(!sd_raw_init())	
	{
		// MMC/SD initialization failed
	}

	else
	{   
		card_open=1;
		// open first partition
		partition = partition_open(	sd_raw_read,sd_raw_read_interval,sd_raw_write,sd_raw_write_interval,0);

		// If the partition did not open, assume the storage device is a "superfloppy", i.e. has no MBR.
		if(!partition)
		{   
			partition = partition_open(sd_raw_read,sd_raw_read_interval,sd_raw_write,sd_raw_write_interval,-1);
			if(!partition)
			{
#if DEBUG			
				usart_puts(&USARTD0_data,"#opening partition failed\r");
#endif			
			}
		}

		//############# open file system
		fs = fat_open(partition);
		if(!fs)
		{
#if DEBUG		
			usart_puts(&USARTD0_data,"#opening filesystem failed\r");
#endif		
		}

		//############# open root directory
		fat_get_dir_entry_of_path(fs, "/", &directory);

		dd = fat_open_dir(fs, &directory);
		if(!dd)
		{
#if DEBUG	
			usart_puts(&USARTD0_data,"#opening root directory failed\r");
#endif	
		}

#if DEBUG
		print_disk_info(fs);	//print some card information as a boot message
		sd_get_Directory();		//print directory entry
#endif	


		//usart_puts(&USARTD0_data,"#Card open\r");
	}
}

void sd_card_close(void)
{
	if(card_open == 1 && card_open == 0)			// Zur Sicherheit noch mal einen sync, damit ist das FS konsistent
	{
		fat_close_dir(dd);// close directory 
		//		usart_puts(&USARTD0_data,"#close: directory\r");
		fat_close(fs);// close file system 
		//		usart_puts(&USARTD0_data,"#close: file system \r");
		partition_close(partition);// close partition 
		//		usart_puts(&USARTD0_data,"#close: partition\r");
		card_open = 0;		
	}
}  

void sd_file_open(const char* filename)
{
	if(file_open == 1) 
	{
		//		usart_puts(&USARTD0_data,"#File schon offen\r");
		return;
	}

	fd = open_file_in_dir(fs, dd, filename);
	if(!fd)
	{
		//		usart_puts(&USARTD0_data,"#Fehler beim oeffen von: ");
		//		usart_puts(&USARTD0_data,filename);
		//		usart_puts(&USARTD0_data,"\r");
	}
	else
	{
		file_open = 1;
		//		usart_puts(&USARTD0_data,"#File: ");
		//		usart_puts(&USARTD0_data,filename);
		//		usart_puts(&USARTD0_data," geoeffnet\r");
	}

} 

void sd_file_new(const char* filename)
{
	// Nun ein neues File erstellen
	if(!fat_create_file(dd, filename , &file_entry))
	{
		//		usart_puts(&USARTD0_data,"#Fehler beim anlegen von: ");
		//		usart_puts(&USARTD0_data,filename);
		//		usart_puts(&USARTD0_data,"\r");
	}
	else
	{
		//		usart_puts(&USARTD0_data,"#File: ");
		//		usart_puts(&USARTD0_data,filename);
		//		usart_puts(&USARTD0_data," erfolgreich angelegt\r");
		sd_raw_sync();
	}
} 

void sd_file_close(void)
{
	if(file_open == 1)			// Zur Sicherheit noch mal einen sync, damit ist das FS konsistent
	{
		sd_raw_sync();		
		fat_close_file(fd);
		//		usart_puts(&USARTD0_data,"#close: file\r");
		file_open=0;	
	}
} 
//########################################################################################## 
void sd_file_write(const char* line)
{
	if(file_open == 1)
	{
		// Auf das Ende des Files positionieren
		// FAT_SEEK_END und offset=0 heissen, vom Ende der Datei aus 0 Bytes lesen,
		// mit anderen Worten, an das Ende des Files gehen.
		int32_t write_offset = 0;
		fat_seek_file(fd, &write_offset, FAT_SEEK_END);

		uint8_t data_len = strlen(line);
		if(fat_write_file(fd, (uint8_t*) line, data_len) != data_len)
		{
			//			usart_puts(&USARTD0_data,"#error writing to file\r");
			return;
		}	
		sd_raw_sync();
	}
}
//##########################################################################################
uint8_t sd_get_disk_info(void)
{
	return print_disk_info(fs);
}	



