/*
 * sd_file_access.c
 *
 *  Created on: 21.10.2016
 *      Author: moritz
 */

#include "sd_file_access.h"

#include <stdbool.h>
#include <string.h>
#include "sdcard/partition.h"
#include "sdcard/sd_raw.h"
#include <stdio.h>

struct fields {
	bool isCardOpen :1;
	struct partition_struct* partition;
	struct fat_fs_struct* filesystem;
	struct fat_dir_entry_struct* root_directory_entry;
	struct fat_dir_struct* root_directory;
	int bytes_read;
} fields;

int sd_open_root(void) {
	if (fields.isCardOpen == true)
		return 1;

	// Setup SPI module and port configuration
	if (!sd_raw_init()) {
		return 0;
		// SD initialization failed
	} else {
		fields.isCardOpen = true;
		// open first partition. The parameters are function pointers to the SD card driver.
		fields.partition = partition_open(sd_raw_read, sd_raw_read_interval,
				sd_raw_write, sd_raw_write_interval, 0);

		// If the partition did not open, assume the storage device is a "superfloppy", i.e. has no MBR.
		if (!fields.partition) {
			printf("failed FAT");
			fields.partition = partition_open(sd_raw_read, sd_raw_read_interval,
					sd_raw_write, sd_raw_write_interval, -1);
			if (!fields.partition) {
				//Still failing
				return 0;
			}
		}

		// Open file system.
		fields.filesystem = fat_open(fields.partition);
		if (!fields.filesystem) {
			// Unknown fail
			return 0;
		}

		// Open root directory.
		fields.root_directory_entry = malloc(
				sizeof(struct fat_dir_entry_struct));
		if (!fat_get_dir_entry_of_path(fields.filesystem, "/",
				fields.root_directory_entry)) {
			return 0;
		}

		fields.root_directory = fat_open_dir(fields.filesystem,
				fields.root_directory_entry);
		if (!fields.root_directory) {
			// Unknown fail
			return 0;
		}
	}
	return 1;
}

/*
 * Opens Or creates file
 */
SD_FILE_t* sd_fopen(const char* filename, enum OPEN_MODE_enum mode) {
	if (!sd_open_root()){
		return NULL;
	}
	// Does file exist
	// search through folder
	struct fat_dir_entry_struct* dir_entry = malloc(
			sizeof(struct fat_dir_entry_struct));
	struct fat_dir_struct* parent_entry = fields.root_directory;
	if (!fat_get_dir_entry_of_path(fields.filesystem, filename, dir_entry)) {
		//No file found, create file

		char buf[32 + 1];
		int p = 0;
		while (*filename != '\0') {
			buf[p] = *filename;

			p++;
			filename++;
			if (p >= 32) {
				// Name to long. Ignore rest.
				while (*filename != '\0' && *filename != '/') {
					filename++;
				}
			}
			//Create Folder if necessary
			if (*filename == '/') {

				buf[p] = '\0';


				bool dirExists = false;

				while(fat_read_dir(parent_entry, dir_entry))
				{
					if(strcmp(buf, dir_entry->long_name) == 0)
					{
						parent_entry = fat_open_dir(fields.filesystem, dir_entry);
						dirExists = true;
						break;

					}
				}

				if(!dirExists){
					printf("create directory %s\n", buf);
					if(!fat_create_dir(parent_entry, buf, dir_entry)){
					}

					// parent entry not needed anymore.
					parent_entry = fat_open_dir(fields.filesystem, dir_entry);
				}
				p = 0;
				filename++;
			}

		}
		buf[p] = '\0';
		if (!(fat_create_file(parent_entry, buf, dir_entry))) {
			//Some fail
			return NULL;
		}
	}
	// Open the file.
	struct fat_file_struct* file_struct = fat_open_file(fields.filesystem,
			dir_entry);

	if(!file_struct){
		return NULL;
	}
	SD_FILE_t *file = malloc(sizeof(SD_FILE_t));

	if(!file){
		//HEAP FULL
		return NULL;
	}

	file->file_struct = file_struct;
	file->mode = mode;

	file->read_offset = 0;
	file->write_offset = 0;

	switch (mode) {
	case OPEN_MODE_R: // R = SET, W = 0,
		fat_seek_file(file_struct, &(file->read_offset), FAT_SEEK_SET);
		break;
	case OPEN_MODE_W: // R = 0, W = SET,

		fat_seek_file(file_struct, &(file->write_offset), FAT_SEEK_SET);
		break;
	case OPEN_MODE_A: // R = 0, W = END,
		fat_seek_file(file_struct, &(file->write_offset), FAT_SEEK_END);
		break;
	case OPEN_MODE_Rp: // R = SET, W = SET,
		fat_seek_file(file_struct, &(file->write_offset), FAT_SEEK_SET);
		fat_seek_file(file_struct, &(file->read_offset), FAT_SEEK_SET);
		break;
	case OPEN_MODE_Wp: // R = SET, W = SET, TRUNCATE
		fat_resize_file(file_struct, 1);
		fat_seek_file(file_struct, &(file->write_offset), FAT_SEEK_SET);
		fat_seek_file(file_struct, &(file->read_offset), FAT_SEEK_SET);
		break;
	case OPEN_MODE_Ap: // R = SET, W = END,
		fat_seek_file(file_struct, &file->read_offset, FAT_SEEK_SET);
		break;
	}
	// Free the allocated mem;
	free(dir_entry);
	return file;

}

