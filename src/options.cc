// Copyright (c) 2013, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#include "dust-server/options.h"

namespace dust_server {

options::options(std::string host, std::string port, std::string password)
    : host_(std::move(host)),
      port_(std::move(port)),
      password_(std::move(password)){
}

options::~options() {
}

std::string options::host() const {
  return host_;
}

std::string options::port() const {
  return port_;
}

std::string options::password() const {
  return password_;
}

std::ostream& operator<<(std::ostream& out, const options& options) {
  std::string pw_val = options.password_;
  if (pw_val.empty()) {
    pw_val = "!SECURITY WARNING! empty";
  }

  out << "\n" << "  dust_server_host: " << options.host_ << "\n"
  << "  dust_server_port: " << options.port_ << "\n"
  << "  dust_server_password: " << pw_val << "\n";
  return out;
}

}  // namespace dust_server
