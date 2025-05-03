#include <vector>
#include <string>
#include <map>
#include <variant>

// --- Defining Bencode Data Types ---
struct BencodeValue;
using BencodeList = std::vector<BencodeValue>;
using BencodeDict = std::map<std::string, BencodeValue>;

// Using std::variant to hold any possible Bencode type
using BencodeVariant = std::variant<
    std::string,
    long long,
    BencodeList,
    BencodeDict
>;

// Wrapper struct to allow easy recursive definition
struct BencodeValue : BencodeVariant {
    // Inherit constructors from std::variant
    using BencodeVariant::BencodeVariant;
};

