/**
 * @file crypto_config.h
 * @brief CycloneCrypto configuration file
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCrypto Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

#ifndef _CRYPTO_CONFIG_H
#define _CRYPTO_CONFIG_H

//Dependencies
#include "RTE_Components.h"

//*** <<< Use Configuration Wizard in Context Menu >>> ***

// <o>Trace level
// <i>Set the desired debugging level
// <i>Default: Info
// <0=>Off
// <1=>Fatal
// <2=>Error
// <3=>Warning
// <4=>Info
// <5=>Debug
#define CRYPTO_TRACE_LEVEL 0

// <q>Assembly optimizations
// <i>Enable assembly optimizations for time-critical routines
// <i>Default: Enabled
#define MPI_ASM_SUPPORT 1

//MD2 support
#ifdef RTE_CYCLONE_CRYPTO_MD2
   #define MD2_SUPPORT ENABLED
#else
   #define MD2_SUPPORT DISABLED
#endif

//MD4 support
#ifdef RTE_CYCLONE_CRYPTO_MD4
   #define MD4_SUPPORT ENABLED
#else
   #define MD4_SUPPORT DISABLED
#endif

//MD5 support
#ifdef RTE_CYCLONE_CRYPTO_MD5
   #define MD5_SUPPORT ENABLED
#else
   #define MD5_SUPPORT DISABLED
#endif

//RIPEMD-128 support
#ifdef RTE_CYCLONE_CRYPTO_RIPEMD128
   #define RIPEMD128_SUPPORT ENABLED
#else
   #define RIPEMD128_SUPPORT DISABLED
#endif

//RIPEMD-160 support
#ifdef RTE_CYCLONE_CRYPTO_RIPEMD160
   #define RIPEMD160_SUPPORT ENABLED
#else
   #define RIPEMD160_SUPPORT DISABLED
#endif

//SHA-1 support
#ifdef RTE_CYCLONE_CRYPTO_SHA1
   #define SHA1_SUPPORT ENABLED
#else
   #define SHA1_SUPPORT DISABLED
#endif

//SHA-224 support
#ifdef RTE_CYCLONE_CRYPTO_SHA224
   #define SHA224_SUPPORT ENABLED
#else
   #define SHA224_SUPPORT DISABLED
#endif

//SHA-256 support
#ifdef RTE_CYCLONE_CRYPTO_SHA256
   #define SHA256_SUPPORT ENABLED
#else
   #define SHA256_SUPPORT DISABLED
#endif

//SHA-384 support
#ifdef RTE_CYCLONE_CRYPTO_SHA384
   #define SHA384_SUPPORT ENABLED
#else
   #define SHA384_SUPPORT DISABLED
#endif

//SHA-512 support
#ifdef RTE_CYCLONE_CRYPTO_SHA512
   #define SHA512_SUPPORT ENABLED
#else
   #define SHA512_SUPPORT DISABLED
#endif

//SHA-512/224 support
#ifdef RTE_CYCLONE_CRYPTO_SHA512_224
   #define SHA512_224_SUPPORT ENABLED
#else
   #define SHA512_224_SUPPORT DISABLED
#endif

//SHA-512/256 support
#ifdef RTE_CYCLONE_CRYPTO_SHA512_256
   #define SHA512_256_SUPPORT ENABLED
#else
   #define SHA512_256_SUPPORT DISABLED
#endif

//SHA3-224 support
#ifdef RTE_CYCLONE_CRYPTO_SHA3_224
   #define SHA3_224_SUPPORT ENABLED
#else
   #define SHA3_224_SUPPORT DISABLED
#endif

//SHA3-256 support
#ifdef RTE_CYCLONE_CRYPTO_SHA3_256
   #define SHA3_256_SUPPORT ENABLED
#else
   #define SHA3_256_SUPPORT DISABLED
#endif

//SHA3-384 support
#ifdef RTE_CYCLONE_CRYPTO_SHA3_384
   #define SHA3_384_SUPPORT ENABLED
#else
   #define SHA3_384_SUPPORT DISABLED
#endif

//SHA3-512 support
#ifdef RTE_CYCLONE_CRYPTO_SHA3_512
   #define SHA3_512_SUPPORT ENABLED
#else
   #define SHA3_512_SUPPORT DISABLED
#endif

//SHAKE128 support
#ifdef RTE_CYCLONE_CRYPTO_SHAKE128
   #define SHAKE128_SUPPORT ENABLED
#else
   #define SHAKE128_SUPPORT DISABLED
#endif

//SHAKE256 support
#ifdef RTE_CYCLONE_CRYPTO_SHAKE256
   #define SHAKE256_SUPPORT ENABLED
#else
   #define SHAKE256_SUPPORT DISABLED
#endif

//Keccak support
#ifdef RTE_CYCLONE_CRYPTO_KECCAK
   #define KECCAK_SUPPORT ENABLED
#else
   #define KECCAK_SUPPORT DISABLED
#endif

//BLAKE2b support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2B
   #define BLAKE2B_SUPPORT ENABLED
#else
   #define BLAKE2B_SUPPORT DISABLED
#endif

//BLAKE2b-160 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2B160
   #define BLAKE2B160_SUPPORT ENABLED
#else
   #define BLAKE2B160_SUPPORT DISABLED
#endif

//BLAKE2b-256 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2B256
   #define BLAKE2B256_SUPPORT ENABLED
#else
   #define BLAKE2B256_SUPPORT DISABLED
#endif

//BLAKE2b-384 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2B384
   #define BLAKE2B384_SUPPORT ENABLED
#else
   #define BLAKE2B384_SUPPORT DISABLED
#endif

//BLAKE2b-512 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2B512
   #define BLAKE2B512_SUPPORT ENABLED
#else
   #define BLAKE2B512_SUPPORT DISABLED
#endif

//BLAKE2s support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2S
   #define BLAKE2S_SUPPORT ENABLED
#else
   #define BLAKE2S_SUPPORT DISABLED
#endif

//BLAKE2s-128 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2S128
   #define BLAKE2S128_SUPPORT ENABLED
#else
   #define BLAKE2S128_SUPPORT DISABLED
#endif

//BLAKE2s-160 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2S160
   #define BLAKE2S160_SUPPORT ENABLED
#else
   #define BLAKE2S160_SUPPORT DISABLED
#endif

//BLAKE2s-224 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2S224
   #define BLAKE2S224_SUPPORT ENABLED
#else
   #define BLAKE2S224_SUPPORT DISABLED
#endif

//BLAKE2s-256 hash support
#ifndef RTE_CYCLONE_CRYPTO_BLAKE2S256
   #define BLAKE2S256_SUPPORT ENABLED
#else
   #define BLAKE2S256_SUPPORT DISABLED
#endif

//Tiger support
#ifdef RTE_CYCLONE_CRYPTO_TIGER
   #define TIGER_SUPPORT ENABLED
#else
   #define TIGER_SUPPORT DISABLED
#endif

//Whirlpool support
#ifdef RTE_CYCLONE_CRYPTO_WHIRLPOOL
   #define WHIRLPOOL_SUPPORT ENABLED
#else
   #define WHIRLPOOL_SUPPORT DISABLED
#endif

//CMAC support
#ifdef RTE_CYCLONE_CRYPTO_CMAC
   #define CMAC_SUPPORT ENABLED
#else
   #define CMAC_SUPPORT DISABLED
#endif

//HMAC support
#ifdef RTE_CYCLONE_CRYPTO_HMAC
   #define HMAC_SUPPORT ENABLED
#else
   #define HMAC_SUPPORT DISABLED
#endif

//GMAC support
#ifdef RTE_CYCLONE_CRYPTO_GMAC
   #define GMAC_SUPPORT ENABLED
#else
   #define GMAC_SUPPORT DISABLED
#endif

//RC4 support
#ifdef RTE_CYCLONE_CRYPTO_RC4
   #define RC4_SUPPORT ENABLED
#else
   #define RC4_SUPPORT DISABLED
#endif

//RC6 support
#ifdef RTE_CYCLONE_CRYPTO_RC6
   #define RC6_SUPPORT ENABLED
#else
   #define RC6_SUPPORT DISABLED
#endif

//IDEA support
#ifdef RTE_CYCLONE_CRYPTO_IDEA
   #define IDEA_SUPPORT ENABLED
#else
   #define IDEA_SUPPORT DISABLED
#endif

//DES support
#ifdef RTE_CYCLONE_CRYPTO_DES
   #define DES_SUPPORT ENABLED
#else
   #define DES_SUPPORT DISABLED
#endif

//Triple DES support
#ifdef RTE_CYCLONE_CRYPTO_DES3
   #define DES3_SUPPORT ENABLED
#else
   #define DES3_SUPPORT DISABLED
#endif

//AES support
#ifdef RTE_CYCLONE_CRYPTO_AES
   #define AES_SUPPORT ENABLED
#else
   #define AES_SUPPORT DISABLED
#endif

//Camellia support
#ifdef RTE_CYCLONE_CRYPTO_CAMELLIA
   #define CAMELLIA_SUPPORT ENABLED
#else
   #define CAMELLIA_SUPPORT DISABLED
#endif

//SEED support
#ifdef RTE_CYCLONE_CRYPTO_SEED
   #define SEED_SUPPORT ENABLED
#else
   #define SEED_SUPPORT DISABLED
#endif

//ARIA support
#ifdef RTE_CYCLONE_CRYPTO_ARIA
   #define ARIA_SUPPORT ENABLED
#else
   #define ARIA_SUPPORT DISABLED
#endif

//PRESENT support
#ifdef RTE_CYCLONE_CRYPTO_PRESENT
   #define PRESENT_SUPPORT ENABLED
#else
   #define PRESENT_SUPPORT DISABLED
#endif

//ECB mode support
#ifdef RTE_CYCLONE_CRYPTO_ECB
   #define ECB_SUPPORT ENABLED
#else
   #define ECB_SUPPORT DISABLED
#endif

//CBC mode support
#ifdef RTE_CYCLONE_CRYPTO_CBC
   #define CBC_SUPPORT ENABLED
#else
   #define CBC_SUPPORT DISABLED
#endif

//CFB mode support
#ifdef RTE_CYCLONE_CRYPTO_CFB
   #define CFB_SUPPORT ENABLED
#else
   #define CFB_SUPPORT DISABLED
#endif

//OFB mode support
#ifdef RTE_CYCLONE_CRYPTO_OFB
   #define OFB_SUPPORT ENABLED
#else
   #define OFB_SUPPORT DISABLED
#endif

//CTR mode support
#ifdef RTE_CYCLONE_CRYPTO_CTR
   #define CTR_SUPPORT ENABLED
#else
   #define CTR_SUPPORT DISABLED
#endif

//XTS mode support
#ifdef RTE_CYCLONE_CRYPTO_XTS
   #define XTS_SUPPORT ENABLED
#else
   #define XTS_SUPPORT DISABLED
#endif

//CCM mode support
#ifdef RTE_CYCLONE_CRYPTO_CCM
   #define CCM_SUPPORT ENABLED
#else
   #define CCM_SUPPORT DISABLED
#endif

//GCM mode support
#ifdef RTE_CYCLONE_CRYPTO_GCM
   #define GCM_SUPPORT ENABLED
#else
   #define GCM_SUPPORT DISABLED
#endif

//ChaCha support
#ifdef RTE_CYCLONE_CRYPTO_CHACHA
   #define CHACHA_SUPPORT ENABLED
#else
   #define CHACHA_SUPPORT DISABLED
#endif

//Poly1305 support
#ifdef RTE_CYCLONE_CRYPTO_POLY1305
   #define POLY1305_SUPPORT ENABLED
#else
   #define POLY1305_SUPPORT DISABLED
#endif

//Chacha20Poly1305 support
#ifdef RTE_CYCLONE_CRYPTO_CHACHA20_POLY1305
   #define CHACHA20_POLY1305_SUPPORT ENABLED
#else
   #define CHACHA20_POLY1305_SUPPORT DISABLED
#endif

//RSA support
#ifdef RTE_CYCLONE_CRYPTO_RSA
   #define RSA_SUPPORT ENABLED
#else
   #define RSA_SUPPORT DISABLED
#endif

//Diffie-Hellman support
#ifdef RTE_CYCLONE_CRYPTO_DH
   #define DH_SUPPORT ENABLED
#else
   #define DH_SUPPORT DISABLED
#endif

//PKCS #5 support
#ifdef RTE_CYCLONE_CRYPTO_PKCS5
   #define PKCS5_SUPPORT ENABLED
#else
   #define PKCS5_SUPPORT DISABLED
#endif

//Elliptic curve cryptography support
#ifdef RTE_CYCLONE_CRYPTO_ECC
   #define EC_SUPPORT ENABLED
#else
   #define EC_SUPPORT DISABLED
#endif

//ECDH support
#ifdef RTE_CYCLONE_CRYPTO_ECDH
   #define ECDH_SUPPORT ENABLED
#else
   #define ECDH_SUPPORT DISABLED
#endif

//ECDSA support
#ifdef RTE_CYCLONE_CRYPTO_ECDSA
   #define ECDSA_SUPPORT ENABLED
#else
   #define ECDSA_SUPPORT DISABLED
#endif

//DSA support
#ifdef RTE_CYCLONE_CRYPTO_DSA
   #define DSA_SUPPORT ENABLED
#else
   #define DSA_SUPPORT DISABLED
#endif

//Multiple precision integer support
#ifdef RTE_CYCLONE_CRYPTO_MPI
   #define MPI_SUPPORT ENABLED
#else
   #define MPI_SUPPORT DISABLED
#endif

//Yarrow PRNG support
#ifdef RTE_CYCLONE_CRYPTO_YARROW
   #define YARROW_SUPPORT ENABLED
#else
   #define YARROW_SUPPORT DISABLED
#endif

//Base64 encoding support
#ifdef RTE_CYCLONE_CRYPTO_BASE64
   #define BASE64_SUPPORT ENABLED
#else
   #define BASE64_SUPPORT DISABLED
#endif

//ASN.1 syntax support
#ifdef RTE_CYCLONE_CRYPTO_ASN1
   #define ASN1_SUPPORT ENABLED
#else
   #define ASN1_SUPPORT DISABLED
#endif

//Object identifier support
#ifdef RTE_CYCLONE_CRYPTO_OID
   #define OID_SUPPORT ENABLED
#else
   #define OID_SUPPORT DISABLED
#endif

//X.509 certificate support
#ifdef RTE_CYCLONE_CRYPTO_X509
   #define X509_SUPPORT ENABLED
#else
   #define X509_SUPPORT DISABLED
#endif

//PEM file support
#ifdef RTE_CYCLONE_CRYPTO_PEM
   #define PEM_SUPPORT ENABLED
#else
   #define PEM_SUPPORT DISABLED
#endif

#endif
