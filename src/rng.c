/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#include <stdint.h>
#include <wmmintrin.h> 
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#else

#define WIN32_LEAN_AND_MEAN

#ifndef WINDOWS_H
#define WINDOWS_H <Windows.h>
#endif

#include WINDOWS_H
#include <bcrypt.h>
#endif

#include "include/rng.h"

#define ROL(x, a) (((x) << (a)) | ((x) >> (32 - (a))))
#define ROR(x, b) (((x) >> (a)) | ((x) << (32 - (a))))

#define QR(a, b, c, d)(		\
	b ^= ROL(a + d, 7),	\
	c ^= ROL(b + a, 9),	\
	d ^= ROL(c + b, 13),	\
	a ^= ROL(d + c, 18))

struct rng_generator {
	uint64_t seed[2];
	uint64_t iv;
	uint64_t ctr;
	union {
		uint64_t state[8]; /* 512/64 */
	uint8_t bytes[64]; /* 512/8 */
	};
	uint64_t byte_ctr;
};

static uint32_t aes_subword(uint32_t w) {
	/* Lookup table */
	static uint8_t sbox[] = {
		0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 
		0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
		0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 
		0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
		0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 
		0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
		0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 
		0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
		0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 
		0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
		0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 
		0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
		0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 
		0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
		0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 
		0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
		0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 
		0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
		0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 
		0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
		0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 
		0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
		0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 
		0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
		0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 
		0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
		0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 
		0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
		0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 
		0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
		0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 
		0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
	};

	return  sbox[w & 0xff] | 
		(sbox[(w >> 8) & 0xff] << 8) |
		(sbox[(w >> 16) & 0xff] << 16) | 
		(sbox[(w >> 24) & 0xff] << 24);
}

/* aes round keygen */
static void aes_keygen(__m128i key, uint32_t *keys) 
{
	/*  Lookup table instead of actual function because speed and memory 
	*   Also the actual function is really weird
	*   bitshift by 24 cause thats how its actually used 
	*/
	static const uint32_t rcon[] = { 
		0x01 << 24, 0x02 << 24, 0x04 << 24, 0x80 << 24, 0x10 << 24, 
		0x20 << 24, 0x40 << 24, 0x80 << 24, 0x1b << 24, 0x36 << 24
	};
	
	/* Optimize out the first round key bits */
	_mm_store_si128(keys, key);
	
	for(int i = 4; i < 44; i++) {
		if(i % 4 == 0) 
			keys[i] = keys[i-4] ^ aes_subword(ROL(keys[i-1], 8)) ^ rcon[i/4];
		else
		 	keys[i] = keys[i-4] ^ keys[i-1];
	}
}

/* aes for fun */
static void aes(struct rng_generator *g) 
{
	/* intel intrinsics make our lives easier */
	__m128i key = _mm_load_si128(g);
	for(int i = 0; i < 8; i += 2, g->ctr++) {
		__m128i ctr = _mm_load_si128(&g->iv);
		__m128i tmp;
		uint32_t keybuf[4*11]; /* 4R */
		aes_keygen(key, keybuf);

		ctr = _mm_aesenc_si128(
			ctr, 
			_mm_aeskeygenassist_si128(key, 0x01)
		);

		for(int k = 0; k < 10; k++) {
			tmp = _mm_load_si128(&keybuf[i * 4]);

			if(k == 9)
				ctr = _mm_aesenclast_si128(ctr, tmp);
			else
			 	ctr = _mm_aesenc_si128(ctr, tmp);
		}

		ctr = _mm_aesenclast_si128(
			ctr, 
			_mm_aeskeygenassist_si128(key, 0x36)
		);

		tmp = _mm_load_si128(&g->state[i]);
		_mm_store_si128(&g->state[i], _mm_xor_si128(tmp, ctr));
	}
}

/* salsa20 because it has less diffusion */
static void salsa20(struct rng_generator *g) {
	/* hard to understand but this creates the matrix, using bitmath 
	*  to avoid using pointer magic. use only 128 bits of key to
	*  keep diffusion lower
	*  that probably also has some weird side-effects but we'll see
	*/

	for(int i = 0; i < 8; i += 4, g->ctr++) {
		uint32_t key[] = {
			0x65787061, 
			(uint32_t)(g->seed[0] >> 32),
			(uint32_t)(g->seed[0] & 0xffffffff), 
			(uint32_t)(g->seed[1] >> 32), 
			0x6e642033,
			(uint32_t)(g->seed[1] & 0xffffffff),
			(uint32_t)(g->iv >> 32), 
			(uint32_t)(g->iv & 0xffffffff),
			(uint32_t)(g->ctr >> 32), 
			(uint32_t)(g->ctr & 0xffffffff),
			0x322d6279, 
			(uint32_t)(g->seed[0] >> 32),
			(uint32_t)(g->seed[0] & 0xffffffff),
			(uint32_t)(g->seed[1] >> 32), 
			(uint32_t)(g->seed[1] & 0xffffffff), 
			0x7465206b
		};

		QR(key[0], key[4], key[8], key[12]);
		QR(key[5], key[9], key[13], key[1]);
		QR(key[10], key[14], key[2], key[6]);
		QR(key[15], key[3], key[7], key[11]);

		QR(key[0], key[1], key[2], key[3]);
		QR(key[5], key[6], key[7], key[4]);
		QR(key[10], key[11], key[8], key[9]);
		QR(key[15], key[12], key[13], key[14]);

		for(int j = 0; j < 16; j += 4) {
			int idx = j ? i : i + (j / 2);
			__m128i x = _mm_load_si128(&key[j]);
			__m128i y = _mm_load_si128(&g->state[idx]);

			_mm_store_si128(&g->state[idx], _mm_xor_si128(x, y));
		}
	}
}

