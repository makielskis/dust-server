#include <memory>

#include "gtest/gtest.h"

#include "boost/asio/io_service.hpp"

#include "dust/document.h"
#include "dust/storage/mem_store.h"
#include "dust-server/dust_server.h"


using namespace dust;

class script_test: public testing::Test {
 public:
  script_test()
      : store_(std::make_shared<mem_store>()),
        io_service_(),
        server_(io_service_, store_)  {
  }

 protected:
  std::shared_ptr<key_value_store> store_;
  boost::asio::io_service* io_service_;
  dust_server::dust_server server_;
};

TEST_F(script_test, construct_from_existing_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"].assign("Hello")
  doc["foo"]["baz"].assign("World")
  return doc["foo"]["bar"].val() + ", " + doc["foo"]["baz"].val())";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}
