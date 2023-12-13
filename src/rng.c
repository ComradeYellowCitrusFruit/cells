#include <x86intrin.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#else
#include <bcrypt.h>
#endif
#include "include/rng.h"

#define ROL(x, a) ((x << a) | (x >> (32 - a)))
#define ROR(x, b) ((x >> a) | (x << (32 - a)))

#define QR(a, b, c, d)(		\
	b ^= ROTL(a + d, 7),	\
	c ^= ROTL(b + a, 9),	\
	d ^= ROTL(c + b, 13),	\
	a ^= ROTL(d + c, 18))

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

/* aes for fun */
static void aes(struct rng_generator *g) 
{
	/* Lookup table instead of actual function because speed and memory */
	static const uint8_t rcon[] = { 
		0x01, 0x02, 0x04, 0x80, 0x10, 
		0x20, 0x40, 0x80, 0x1b, 0x36 
	};

	/* intel intrinsics make our lives easier */
	__m128i key = _mm_load_si128(g);
	for(int i = 0; i < 8; i += 2, g->ctr++) {
		__m128i ctr = _mm_load_si128(&g->iv);
		__m128i tmp;

		for(int j = 0; j < 9; j++)
			ctr = _mm_aesenc_si128(
				ctr, 
				_mm_aeskeygenassist_si128(key, rcon[j])
			);

		ctr = _mm_aesenclast_si128(
			ctr, 
			_mm_aeskeygenassist_si128(key, rcon[9])
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
		104, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != STATUS_SUCCESS) {
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
		*  I also thought about using RC4, A5 (GSM), and Blowfish
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