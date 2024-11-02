#include"compress.h"
#include"encrypt.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "compress.h"
#include "encrypt.h"
#define TMPEXTENSION ".uncompressed"

int main(int argc, char* argv[]) {
	// pointless bcs we dont use rand() but o well
	srand(time(NULL));

	if (argc < 3) {
		printf("(bad) EncryptionTool v1.0 by Joshua F.\n");
		printf("Usage: ./EncryptionTool <operation> <input file> <output file> [<IPK>]\n");
		printf("Operations:\n");
		printf("  e: encrypt and compress\n");
		printf("  d: decrypt and decompress\n");
		printf("Info:\n");
		printf("  I.P.K, Items per key, the amount of items before\n  creating a new key. lower for more security\n  and increase for smaller file size(on keys, not output)**\n");
		return 1;
	}

	if (argv[1][0] == 'e' && argc == 5) {
		size_t pathl = strlen(argv[3]);
		char path[pathl + 16];
		strcpy(path, argv[3]);
		snprintf(path, sizeof(path), "%s%s", argv[3], TMPEXTENSION);

		int IPK = 0;
		if (argv[4][0] == 'r')
			IPK = secure_random_byte();
		else
			IPK = strtoul(argv[4], NULL, 0);
		Encrypt(argv[2], path, IPK, argv[3]);
		if (compressFile(path, argv[3]) != 0) {
			fprintf(stderr, "Error compressing encrypted file.\n");
			return 1;
		}
		remove(path);
	} else if (argv[1][0] == 'd' && argc == 4) {
		size_t pathl = strlen(argv[3]);
		char path[pathl + 16];
		strcpy(path, argv[3]);
		snprintf(path, sizeof(path), "%s%s", argv[3], TMPEXTENSION);

		if (decompressFile(argv[2], path) != 0) {
			fprintf(stderr, "Error decompressing file.\n");
			return 1;
		}
		Decrypt(path, argv[3], argv[2]);
		remove(path);
	} else {
		printf("Invalid parameters.\n");
		return 1;
	}

	return 0;
}
