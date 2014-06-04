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

// Derived from https://github.com/bitpay/bitcore/blob/master/src/eckey.h

#ifndef CRYPTO_CORE_ECKEY_H
#define CRYPTO_CORE_ECKEY_H

#include <string>

#include <stdint.h>

typedef struct ec_key_st EC_KEY;


namespace CryptoCore {
  class ECKey {
    EC_KEY *ec;

    bool compressed;
    bool hasPrivate;
    bool hasPublic;

  public:
    ECKey();
    ~ECKey();

    bool getCompressed() const {return compressed;}
    void setCompressed(bool compressed) {this->compressed = compressed;}

    std::string getPrivate() const;
    void setPrivate(const std::string &pri);

    std::string getPublic() const;
    void setPublic(const std::string &pub);

    std::string toDER() const;
    void fromDER(const std::string &der);

    void generate();
    void regenerate();

    bool verify(const std::string &hash, const std::string &sig) const;
    std::string sign(const std::string &hash) const;

    static void add(const uint8_t point0[65], const uint8_t point1[65],
                    uint8_t result[65]);
    static void multiply(const uint8_t point0[65], const uint8_t x[32],
                         uint8_t result[65]);
  };
}

#endif // CRYPTO_CORE_ECKEY_H
