/******************************************************************************\

                    Copyright 2014. Cauldron Development LLC
                              All Rights Reserved.

        This software is free software: you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public License
        as published by the Free Software Foundation, either version 2.1 of
        the License, or (at your option) any later version.

        This software is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
        Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
        License along with the C! library.  If not, see
        <http://www.gnu.org/licenses/>.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

// Derived from https://github.com/bitpay/bitcore/blob/master/src/eckey.cc

#include "ECKey.h"

#include <openssl/ecdsa.h>
#include <openssl/evp.h>

#include <stdexcept>


using namespace CryptoCore;
using namespace std;


namespace {
  template <typename T>
  struct SmartArrayPointer {
    T *ptr;
    SmartArrayPointer(T *ptr) : ptr(ptr) {}
    ~SmartArrayPointer() {delete [] ptr;}
  };
}


ECKey::ECKey() :
  ec(EC_KEY_new_by_curve_name(NID_secp256k1)), compressed(true),
  hasPrivate(false), hasPublic(false) {
  if (!ec) throw runtime_error("EC_KEY_new_by_curve_name() failed");
}


ECKey::~ECKey() {
  EC_KEY_free(ec);
}


string ECKey::getPrivate() const {
  if (!hasPrivate) return string();

  const BIGNUM *bn = EC_KEY_get0_private_key(ec);
  if (!bn) throw runtime_error("EC_KEY_get0_private_key() failed");

  int size = BN_num_bytes(bn);
  if (32 < size)
    throw runtime_error("Secret too large: Incorrect curve parameters?");

  SmartArrayPointer<uint8_t> buf = new uint8_t[32];
  int n = BN_bn2bin(bn, &buf.ptr[32 - size]);

  if (n != size) throw runtime_error("BN_bn2bin() failed");

  return string((char *)buf.ptr, 32);
}


void ECKey::setPrivate(const string &pri) {
  BIGNUM *bn = BN_bin2bn((uint8_t *)pri.data(), pri.length(), BN_new());
  EC_KEY_set_private_key(ec, bn);
  BN_clear_free(bn);
  hasPrivate = true;
}


string ECKey::getPublic() const {
  if (!hasPublic) return string();

  // Set compressed/uncompressed (we prefer compressed)
  EC_KEY_set_conv_form(ec, compressed ? POINT_CONVERSION_COMPRESSED :
                       POINT_CONVERSION_UNCOMPRESSED);

  // Export public
  int size = i2o_ECPublicKey(ec, 0);
  if (!size) throw runtime_error("i2o_ECPublicKey() failed");

  SmartArrayPointer<uint8_t> pub = new uint8_t[size];
  uint8_t *pubEnd = pub.ptr;

  if (i2o_ECPublicKey(ec, &pubEnd) != size)
    throw runtime_error("i2o_ECPublicKey() failed");

  return string((char *)pub.ptr, size);
}


void ECKey::setPublic(const std::string &pub) {
  const uint8_t *ptr = (uint8_t *)pub.data();

  if (!o2i_ECPublicKey(&ec, &ptr, pub.length()))
    throw runtime_error("Invalid public key");

  hasPublic = true;
}


string ECKey::toDER() const {
  if (!hasPrivate || !hasPublic) return string();

  // Export DER
  int size = i2d_ECPrivateKey(ec, 0);
  if (!size) throw runtime_error("i2d_ECPrivateECKey() failed");

  SmartArrayPointer<uint8_t> der = new uint8_t[size];
  uint8_t *end = der.ptr;

  if (i2d_ECPrivateKey(ec, &end) != size)
    throw runtime_error("i2d_ECPrivateKey() failed");

  return string((char *)der.ptr, size);
}


void ECKey::fromDER(const std::string &der) {
  const uint8_t *data = (uint8_t *)der.data();

  if (!d2i_ECPrivateKey(&ec, &data, der.length()))
    throw runtime_error("d2i_ECPrivateKey() fail");

  hasPrivate = true;
  hasPublic = true;
}


void ECKey::generate() {
  if (!EC_KEY_generate_key(ec))
    throw runtime_error("EC_KEY_generate_key() failed");

  hasPublic = true;
  hasPrivate = true;
}


void ECKey::regenerate() {
  if (!hasPrivate) throw runtime_error("Regeneration requires a private key");

  EC_KEY *old = ec;
  ec = EC_KEY_new_by_curve_name(NID_secp256k1);
  if (!ec)
    throw runtime_error("EC_KEY_new_by_curve_name(NID_secp256k1) failed");

  const BIGNUM *pri = EC_KEY_get0_private_key(old);
  const EC_GROUP *group = EC_KEY_get0_group(ec);
  BN_CTX *ctx = BN_CTX_new();
  EC_POINT *pub = EC_POINT_new(group);

  if (ctx && pub && EC_POINT_mul(group, pub, pri, 0, 0, ctx)) {
    EC_KEY_set_private_key(ec, pri);
    EC_KEY_set_public_key(ec, pub);
    hasPublic = true;
  }

  if (pub) EC_POINT_free(pub);
  if (ctx) BN_CTX_free(ctx);
  EC_KEY_free(old);
}


bool ECKey::verify(const string &hash, const string &sig) const {
  if (hash.length() != 32) throw runtime_error("hash must have length 32");

  int result = ECDSA_verify(0, (uint8_t *)hash.data(), hash.length(),
                            (uint8_t *)sig.data(), sig.length(), ec);

  switch (result) {
  case -1: throw runtime_error("Error during ECDSA_verify");
  case 0: return false;
  case 1: return true;
  default: throw runtime_error("ECDSA_verify gave undefined return value");
  }
}


string ECKey::sign(const string &hash) const {
  if (!hasPrivate) throw runtime_error("Key does not have a private key set");
  if (hash.length() != 32) throw runtime_error("hash must be 32 bytes long");

  // Create signature
  ECDSA_SIG *sig =
    ECDSA_do_sign((uint8_t *)hash.data(), hash.length(), ec);

  // Compute size
  int derSize = i2d_ECDSA_SIG(sig, 0);
  if (!derSize) throw runtime_error("i2d_ECDSA_SIG() failed");

  SmartArrayPointer<uint8_t> der = new uint8_t[derSize];
  uint8_t *end = der.ptr;

  // Export DER
  int sigSize = i2d_ECDSA_SIG(sig, &end);

  ECDSA_SIG_free(sig);

  if (sigSize != derSize) throw runtime_error("i2d_ECDSA_SIG() failed");

  return string((char *)der.ptr, derSize);
}


void ECKey::add(const uint8_t point0[65], const uint8_t point1[65],
                uint8_t result[65]) {
  EC_KEY *ec = EC_KEY_new_by_curve_name(NID_secp256k1);
  const EC_GROUP *group = EC_KEY_get0_group(ec);
  EC_POINT *p0 = EC_POINT_new(group);
  EC_POINT *p1 = EC_POINT_new(group);
  EC_POINT *r = EC_POINT_new(group);
  BIGNUM *p0x = BN_bin2bn(&point0[1], 32, BN_new());
  BIGNUM *p0y = BN_bin2bn(&point0[33], 32, BN_new());
  BIGNUM *p1x = BN_bin2bn(&point1[1], 32, BN_new());
  BIGNUM *p1y = BN_bin2bn(&point1[33], 32, BN_new());
  BIGNUM *rx = BN_new();
  BIGNUM *ry = BN_new();
  BN_CTX *ctx = BN_CTX_new();

  EC_POINT_set_affine_coordinates_GFp(group, p0, p0x, p0y, ctx);
  EC_POINT_set_affine_coordinates_GFp(group, p1, p1x, p1y, ctx);
  EC_POINT_add(group, r, p0, p1, ctx);
  EC_POINT_get_affine_coordinates_GFp(group, r, rx, ry, ctx);
  EC_POINT_point2oct(group, r, POINT_CONVERSION_UNCOMPRESSED, result, 65, ctx);

  BN_CTX_free(ctx);
  BN_clear_free(ry);
  BN_clear_free(rx);
  BN_clear_free(p0x);
  BN_clear_free(p0y);
  BN_clear_free(p1x);
  BN_clear_free(p1y);
  EC_POINT_free(r);
  EC_POINT_free(p1);
  EC_POINT_free(p0);
  EC_KEY_free(ec);
}


void ECKey::multiply(const uint8_t point0[65], const uint8_t xval[32],
                     uint8_t result[65]) {
  EC_KEY *ec = EC_KEY_new_by_curve_name(NID_secp256k1);
  const EC_GROUP *group = EC_KEY_get0_group(ec);
  EC_POINT *p0 = EC_POINT_new(group);
  EC_POINT *r = EC_POINT_new(group);
  BIGNUM *p0x = BN_bin2bn(&point0[1], 32, BN_new());
  BIGNUM *p0y = BN_bin2bn(&point0[33], 32, BN_new());
  BIGNUM *x = BN_bin2bn(&xval[0], 32, BN_new());
  BIGNUM *rx = BN_new();
  BIGNUM *ry = BN_new();
  BN_CTX *ctx = BN_CTX_new();

  EC_POINT_set_affine_coordinates_GFp(group, p0, p0x, p0y, ctx);
  EC_POINT_mul(group, r, 0, p0, x, ctx);
  EC_POINT_get_affine_coordinates_GFp(group, r, rx, ry, ctx);
  EC_POINT_point2oct(group, r, POINT_CONVERSION_UNCOMPRESSED, result, 65, ctx);

  BN_CTX_free(ctx);
  BN_clear_free(ry);
  BN_clear_free(rx);
  BN_clear_free(p0x);
  BN_clear_free(p0y);
  BN_clear_free(x);
  EC_POINT_free(r);
  EC_POINT_free(p0);
  EC_KEY_free(ec);
}
