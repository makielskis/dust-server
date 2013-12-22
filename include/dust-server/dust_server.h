#include "dust/storage/key_value_store.h"

#include <memory>

#include "boost/asio/io_service.hpp"

namespace dust_server {

class dust_server {
 public:
  dust_server(boost::asio::io_service* io_service,
              std::shared_ptr<dust::key_value_store> store);

  std::string apply_script(std::string script);

 private:
  boost::asio::io_service* io_service_;
  std::shared_ptr<dust::key_value_store> store_;
};

}
