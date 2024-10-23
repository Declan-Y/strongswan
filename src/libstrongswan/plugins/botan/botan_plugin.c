/*
 * Copyright (C) 2018 Tobias Brunner
 * Copyright (C) 2018 Andreas Steffen
 *
 * Copyright (C) 2018 René Korthaus
 * Copyright (C) 2018 Konstantinos Kolelis
 * Rohde & Schwarz Cybersecurity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "botan_plugin.h"
#include "botan_rng.h"
#include "botan_hasher.h"
#include "botan_crypter.h"
#include "botan_diffie_hellman.h"
#include "botan_hmac.h"
#include "botan_kdf.h"
#include "botan_rsa_public_key.h"
#include "botan_rsa_private_key.h"
#include "botan_ec_diffie_hellman.h"
#include "botan_ec_public_key.h"
#include "botan_ec_private_key.h"
#include "botan_ed_public_key.h"
#include "botan_ed_private_key.h"
#include "botan_aead.h"
#include "botan_util_keys.h"
#include "botan_x25519.h"

#include <library.h>

#include <botan/build.h>
#include <botan/ffi.h>

typedef struct private_botan_plugin_t private_botan_plugin_t;

/**
 * private data of botan_plugin
 */
struct private_botan_plugin_t {

	/**
	 * public functions
	 */
	botan_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_botan_plugin_t *this)
{
	return "botan";
}

METHOD(plugin_t, get_features, int,
	private_botan_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {

#ifdef BOTAN_HAS_DIFFIE_HELLMAN
		/* MODP DH groups */
		PLUGIN_REGISTER(KE, botan_diffie_hellman_create),
			PLUGIN_PROVIDE(KE, MODP_3072_BIT),
			PLUGIN_PROVIDE(KE, MODP_4096_BIT),
			PLUGIN_PROVIDE(KE, MODP_6144_BIT),
			PLUGIN_PROVIDE(KE, MODP_8192_BIT),
			PLUGIN_PROVIDE(KE, MODP_2048_BIT),
			PLUGIN_PROVIDE(KE, MODP_2048_224),
			PLUGIN_PROVIDE(KE, MODP_2048_256),
			PLUGIN_PROVIDE(KE, MODP_1536_BIT),
			PLUGIN_PROVIDE(KE, MODP_1024_BIT),
			PLUGIN_PROVIDE(KE, MODP_1024_160),
			PLUGIN_PROVIDE(KE, MODP_768_BIT),
			PLUGIN_PROVIDE(KE, MODP_CUSTOM),
#endif
#ifdef BOTAN_HAS_ECDH
		/* EC DH groups */
		PLUGIN_REGISTER(KE, botan_ec_diffie_hellman_create),
			PLUGIN_PROVIDE(KE, ECP_256_BIT),
			PLUGIN_PROVIDE(KE, ECP_384_BIT),
			PLUGIN_PROVIDE(KE, ECP_521_BIT),
			PLUGIN_PROVIDE(KE, ECP_256_BP),
			PLUGIN_PROVIDE(KE, ECP_384_BP),
			PLUGIN_PROVIDE(KE, ECP_512_BP),
#endif
#ifdef BOTAN_HAS_X25519
		PLUGIN_REGISTER(KE, botan_x25519_create),
			PLUGIN_PROVIDE(KE, CURVE_25519),
#endif

		/* crypters */
#if defined(BOTAN_HAS_AES)
		PLUGIN_REGISTER(CRYPTER, botan_crypter_create),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_ECB, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_ECB, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_ECB, 32),
	#ifdef BOTAN_HAS_MODE_CBC
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 32),
	#endif
	#ifdef BOTAN_HAS_MODE_CFB
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CFB, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CFB, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CFB, 32),
	#endif
#endif

		/* AEAD */
#if (defined(BOTAN_HAS_AES) && \
		(defined(BOTAN_HAS_AEAD_GCM) || defined(BOTAN_HAS_AEAD_CCM))) || \
	defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305)
		PLUGIN_REGISTER(AEAD, botan_aead_create),
#ifdef BOTAN_HAS_AES
	#ifdef BOTAN_HAS_AEAD_GCM
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  32),
	#endif
	#ifdef BOTAN_HAS_AEAD_CCM
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  32),
	#endif
#endif
#ifdef BOTAN_HAS_AEAD_CHACHA20_POLY1305
			PLUGIN_PROVIDE(AEAD, ENCR_CHACHA20_POLY1305, 32),
#endif
#endif

		/* hashers */
		PLUGIN_REGISTER(HASHER, botan_hasher_create),
#ifdef BOTAN_HAS_MD5
			PLUGIN_PROVIDE(HASHER, HASH_MD5),
#endif
#ifdef BOTAN_HAS_SHA1
			PLUGIN_PROVIDE(HASHER, HASH_SHA1),
#endif
#ifdef BOTAN_HAS_SHA2_32
			PLUGIN_PROVIDE(HASHER, HASH_SHA224),
			PLUGIN_PROVIDE(HASHER, HASH_SHA256),
#endif
#ifdef BOTAN_HAS_SHA2_64
			PLUGIN_PROVIDE(HASHER, HASH_SHA384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA512),
#endif
#ifdef BOTAN_HAS_SHA3
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_224),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_256),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_512),
#endif

		/* prfs */
#ifdef BOTAN_HAS_HMAC
		PLUGIN_REGISTER(PRF, botan_hmac_prf_create),
