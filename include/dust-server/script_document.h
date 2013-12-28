// Copyright (c) 2013, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#ifndef DUST_SERVER_SCRPT_DOCUMENT_H_
#define DUST_SERVER_SCRPT_DOCUMENT_H_

#include "dust/document.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "LuaBridge/LuaBridge.h"

namespace dust_server {

class script_document : public dust::document {
 public:
  script_document(
      std::weak_ptr<dust::key_value_store> store,
      std::string full_path);

  script_document(
      std::weak_ptr<dust::key_value_store> store,
      std::string path,
      std::string index);

  script_document(dust::document&);

  script_document index_op(script_document doc, luabridge::LuaRef key);
};

}

#endif  // DUST_SERVER_SCRPT_DOCUMENT_H_
