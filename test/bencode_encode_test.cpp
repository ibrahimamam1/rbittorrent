#include "../src/bencode/encode.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(EncodeTest, EncodeStrings) {
  // Input data as BencodeValue containing strings
  std::vector<BencodeValue> test_values = {
      BencodeValue("Hello"), BencodeValue("Hello World"),
      BencodeValue(""), // Empty string
      BencodeValue(" ") // String with a space
  };

  // Expected Bencode outputs
  std::vector<std::string> expected_values = {"5:Hello", "11:Hello World",
                                              "0:", "1: "};

  for (size_t i = 0; i < test_values.size(); ++i) {
    EXPECT_EQ(BencodeEncoder::encode(test_values[i]), expected_values[i])
        << "Test case failed at index " << i;
  }
}

TEST(EncodeTest, EncodeInts) {
  // Input data as BencodeValue containing long long
  std::vector<BencodeValue> test_values = {
      BencodeValue(123LL), BencodeValue(-123LL), BencodeValue(0LL)};

  // Expected Bencode outputs
  std::vector<std::string> expected_values = {"i123e", "i-123e", "i0e"};

  for (size_t i = 0; i < test_values.size(); ++i) {
    EXPECT_EQ(BencodeEncoder::encode(test_values[i]), expected_values[i])
        << "Test case failed at index " << i;
  }
}

TEST(EncodeTest, EncodeLists) {
  // Input data as BencodeValue containing BencodeList
  std::vector<BencodeValue> test_values = {
      // [orange, banana]
      BencodeValue(BencodeList{BencodeValue("orange"), BencodeValue("banana")}),
      // [orange, [red, blue]]
      BencodeValue(
          BencodeList{BencodeValue("orange"),
                      BencodeValue(BencodeList{// Nested list
                                               BencodeValue("red"),
                                               BencodeValue("blue")})}),
      // []
      BencodeValue(BencodeList{}) // Empty list
  };

  // Expected Bencode outputs
  std::vector<std::string> expected_values = {"l6:orange6:bananae",
                                              "l6:orangel3:red4:blueee", "le"};

  for (size_t i = 0; i < test_values.size(); ++i) {
    EXPECT_EQ(BencodeEncoder::encode(test_values[i]), expected_values[i])
        << "Test case failed at index " << i;
  }
}

TEST(EncodeTest, EncodeDictionaries) {
  // Input data as BencodeValue containing BencodeDict
  std::vector<BencodeValue> test_values = {
      // {orange:banana, red:blue} -> keys: orange, red
      BencodeValue(BencodeDict{{"orange", BencodeValue("banana")},
                               {"red", BencodeValue("blue")}}),
      // {orange:[red,blue], banana:yellow} -> keys: orange, banana
      BencodeValue(BencodeDict{
          {"orange", BencodeValue(BencodeList{BencodeValue("red"),
                                              BencodeValue("blue")})},
          {"banana", BencodeValue("yellow")}}),
      // {}
      BencodeValue(BencodeDict{}), // Empty dictionary
      // {fizz:{foo:bar,spam:dam}, buzz:{bar:foo}, dam:spam} -> keys: fizz,
      // buzz, dam
      BencodeValue(BencodeDict{
          {"fizz", BencodeValue(BencodeDict{
                       // Nested dict 1
                       {"foo", BencodeValue("bar")}, // keys: foo, spam
                       {"spam", BencodeValue("dam")}})},
          {"buzz", BencodeValue(BencodeDict{
                       // Nested dict 2
                       {"bar", BencodeValue("foo")} // keys: bar
                   })},
          {"dam", BencodeValue("spam")}})};

  // Expected Bencode outputs (keys sorted alphabetically)
  std::vector<std::string> expected_values = {
      // Keys: orange, red -> Sorted: orange, red
      "d6:orange6:banana3:red4:bluee",
      // Keys: orange, banana -> Sorted: banana, orange
      "d6:banana6:yellow6:orangel3:red4:blueee",
      // Empty Dic:t
      "de",
      // Keys: fizz, buzz, dam -> Sorted: buzz, dam, fizz
      // Nested fizz keys: foo, spam -> Sorted: foo, spam
      // Nested buzz keys: bar -> Sorted: bar
      "d4:buzzd3:bar3:fooe3:dam4:spam4:fizzd3:foo3:bar4:spam3:damee"};

  for (size_t i = 0; i < test_values.size(); ++i) {
    EXPECT_EQ(BencodeEncoder::encode(test_values[i]), expected_values[i])
        << "Test case failed at index " << i;
  }
}
