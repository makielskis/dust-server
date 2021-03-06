// Copyright (c) 2014, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#include <memory>
#include <functional>

#include "dust/storage/key_value_store.h"

#include "dust-server/http_service.h"
#include "dust-server/base64_decode.h"

#include "boost/lexical_cast.hpp"
#include "boost/asio/io_service.hpp"

#include "http_server/url_decode.hpp"
#include "http_server/request.hpp"
#include "http_server/reply.hpp"

namespace dust_server {

using std::placeholders::_1;
using std::placeholders::_2;

http_service::http_service(boost::asio::io_service* io_service,
                           std::shared_ptr<dust::key_value_store> store,
                           const options& config)
    : io_service_(io_service),
      request_handler_(std::bind(&http_service::handle_request, this, _1, _2)),
      http_server_(*io_service_, config.host(), config.port(),
                   request_handler_),
      lua_con_(store),
      username_("admin"),
      password_(config.password()) {
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

void http_service::handle_request(const http::server::request& req,
                                  http::server::reply& rep) {
  using http::server::header;

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
    rep.status = http::server::reply::unauthorized;
    rep.headers.push_back( { "WWW-Authenticate", "Basic realm=\"dustDB\"" });
    return;
  }

  // Invalid username/password. -> STOP
  if (!authorized(auth)) {
    rep.status = http::server::reply::unauthorized;
    return;
  }

  // Decode content if required.
  std::string script;
  if (urlencoded) {
    std::cout << "script is url-encoded\n";
    http::server::url_decode(req.content, script);
  } else {
    script = req.content;
  }

  // Execute script.
  std::cout << "script:\n'" << script << "'\n";
  std::string result = lua_con_.apply_script(script);
  std::cout << "result: '" << result << "'\n";

  // Send result.
  rep.content = result;
  rep.status = http::server::reply::ok;
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = "text/plain";
}

}  // namespace dust_server
