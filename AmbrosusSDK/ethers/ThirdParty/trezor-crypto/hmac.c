/**
 * Copyright (c) 2013-2014 Tomas Dzetkulic
 * Copyright (c) 2013-2014 Pavol Rusnak
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>

#include "hmac.h"
#include "macros.h"

void hmac_sha256_Init(_HMAC_SHA256_CTX *hctx, const uint8_t *key, const uint32_t keylen)
{
	uint8_t i_key_pad[SHA256_BLOCK_LENGTH];
	memset(i_key_pad, 0, SHA256_BLOCK_LENGTH);
	if (keylen > SHA256_BLOCK_LENGTH) {
		sha256_Raw(key, keylen, i_key_pad);
	} else {
		memcpy(i_key_pad, key, keylen);
	}
	for (int i = 0; i < SHA256_BLOCK_LENGTH; i++) {
		hctx->o_key_pad[i] = i_key_pad[i] ^ 0x5c;
		i_key_pad[i] ^= 0x36;
	}
	sha256_Init(&(hctx->ctx));
	sha256_Update(&(hctx->ctx), i_key_pad, SHA256_BLOCK_LENGTH);
	MEMSET_BZERO(i_key_pad, sizeof(i_key_pad));
}

void hmac_sha256_Update(_HMAC_SHA256_CTX *hctx, const uint8_t *msg, const uint32_t msglen)
{
	sha256_Update(&(hctx->ctx), msg, msglen);
}

void hmac_sha256_Final(_HMAC_SHA256_CTX *hctx, uint8_t *hmac)
{
	uint8_t hash[SHA256_DIGEST_LENGTH];
	sha256_Final(&(hctx->ctx), hash);
	sha256_Init(&(hctx->ctx));
	sha256_Update(&(hctx->ctx), hctx->o_key_pad, SHA256_BLOCK_LENGTH);
	sha256_Update(&(hctx->ctx), hash, SHA256_DIGEST_LENGTH);
	sha256_Final(&(hctx->ctx), hmac);
	MEMSET_BZERO(hash, sizeof(hash));
	MEMSET_BZERO(hctx, sizeof(_HMAC_SHA256_CTX));
}

void hmac_sha256(const uint8_t *key, const uint32_t keylen, const uint8_t *msg, const uint32_t msglen, uint8_t *hmac)
{
	_HMAC_SHA256_CTX hctx;
	hmac_sha256_Init(&hctx, key, keylen);
	hmac_sha256_Update(&hctx, msg, msglen);
	hmac_sha256_Final(&hctx, hmac);
}

void hmac_sha256_prepare(const uint8_t *key, const uint32_t keylen, uint32_t *opad_digest, uint32_t *ipad_digest)
{
	int i;
	uint32_t buf[SHA256_BLOCK_LENGTH/sizeof(uint32_t)];
	uint32_t o_key_pad[16], i_key_pad[16];

	memset(buf, 0, SHA256_BLOCK_LENGTH);
	if (keylen > SHA256_BLOCK_LENGTH) {
		sha256_Raw(key, keylen, (uint8_t*) buf);
	} else {
		memcpy(buf, key, keylen);
	}

	for (i = 0; i < 16; i++) {
		uint32_t data;
#if BYTE_ORDER == LITTLE_ENDIAN
		REVERSE32(buf[i], data);
#else
		data = buf[i];
#endif
		o_key_pad[i] = data ^ 0x5c5c5c5c;
		i_key_pad[i] = data ^ 0x36363636;
	}

	sha256_Transform(sha256_initial_hash_value, o_key_pad, opad_digest);
	sha256_Transform(sha256_initial_hash_value, i_key_pad, ipad_digest);
}

void hmac_sha512_Init(HMAC_SHA512_CTX *hctx, const uint8_t *key, const uint32_t keylen)
{
	uint8_t i_key_pad[SHA512_BLOCK_LENGTH];
	memset(i_key_pad, 0, SHA512_BLOCK_LENGTH);
	if (keylen > SHA512_BLOCK_LENGTH) {
		sha512_Raw(key, keylen, i_key_pad);
	} else {
		memcpy(i_key_pad, key, keylen);
	}
	for (int i = 0; i < SHA512_BLOCK_LENGTH; i++) {
		hctx->o_key_pad[i] = i_key_pad[i] ^ 0x5c;
		i_key_pad[i] ^= 0x36;
	}
	sha512_Init(&(hctx->ctx));
	sha512_Update(&(hctx->ctx), i_key_pad, SHA512_BLOCK_LENGTH);
	MEMSET_BZERO(i_key_pad, sizeof(i_key_pad));
}

void hmac_sha512_Update(HMAC_SHA512_CTX *hctx, const uint8_t *msg, const uint32_t msglen)
{
	sha512_Update(&(hctx->ctx), msg, msglen);
}

void hmac_sha512_Final(HMAC_SHA512_CTX *hctx, uint8_t *hmac)
{
	uint8_t hash[SHA512_DIGEST_LENGTH];
	sha512_Final(&(hctx->ctx), hash);
	sha512_Init(&(hctx->ctx));
	sha512_Update(&(hctx->ctx), hctx->o_key_pad, SHA512_BLOCK_LENGTH);
	sha512_Update(&(hctx->ctx), hash, SHA512_DIGEST_LENGTH);
	sha512_Final(&(hctx->ctx), hmac);
	MEMSET_BZERO(hash, sizeof(hash));
	MEMSET_BZERO(hctx, sizeof(HMAC_SHA512_CTX));
}

void hmac_sha512(const uint8_t *key, const uint32_t keylen, const uint8_t *msg, const uint32_t msglen, uint8_t *hmac)
{
	HMAC_SHA512_CTX hctx;
	hmac_sha512_Init(&hctx, key, keylen);
	hmac_sha512_Update(&hctx, msg, msglen);
	hmac_sha512_Final(&hctx, hmac);
}

void hmac_sha512_prepare(const uint8_t *key, const uint32_t keylen, uint64_t *opad_digest, uint64_t *ipad_digest)
{
	int i;
	uint64_t buf[SHA512_BLOCK_LENGTH/sizeof(uint64_t)];
	uint64_t o_key_pad[16], i_key_pad[16];

	memset(buf, 0, SHA512_BLOCK_LENGTH);
	if (keylen > SHA512_BLOCK_LENGTH) {
		sha512_Raw(key, keylen, (uint8_t*)buf);
	} else {
		memcpy(buf, key, keylen);
	}

	for (i = 0; i < 16; i++) {
		uint64_t data;
#if BYTE_ORDER == LITTLE_ENDIAN
		REVERSE64(buf[i], data);
#else
		data = buf[i];
#endif
		o_key_pad[i] = data ^ 0x5c5c5c5c5c5c5c5c;
		i_key_pad[i] = data ^ 0x3636363636363636;
	}

	sha512_Transform(sha512_initial_hash_value, o_key_pad, opad_digest);
	sha512_Transform(sha512_initial_hash_value, i_key_pad, ipad_digest);
}
