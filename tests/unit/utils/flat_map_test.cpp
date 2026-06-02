#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "cplib.hpp"
#include "gtest/gtest.h"

using cplib::FlatMap;

namespace {
struct Greater {
  auto operator()(int lhs, int rhs) const -> bool { return lhs > rhs; }
};
}  // namespace

TEST(FlatMapTest, ConstructorsAndCapacity) {
  FlatMap<int, std::string> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.size(), 0);
  EXPECT_LE(empty.capacity(), empty.max_size());

  empty.reserve(8);
  EXPECT_GE(empty.capacity(), 8);

  FlatMap<int, std::string> from_init = {{3, "three"}, {1, "one"}, {2, "two"}, {2, "ignored"}};
  ASSERT_EQ(from_init.size(), 3);
  EXPECT_EQ(from_init.begin()->first, 1);
  EXPECT_EQ(from_init.begin()->second, "one");

  const std::array<std::pair<int, std::string_view>, 4> items = {
      std::pair{4, std::string_view("four")},
      std::pair{2, std::string_view("two")},
      std::pair{3, std::string_view("three")},
      std::pair{2, std::string_view("duplicate")},
  };
  FlatMap<int, std::string_view> from_range(items.begin(), items.end());
  ASSERT_EQ(from_range.size(), 3);
  EXPECT_EQ(from_range.find(2)->second, "two");

  FlatMap<int, int, Greater> desc(Greater{});
  desc.insert({1, 10});
  desc.insert({3, 30});
  desc.insert({2, 20});
  ASSERT_EQ(desc.size(), 3);
  EXPECT_EQ(desc.begin()->first, 3);
  EXPECT_EQ(desc.rbegin()->first, 1);
}

TEST(FlatMapTest, ElementAccessAndModifiers) {
  FlatMap<int, std::string> map;

  map[2] = "two";
  map[1] = "one";
  EXPECT_EQ(map.at(1), "one");
  EXPECT_EQ(map.at(2), "two");

  std::string three = "three";
  auto [it_insert, inserted] = map.insert({3, three});
  EXPECT_TRUE(inserted);
  EXPECT_EQ(it_insert->second, "three");

  auto [it_dup, inserted_dup] = map.insert({3, "tres"});
  EXPECT_FALSE(inserted_dup);
  EXPECT_EQ(it_dup->second, "three");

  auto [it_emplace, emplaced] = map.emplace(4, "four");
  EXPECT_TRUE(emplaced);
  EXPECT_EQ(it_emplace->second, "four");

  auto [it_emplace_dup, emplaced_dup] = map.emplace(4, "ignored");
  EXPECT_FALSE(emplaced_dup);
  EXPECT_EQ(it_emplace_dup->second, "four");

  auto erased_it = map.erase(map.find(2));
  ASSERT_NE(erased_it, map.end());
  EXPECT_EQ(erased_it->first, 3);
  EXPECT_FALSE(map.contains(2));

  EXPECT_EQ(map.erase(4), 1);
  EXPECT_EQ(map.erase(4), 0);
  EXPECT_FALSE(map.contains(4));
}

TEST(FlatMapTest, LookupFunctions) {
  FlatMap<int, int> map = {{10, 1}, {20, 2}, {30, 3}};
  const auto &const_map = map;

  auto find_it = map.find(20);
  ASSERT_NE(find_it, map.end());
  EXPECT_EQ(find_it->second, 2);
  EXPECT_EQ(map.find(99), map.end());
  EXPECT_EQ(const_map.find(10)->second, 1);

  EXPECT_TRUE(map.contains(30));
  EXPECT_FALSE(map.contains(40));

  auto lower_existing = map.lower_bound(20);
  ASSERT_NE(lower_existing, map.end());
  EXPECT_EQ(lower_existing->first, 20);

  auto lower_middle = map.lower_bound(25);
  ASSERT_NE(lower_middle, map.end());
  EXPECT_EQ(lower_middle->first, 30);

  auto lower_end = map.lower_bound(31);
  EXPECT_EQ(lower_end, map.end());

  auto upper_existing = map.upper_bound(20);
  ASSERT_NE(upper_existing, map.end());
  EXPECT_EQ(upper_existing->first, 30);

  auto upper_before_all = map.upper_bound(5);
  ASSERT_NE(upper_before_all, map.end());
  EXPECT_EQ(upper_before_all->first, 10);

  auto upper_end = const_map.upper_bound(30);
  EXPECT_EQ(upper_end, const_map.end());
}

TEST(FlatMapTest, IteratorsAndComparison) {
  FlatMap<int, std::string> map = {{2, "two"}, {1, "one"}, {3, "three"}};
  const auto &const_map = map;

  ASSERT_NE(map.begin(), map.end());
  EXPECT_EQ(map.begin()->first, 1);
  EXPECT_EQ(const_map.begin()->first, 1);
  EXPECT_EQ(const_map.cbegin()->first, 1);

  EXPECT_EQ(map.rbegin()->first, 3);
  EXPECT_EQ(const_map.rbegin()->first, 3);
  EXPECT_EQ(const_map.crbegin()->first, 3);

  auto end_it = map.end();
  --end_it;
  EXPECT_EQ(end_it->first, 3);

  auto rend_it = const_map.rend();
  --rend_it;
  EXPECT_EQ(rend_it->first, 1);
  auto crend_it = const_map.crend();
  --crend_it;
  EXPECT_EQ(crend_it->first, 1);

  FlatMap<int, std::string> same = {{1, "one"}, {2, "two"}, {3, "three"}};
  FlatMap<int, std::string> larger = {{1, "one"}, {2, "two"}, {4, "four"}};
  EXPECT_TRUE(map == same);
  EXPECT_TRUE((map <=> same) == 0);
  EXPECT_TRUE((map <=> larger) < 0);
}
