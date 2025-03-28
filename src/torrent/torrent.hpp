#include <string>
#include <vector>

#include "../lib/nlohmann/json.hpp"

typedef struct file{
  std::string name;
  size_t length;
}File;

typedef struct torrent{
  std::string tracker_url;
  std::string piece_length;
  std::string name;
  std::string pieces;
  size_t length = 0;
  std::vector<File>files;
}Torrent;

Torrent parse_torrent_file(std::string filepath);
