#include <memory>
#include <functional>

#include "dust/storage/key_value_store.h"

#include "dust-server/http_service.h"
#include "dust-server/base64_decode.h"

#include "boost/asio/io_service.hpp"

#include "request.hpp"
#include "reply.hpp"

namespace dust_server {

namespace http_server = http::server4;
using std::placeholders::_1;
using std::placeholders::_2;

http_service::http_service(boost::asio::io_service* io_service,
                           std::shared_ptr<dust::key_value_store> store,
                           const std::string& ip,
                           const std::string& port,
                           const std::string& username,
                           const std::string& password)
    : io_service_(io_service),
      http_server_(*io_service_, ip, port,
                   std::bind(&http_service::handle_request, this, _1, _2)),
      lua_con_(store),
      username_(std::move(username)),
      password_(std::move(password)) {
}

void http_service::start_server() {
  http_server_();
}

bool http_service::authorized(const std::string& auth) const {
  // Decode base64.
  std::string credentials = decode_base64(auth.substr(6));

  // Split username and password.
  size_t split = credentials.find_first_of(":");
  if (split == std::string::npos) {
    return false;
  }
  std::string username = credentials.substr(0, split);
  std::string password = credentials.substr(split + 1, std::string::npos);

  return username == username_ && password == password_;
}

void http_service::handle_request(const http_server::request& req,
                                  http_server::reply& rep) {
  using http_server::header;

  // Extract headers.
  bool urlencoded = false;
  std::string auth = "";
  for (const auto& h : req.headers) {
    if (h.name == "Content-Type" && h.value.find("urlencoded")
        != std::string::npos) {
      urlencoded = true;
    } else if (h.name == "Authorization") {
      auth = h.value;
    }
  }

  // No authorization sent.
  if (auth.empty() || auth.substr(0, 5) != "Basic") {
    rep.status = http_server::reply::unauthorized;
    rep.headers.push_back( { "WWW-Authenticate", "Basic realm=\"dustDB\"" });
    return;
  }

  // Invalid username/password. -> STOP
  if (!authorized(auth)) {
    rep.status = http_server::reply::unauthorized;
    return;
  }

  // Decode content if required.
  std::string script;
  if (urlencoded) {
    std::cout << "script is url-encoded\n";
    url_decode(req.content, script);
  } else {
    script = req.content;
  }

  // Execute script.
  std::string result = lua_con_.apply_script(script);
  std::cout << "script: '" << script << "'\n";
  std::cout << "result: '" << result << "'\n";

  // Send result.
  rep.content = result;
  rep.status = http_server::reply::ok;
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = "text/plain";
}

bool http_service::url_decode(const std::string& in, std::string& out) {
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
