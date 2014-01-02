// Copyright (c) 2014, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#ifndef DUST_SERVER_DUST_SERVER_H_
#define DUST_SERVER_DUST_SERVER_H_

#include <memory>
#include <string>

#include "server.hpp"
#include "request_handler.hpp"

#include "dust/storage/key_value_store.h"

#include "dust-server/lua_connection.h"
#include "dust-server/options.h"

namespace boost {
namespace asio {
class io_service;
}
}

namespace http_server {
class request;
class reply;
}

namespace dust_server {

class options;

class http_service {
 public:
  http_service(boost::asio::io_service* io_service,
               std::shared_ptr<dust::key_value_store> store,
               const options& config);

 private:
  bool authorized(const std::string& auth) const;
  void handle_request(const http::server::request& request,
                      http::server::reply& reply);

  boost::asio::io_service* io_service_;
  http::server::request_handler request_handler_;
  http::server::server http_server_;
  dust_server::lua_connection lua_con_;
  const std::string username_;
  const std::string password_;
};

}  // namespace dust_server

#endif  // DUST_SERVER_DUST_SERVER_H_
