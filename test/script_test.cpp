#include <memory>

#include "gtest/gtest.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/asio/io_service.hpp"

#include "dust/document.h"
#include "dust/storage/mem_store.h"
#include "dust-server/lua_connection.h"

using namespace dust;

class script_test : public testing::Test {
 public:
  script_test()
      : store_(std::make_shared<mem_store>()),
        io_service_(),
        lua_con_(store_)  {
  }

 protected:
  std::shared_ptr<key_value_store> store_;
  boost::asio::io_service* io_service_;
  dust_server::lua_connection lua_con_;
};
TEST_F(script_test, assign_and_read) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("h"):set("Hello")
  doc:get("foo"):get("w"):set(", World")
  return doc:get("foo"):get("h"):val() .. doc:get("foo"):get("w"):val()
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}

TEST_F(script_test, value_of_nonexistent_entry) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  result = doc:get("non"):get("existant"):val()
  return result
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("error: Database value does not exist", result);
}

TEST_F(script_test, override_composite_with_value) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("bar"):set("Hello")
  doc:get("foo"):set("impossible")
  return "test failed"
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("error: Can't override value with composite", result);
}

TEST_F(script_test, override_value_with_composite) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):set("Hello")
  doc:get("foo"):get("bar"):set("impossible")
  return "test failed"
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("error: Inconsistent database: parent is not a composite", result);
}

TEST_F(script_test, children) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("bar"):set("Hello, ")
  doc:get("foo"):get("baz"):set("World")

  children = doc:get("foo"):children()

  return children[0]:val() .. children[1]:val()
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("Hello, World", result);
}

TEST_F(script_test, children_count_operator) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("a"):set("1")
  doc:get("foo"):get("b"):set("2")
  doc:get("foo"):get("c"):set("3")
  doc:get("foo"):get("d"):set("4")
  doc:get("foo"):get("e"):set("5")

  children = doc:get("foo"):children()

  return tostring(#children)
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("5", result);
}

TEST_F(script_test, index) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("bar"):get("baz"):set("Hello")

  return doc:get("foo"):get("bar"):get("baz"):index()
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("baz", result);
}

TEST_F(script_test, positive_exists) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("bar"):get("baz"):set("Hello")

  return tostring(doc:get("foo"):get("bar"):get("baz"):exists())
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("true", result);
}

TEST_F(script_test, negative_exists) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  return tostring(doc:get("non"):get("existant"):get("value"):exists())
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("false", result);
}

TEST_F(script_test, positive_is_composite) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("bar"):get("baz"):set("Hello")

  return tostring(doc:get("foo"):get("bar"):is_composite())
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("true", result);
}

TEST_F(script_test, negative_is_composite) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):set("Hello")

  return tostring(doc:get("foo"):is_composite())
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("false", result);
}

TEST_F(script_test, remove) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("bar"):get("baz"):set("Hello")
  created = doc:get("foo"):exists()

  doc:get("foo"):remove();
  existig = doc:get("foo"):exists()

  return tostring(created and not existing)
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("true", result);
}

TEST_F(script_test, context_pollution) {
  std::string write_script = R"(
pollution = "foobar"

function run(db)
  local doc = db:get_document("users")
  return "script_1"
end
)";

  std::string read_script = R"(
function run(db)
  local doc = db:get_document("users")
  return tostring(pollution)
end
)";

  lua_con_.apply_script(write_script);
  std::string result = lua_con_.apply_script(read_script);

  ASSERT_NE(result, "foobar");
}

TEST_F(script_test, document_return_json) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("a"):set("1")
  doc:get("foo"):get("b"):set("2")
  doc:get("foo"):get("c"):set("3")
  doc:get("foo"):get("d"):set("4")
  doc:get("foo"):get("e"):get("XY"):set("5")
  return tostring(doc:get("foo"))
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ(R"({"a":"1","b":"2","c":"3","d":"4","e":{"XY":"5"}})", result);
}

TEST_F(script_test, root_document_return_json) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:get("foo"):get("a"):set("1")
  doc:get("foo"):get("b"):set("2")
  doc:get("foo"):get("c"):set("3")
  doc:get("foo"):get("d"):set("4")
  doc:get("foo"):get("e"):get("XY"):set("5")
  return tostring(doc)
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ(R"({"foo":{"a":"1","b":"2","c":"3","d":"4","e":{"XY":"5"}}})", result);
}

TEST_F(script_test, from_json) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  doc:from_json('{"foo":{"a":"1","b":"2","c":"3","d":"4","e":{"XY":"5"}}}')
  return tostring(doc)
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ(R"({"foo":{"a":"1","b":"2","c":"3","d":"4","e":{"XY":"5"}}})", result);
}


TEST_F(script_test, non_string_return_nil) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  return nil
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("error: non-string return", result);
}

TEST_F(script_test, invalid_syntax) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
  Cannot parse this!
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_TRUE(result.find("expected") != std::string::npos);
}

TEST_F(script_test, missing_return) {
  std::string script = R"(
function run(db)
  local doc = db:get_document("users")
end
)";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("error: non-string return", result);
}

TEST_F(script_test, missing_run_method) {
  std::string script = "";

  std::string result = lua_con_.apply_script(script);
  ASSERT_EQ("run method not defined", result);
}