#ifdef BOTAN_HAS_SHA1
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA1),
#endif
#ifdef BOTAN_HAS_SHA2_32
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_256),
#endif
#ifdef BOTAN_HAS_SHA2_64
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_384),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_512),
#endif
		/* signer */
		PLUGIN_REGISTER(SIGNER, botan_hmac_signer_create),
#ifdef BOTAN_HAS_SHA1
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_96),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_128),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_160),
#endif
#ifdef BOTAN_HAS_SHA2_32
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_128),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_256),
#endif
#ifdef BOTAN_HAS_SHA2_64
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_192),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_384),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_256),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_512),
#endif
#endif /* BOTAN_HAS_HMAC */

		/* kdfs */
#ifdef BOTAN_HAS_HKDF
		PLUGIN_REGISTER(SIGNER, botan_kdf_create),
			PLUGIN_PROVIDE(KDF, KDF_PRF),
			PLUGIN_PROVIDE(KDF, KDF_PRF_PLUS),
#endif /* BOTAN_HAS_HKDF */

		/* generic key loaders */
#if defined (BOTAN_HAS_RSA) || defined(BOTAN_HAS_ECDSA) || \
	defined(BOTAN_HAS_ED25519)
		PLUGIN_REGISTER(PUBKEY, botan_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ANY),
#ifdef BOTAN_HAS_RSA
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
#endif
#ifdef BOTAN_HAS_ECDSA
			PLUGIN_PROVIDE(PUBKEY, KEY_ECDSA),
#endif
#ifdef BOTAN_HAS_ED25519
			PLUGIN_PROVIDE(PUBKEY, KEY_ED25519),
#endif
		PLUGIN_REGISTER(PRIVKEY, botan_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
#ifdef BOTAN_HAS_RSA
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
#endif
#ifdef BOTAN_HAS_ECDSA
			PLUGIN_PROVIDE(PRIVKEY, KEY_ECDSA),
#endif
#ifdef BOTAN_HAS_ED25519
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED25519),
#endif
#endif
		/* RSA */
#ifdef BOTAN_HAS_RSA
		/* public/private key loading/generation */
		PLUGIN_REGISTER(PUBKEY, botan_rsa_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
		PLUGIN_REGISTER(PRIVKEY, botan_rsa_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
		PLUGIN_REGISTER(PRIVKEY_GEN, botan_rsa_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_RSA),
		/* encryption/signature schemes */
#ifdef BOTAN_HAS_EMSA_PKCS1
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_NULL),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_NULL),
#ifdef BOTAN_HAS_SHA1
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA1),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA1),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
#endif
#ifdef BOTAN_HAS_SHA2_32
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_224),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_224),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_256),
#endif
#ifdef BOTAN_HAS_SHA2_64
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_384),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_512),
#endif
#ifdef BOTAN_HAS_SHA3
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_224),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_256),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_384),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_224),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_512),
#endif
#endif
#ifdef BOTAN_HAS_EMSA_PSSR
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PSS),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PSS),
#endif
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_PKCS1),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT,  ENCRYPT_RSA_PKCS1),
#ifdef BOTAN_HAS_EME_OAEP
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA1),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT,  ENCRYPT_RSA_OAEP_SHA1),
#ifdef BOTAN_HAS_SHA2_32
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA224),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT,  ENCRYPT_RSA_OAEP_SHA224),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA256),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT,  ENCRYPT_RSA_OAEP_SHA256),
#endif
#ifdef BOTAN_HAS_SHA2_64
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA384),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT,  ENCRYPT_RSA_OAEP_SHA384),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA512),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT,  ENCRYPT_RSA_OAEP_SHA512),
#endif
#endif
#endif /* BOTAN_HAS_RSA */

#ifdef BOTAN_HAS_ECDSA
		/* EC private/public key loading */
		PLUGIN_REGISTER(PRIVKEY, botan_ec_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ECDSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
		PLUGIN_REGISTER(PRIVKEY_GEN, botan_ec_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ECDSA),
#ifdef BOTAN_HAS_EMSA_RAW
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_NULL),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_NULL),
#endif
#ifdef BOTAN_HAS_EMSA1
#ifdef BOTAN_HAS_SHA1
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA1_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA1_DER),
#endif
#ifdef BOTAN_HAS_SHA2_32
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA256_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA256_DER),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_256),
#endif
#ifndef BOTAN_HAS_SHA2_64
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA384_DER),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA512_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA384_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA512_DER),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_384),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_521),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_521),
#endif
#endif /* BOTAN_HAS_EMSA1 */
#endif /* BOTAN_HAS_ECDSA */

#ifdef BOTAN_HAS_ED25519
		/* EdDSA private/public key loading */
		PLUGIN_REGISTER(PUBKEY, botan_ed_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ED25519),
		PLUGIN_REGISTER(PRIVKEY, botan_ed_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED25519),
		PLUGIN_REGISTER(PRIVKEY_GEN, botan_ed_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ED25519),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ED25519),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ED25519),
		/* register a pro forma identity hasher, never instantiated */
		PLUGIN_REGISTER(HASHER, return_null),
			PLUGIN_PROVIDE(HASHER, HASH_IDENTITY),
#endif

		/* random numbers */
#if BOTAN_HAS_SYSTEM_RNG
#if BOTAN_HAS_HMAC_DRBG
		PLUGIN_REGISTER(RNG, botan_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_WEAK),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE)
#endif
#endif
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_botan_plugin_t *this)
{
	free(this);
}

/*
 * Described in header
 */
plugin_t *botan_plugin_create()
{
	private_botan_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
