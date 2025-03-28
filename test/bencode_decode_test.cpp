#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/bencode/decode.hpp"

//Test for decoding strings
TEST(DecodeTests, DecodeStrings) {
  // Valid cases
  EXPECT_EQ(decode_bencoded_value("5:Hello"), "\"Hello\""); //normal word 
  EXPECT_EQ(decode_bencoded_value("11:Hello world"), "\"Hello world\""); //two word string
  EXPECT_EQ(decode_bencoded_value("0:"), "\"\""); //empty string

  // Invalid cases
  EXPECT_THROW(decode_bencoded_value("Hello"), std::runtime_error); // Missing number
  EXPECT_THROW(decode_bencoded_value("5Hello"), std::runtime_error); // Missing column
  EXPECT_THROW(decode_bencoded_value("6:Hello"), std::runtime_error); // Length > word length
  EXPECT_THROW(decode_bencoded_value("-3:abc"), std::runtime_error); // Negative length
  EXPECT_THROW(decode_bencoded_value("3a:hello"), std::runtime_error); // Non-integer length
  EXPECT_THROW(decode_bencoded_value(""), std::runtime_error); // Empty input
}

TEST(DecodeTests, DecodeIntegers){
  EXPECT_EQ(decode_bencoded_value("i52e"), "\"52\""); //positive number
  EXPECT_EQ(decode_bencoded_value("i-40e"), "\"-40\""); //negative number
  EXPECT_EQ(decode_bencoded_value("i0e"), "\"0\""); //zero
  EXPECT_EQ(decode_bencoded_value("i123456789012345e"), "\"123456789012345\""); //very large number
  
  EXPECT_THROW(decode_bencoded_value("i10"), std::runtime_error); //missing e
  EXPECT_THROW(decode_bencoded_value("i10ee"), std::runtime_error); //extra e
  EXPECT_THROW(decode_bencoded_value("i10ae"), std::runtime_error); //invalid number
}

TEST(DecodeTests, DecodeLists) {
  // Valid cases
  EXPECT_EQ(decode_bencoded_value("l5:hello5:worlde"), "[\"hello\",\"world\"]"); // Simple list with strings
  EXPECT_EQ(decode_bencoded_value("l5:helloe"), "[\"hello\"]"); // Single-element list
  EXPECT_EQ(decode_bencoded_value("l5:helloi42ee"), "[\"hello\",\"42\"]"); // Mixed types (string and integer)
  EXPECT_EQ(decode_bencoded_value("le"), "null"); // Empty list
  EXPECT_EQ(decode_bencoded_value("li9223372036854775807ee"), "[\"9223372036854775807\"]"); // Large integer
  EXPECT_EQ(decode_bencoded_value("li-42ee"), "[\"-42\"]"); // Negative integer
  EXPECT_EQ(decode_bencoded_value("li42ei-7ei0ee"), "[\"42\",\"-7\",\"0\"]"); // Multiple integers
  EXPECT_EQ(decode_bencoded_value("l0:e"), "[\"\"]"); // List with empty string
  EXPECT_EQ(decode_bencoded_value("l5:helloi42e3:catl4:dogsee"), "[\"hello\",\"42\",\"cat\",[\"dogs\"]]"); // recursive lists

  // Invalid cases
  EXPECT_THROW(decode_bencoded_value("l5:hello"), std::runtime_error); // Missing 'e'
  EXPECT_THROW(decode_bencoded_value("l5:helloxe"), std::runtime_error); // Unexpected character
  EXPECT_THROW(decode_bencoded_value("l10:hello123456e"), std::runtime_error); // String length exceeds input
  EXPECT_THROW(decode_bencoded_value("l"), std::runtime_error); // Incomplete list
  EXPECT_THROW(decode_bencoded_value("li42"), std::runtime_error); // Unterminated integer in list
  EXPECT_THROW(decode_bencoded_value("l5:helloee"), std::runtime_error); // Extra 'e'
}

TEST(DecodeTests, DecodeDictionaries) {
  // Valid cases
  EXPECT_EQ(decode_bencoded_value("d3:bar4:spam3:fooi42ee"), "{\"bar\":\"spam\",\"foo\":\"42\"}");
  EXPECT_EQ(decode_bencoded_value("de"), "null"); // Empty dictionary
  EXPECT_EQ(decode_bencoded_value("d0:0:e"), "{\"\":\"\"}"); // Empty key and value
  EXPECT_EQ(decode_bencoded_value("d3:key5:valuee"), "{\"key\":\"value\"}");
  EXPECT_EQ(decode_bencoded_value("d4:spaml5:hello5:worldee"), "{\"spam\":[\"hello\",\"world\"]}");
  // Nested dictionary
  EXPECT_EQ(decode_bencoded_value("d3:food3:bar4:spamee"), "{\"foo\":{\"bar\":\"spam\"}}");

  // Invalid cases
  EXPECT_THROW(decode_bencoded_value("d3:bar4:spam3:fooi42e"), std::runtime_error); // Missing final 'e'
  EXPECT_THROW(decode_bencoded_value("d3:bar4:spam3:fooi42eee"), std::runtime_error); // Extra 'e'
  EXPECT_THROW(decode_bencoded_value("d3:bar4:spam3:foo"), std::runtime_error); // Incomplete dictionary
}


int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

