#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/torrent/torrent.hpp"

TEST(ParseTest, parse){
  TorrentHelper helper;

  //path are specified assuming cwd is build/ when test is executed
  std::string basePath = "../test/torrent_files/";
  EXPECT_EQ(helper.parseTorrentFile(basePath + "good.torrent"), true);
  EXPECT_EQ(helper.parseTorrentFile(basePath + "missing_announce.torrent"), false);
  EXPECT_EQ(helper.parseTorrentFile(basePath + "missing_info.torrent"), false);
  EXPECT_EQ(helper.parseTorrentFile(basePath + "missing_name.torrent"), false);
  EXPECT_EQ(helper.parseTorrentFile(basePath + "missing_piecelength.torrent"), false);
}
