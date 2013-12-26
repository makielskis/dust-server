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
        server_(io_service_, store_, "testuser", "testpass")  {
  }

 protected:
  std::shared_ptr<key_value_store> store_;
  boost::asio::io_service* io_service_;
  dust_server::dust_server server_;
};

TEST_F(script_test, assign_and_read) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("h"):set("Hello")
  doc:get("foo"):get("w"):set(", World")
  return doc:get("foo"):get("h"):val() .. doc:get("foo"):get("w"):val()
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}

TEST_F(script_test, value_of_nonexistent_entry) {
  std::string script = R"(
function run(doc)
  result = doc:get("non"):get("existant"):val()
  return result
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("error: Database value does not exist", result);
}

TEST_F(script_test, override_composite_with_value) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("bar"):set("Hello")
  doc:get("foo"):set("impossible")
  return "test failed"
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("error: Can't override value with composite", result);
}

TEST_F(script_test, override_value_with_composite) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):set("Hello")
  doc:get("foo"):get("bar"):set("impossible")
  return "test failed"
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("error: Inconsistent database: parent is not a composite", result);
}

TEST_F(script_test, children) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("bar"):set("Hello, ")
  doc:get("foo"):get("baz"):set("World")

  children = doc:get("foo"):children()

  return children[0]:val() .. children[1]:val()
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}

TEST_F(script_test, children_count_operator) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("a"):set("1")
  doc:get("foo"):get("b"):set("2")
  doc:get("foo"):get("c"):set("3")
  doc:get("foo"):get("d"):set("4")
  doc:get("foo"):get("e"):set("5")

  children = doc:get("foo"):children()

  return #children
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("5", result);
}

TEST_F(script_test, index) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("bar"):get("baz"):set("Hello")

  return doc:get("foo"):get("bar"):get("baz"):index()
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("baz", result);
}

TEST_F(script_test, positive_exists) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("bar"):get("baz"):set("Hello")

  return tostring(doc:get("foo"):get("bar"):get("baz"):exists())
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("true", result);
}

TEST_F(script_test, negative_exists) {
  std::string script = R"(
function run(doc)
  return tostring(doc:get("non"):get("existant"):get("value"):exists())
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("false", result);
}

TEST_F(script_test, positive_is_composite) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("bar"):get("baz"):set("Hello")

  return tostring(doc:get("foo"):get("bar"):is_composite())
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("true", result);
}

TEST_F(script_test, negative_is_composite) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):set("Hello")

  return tostring(doc:get("foo"):is_composite())
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("false", result);
}

TEST_F(script_test, remove) {
  std::string script = R"(
function run(doc)
  doc:get("foo"):get("bar"):get("baz"):set("Hello")
  created = doc:get("foo"):exists()

  doc:get("foo"):remove();
  existig = doc:get("foo"):exists()

  return tostring(created and not existing)
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("true", result);
}

TEST_F(script_test, context_pollution) {
  std::string write_script = R"(
pollution = "foobar"

function run(doc)
  return "script_1"
end
)";

  std::string read_script = R"(
function run(doc)
  return tostring(pollution)
end
)";

  server_.apply_script(write_script);
  std::string result = server_.apply_script(read_script);

  ASSERT_NE(result, "foobar");
}

TEST_F(script_test, non_string_return_nil) {
  std::string script = R"(
function run(doc)
  return nil
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("nil", result);
}

TEST_F(script_test, invalid_syntax) {
  std::string script = R"(
function run(doc)
  Cannot parse this!
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_TRUE(result.find("expected") != std::string::npos);
}

TEST_F(script_test, missing_return) {
  std::string script = R"(
function run(doc)
end
)";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("nil", result);
}

TEST_F(script_test, missing_run_method) {
  std::string script = "";

  std::string result = server_.apply_script(script);
  ASSERT_EQ("run method not defined", result);
}
