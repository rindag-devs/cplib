#include <string>
#include <vector>

#include "cplib.hpp"
#include "gtest/gtest.h"

TEST(CmdArgsTest, Parsing) {
  std::vector<std::string> raw = {"pos0", "--flag", "--key=value", "--opt", "val", "pos1", "pos2"};

  cplib::cmd_args::ParsedArgs args(raw);

  EXPECT_TRUE(args.has_flag("flag"));

  ASSERT_TRUE(args.vars.count("key"));
  EXPECT_EQ(args.vars.at("key"), "value");

  ASSERT_TRUE(args.vars.count("opt"));
  EXPECT_EQ(args.vars.at("opt"), "val");

  ASSERT_EQ(args.ordered.size(), 3);
  EXPECT_EQ(args.ordered[0], "pos0");
  EXPECT_EQ(args.ordered[1], "pos1");
  EXPECT_EQ(args.ordered[2], "pos2");
}
