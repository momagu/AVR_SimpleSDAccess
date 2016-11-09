/*
 * sd_file_access.h
 *
 *  Created on: 21.10.2016
 *      Author: moritz
 */

#ifndef INTERFACE_DRIVERS_SD_CARD_SD_FILE_ACCESS_H_
#define INTERFACE_DRIVERS_SD_CARD_SD_FILE_ACCESS_H_

#include "sdcard/fat.h"

enum OPEN_MODE_enum {
	OPEN_MODE_R, // Read only
	OPEN_MODE_W, // Write. Create new file if non existent
	OPEN_MODE_A, // Writing, appending. Create new File if non existent.
	OPEN_MODE_Rp, // Read and Write
	OPEN_MODE_Wp, // Read Write truncating. This will truncate the file at opening
	OPEN_MODE_Ap, // Read and Write. The reading starts at the beginning, Writing will only be appended.
};

typedef struct SD_FILE_struct {
	struct fat_file_struct* file_struct;
	int32_t write_offset;
	int32_t read_offset;
	enum OPEN_MODE_enum mode;
} SD_FILE_t;

SD_FILE_t* sd_fopen(const char* filename, enum OPEN_MODE_enum mode);

void sd_fclose(SD_FILE_t* file);

void sd_fputc(SD_FILE_t* file, char c);
void sd_fputs(SD_FILE_t* file, char* s, int length);
void sd_fputcs(SD_FILE_t* file, const char* s);

int sd_fgetc(SD_FILE_t* file, char* buf);
int sd_fgets(SD_FILE_t* file, char* buf, int n);

int sd_readLine(SD_FILE_t* file,char* line_buf, int max);

#endif /* INTERFACE_DRIVERS_SD_CARD_SD_FILE_ACCESS_H_ */
