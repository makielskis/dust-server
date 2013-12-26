#include "gtest/gtest.h"

#include "boost/asio/io_service.hpp"

#include "dust/storage/mem_store.h"
#include "dust-server/dust_server.h"

using namespace dust;

class server_test: public testing::Test {
 public:
  server_test()
      : store_(std::make_shared<mem_store>()),
        io_service_(),
        server_(&io_service_, store_, "testuser", "testpass")  {
  }

 protected:
  std::shared_ptr<key_value_store> store_;
  boost::asio::io_service io_service_;
  dust_server::dust_server server_;
};

TEST_F(server_test, assign_test) {
 // server_.start_server();
}
