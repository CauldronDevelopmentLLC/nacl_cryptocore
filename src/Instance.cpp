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

#include "Instance.h"

#include "ECKey.h"

#include <ppapi/cpp/var_array.h>
#include <ppapi/cpp/var_array_buffer.h>

#include <stdexcept>

using namespace std;


CryptoCore::Instance *CryptoCore::Instance::singleton = 0;


CryptoCore::Instance::Instance(PP_Instance instance) :
  pp::Instance(instance) {
  if (singleton) throw runtime_error("There can be only one. . .Instance");
  singleton = this;
}


CryptoCore::Instance::~Instance() {
  singleton = 0;
}


CryptoCore::Instance &CryptoCore::Instance::instance() {
  if (!singleton) throw runtime_error("Instance not allocated");
  return *singleton;
}


void CryptoCore::Instance::HandleMessage(const pp::Var &msg) {
  try {
    if (msg.is_array()) {
      pp::VarArray msgArray(msg);

      if (!msgArray.GetLength() || !msgArray.Get(0).is_string()) return;
      string cmd = msgArray.Get(0).AsString();

      if (cmd == "add" && msgArray.Get(1).is_array_buffer() &&
          msgArray.Get(2).is_array_buffer()) {

        pp::VarArrayBuffer point0(msgArray.Get(1));
        pp::VarArrayBuffer point1(msgArray.Get(2));

        if (point0.ByteLength() != 65)
          throw runtime_error("point0 must be 65 bytes");
        if (point1.ByteLength() != 65)
          throw runtime_error("point1 must be 65 bytes");

        pp::VarArrayBuffer result(65);

        ECKey::add((const uint8_t *)point0.Map(), (const uint8_t *)point1.Map(),
                   (uint8_t *)result.Map());

        pp::VarArray response;
        response.SetLength(2);
        response.Set(0, "add_result");
        response.Set(1, result);

        PostMessage(response);

      } else if (cmd == "multiply" && msgArray.Get(1).is_array_buffer() &&
          msgArray.Get(2).is_array_buffer()) {

        pp::VarArrayBuffer point0(msgArray.Get(1));
        pp::VarArrayBuffer x(msgArray.Get(2));

        if (point0.ByteLength() != 65)
          throw runtime_error("point0 must be 65 bytes");
        if (x.ByteLength() != 32)
          throw runtime_error("x must be 32 bytes");

        pp::VarArrayBuffer result(65);

        ECKey::multiply((const uint8_t *)point0.Map(), (const uint8_t *)x.Map(),
                        (uint8_t *)result.Map());

        pp::VarArray response;
        response.SetLength(2);
        response.Set(0, "multiply_result");
        response.Set(1, result);

        PostMessage(response);

      } else throw runtime_error("Unknown command " + cmd);
    }
  } catch (const exception &e) {
    log(string("Exception: ") + e.what());
  }
}


void CryptoCore::Instance::log(const string &msg) {
  PostMessage("CryptoCore: " + msg);
}
