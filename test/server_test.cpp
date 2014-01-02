#include "gtest/gtest.h"

#include "boost/asio/io_service.hpp"

#include "dust/storage/mem_store.h"

#include "dust-server/http_service.h"
#include "dust-server/options.h"

using namespace dust;
using namespace dust_server;

class server_test: public testing::Test {
public:
  server_test()
      : io_service_(),
        server_(&io_service_, std::make_shared<mem_store>(),
                dust_server::options("localhost", "9004", "mypass")) {
  }

protected:
  boost::asio::io_service io_service_;
  dust_server::http_service server_;
};

TEST_F(server_test, server_test) {
  io_service_.run();
}
