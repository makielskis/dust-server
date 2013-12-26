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

void dust_server::start_server() {
  // Create message handler.
  using http_server::request;
  using http_server::header;
  using http_server::reply;

  typedef boost::function<void(const request&, reply&)> handler;
  handler req_handler = [this](const request& req, reply& rep) {
    // Extract headers.
    bool urlencoded = false;
    std::string auth = "";

    for (const auto& h : req.headers) {
      if (h.name == "Content-Type"
          && h.value.find("urlencoded") != std::string::npos) {
        urlencoded = true;
      }

      if (h.name == "Authorization") {
        auth = h.value;
      }
    }

    // No authorization sent. -> STOP
    std::string debug =  auth.empty() ? auth : auth.substr(0, 5);
    std::cout << "auth: '" << debug << "'\n";
    if (auth.empty() || auth.substr(0, 5) != "Basic") {
      rep.content = "authorization required";
      rep.status = reply::unauthorized;
      rep.headers.resize(3);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/plain";
      rep.headers[2].name = "WWW-Authenticate";
      rep.headers[2].value = "Basic realm=\"dustDB\"";

      return;
    }

    // Check authorization.
    std::string credentials = decode_base64(auth.substr(6));
    std::cout << "cedentials: " << credentials << "\n";
    size_t split = credentials.find_first_of(":");

    // Bad credentials format. -> STOP
    if (split == std::string::npos) {
      rep.content = "invalid authorization request";
      rep.status = reply::bad_request;
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/plain";

      return;
    }

    std::string username = credentials.substr(0, split);
    std::string password = credentials.substr(split + 1, std::string::npos);

    std::cout << "username: " << username << "\n";
    std::cout << "password: " << password << "\n";

    // Invalid username/password. -> STOP
    if (username != username_ || password != password_) {
      rep.content = "access denied";
      rep.status = reply::unauthorized;
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/plain";

      return;
    }

    // Decode content if required.
    std::string script;
    if (urlencoded) {
      url_decode(req.content, script);
    } else {
      script = req.content;
    }

    // Execute script.
    std::string result = apply_script(script);
    std::cout << "script: '" << script << "'\n";
    std::cout << "result: '" << result << "'\n";

    // Send result.
    rep.content = result;
    rep.status = reply::ok;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "text/plain";
  };

  // Instantiate and run server.
  http_server::server(*io_service_, "0.0.0.0", "9091", req_handler)();
  io_service_->run();
}

std::string dust_server::apply_script(std::string script) {
  try {
    state_wrapper state;
    luaL_openlibs(state.get());
    registerLuaDocument(state);

    // Load script.
    if (luaL_dostring(state.get(), script.c_str())) {
      // compile-time error
      std::string err = lua_tostring(state.get(), -1);
      std::cerr << err << "\n";
      return err;
    }

    // Get run function form lua.
    auto lua_run = getGlobal(state.get(), "run");

    if (lua_run.isNil()) {
      // run function is not defined
      std::cerr << "run method not defined" << "\n";
      return "run method not defined";
    }

    // Pass root document to run and execute.
    dust::document root_doc(store_, "");
    return lua_run(root_doc);
  } catch (const LuaException& e) {
    std::string details(e.what());

    std::cerr << details;
    return details;
  }

  return "undefined error";
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

bool dust_server::url_decode(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return true;
}

}
