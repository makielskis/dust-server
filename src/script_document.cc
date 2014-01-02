// Copyright (c) 2014, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#include "dust-server/script_document.h"

#include "dust/document.h"

#include "LuaBridge/LuaBridge.h"

namespace dust_server {

script_document::script_document(std::weak_ptr<dust::key_value_store> store,
                                 std::string full_path)
    : dust::document(store, full_path) {
}

script_document::script_document(std::weak_ptr<dust::key_value_store> store,
                                 std::string path,
                                 std::string index)
    : dust::document(store, path, index) {
}

script_document::script_document(dust::document& doc)
    : dust::document(doc) {
}

script_document script_document::index_op(script_document, luabridge::LuaRef key) {
  dust::document doc = operator[](key);
  return doc;
}

}
