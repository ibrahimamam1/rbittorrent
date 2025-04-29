#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/bencode/decode.hpp"

//Test for decoding strings
TEST(DecodeTests, DecodeStrings) {
  // Valid cases
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("5:Hello").dump(), "\"Hello\""); //normal word 
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("11:Hello world").dump(), "\"Hello world\""); //two word string
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("0:").dump(), "\"\""); //empty string

  // Invalid cases (EXPECT_THROW checks if calling the function throws, 
  // so adding .dump() which operates on the return value is not applicable)
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("Hello"), std::runtime_error); // Missing number
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("5Hello"), std::runtime_error); // Missing column
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("6:Hello"), std::runtime_error); // Length > word length
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("-3:abc"), std::runtime_error); // Negative length
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("3a:hello"), std::runtime_error); // Non-integer length
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value(""), std::runtime_error); // Empty input
}

TEST(DecodeTests, DecodeIntegers){
  // Valid cases
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("i52e").dump(), "\"52\""); //positive number
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("i-40e").dump(), "\"-40\""); //negative number
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("i0e").dump(), "\"0\""); //zero
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("i123456789012345e").dump(), "\"123456789012345\""); //very large number
  
  // Invalid cases (EXPECT_THROW checks if calling the function throws)
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("i10"), std::runtime_error); //missing e
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("i10ee"), std::runtime_error); //extra e
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("i10ae"), std::runtime_error); //invalid number
}

TEST(DecodeTests, DecodeLists) {
  // Valid cases
  // This case already dumps a variable holding the result, which is equivalent
  json decoded = BencodeDecoder::decode_bencoded_value("l5:hello5:worlde");
  EXPECT_EQ(decoded.dump(), "[\"hello\",\"world\"]"); 
  
  // Cases where the return value is directly compared
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("l5:helloe").dump(), "[\"hello\"]"); // Single-element list
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("l5:helloi42ee").dump(), "[\"hello\",\"42\"]"); // Mixed types (string and integer)
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("le").dump(), "null"); // Empty list (json::dump() for an empty list returns "null" for some versions/configs, or "[]") - Assuming "null" based on original test
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("li9223372036854775807ee").dump(), "[\"9223372036854775807\"]"); // Large integer
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("li-42ee").dump(), "[\"-42\"]"); // Negative integer
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("li42ei-7ei0ee").dump(), "[\"42\",\"-7\",\"0\"]"); // Multiple integers
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("l0:e").dump(), "[\"\"]"); // List with empty string
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("l5:helloi42e3:catl4:dogsee").dump(), "[\"hello\",\"42\",\"cat\",[\"dogs\"]]"); // recursive lists

  // Invalid cases (EXPECT_THROW checks if calling the function throws)
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("l5:hello"), std::runtime_error); // Missing 'e'
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("l5:helloxe"), std::runtime_error); // Unexpected character
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("l10:hello123456e"), std::runtime_error); // String length exceeds input
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("l"), std::runtime_error); // Incomplete list
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("li42"), std::runtime_error); // Unterminated integer in list
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("l5:helloee"), std::runtime_error); // Extra 'e'
}

TEST(DecodeTests, DecodeDictionaries) {
  // Valid cases
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("d3:bar4:spam3:fooi42ee").dump(), "{\"bar\":\"spam\",\"foo\":\"42\"}");
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("de").dump(), "null"); // Empty dictionary (json::dump() for an empty dictionary returns "null" for some versions/configs, or "{}") - Assuming "null" based on original test
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("d0:0:e").dump(), "{\"\":\"\"}"); // Empty key and value
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("d3:key5:valuee").dump(), "{\"key\":\"value\"}");
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("d4:spaml5:hello5:worldee").dump(), "{\"spam\":[\"hello\",\"world\"]}");
  // Nested dictionary
  EXPECT_EQ(BencodeDecoder::decode_bencoded_value("d3:food3:bar4:spamee").dump(), "{\"foo\":{\"bar\":\"spam\"}}");

  // Invalid cases (EXPECT_THROW checks if calling the function throws)
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("d3:bar4:spam3:fooi42e"), std::runtime_error); // Missing final 'e'
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("d3:bar4:spam3:fooi42eee"), std::runtime_error); // Extra 'e'
  EXPECT_THROW(BencodeDecoder::decode_bencoded_value("d3:bar4:spam3:foo"), std::runtime_error); // Incomplete dictionary
}
