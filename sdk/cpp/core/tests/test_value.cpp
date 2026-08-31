#include <spdlog/spdlog.h>

#include <iostream>

#include "../src/types.hpp"
#include "catch.hpp"

using namespace ydk;

class TestIdentity1 : public Identity {
 public:
  TestIdentity1() : Identity("http://test.com", "test", "test-identity") {}
};

class TestEnum1 : public Enum {
 public:
  TestEnum1() {}
  ~TestEnum1() {}

  static const Enum::YLeaf one;
  static const Enum::YLeaf two;

  static int get_enum_value(const std::string& name) {
    if (name == "one") return 1;
    if (name == "two") return 2;
    return -1;
  }
};

const Enum::YLeaf TestEnum1::one{1, "one"};
const Enum::YLeaf TestEnum1::two{2, "two"};

TEST_CASE("test_uint8") {
  YLeaf test_value{YType::uint8, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_uint16") {
  YLeaf test_value{YType::uint16, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_uint32") {
  YLeaf test_value{YType::uint32, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_uint64") {
  YLeaf test_value{YType::uint64, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_int8") {
  YLeaf test_value{YType::int8, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_int16") {
  YLeaf test_value{YType::int16, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_int32") {
  YLeaf test_value{YType::int32, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_int64") {
  YLeaf test_value{YType::int64, "name"};
  test_value = 4;
  REQUIRE(test_value.get() == "4");
}

TEST_CASE("test_empty") {
  YLeaf test_value{YType::empty, "name"};
  test_value = Empty();
  REQUIRE(test_value.get() == "");
}

TEST_CASE("test_identity") {
  YLeaf test_value{YType::identityref, "name"};
  test_value = TestIdentity1{};
  INFO(test_value.get());
  REQUIRE(test_value.get() == "test-identity");
  REQUIRE(test_value.value_namespace == "http://test.com");
  REQUIRE(test_value.value_namespace_prefix == "test");
}

TEST_CASE("test_enum_") {
  YLeaf test_value{YType::enumeration, "enumval"};
  test_value = TestEnum1::one;
  INFO(test_value.get());
  REQUIRE(test_value.get() == "one");
  REQUIRE(test_value.enum_value == 1);
  REQUIRE(TestEnum1::get_enum_value("one") == 1);
  REQUIRE(TestEnum1::get_enum_value("two") == 2);
  REQUIRE(TestEnum1::get_enum_value("abc") == -1);
}

TEST_CASE("test_str") {
  YLeaf test_value{YType::str, "name"};
  test_value = "hello";
  REQUIRE(test_value.get() == "hello");
}

TEST_CASE("test_bool") {
  YLeaf test_value{YType::boolean, "name"};
  test_value = true;
  REQUIRE(test_value.get() == "true");

  test_value = false;
  REQUIRE(test_value.get() == "false");
}

TEST_CASE("test_bits") {
  YLeaf test_value{YType::bits, "bits-field"};
  test_value["bit1"] = true;
  test_value["bit2"] = true;
  test_value["bit3"] = true;
  test_value["bit4"] = true;
  REQUIRE(test_value.get() == "bit1 bit2 bit3 bit4");

  test_value["bit3"] = false;
  REQUIRE(test_value.get() == "bit1 bit2 bit4");
}

TEST_CASE("test_bits_assign") {
  YLeaf test_value{YType::bits, "bits-field"};
  Bits test{};
  test["bit1"] = true;
  test["bit2"] = true;
  test["bit3"] = true;
  test["bit4"] = true;
  test_value = test;
  REQUIRE(test_value.get() == "bit1 bit2 bit3 bit4");

  test["bit3"] = false;
  test_value = test;
  REQUIRE(test_value.get() == "bit1 bit2 bit4");

  std::vector<YLeaf> vs;
  vs.push_back(test_value);
  REQUIRE(vs[0].get() == "bit1 bit2 bit4");
  REQUIRE(test_value.get() == "bit1 bit2 bit4");
}

TEST_CASE("test_deci64") {
  YLeaf test_value{YType::decimal64, "value"};
  test_value = Decimal64("3.2");
  REQUIRE(test_value.get() == "3.2");

  test_value = Decimal64("1.2");
  REQUIRE(test_value.get() == "1.2");
}

// ============== YLeaf LeafRef Metadata Tests ==============

TEST_CASE("test_yleaf_leafref_constructor") {
  // Test default constructor - no leafref
  YLeaf yleaf_default{YType::str, "myname"};
  REQUIRE(yleaf_default.is_leafref() == false);
  REQUIRE(yleaf_default.get_leafref_path() == "");
  
  // Test leafref constructor
  YLeaf yleaf_leafref{YType::str, "myname", "/some/path"};
  REQUIRE(yleaf_leafref.is_leafref() == true);
  REQUIRE(yleaf_leafref.get_leafref_path() == "/some/path");
}

TEST_CASE("test_yleaf_leafref_get_name_leafdata") {
  YLeaf yleaf_leafref{YType::str, "myname", "/config/target"};
  yleaf_leafref = "test-value";
  
  auto leaf_data = yleaf_leafref.get_name_leafdata();
  REQUIRE(leaf_data.first == "myname");
  REQUIRE(leaf_data.second.value == "test-value");
  REQUIRE(leaf_data.second.is_leafref == true);
  REQUIRE(leaf_data.second.leafref_path == "/config/target");
}

TEST_CASE("test_yleaf_non_leafref_baseline") {
  YLeaf yleaf_non_leafref{YType::int32, "counter"};
  yleaf_non_leafref = 42;
  
  auto leaf_data = yleaf_non_leafref.get_name_leafdata();
  REQUIRE(leaf_data.second.value == "42");
  REQUIRE(leaf_data.second.is_leafref == false);
  REQUIRE(leaf_data.second.leafref_path == "");
}

TEST_CASE("test_yleaf_leafref_copy_move") {
  YLeaf original{YType::str, "orig", "/path/to/ref"};
  original = "original-value";
  
  // Copy constructor
  YLeaf copy_ctor{original};
  REQUIRE(copy_ctor.is_leafref() == true);
  REQUIRE(copy_ctor.get_leafref_path() == "/path/to/ref");
  REQUIRE(copy_ctor.get() == "original-value");
  
  // Copy assignment
  YLeaf copy_assign{YType::str, "temp"};
  copy_assign = original;
  REQUIRE(copy_assign.is_leafref() == true);
  REQUIRE(copy_assign.get_leafref_path() == "/path/to/ref");
  
  // Move constructor
  YLeaf move_ctor{std::move(YLeaf{YType::str, "move_orig", "/move/path"})};
  REQUIRE(move_ctor.is_leafref() == true);
  REQUIRE(move_ctor.get_leafref_path() == "/move/path");
}
