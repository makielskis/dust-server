#include "dust/storage/key_value_store.h"

#include <memory>

#include "dust-server/lua_state_wrapper.h"

namespace boost { namespace asio {
class io_service;
} }

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

class dust_server {
 public:
  dust_server(boost::asio::io_service* io_service,
              std::shared_ptr<dust::key_value_store> store,
              std::string username,
              std::string password);

  std::string apply_script(std::string script);

  void start_server();

 private:
  void registerLuaDocument(const state_wrapper& L);
  void do_string(const state_wrapper&, const std::string& script);

  boost::asio::io_service* io_service_;
  std::shared_ptr<dust::key_value_store> store_;
  std::string username_;
  std::string password_;
};

}