void sd_fclose(SD_FILE_t* file) {
	fat_close_file(file->file_struct);
	free(file);

}

void sd_fputc(SD_FILE_t* file, char c) {
	if (file->mode == OPEN_MODE_R) {
		//Reading not allowed in this mode;
		return;
	}
	fat_seek_file(file->file_struct, &(file->write_offset), FAT_SEEK_SET);
	file->write_offset += fat_write_file(file->file_struct, (const uint8_t*) &c,
			1);
}
void sd_fputs(SD_FILE_t* file, char* s, int length) {
	if (file->mode == OPEN_MODE_R) {
		//Reading not allowed in this mode;
		return;
	}
	fat_seek_file(file->file_struct, &(file->write_offset), FAT_SEEK_SET);
	file->write_offset += fat_write_file(file->file_struct, (uint8_t*) s,
			length);
}
void sd_fputcs(SD_FILE_t* file, const char* s) {
	if (file->mode == OPEN_MODE_R) {
		//Reading not allowed in this mode;
		return;
	}
	fat_seek_file(file->file_struct, &(file->write_offset), FAT_SEEK_SET);
	file->write_offset += fat_write_file(file->file_struct, (uint8_t*) s,
			strlen(s));
}

int sd_fgetc(SD_FILE_t* file, char* buf) {
	if (file->mode == OPEN_MODE_W || file->mode == OPEN_MODE_A) {
		//Reading not allowed in these modes;
		return 0;
	}
	fat_seek_file(file->file_struct, &(file->read_offset), FAT_SEEK_SET);
	fields.bytes_read = fat_read_file(file->file_struct, (uint8_t*) buf, 1);
	file->read_offset += fields.bytes_read;
	return fields.bytes_read;
}

int sd_fgets(SD_FILE_t* file, char* buf, int n) {
	if (file->mode == OPEN_MODE_W || file->mode == OPEN_MODE_A) {
		//Reading not allowed in these modes;
		return 0;
	}
	fat_seek_file(file->file_struct, &(file->read_offset), FAT_SEEK_SET);
	fields.bytes_read = fat_read_file(file->file_struct, (uint8_t*) buf, n);
	file->read_offset += fields.bytes_read;
	return fields.bytes_read;
}

// returns: chars read
int sd_readLine(SD_FILE_t* file,char* line_buf, int max){
	return 0;
	char c;
	int space = max; // We need one char for '\0'
	//sd_fgetc returns 0 at EOF

	while(space > 1 && sd_fgetc(file, &c) && c != '\n' && c != '\0' && c != '\r' ){
		*line_buf = c;
		line_buf++;
		space--;
	}
	//Buffer full, everything in it is important!
	if(space == 1)
		line_buf++;

	*line_buf = '\0'; // end String /probably override \n
	return max-space;
}

