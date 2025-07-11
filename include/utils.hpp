/*
 * This file is part of CPLib.
 *
 * CPLib is free software: you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CPLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with CPLib. If
 * not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CPLIB_UTILS_HPP_
#define CPLIB_UTILS_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifdef CPLIB_USE_FMT_LIB
#define FMT_HEADER_ONLY
#include "fmt/base.h"
#include "fmt/core.h"
#else
#include <format>
#endif

namespace cplib {
/**
 * Panic the program with a message and exit.
 *
 * If the program has been registered as a CPLib program, `panic` will call the `quit` method of
 * State with `INTERNAL_ERROR` status.
 *
 * @param message The message to print.
 */
[[noreturn]] auto panic(std::string_view message) -> void;

/**
 * Format string using std::format syntax.
template <class... Args>
 *
 * @param fmt The format string.
 * @param args The variadic arguments to be formatted.
 * @return The formatted string.
 */
#ifdef CPLIB_USE_FMT_LIB
template <class... Args>
[[nodiscard]] auto format(fmt::format_string<Args...> fmt, Args &&...args) -> std::string;

template <typename T>
concept formattable = requires(T &v, fmt::format_context ctx) {
  fmt::formatter<std::remove_cvref_t<T> >().format(v, ctx);
};
#else
template <class... Args>
[[nodiscard]] auto format(std::format_string<Args...> fmt, Args &&...args) -> std::string;

template <typename T>
concept formattable = requires(T &v, std::format_context ctx) {
  std::formatter<std::remove_cvref_t<T> >().format(v, ctx);
};
#endif

/**
 * Determine whether the two floating-point values are equals within the accuracy range.
 *
 * @tparam T The type of the values.
 * @param expected The expected floating-point value.
 * @param result The actual floating-point value.
 * @param max_err The maximum allowable error.
 * @return True if the values are equal within the accuracy range, false otherwise.
 */
template <class T>
auto float_equals(T expected, T result, T max_err) -> bool;

/**
 * Calculate the minimum relative and absolute error between two floating-point values.
 *
 * @tparam T The type of the values.
 * @param expected The expected floating-point value.
 * @param result The actual floating-point value.
 * @return The minimum error between the values.
 */
template <class T>
auto float_delta(T expected, T result) -> T;

/**
 * Compress string to at most 64 bytes.
 *
 * @param s The input string.
 * @return The compressed string.
 */
auto compress(std::string_view s) -> std::string;

/**
 * Trim spaces at beginning and end of string.
 *
 * @param s The input string.
 * @return The trimmed string.
 */
auto trim(std::string_view s) -> std::string;

/**
 * Concatenate the values between [first,last) into a string without separator.
 *
 * @tparam It The type of the iterator.
 * @param first Iterator to the first value.
 * @param last Iterator to the last value (exclusive).
 * @return The concatenated string.
 */
template <class It>
auto join(It first, It last) -> std::string;

/**
 * Concatenate the values between [first,last) into a string through separator.
 *
 * @tparam It The type of the iterator.
 * @param first Iterator to the first value.
 * @param last Iterator to the last value (exclusive).
 * @param separator The separator.
 * @return The concatenated string.
 */
template <class It, class Sep>
auto join(It first, It last, Sep separator) -> std::string;

/**
 * Splits string s by character separators returning exactly k+1 items, where k is the number of
 * separator occurrences. Split the string into a list of strings using separator.
 *
 * @param s The input string.
 * @param separator The separator character.
 * @return The vector of split strings.
 */
auto split(std::string_view s, char separator) -> std::vector<std::string>;

/**
 * Similar to `split`, but ignores the empty string.
 *
 * @param s The input string.
 * @param separator The separator character.
 * @return The vector of tokenized strings.
 */
auto tokenize(std::string_view s, char separator) -> std::vector<std::string>;

/**
 * `UniqueFunction` is similar to `std::function`, it is a wrapper for functions.
 *
 * The difference between it and `std::function` is that `std::function` requires the wrapped object
 * to be copy-assignable, but `UniqueFunction` only requires it to be move-assignable.
 *
 * In the same way, `UniqueFunction` itself cannot be copy-assignable, but can only be
 * move-assignable.
 *
 * @tparam T The type of the stored function.
 */
template <typename T>
struct UniqueFunction;

/**
 * Template specialization for Ret(Args...) type.
 *
 * @tparam Ret The return type of the stored function.
 * @tparam Args The argument types of the stored function.
 */
template <typename Ret, typename... Args>
struct UniqueFunction<Ret(Args...)> {
 public:
  /**
   * Creates an empty UniqueFunction.
   */
  UniqueFunction() = default;

  /**
   * Creates an empty UniqueFunction.
   */
  explicit UniqueFunction(std::nullptr_t);

  /**
   * Constructor.
   *
   * @param t The function to be stored.
   * @tparam T The type of the function to be stored.
   */
  template <class T>
  UniqueFunction(T t);  // NOLINT(google-explicit-constructor)

  /**
   * Function call operator.
   *
   * @param args The arguments to be passed to the stored function.
   * @return The return value of the stored function.
   */
  auto operator()(Args... args) const -> Ret;

 private:
  /**
   * Base class for polymorphism.
   */
  struct Base {
    virtual ~Base() = default;

    /**
     * Function call operator for derived classes.
     *
     * @param args The arguments to be passed to the derived function.
     * @return The return value of the derived function.
     */
    virtual auto operator()(Args &&...args) -> Ret = 0;
  };

  /**
   * Data class storing the actual function.
   *
   * @tparam T The type of the stored function.
   */
  template <typename T>
  struct Data : Base {
    T func;

    explicit Data(T &&t);

    /**
     * Function call operator implementation.
     *
     * @param args The arguments to be passed to the stored function.
     * @return The return value of the stored function.
     */
    auto operator()(Args &&...args) -> Ret override;
  };

  std::unique_ptr<Base> ptr{nullptr};
};

/**
 * `WorkMode` indicates the current mode of cplib.
 */
enum struct WorkMode : std::uint8_t {
  NONE,
  CHECKER,
  INTERACTOR,
  VALIDATOR,
  GENERATOR,
};

/**
 * Get current work mode of cplib.
 *
 * @return The current work mode.
 */
auto get_work_mode() -> WorkMode;
}  // namespace cplib

#include "utils.i.hpp"  // IWYU pragma: export

#endif
