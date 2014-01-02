// Copyright (c) 2013, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#ifndef DUST_SERVER_SCRPT_DOCUMENT_H_
#define DUST_SERVER_SCRPT_DOCUMENT_H_

#include <string>
#include <istream>


namespace dust_server {

class options {
 public:
  friend std::ostream& operator<<(std::ostream& out, const options& options);

  options(std::string host, std::string port, std::string password);
  virtual ~options();

  std::string host() const;
  std::string port() const;
  std::string password() const;

 protected:
  std::string host_;
  std::string port_;
  std::string password_;
};

std::ostream& operator<<(std::ostream& out, const options& options);

}  // namespace dust_server

#endif  // DUST_SERVER_SCRPT_DOCUMENT_H_
