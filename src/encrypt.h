#ifndef _ENCRYPT_
#define _ENCRYPT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/random.h>
#include <unistd.h>
#define EXTENSION ".enckeys"

unsigned char rolo(unsigned char x) {
	return (x << 1) | (x >> 7);
}

unsigned char roro(unsigned char x) {
	return (x >> 1) | (x << 7);
}

unsigned char rol(unsigned char x, unsigned char y) {
	unsigned char o = x;
	while (y) {
		o = rolo(o);
		--y;
	}
	return o;
}

unsigned char ror(unsigned char x, unsigned char y) {
	unsigned char o = x;
	while (y) {
		o = roro(o);
		--y;
	}
	return o;
}

unsigned char secure_random_byte() {
	unsigned char x;
	if (getrandom(&x, sizeof(x), 0) == -1) {
		perror("getrandom");
		exit(EXIT_FAILURE);
	}
	return x;
}

void Encrypt(const char* path, const char* pathOut, unsigned int ItemsPerKey, const char* keyFileName) {
	FILE* f = fopen(path, "rb");
	if (f == NULL) {
		fprintf(stderr, "Error: Failed to open file '%s'\n", path);
		return;
	}

	fseek(f, 0, SEEK_END);
	size_t l = ftell(f);
	fseek(f, 0, SEEK_SET);

	unsigned char* buff = malloc(l);
	if (buff == NULL) {
		fclose(f);
		exit(1);
	}

	size_t bytesRead = fread(buff, 1, l, f);
	if (bytesRead != l) {
		printf("Error reading file.\n");
		fclose(f);
		free(buff);
		return;
	}
	fclose(f);

	f = fopen(pathOut, "wb");
	if (f == NULL) {
		fprintf(stderr, "Error: Failed to open out file '%s'\n", pathOut);
		free(buff);
		return;
	}

	printf("length: %lu\n", l);
	char x = 0;

	size_t pathl = strlen(keyFileName);
	char secrets_path[pathl + 16];
	strcpy(secrets_path, keyFileName);
	snprintf(secrets_path, sizeof(secrets_path), "%s%s", keyFileName, EXTENSION);
	
	FILE* secrets = fopen(secrets_path, "wb");

	size_t suc = fwrite(&ItemsPerKey, sizeof(ItemsPerKey), 1, secrets);
	if (suc != 1) {
		printf("Error writing IPK.\n");
		fclose(f);
		fclose(secrets);
		free(buff);
		return;
	}

	for (size_t i = 0, c=0; i < l; ++i,++c) {
		if (c > ItemsPerKey) {
			x = secure_random_byte();
			fprintf(secrets, "%c", x);
			c = 0;
		}
		buff[i] = rol(buff[i], rol(i, x)) ^ x;
	}

	fclose(secrets);

	size_t bytesWritten = fwrite(buff, 1, l, f);
	if (bytesWritten != l) {
		printf("Error writing to output file.\n");
	}
	fclose(f);
	free(buff);
	buff = NULL;
}

void Decrypt(const char* path, const char* pathOut, const char* keyFileName) {
	FILE* f = fopen(path, "rb");
	if (f == NULL) {
		fprintf(stderr, "Error: Failed to open file '%s'\n", path);
		return;
	}

	fseek(f, 0, SEEK_END);
	size_t l = ftell(f);
	fseek(f, 0, SEEK_SET);
 
	unsigned char* buff = malloc(l);
	if (buff == NULL) {
		fclose(f);
		exit(1);
	}

	size_t bytesRead = fread(buff, 1, l, f);
	if (bytesRead != l) {
		printf("Error reading file.\n");
		fclose(f);
		free(buff);
		return;
	}
	fclose(f);

	// Read secrets
	size_t pathl = strlen(keyFileName);
	char secrets_path[pathl + 16];
	strcpy(secrets_path, keyFileName);
	snprintf(secrets_path, sizeof(secrets_path), "%s%s", keyFileName, EXTENSION);

	FILE* secrets = fopen(secrets_path, "rb");
	if (secrets == NULL) {
		printf("Failed to open secrets file.\n");
		free(buff);
		return;
	}

	printf("length: %lu\n", l);
	char x = 0;

	unsigned int ItemsPerKey = 1;
	size_t suc = fread(&ItemsPerKey, sizeof(ItemsPerKey), 1, secrets);
	if (suc != 1) {
		printf("Error reading IPK.\n");
		fclose(secrets);
		free(buff);
		return;
	}

	for (size_t i = 0, c=0; i < l; ++i,++c) {
		if (c > ItemsPerKey) {
			if (fread(&x, 1, 1, secrets) != 1) {
				printf("Error reading secrets file.\n");
				fclose(secrets);
				free(buff);
				return;
			}
			c = 0;
		}
		buff[i] = ror((buff[i] ^ x), rol(i, x));
	}

	fclose(secrets);
	
	f = fopen(pathOut, "wb");
	if (f == NULL) {
		fprintf(stderr, "Error: Failed to open out file '%s'\n", pathOut);
		free(buff);
		return;
	}

	size_t bytesWritten = fwrite(buff, 1, l, f);
	if (bytesWritten != l) {
		printf("Error writing to output file.\n");
	}
	
	fclose(f);
	free(buff);
}

#endif