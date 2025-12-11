#include <string>

#include "cplib.hpp"
#include "gtest/gtest.h"

using cplib::FlatMap;

TEST(FlatMapTest, BasicOperations) {
  FlatMap<int, std::string> map;

  EXPECT_TRUE(map.empty());

  map[1] = "one";
  map.insert({2, "two"});
  map.emplace(3, "three");

  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map.at(1), "one");
  EXPECT_EQ(map[2], "two");

  EXPECT_TRUE(map.contains(3));
  EXPECT_FALSE(map.contains(4));

  map.erase(2);
  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.contains(2));
}

TEST(FlatMapTest, InitializerList) {
  FlatMap<int, int> map = {{1, 10}, {3, 30}, {2, 20}};
  // Should be sorted
  auto it = map.begin();
  EXPECT_EQ(it->first, 1);
  EXPECT_EQ((++it)->first, 2);
  EXPECT_EQ((++it)->first, 3);
}

TEST(FlatMapTest, LowerBound) {
  FlatMap<int, int> map = {{10, 1}, {20, 2}, {30, 3}};

  auto it = map.lower_bound(15);
  ASSERT_NE(it, map.end());
  EXPECT_EQ(it->first, 20);

  auto it2 = map.lower_bound(31);
  EXPECT_EQ(it2, map.end());
}
