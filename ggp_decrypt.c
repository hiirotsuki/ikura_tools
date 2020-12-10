#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>

#ifdef _WIN32
#define le32toh(x) x
#elif __hpux
#define le32toh(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | \
			(((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
#else
#include <endian.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

void usage(const char *);
void errexit(const char *);

int main(int argc, char *argv[])
{
	FILE *in_file, *out_file, *region_file;
	uint32_t data_offset, data_length, region_offset, region_length = 0, i;
	const uint8_t sig[] = {'G', 'G', 'P', 'F', 'A', 'I', 'K', 'E'};
	uint8_t buf[36], key[8], *png_buf, *region_buf;
	char out_name[PATH_MAX], *fileext;

	if(argc < 2)
		usage(basename(argv[0]));

	if(strlen(argv[1]) > (PATH_MAX - 4)) {
		errexit("filename too long");
	}

	strcpy(out_name, argv[1]);
	fileext = strrchr(out_name, '.');

	if(fileext == NULL || (strcmp(fileext, ".ggp") && strcmp(fileext, ".gg") && \
			strcmp(fileext, ".GGP") && strcmp(fileext, ".GG")))
		errexit("bad file extension");

	if((in_file = fopen(argv[1], "rb")) == 0)
		errexit("failed to open input file");

	if(fread(buf, 1, 36, in_file) != 36)
		errexit("truncated file");

	if(memcmp(buf, sig, 8))
		errexit("invalid image signature");

	/* all is good, continue. */
	fileext[1] = 'p';
	fileext[2] = 'n';
	fileext[3] = 'g';
	fileext[4] = '\0';

	/* keygen */
	for(i = 0; i < 8; i++)
		key[i] = buf[i] ^ buf[i + 12];

	data_offset = le32toh(*(uint32_t*)&(buf[20]));
	data_length = le32toh(*(uint32_t*)&(buf[24]));

	region_offset = le32toh(*(uint32_t*)&(buf[28]));
	region_length = le32toh(*(uint32_t*)&(buf[32]));

	fseek(in_file, data_offset, SEEK_SET);

	if((png_buf = malloc(data_length)) == 0) {
		fclose(in_file);
		errexit("not enough memory");
	}

	if(fread(png_buf, 1, data_length, in_file) != data_length) {
		free(png_buf);
		errexit("truncated file");
	}

	for(i = 0; i < data_length; i++)
		png_buf[i] ^= key[i % 8];

	if((out_file = fopen(out_name, "wb")) == 0) {
		free(png_buf);
		errexit("failed to open output file");
	}

	if(fwrite(png_buf, 1, data_length, out_file) != data_length) {
		free(png_buf);
		errexit("failed writing png");
	}

	fclose(out_file);
	free(png_buf);

	/* all done? */
	if(region_length == 0)
		return 0;

	fileext[1] = 'r';
	fileext[2] = 'e';
	fileext[3] = 'g';
	fileext[4] = 'i';
	fileext[5] = 'o';
	fileext[6] = 'n';
	fileext[7] = '\0';

	if((region_file = fopen(out_name, "wb")) == 0)
		errexit("failed to open region file");

	fseek(in_file, region_offset, SEEK_SET);

	if((region_buf = malloc(region_length)) == 0)
		errexit("not enough memory");

	if(fread(region_buf, 1, region_length, in_file) != region_length) {
		free(region_buf);
		errexit("truncated file");
	}
	fclose(in_file);

	if(fwrite(region_buf, 1, region_length, region_file) != region_length) {
		free(region_buf);
		errexit("couldn't write region file");
	}

	free(region_buf);
	fclose(region_file);
	return 0;
}

void usage(const char *name)
{
	fprintf(stderr, "%s: <input file>\n", name);
	fprintf(stderr, "%s: unpacks Ikura encrypted PNG files\n", name);
	fprintf(stderr, "%s: optionally unpacks region data if found\n", name);
	exit(1);
}

void errexit(const char *err)
{
	fprintf(stderr, "%s\n", err);
	exit(1);
}

