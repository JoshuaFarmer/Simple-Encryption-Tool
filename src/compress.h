#ifndef _COMPRESS_
#define _COMPRESS_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#define CHUNK 16384

int compressFile(const char *inputFile, const char *outputFile) {
	FILE *in = fopen(inputFile, "rb");
	FILE *out = fopen(outputFile, "wb");
	if (!in || !out) {
		return -1; // Error opening files
	}

	unsigned char inbuf[CHUNK];
	unsigned char outbuf[CHUNK];

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
		fclose(in);
		fclose(out);
		return -1; // Error initializing zlib
	}

	int flush;
	do {
		strm.avail_in = fread(inbuf, 1, CHUNK, in);
		if (ferror(in)) {
			deflateEnd(&strm);
			fclose(in);
			fclose(out);
			return -1; // Error reading input file
		}
		flush = feof(in) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = inbuf;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = outbuf;
			deflate(&strm, flush);
			size_t have = CHUNK - strm.avail_out;
			fwrite(outbuf, 1, have, out);
		} while (strm.avail_out == 0);

	} while (flush != Z_FINISH);

	deflateEnd(&strm);
	fclose(in);
	fclose(out);
	return 0;
}

int decompressFile(const char *inputFile, const char *outputFile) {
	FILE *in = fopen(inputFile, "rb");
	FILE *out = fopen(outputFile, "wb");
	if (!in || !out) {
		return -1; // Error opening files
	}

	unsigned char inbuf[CHUNK];
	unsigned char outbuf[CHUNK];

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	if (inflateInit(&strm) != Z_OK) {
		fclose(in);
		fclose(out);
		return -1; // Error initializing zlib
	}

	int ret;
	do {
		strm.avail_in = fread(inbuf, 1, CHUNK, in);
		if (ferror(in)) {
			inflateEnd(&strm);
			fclose(in);
			fclose(out);
			return -1; // Error reading input file
		}
		strm.next_in = inbuf;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = outbuf;
			ret = inflate(&strm, Z_NO_FLUSH);
			if (ret < 0) {
					inflateEnd(&strm);
					fclose(in);
					fclose(out);
					return -1; // Error during inflation
			}
			size_t have = CHUNK - strm.avail_out;
			fwrite(outbuf, 1, have, out);
		} while (strm.avail_out == 0);

	} while (ret != Z_STREAM_END);

	inflateEnd(&strm);
	fclose(in);
	fclose(out);
	return 0;
}

#endif
