#include <memory>

#include "gtest/gtest.h"

#include "boost/algorithm/string/predicate.hpp"
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

TEST_F(script_test, assign_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"].assign("Hello")
  doc["foo"]["baz"].assign("World")
  return doc["foo"]["bar"].val() + ", " + doc["foo"]["baz"].val())";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}

TEST_F(script_test, value_of_nonexistent_entry_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["non"]["existent"].val()
  return "test failed")";

  std::string result = server_.apply_script(script);
  ASSERT_TRUE(boost::algorithm::ends_with(result, "value does not exist\n"));
}

TEST_F(script_test, store_override_composite_fail_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"].assign("Hello")
  doc["foo"].assign("impossible")
  return "test failed")";

  std::string result = server_.apply_script(script);
  ASSERT_TRUE(boost::algorithm::ends_with(result, "override value with composite\n"));
}

TEST_F(script_test, store_override_value_fail_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"].assign("Hello")
  doc["foo"]["bar"].assign("impossible")
  return "test failed")";

  std::string result = server_.apply_script(script);
  ASSERT_TRUE(boost::algorithm::ends_with(result, "parent is not a composite\n"));
}

TEST_F(script_test, children_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"].assign("Hello")
  doc["foo"]["baz"].assign("World")

  children = doc["foo"].children()

  return children[0].val() + ", " + children[1].val())";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}

TEST_F(script_test, index_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"]["baz"].assign("Hello")

  return doc["foo"]["bar"]["baz"].index())";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("baz", result);
}

TEST_F(script_test, exists_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"]["baz"].assign("Hello")

  return "%s" % doc["foo"]["bar"]["baz"].exists() + "," "%s" % doc["non"]["existent"].exists() )";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("True,False", result);
}

TEST_F(script_test, is_composite_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"]["baz"].assign("Hello")

  return "%s" % doc["foo"]["bar"].is_composite() + "," "%s" % doc["foo"]["bar"]["baz"].is_composite() )";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("True,False", result);
}

TEST_F(script_test, remove_test) {
  std::string script = R"(
import Dust
def run(doc):
  doc["foo"]["bar"]["baz"].assign("Hello")

  created = doc["foo"].exists();

  doc["foo"].remove();

  return "%s" % created + "," "%s" % doc["foo"].exists() )";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("True,False", result);
}
