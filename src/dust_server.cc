#include "dust-server/dust_server.h"

#include "server.hpp"
#include "request.hpp"
#include "reply.hpp"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "LuaBridge/LuaBridge.h"

#include "dust-server/base64_decode.h"
#include "dust-server/lua_state_wrapper.h"

#include "dust/document.h"

namespace dust_server {

namespace http_server = http::server4;
using namespace luabridge;

dust_server::dust_server(boost::asio::io_service* io_service,
                         std::shared_ptr<dust::key_value_store> store,
                         std::string username,
                         std::string password)
    : io_service_(io_service),
      store_(store),
      username_(std::move(username)),
      password_(std::move(password)) {
}

std::string dust_server::apply_script(std::string script) {
  try {
    // Create and initialize lua state.
    state_wrapper state;
    luaL_openlibs(state.get());
    registerLuaDocument(state);

    // Load script.
    try {
      do_string(state, script);
    } catch (const lua_error& error) {
      return error.what();
    }

    // Get run function form lua.
    auto lua_run = getGlobal(state.get(), "run");

    if (lua_run.isNil()) {
      // run function is not defined
      return "run method not defined";
    }

    // Pass root document to run and execute.
    dust::document root_doc(store_, "");
    return lua_run(root_doc).tostring();
  } catch (const LuaException& e) {
    return std::string("error: ") + e.what();
  }

  return "undefined error";
}

void dust_server::do_string(const state_wrapper& state_wrap, const std::string& script) {
  lua_State* state = state_wrap.get();

  // Load buffer to state.
  int ret = luaL_loadbuffer(state,
                            script.c_str(), script.length(), "");

  // Check load error.
  if (LUA_OK != ret) {
    std::string error;

    // Extract error string.
    const char* s = nullptr;
    if (lua_isstring(state, -1)) {
      s = lua_tostring(state, -1);
    }

    // If extracted error string is a nullptr or empty:
    // Translate error code.
    if (nullptr == s || std::strlen(s) == 0) {
      switch (ret) {
        case LUA_ERRSYNTAX: error = "syntax error"; break;
        case LUA_ERRMEM:    error = "out of memory"; break;
        case LUA_ERRGCMM:   error = "gc error"; break;
        default:            error = "unknown error code"; break;
      }
    } else {
      error = s;
    }

    throw lua_error(error.empty() ? "unknown error" : error);
  }

  // Run loaded buffer and check for errors.
  if (0 != (ret = lua_pcall(state, 0, LUA_MULTRET, 0))) {
    std::string error;

    const char* s = nullptr;
    if (lua_isstring(state, -1)) {
      s = lua_tostring(state, -1);
    }

    if (nullptr == s || std::strlen(s) == 0) {
      switch (ret) {
        case LUA_ERRRUN:  error = "runtime error"; break;
        case LUA_ERRMEM:  error = "out of memory"; break;
        case LUA_ERRERR:  error = "error handling error"; break;
        case LUA_ERRGCMM: error = "gc error"; break;
        default:          error = "unknown error code"; break;
      }
    } else {
      error = s;
    }

    throw lua_error(error.empty() ? "unknown error" : error);
  }
}

void dust_server::registerLuaDocument(const state_wrapper& L) {
  typedef std::vector<dust::document> doc_vec;
  doc_vec::reference (doc_vec::*at_member)(doc_vec::size_type) = &doc_vec::at;
  getGlobalNamespace(L.get())
    .beginClass<dust::document>("Document")
      .addConstructor <void (*)(const dust::document)>()
      .addFunction("get", &dust::document::operator[])
      .addFunction("set", &dust::document::assign)
      .addFunction("index", &dust::document::index)
      .addFunction("val", &dust::document::val)
      .addFunction("exists", &dust::document::exists)
      .addFunction("remove", &dust::document::remove)
      .addFunction("is_composite", &dust::document::is_composite)
      .addFunction("children", &dust::document::children)
    .endClass()
    .beginClass<doc_vec>("DocumentVector")
      .addConstructor <void (*)(void)>()
      .addFunction("__index", at_member)
      .addFunction("__len", &doc_vec::size)
    .endClass();
}

}
