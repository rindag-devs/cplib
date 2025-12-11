#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "cplib.hpp"
#include "gtest/gtest.h"

using cplib::Random;

TEST(RandomTest, SeedingDeterminism) {
  Random rnd1(12345);
  Random rnd2(12345);

  EXPECT_EQ(rnd1.next<int>(0, 1000), rnd2.next<int>(0, 1000));
  EXPECT_EQ(rnd1.next<double>(0, 1), rnd2.next<double>(0, 1));
}

TEST(RandomTest, RangeBounds) {
  Random rnd(42);
  for (int i = 0; i < 100; ++i) {
    int val = rnd.next(-10, 10);
    EXPECT_GE(val, -10);
    EXPECT_LE(val, 10);
  }
}

TEST(RandomTest, WeightedNext) {
  Random rnd(42);
  // Type > 0 prefers higher numbers (max of k trials)
  // Type < 0 prefers lower numbers (min of k trials)

  std::int64_t sum_high = 0;
  std::int64_t sum_low = 0;
  int N = 100;

  for (int i = 0; i < N; ++i) sum_high += rnd.wnext(0, 100, 5);
  for (int i = 0; i < N; ++i) sum_low += rnd.wnext(0, 100, -5);

  EXPECT_GT(sum_high, sum_low);
}

TEST(RandomTest, ChoiceAndShuffle) {
  Random rnd(42);
  std::vector<int> v = {1, 2, 3, 4, 5};

  auto picked = rnd.choice(v);
  EXPECT_GE(*picked, 1);
  EXPECT_LE(*picked, 5);

  std::vector<int> original = v;
  rnd.shuffle(v);
  // Elements should be same, just order likely changed
  std::ranges::sort(v);
  EXPECT_EQ(v, original);
}

TEST(RandomTest, WeightedChoiceMap) {
  Random rnd(42);
  // Item -> Weight
  std::map<std::string, int> weights = {{"rare", 1}, {"common", 1000}};

  int common_cnt = 0;
  for (int i = 0; i < 100; ++i) {
    auto it = rnd.weighted_choice(weights);
    if (it->first == "common") common_cnt++;
  }

  EXPECT_GT(common_cnt, 90);
}
