#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/bencode/decode.hpp"

//Test for decoding strings
TEST(DecodeTests, DecodeStrings) {
  // Valid cases
  EXPECT_EQ(decode_string("5:Hello"), "\"Hello\""); //normal word 
  EXPECT_EQ(decode_string("11:Hello world"), "\"Hello world\""); //two word string
  EXPECT_EQ(decode_string("0:"), "\"\""); //empty string

  // Invalid cases
  EXPECT_THROW(decode_string("Hello"), std::runtime_error); // Missing number
  EXPECT_THROW(decode_string("5Hello"), std::runtime_error); // Missing column
  EXPECT_THROW(decode_string("6:Hello"), std::runtime_error); // Length > word length
  EXPECT_THROW(decode_string("-3:abc"), std::runtime_error); // Negative length
  EXPECT_THROW(decode_string("3a:hello"), std::runtime_error); // Non-integer length
  EXPECT_THROW(decode_string(""), std::runtime_error); // Empty input
}

TEST(DecodeTests, DecodeIntegers){
  EXPECT_EQ(decode_int("i52e"), "52"); //positive number
  EXPECT_EQ(decode_int("i-40e"), "-40"); //negative number
  EXPECT_EQ(decode_int("i0e"), "0"); //zero
  EXPECT_EQ(decode_int("i123456789012345e"), "123456789012345"); //very large number
  
  EXPECT_THROW(decode_int("i10"), std::runtime_error); //missing e
  EXPECT_THROW(decode_int("i10ee"), std::runtime_error); //extra e
  EXPECT_THROW(decode_int("i10ae"), std::runtime_error); //invalid number
}

TEST(DecodeTests, DecodeLists) {
  // Valid cases
  EXPECT_EQ(decode_list("l5:hello5:worlde"), "[\"hello\",\"world\"]"); // Simple list with strings
  EXPECT_EQ(decode_list("l5:helloe"), "[\"hello\"]"); // Single-element list
  EXPECT_EQ(decode_list("l5:helloi42ee"), "[\"hello\",42]"); // Mixed types (string and integer)
  EXPECT_EQ(decode_list("le"), "[]"); // Empty list
  EXPECT_EQ(decode_list("li9223372036854775807ee"), "[9223372036854775807]"); // Large integer
  EXPECT_EQ(decode_list("li-42ee"), "[-42]"); // Negative integer
  EXPECT_EQ(decode_list("li42ei-7ei0ee"), "[42,-7,0]"); // Multiple integers
  EXPECT_EQ(decode_list("l0:e"), "[\"\"]"); // List with empty string
  EXPECT_EQ(decode_list("l5:helloi42e3:catl4:dogsee"), "[\"hello\",42,\"cat\",[\"dogs\"]]"); // recursive lists

  // Invalid cases
  EXPECT_THROW(decode_list("l5:hello"), std::runtime_error); // Missing 'e'
  EXPECT_THROW(decode_list("l5:helloxe"), std::runtime_error); // Unexpected character
  EXPECT_THROW(decode_list("l10:hello12345e"), std::runtime_error); // String length exceeds input
  EXPECT_THROW(decode_list("l"), std::runtime_error); // Incomplete list
  EXPECT_THROW(decode_list("li42"), std::runtime_error); // Unterminated integer in list
  EXPECT_THROW(decode_list("l5:helloee"), std::runtime_error); // Extra 'e'
}

int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

