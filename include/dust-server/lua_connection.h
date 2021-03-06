// Copyright (c) 2014, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#ifndef DUST_SERVER_LUA_CONNECTION_H_
#define DUST_SERVER_LUA_CONNECTION_H_

#include <memory>

#include "dust/storage/key_value_store.h"
#include "dust/document.h"

#include "dust-server/lua_state_wrapper.h"

namespace dust_server {

using botscript::state_wrapper;

/// This exception indicates an error that occured at lua script execution.
class lua_error : public std::exception {
 public:
  explicit lua_error(const std::string& what)
    : error(what) {
  }

  ~lua_error() throw() {
  }

  virtual const char* what() const throw() {
    return error.c_str();
  }

 private:
  std::string error;
};

class lua_connection {
 public:
  lua_connection(std::shared_ptr<dust::key_value_store> store);
  std::string apply_script(const std::string& script);

 private:
  void registerLuaDocument(const state_wrapper& L);
  void do_string(const state_wrapper&, const std::string& script);
  dust::document get_document(const std::string& index);

  std::shared_ptr<dust::key_value_store> store_;
};

}  // namespace dust_server

#endif  // DUST_SERVER_LUA_CONNECTION_H_
