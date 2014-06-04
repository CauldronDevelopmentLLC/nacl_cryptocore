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

#ifndef CRYPTO_CORE_INSTANCE_H
#define CRYPTO_CORE_INSTANCE_H

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var.h>

#include <string>


namespace CryptoCore {
  class Instance : public pp::Instance {
    static Instance *singleton;

  public:
    Instance(PP_Instance instance);
    ~Instance();

    static Instance &instance();

    // From pp::Instance
    void HandleMessage(const pp::Var &msg);


    void log(const std::string &msg);
  };
}

#endif // CRYPTO_CORE_INSTANCE_H