generator_handle new_generator() 
{
	generator_handle gen = malloc(sizeof(struct rng_generator));
	reseed(gen);
	return gen;
}

void free_generator(generator_handle h) {
	free(h);
}

void reseed(generator_handle h) {
#ifndef _WIN32
	int urand = open("/dev/urandom", O_RDONLY);
	int res;

	if(urand == -1) 
		exit(errno);

	do {
		res = read(urand, h, 104);
	} while(res != 104);
#else
	if(BCryptGenRandom(NULL, h, 
		104, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
		exit(-1);
	}
#endif

	h->byte_ctr = 0;
	h->ctr      = 0;

	aes(h);
}

void refresh(generator_handle h) {
	/* every ~16,777,216 bytes, use aes instead of salsa20 */
	if(((h->ctr << 8) >> 24) > 1) {
		uint32_t a = 0xdeadbeef, b = 0x6675636b, c = ~h->iv;
		aes(h);
		QR(h->iv, a, b, c); /* scramble the iv */
	} else {
		/* use salsa20 to increase the chance of collisions
		*  Initally I was gonna use (3)DES, but I couldn't find a
		*  good description of how it works and thus couldn't
		*  make a decent implementation.
		*  I also thought about using RC4, A5/1 (GSM), and Blowfish
		*  but came across the same problem
		*/

		salsa20(h);
	}
}

uint8_t gen8(generator_handle h) {
	if(h->byte_ctr == 64) {
		refresh(h);
		h->byte_ctr = 0;
	}

	return h->bytes[h->byte_ctr++];
}

uint16_t gen16(generator_handle h) {
	uint16_t ret;
	h->byte_ctr = (h->byte_ctr + 1) & -2;

	if(h->byte_ctr + 2 > 64) {
		refresh(h);
		h->byte_ctr = 0;
	}

	ret = *(uint16_t*)&h->bytes[h->byte_ctr];
	h->byte_ctr += 2;
	return ret;
}

uint32_t gen32(generator_handle h) {
	uint32_t ret;
	h->byte_ctr = (h->byte_ctr + 3) & -4;

	if(h->byte_ctr + 4 > 64) {
		refresh(h);
		h->byte_ctr = 0;
	}

	ret = *(uint32_t*)&h->bytes[h->byte_ctr];
	h->byte_ctr += 4;
	return ret;
}

uint64_t gen64(generator_handle h) {
	uint64_t ret;
	h->byte_ctr = (h->byte_ctr + 7) & -8;

	if(h->byte_ctr + 8 > 64) {
		refresh(h);
		h->byte_ctr = 0;
	}

	ret = h->state[h->byte_ctr >> 3];
	h->byte_ctr += 8;
	return ret;
}

void gen_bytes(generator_handle h, void *data, uint64_t bytes) {
	uint8_t *d = data;
	if(bytes >> 6 > 1) {
		/* pretty much just generate new bytes then copy said bytes
		*  to new buffer in 64 byte blocks, subtracting the bytes left
		*  by 64 each time, if more remain and are less than 64
		*  copy them and increment h->byte_ctr
		*/
		h->byte_ctr = 0;
		refresh(h);

		while(bytes >> 6) {
			memcpy(d, h->bytes, 64);
			d += 64;
			bytes -= 64;
			refresh(h);
		}
	}
	
	switch(bytes) {
		case 0:
		/* since the 64 bytes case will continue to here
		*  even if there are no bytes left to copy
		*  we need this sentinel case
		*/
		return;

		case 1:
		*d = gen8(h);
		return;

		case 2:
		*(uint16_t*)d = gen16(h);
		return;

		case 4:
		*(uint32_t*)d = gen32(h);
		return;

		case 8:
		*(uint64_t*)d = gen64(h);
		return;

		default:
		if(h->byte_ctr + bytes > 64) {
			refresh(h);
			h->byte_ctr = 0;
		}

		memcpy(d, h->bytes, bytes);
		h->byte_ctr += bytes;
	}
}