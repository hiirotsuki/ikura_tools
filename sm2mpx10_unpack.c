/*  sm2mpx10 archive unpacking utility                                              */
/*  Copyright (C) 2020  Ørjan Malde                                                 */
/*                                                                                  */
/*  This program is free software; you can redistribute it and/or                   */
/*  modify it under the terms of the GNU General Public License                     */
/*  as published by the Free Software Foundation; either version 2                  */
/*  of the License, or (at your option) any later version.                          */
/*                                                                                  */
/*  This program is distributed in the hope that it will be useful,                 */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/*  GNU General Public License for more details.                                    */
/*                                                                                  */
/*  You should have received a copy of the GNU General Public License               */
/*  along with this program; if not, write to the Free Software                     */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct
{
	uint8_t magic[8]; /* SM2MPX10 */
	uint32_t file_num;
	uint32_t index_size;
	char archive_name[12];
	uint32_t header_size; /* == 32 bytes */
} sm2mpx10_header;

typedef struct
{
	char name[12];
	uint32_t offset;
	uint32_t size;
} entry;
#pragma pack(pop)

int main(int argc, char *argv[])
{
	sm2mpx10_header *header;
	uint32_t i, j;
	FILE *archive_file, *out_file;
	uint8_t *header_buf, *filelist_buf;
	const uint8_t sig[] = {'S','M','2','M','P','X','1','0'};
	char tmpname[13];

	if(argc < 2)
		exit(1);

	header_buf = malloc(sizeof(sm2mpx10_header));

	if(!header_buf) {
		puts("malloc failure.");
		exit(1);
	}

	archive_file = fopen(argv[1], "rb");

	fread(header_buf, sizeof(sm2mpx10_header), 1, archive_file);

	if(memcmp(sig, header_buf, 8)) {
		puts("not an ikura archive");
		exit(1);
	}

	header = (sm2mpx10_header *)header_buf;

#if 0
	printf("files: %u\nindex_size: %u\narchive name: %s\nheader size: %u\n", header->file_num, \
		header->index_size, header->archive_name, header->header_size);
#endif

	filelist_buf = malloc(header->file_num * sizeof(entry));

	if(!filelist_buf) {
		puts("malloc failure");
		exit(1);
	}

	fread(filelist_buf, sizeof(entry), header->file_num, archive_file);

	for(i = 0; i < header->file_num; i++) {
		entry *e = (entry *)(filelist_buf + (i * sizeof(entry)));

#if 0
		printf("name: %.12s, size: %u, offset: %u\n", e->name, e->size, e->offset);
#endif

		/* sigh... */
		memcpy(tmpname, e->name, 12);
		tmpname[13] = '\0';

		/* write each file */
		out_file = fopen(tmpname, "wb");
		fseek(archive_file, e->offset, SEEK_SET);

		for(j = 0; j < e->size; j++) {
			(void)fputc(fgetc(archive_file), out_file);
		}
		fclose(out_file);
	}

	fclose(archive_file);
	free(filelist_buf);
	free(header_buf);
	return 0;
}
