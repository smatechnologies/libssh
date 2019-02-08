/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2014 by Aris Adamantiadis <aris@badcode.be>
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The SSH Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "config.h"

#include <stdio.h>

#include "libssh/priv.h"
#include "libssh/bignum.h"
#include "libssh/string.h"

ssh_string ssh_make_bignum_string(bignum num) {
  ssh_string ptr = NULL;
  int pad = 0;
  unsigned int len = bignum_num_bytes(num);
  unsigned int bits = bignum_num_bits(num);

  if (len == 0) {
      return NULL;
  }

  /* If the first bit is set we have a negative number */
  if (!(bits % 8) && bignum_is_bit_set(num, bits - 1)) {
    pad++;
  }

#ifdef DEBUG_CRYPTO
  SSH_LOG(SSH_LOG_CRYPTO, "%d bits, %d bytes, %d padding", bits, len, pad);
#endif /* DEBUG_CRYPTO */

  ptr = ssh_string_new(len + pad);
  if (ptr == NULL) {
    return NULL;
  }

  /* We have a negative number so we need a leading zero */
  if (pad) {
    ptr->data[0] = 0;
  }

#ifdef HAVE_LIBGCRYPT
  bignum_bn2bin(num, len, ptr->data + pad);
#elif HAVE_LIBCRYPTO
  bignum_bn2bin(num, ptr->data + pad);
#elif HAVE_LIBMBEDCRYPTO
  bignum_bn2bin(num, ptr->data + pad);
#endif

  return ptr;
}

bignum ssh_make_string_bn(ssh_string string){
  bignum bn = NULL;
  size_t len = ssh_string_len(string);

#ifdef DEBUG_CRYPTO
  SSH_LOG(SSH_LOG_CRYPTO, "Importing a %d bits, %d bytes object ...",
      len * 8, len);
#endif /* DEBUG_CRYPTO */

#ifdef HAVE_LIBGCRYPT
  bignum_bin2bn(string->data, len, &bn);
#elif defined HAVE_LIBCRYPTO
  bn = bignum_bin2bn(string->data, len, NULL);
#elif defined HAVE_LIBMBEDCRYPTO
  bn = bignum_new();
  bignum_bin2bn(string->data, len, bn);
#endif

  return bn;
}

void ssh_make_string_bn_inplace(ssh_string string,
                                UNUSED_PARAM(bignum bnout))
{
  UNUSED_VAR(size_t len) = ssh_string_len(string);
#ifdef HAVE_LIBGCRYPT
  /* XXX: FIXME as needed for LIBGCRYPT ECDSA codepaths. */
#elif defined HAVE_LIBCRYPTO
  bignum_bin2bn(string->data, len, bnout);
#elif defined HAVE_LIBMBEDCRYPTO
  bignum_bin2bn(string->data, len, bnout);
#endif
}

/* prints the bignum on stderr */
void ssh_print_bignum(const char *which, const bignum num) {
#ifdef HAVE_LIBGCRYPT
  unsigned char *hex = NULL;
  bignum_bn2hex(num, &hex);
#elif defined HAVE_LIBCRYPTO
  char *hex = NULL;
  hex = bignum_bn2hex(num);
#elif defined HAVE_LIBMBEDCRYPTO
  char *hex = NULL;
  hex = bignum_bn2hex(num);
#endif
  SSH_LOG(SSH_LOG_CRYPTO, "%s value: %s", which, (hex == NULL) ? "(null)" : (char *)hex);
#ifdef HAVE_LIBGCRYPT
  SAFE_FREE(hex);
#elif defined HAVE_LIBCRYPTO
  OPENSSL_free(hex);
#elif defined HAVE_LIBMBEDCRYPTO
  SAFE_FREE(hex);
#endif
}
