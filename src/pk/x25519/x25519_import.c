/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt_private.h"

/**
  @file x25519_import.c
  Import a X25519 key from a binary packet, Steffen Jaeckel
*/

#ifdef LTC_X25519

/**
  Import a X25519 key from a binary packet
  @param in     The packet to read
  @param inlen  The length of the input packet
  @param key    [out] Where to import the key to
  @return CRYPT_OK if successful, on error all allocated memory is freed automatically
*/
int x25519_import(const unsigned char *in, unsigned long inlen, curve25519_key *key)
{
   int err;
   oid_st oid;
   ltc_asn1_list alg_id[1];
   unsigned char private_key[34];
   unsigned long version, key_len;
   unsigned long tmpoid[16];

   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(key != NULL);

   /* There's only one case where the inlen is equal to the pubkey-size
    * and that's a raw pubkey, so let's just do a raw import.
    */
   if (inlen == sizeof(key->pub)) {
      XMEMCPY(key->pub, in, sizeof(key->pub));
      key->type = PK_PUBLIC;
      key->algo = PKA_X25519;
      return CRYPT_OK;
   }

   key_len = sizeof(key->pub);
   if ((err = x509_decode_subject_public_key_info(in, inlen, PKA_X25519, key->pub, &key_len, LTC_ASN1_EOL, NULL, 0)) == CRYPT_OK) {
      key->type = PK_PUBLIC;
      key->algo = PKA_X25519;
      return CRYPT_OK;
   }

   LTC_SET_ASN1(alg_id, 0, LTC_ASN1_OBJECT_IDENTIFIER, tmpoid, sizeof(tmpoid)/sizeof(tmpoid[0]));

   key_len = sizeof(private_key);
   if ((err = der_decode_sequence_multi(in, inlen,
                             LTC_ASN1_SHORT_INTEGER,    1UL, &version,
                             LTC_ASN1_SEQUENCE,         1UL, alg_id,
                             LTC_ASN1_OCTET_STRING, key_len, private_key,
                             LTC_ASN1_EOL,              0UL, NULL)) != CRYPT_OK) {
      goto out;
   }

   if ((err = pk_get_oid(PKA_X25519, &oid)) != CRYPT_OK) {
      goto out;
   }
   if ((alg_id[0].size != oid.OIDlen) ||
         XMEMCMP(oid.OID, alg_id[0].data, oid.OIDlen * sizeof(oid.OID[0]))) {
      /* OID mismatch */
      err = CRYPT_PK_INVALID_TYPE;
      goto out;
   }

   if (version == 0) {
      key_len = sizeof(key->priv);
      if ((err = der_decode_octet_string(private_key, sizeof(private_key), key->priv, &key_len)) == CRYPT_OK) {
         crypto_scalarmult_base(key->pub, key->priv);
         key->type = PK_PRIVATE;
         key->algo = PKA_X25519;
      }
   } else {
      err = CRYPT_PK_INVALID_TYPE;
   }

out:
#ifdef LTC_CLEAN_STACK
   zeromem(private_key, sizeof(private_key));
#endif

   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
