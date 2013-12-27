#include <memory>
#include <string>

#include "dust/storage/key_value_store.h"

#include "dust-server/lua_connection.h"

namespace boost { namespace asio {
class io_service;
} }

namespace dust_server {

class http_service {
 public:
  http_service(boost::asio::io_service* io_service,
               std::shared_ptr<dust::key_value_store> store,
               const std::string& username, const std::string& password);

  void start_server();

 private:
  boost::asio::io_service* io_service_;
  dust_server::lua_connection lua_con_;
  const std::string username_;
  const std::string password_;

  static bool url_decode(const std::string& in, std::string& out);
};

}
