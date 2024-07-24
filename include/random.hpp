/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_RANDOM_HPP_
#define CPLIB_RANDOM_HPP_

#include <cstdint>           // for uint32_t
#include <initializer_list>  // for initializer_list
#include <random>            // for mt19937_64
#include <type_traits>       // for enable_if

namespace cplib {
/**
 * Random number generator that provides various methods to generate random numbers and perform
 * random operations.
 */
class Random {
 public:
  using Engine = std::mt19937_64;

  /**
   * Construct a random generator with default seed.
   */
  explicit Random();

  /**
   * Construct a random generator with given seed.
   *
   * @param seed The seed of the random generator.
   */
  explicit Random(std::uint64_t seed);

  /**
   * Construct a random generator with seed which generated by `argc` and `argv`
   *
   * @param argc The number of command-line arguments.
   * @param argv The array of command-line arguments.
   */
  explicit Random(int argc, char** argv);

  /**
   * Reseed the generator with a new seed.
   *
   * @param seed The new seed value.
   */
  auto reseed(std::uint64_t seed) -> void;

  /**
   * Reseed the generator with a seed generated by `argc` and `argv`.
   *
   * @param argc The number of command-line arguments.
   * @param argv The array of command-line arguments.
   */
  auto reseed(int argc, char** argv) -> void;

  /**
   * Get the engine used by the random generator.
   *
   * @return A reference to the engine.
   */
  auto engine() -> Engine&;

  /**
   * Generate a random integer in the range [from, to].
   *
   * @tparam T The type of the integer.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated integer.
   */
  template <class T>
  auto next(T from, T to) -> std::enable_if_t<std::is_integral_v<T>, T>;

  /**
   * Generate a random floating-point number in the range [from, to].
   *
   * @tparam T The type of the floating-point number.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated floating-point number.
   */
  template <class T>
  auto next(T from, T to) -> std::enable_if_t<std::is_floating_point_v<T>, T>;

  /**
   * Generate a random boolean value.
   *
   * @tparam T The type of the boolean.
   * @return The randomly generated boolean value.
   */
  template <class T>
  auto next() -> std::enable_if_t<std::is_same_v<T, bool>, bool>;

  /**
   * Generate a random boolean value with a given probability of being true.
   *
   * @tparam T The type of the boolean.
   * @param true_prob The probability of the boolean being true.
   * @return The randomly generated boolean value.
   */
  template <class T>
  auto next(double true_prob) -> std::enable_if_t<std::is_same_v<T, bool>, bool>;

  /**
   * Return a random value from the given initializer_list.
   *
   * @tparam T The type of the value.
   * @param init_list The initializer_list to choose from.
   * @return A random value from the initializer_list.
   */
  template <class T>
  auto choice(std::initializer_list<T> init_list) -> T;

  /**
   * Return a random iterator from the given range.
   *
   * @tparam It The type of the iterator.
   * @param first The beginning of the range.
   * @param last The end of the range.
   * @return A random iterator from the range.
   */
  template <class It>
  auto choice(It first, It last) -> It;

  /**
   * Return a random iterator from the given container.
   *
   * @tparam Container The type of the container.
   * @param container The container to choose from.
   * @return A random iterator from the container.
   */
  template <class Container>
  auto choice(Container& container) -> decltype(std::begin(container));

  /**
   * Return a random iterator from the given map container by utilizing the values of the map
   * container as weights for weighted random number generation.
   *
   * @tparam Map The type of the map container.
   * @param map The map container to choose from.
   * @return A random iterator from the map container.
   */
  template <class Map>
  auto weighted_choice(const Map& map) -> decltype(std::begin(map));

  /**
   * Shuffle the elements in the given range.
   *
   * @tparam RandomIt The type of the random access iterator.
   * @param first The beginning of the range.
   * @param last The end of the range.
   */
  template <class RandomIt>
  auto shuffle(RandomIt first, RandomIt last) -> void;

  /**
   * Shuffle the elements in the given container.
   *
   * @tparam Container The type of the container.
   * @param container The container to shuffle.
   */
  template <class Container>
  auto shuffle(Container& container) -> void;

 private:
  Engine engine_;
};
}  // namespace cplib

#include "random.i.hpp"  // IWYU pragma: export

#endif
