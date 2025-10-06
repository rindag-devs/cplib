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

#ifndef CPLIB_VAR_HPP_
#define CPLIB_VAR_HPP_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "io.hpp"
#include "json.hpp"
#include "pattern.hpp"
#include "trace.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

#ifndef CPLIB_READER_TRACE_LEVEL_MAX
#define CPLIB_READER_TRACE_LEVEL_MAX static_cast<int>(::cplib::trace::Level::FULL)
#endif

namespace cplib::var {
template <class T, class D>
struct Var;

/**
 * `ReaderTrace` represents trace information for a variable.
 */
struct ReaderTrace {
  std::string var_name;
  io::Position pos;

  /// The length of the variable in the raw stream, units of bytes.
  /// Incomplete variables will have zero length.
  std::size_t byte_length{0};

  explicit ReaderTrace(std::string var_name, io::Position pos);

  /// Get name of node;
  [[nodiscard]] auto node_name() const -> std::string;

  /// Convert trace to plain text.
  [[nodiscard]] auto to_plain_text() const -> std::string;

  /// Convert trace to colored text.
  [[nodiscard]] auto to_colored_text() const -> std::string;

  /// Convert incomplete trace to JSON map.
  [[nodiscard]] auto to_stack_json() const -> json::Value;

  /// Convert complete trace to JSON map.
  [[nodiscard]] auto to_tree_json() const -> json::Value;
};

/**
 * `Reader` represents a traced input stream with line and column information.
 */
struct Reader : trace::Traced<ReaderTrace> {
 public:
  using FailFunc = UniqueFunction<auto(const Reader&, std::string_view)->void>;

  /// Create a reader of input stream.
  explicit Reader(std::unique_ptr<io::InStream> inner, trace::Level trace_level,
                  FailFunc fail_func);

  /// Copy constructor (deleted to prevent copying).
  Reader(const Reader&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Reader&) -> Reader& = delete;

  /// Move constructor.
  Reader(Reader&&) noexcept = default;

  /// Move assignment operator.
  auto operator=(Reader&&) noexcept -> Reader& = default;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  [[nodiscard]] auto inner() -> io::InStream&;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  [[nodiscard]] auto inner() const -> const io::InStream&;

  /**
   * Call fail func with a message.
   *
   * @param message The error message.
   */
  [[noreturn]] auto fail(std::string_view message) -> void;

  /**
   * Read a variable based on a variable reading template.
   *
   * @tparam T The target type of the variable reading template.
   * @tparam D The derived class of the variable reading template.
   * @param v The variable reading template.
   * @return The value read from the input stream.
   */
  template <class T, class D>
  auto read(const Var<T, D>& v) -> T;

  /**
   * Read multiple variables and put them into a tuple.
   *
   * @tparam T The types of the variable reading templates.
   * @param vars The variable reading templates.
   * @return A tuple containing the values read from the input stream.
   */
  template <class... T>
  auto operator()(T... vars) -> std::tuple<typename T::Var::Target...>;

 private:
  std::unique_ptr<io::InStream> inner_;
  FailFunc fail_func_;
};

template <class T>
struct Vec;

/**
 * `Var` describes a "variable reading template".
 *
 * @tparam T The target type of the variable reading template.
 * @tparam D The derived class of the variable reading template. When other classes inherit from
 * `Var`, this template parameter should be the class itself.
 */
template <class T, class D>
struct Var {
 public:
  using Target = T;
  using Derived = D;

  /**
   * Destructor.
   */
  virtual ~Var() = 0;

  /**
   * Get the name of the variable reading template.
   *
   * @return The name of the variable as a string_view.
   */
  [[nodiscard]] auto name() const -> std::string_view;

  /**
   * Clone itself, the derived class (template class D) needs to implement the copy constructor.
   *
   * @return A copy of the variable.
   */
  [[nodiscard]] auto clone() const -> D;

  /**
   * Creates a copy with a new name.
   *
   * @param name The new name for the variable.
   * @return A copy of the variable with the new name.
   */
  [[nodiscard]] auto renamed(std::string_view name) const -> D;

  /**
   * Parse a variable from a string.
   *
   * @param s The string representation of the variable.
   * @return The parsed value of the variable.
   */
  [[nodiscard]] auto parse(std::string_view s) const -> T;

  /**
   * Creates a `var::Vec<D>` containing self of size `len`.
   *
   * @param len The size of the `var::Vec`.
   * @return A `var::Vec<D>` containing copies of the variable.
   */
  auto operator*(std::size_t len) const -> Vec<D>;

  /**
   * Read the value of the variable from a `Reader` object.
   *
   * @param in The `Reader` object to read from.
   * @return The value of the variable.
   */
  virtual auto read_from(Reader& in) const -> T = 0;

 private:
  /**
   * Default constructor.
   */
  explicit Var();

  /**
   * Constructor with a specified name.
   *
   * @param name The name of the variable.
   */
  explicit Var(std::string name);

  friend D;

  std::string name_;
};

/**
 * `Int` is a variable reading template, indicating to read an integer in a given range in decimal
 * form.
 *
 * @tparam T The target type of the variable reading template.
 */
template <std::integral T>
struct Int : Var<T, Int<T>> {
 public:
  std::optional<T> min, max;

  /**
   * Default constructor.
   */
  explicit Int();

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the Int variable.
   */
  explicit Int(std::string name);

  /**
   * Constructor with min and max parameters.
   *
   * @param min The minimum value of the Int variable.
   * @param max The maximum value of the Int variable.
   */
  explicit Int(std::optional<T> min, std::optional<T> max);

  /**
   * Constructor with min, max, and name parameters.
   *
   * @param name The name of the Int variable.
   * @param min The minimum value of the Int variable.
   * @param max The maximum value of the Int variable.
   */
  explicit Int(std::string name, std::optional<T> min, std::optional<T> max);

  /**
   * Read the value of the Int variable from a reader.
   *
   * @param in The reader to read from.
   * @return The read value.
   */
  auto read_from(Reader& in) const -> T override;
};

/**
 * `Float` is a variable reading template, indicating to read a floating-point number in a given
 * range in fixed form or scientific form.
 */
template <std::floating_point T>
struct Float : Var<T, Float<T>> {
 public:
  std::optional<T> min, max;

  /**
   * Default constructor.
   */
  explicit Float();

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the Float variable.
   */
  explicit Float(std::string name);

  /**
   * Constructor with min and max range parameters.
   *
   * @param min The minimum value of the Float variable.
   * @param max The maximum value of the Float variable.
   */
  explicit Float(std::optional<T> min, std::optional<T> max);

  /**
   * Constructor with min and max range parameters and name parameter.
   *
   * @param name The name of the Float variable.
   * @param min The minimum value of the Float variable.
   * @param max The maximum value of the Float variable.
   */
  explicit Float(std::string name, std::optional<T> min, std::optional<T> max);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader& in) const -> T override;
};

/**
 * `StrictFloat` is a variable reading template, indicating to read a floating-point number in a
 * given range in fixed for with digit count restrictions.
 */
template <std::floating_point T>
struct StrictFloat : Var<T, StrictFloat<T>> {
 public:
  T min, max;
  std::size_t min_n_digit, max_n_digit;

  /**
   * Constructor with min, max range, and digit count restrictions parameters.
   *
   * @param min The minimum value of the StrictFloat variable.
   * @param max The maximum value of the StrictFloat variable.
   * @param min_n_digit The minimum number of digits of the StrictFloat variable.
   * @param max_n_digit The maximum number of digits of the StrictFloat variable.
   */
  explicit StrictFloat(T min, T max, std::size_t min_n_digit, std::size_t max_n_digit);

  /**
   * Constructor with min, max range, digit count restrictions parameters, and name parameter.
   *
   * @param name The name of the StrictFloat variable.
   * @param min The minimum value of the StrictFloat variable.
   * @param max The maximum value of the StrictFloat variable.
   * @param min_n_digit The minimum number of digits of the StrictFloat variable.
   * @param max_n_digit The maximum number of digits of the StrictFloat variable.
   */
  explicit StrictFloat(std::string name, T min, T max, std::size_t min_n_digit,
                       std::size_t max_n_digit);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader& in) const -> T override;
};

/**
 * `YesNo` is a variable reading template, indicating to read an boolean value.
 *
 * "Yes" mean true, "No" mean false. Case insensitive.
 *
 * @tparam T The target type of the variable reading template.
 */
struct YesNo : Var<bool, YesNo> {
 public:
  /**
   * Default constructor.
   */
  explicit YesNo();

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the YesNo variable.
   */
  explicit YesNo(std::string name);

  /**
   * Read the value of the YesNo variable from a reader.
   *
   * @param in The reader to read from.
   * @return The read value.
   */
  auto read_from(Reader& in) const -> bool override;
};

/**
 * `String` is a variable reading template, indicating to read a whitespace separated string.
 */
struct String : Var<std::string, String> {
 public:
  std::optional<Pattern> pat;

  /**
   * Default constructor.
   */
  explicit String();

  /**
   * Constructor with pattern parameter.
   *
   * @param pat The pattern of the String variable.
   */
  explicit String(Pattern pat);

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the String variable.
   */
  explicit String(std::string name);

  /**
   * Constructor with pattern and name parameters.
   *
   * @param name The name of the String variable.
   * @param pat The pattern of the String variable.
   * */
  explicit String(std::string name, Pattern pat);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader& in) const -> std::string override;
};

/**
 * `Separator` is a variable reading template, indicating to read a character as a separator.
 *
 * - If it is in strict mode, read exact one character determine whether it is same as `sep`.
 * - Otherwise, if `std::isspace(sep)`, read the next consecutive whitespaces.
 * - Otherwise, try skipping blanks and read exact one character `sep`.
 */
struct Separator : Var<std::nullopt_t, Separator> {
 public:
  char sep;

  /**
   * Constructs a `Separator` object with the specified separator character.
   *
   * @param sep The separator character.
   */
  explicit Separator(char sep);

  /**
   * Constructs a `Separator` object with the specified separator character and name.
   *
   * @param name The name of the `Separator`.
   * @param sep The separator character.
   */
  explicit Separator(std::string name, char sep);

  /**
   * Reads the separator character from the input reader.
   *
   * @param in The input reader.
   * @return `std::nullopt` to indicate that no value is read.
   */
  auto read_from(Reader& in) const -> std::nullopt_t override;
};

/**
 * `Line` is a variable reading template, indicating to read a end-of-line separated string.
 */
struct Line : Var<std::string, Line> {
 public:
  std::optional<Pattern> pat;

  /**
   * Constructs a `Line` object.
   */
  explicit Line();

  /**
   * Constructs a `Line` object with the specified pattern.
   *
   * @param pat The pattern to match for the line.
   */
  explicit Line(Pattern pat);

  /**
   * Constructs a `Line` object with the specified name.
   *
   * @param name The name of the `Line`.
   */
  explicit Line(std::string name);

  /**
   * Constructs a `Line` object with the specified pattern and name.
   *
   * @param name The name of the `Line`.
   * @param pat The pattern to match for the line.
   */
  explicit Line(std::string name, Pattern pat);

  /**
   * Reads the line from the input reader.
   *
   * @param in The input reader.
   * @return The read line as a string.
   */
  auto read_from(Reader& in) const -> std::string override;
};

/**
 * `Vec` is a variable reading template, used to read a fixed length of variables of the same type
 * and return them as `std::vector`.
 *
 * @tparam T The type of the inner variable reading template.
 */
template <class T>
struct Vec : Var<std::vector<typename T::Var::Target>, Vec<T>> {
 public:
  /// The type of the element in the vector.
  T element;
  /// The length of the vector.
  std::size_t len;
  /// The separator between elements.
  Separator sep;

  /**
   * Constructor.
   *
   * @param element The type of the element in the vector.
   * @param len The length of the vector.
   */
  explicit Vec(T element, std::size_t len);

  /**
   * Constructor with separator.
   *
   * @param element The type of the element in the vector.
   * @param len The length of the vector.
   * @param sep The separator between elements.
   */
  explicit Vec(T element, std::size_t len, Separator sep);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The vector of elements.
   */
  auto read_from(Reader& in) const -> std::vector<typename T::Var::Target> override;
};

/**
 * `Mat` is a variable reading template used to read a fixed-size variable matrix. The elements in
 * the matrix have the same type.
 *
 * @tparam T The type of the inner variable reading template.
 */
template <class T>
struct Mat : Var<std::vector<std::vector<typename T::Var::Target>>, Mat<T>> {
 public:
  /// The type of the element in the matrix.
  T element;
  /// The length of the first dimension of the matrix.
  std::size_t len0;
  /// The length of the second dimension of the matrix.
  std::size_t len1;
  /// The separator used for the first dimension.
  Separator sep0;
  /// The separator used for the second dimension.
  Separator sep1;

  /**
   * Constructor.
   *
   * @param element The type of the element in the matrix.
   * @param len0 The length of the first dimension of the matrix.
   * @param len1 The length of the second dimension of the matrix.
   */
  explicit Mat(T element, std::size_t len0, std::size_t len1);

  /**
   * Constructor with separators.
   *
   * @param element The type of the element in the matrix.
   * @param len0 The length of the first dimension of the matrix.
   * @param len1 The length of the second dimension of the matrix.
   * @param sep0 The separator used for the first dimension.
   * @param sep1 The separator used for the second dimension.
   */
  explicit Mat(T element, std::size_t len0, std::size_t len1, Separator sep0, Separator sep1);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The matrix of elements.
   */
  auto read_from(Reader& in) const -> std::vector<std::vector<typename T::Var::Target>> override;
};

/**
 * `Pair` is a variable reading template that reads two variables separated by a given separator
 * and returns them as `std::pair`.
 *
 * @tparam F The type of the first variable reading template.
 * @tparam S The type of the second variable reading template.
 */
template <class F, class S>
struct Pair : Var<std::pair<typename F::Var::Target, typename S::Var::Target>, Pair<F, S>> {
 public:
  /// The first element of the pair.
  F first;
  /// The second element of the pair.
  S second;
  /// The separator used when converting to string.
  Separator sep;

  /**
   * Constructor from std::pair.
   *
   * @param pr The std::pair to initialize the Pair with.
   */
  explicit Pair(std::pair<F, S> pr);

  /**
   * Constructor from std::pair with separator.
   *
   * @param pr The std::pair to initialize the Pair with.
   * @param sep The separator used when converting to string.
   */
  explicit Pair(std::pair<F, S> pr, Separator sep);

  /**
   * Constructor from std::pair with name.
   *
   * @param name The name of the pair.
   * @param pr The std::pair to initialize the Pair with.
   */
  explicit Pair(std::string name, std::pair<F, S> pr);

  /**
   * Constructor from std::pair with separator and name.
   *
   * @param name The name of the pair.
   * @param pr The std::pair to initialize the Pair with.
   * @param sep The separator used when converting to string.
   */
  explicit Pair(std::string name, std::pair<F, S> pr, Separator sep);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The pair of elements.
   */
  auto read_from(Reader& in) const
      -> std::pair<typename F::Var::Target, typename S::Var::Target> override;
};

/**
 * Tuple of variables.
 *
 * @tparam T The type of the variable reading templates.
 */
template <class... T>
struct Tuple : Var<std::tuple<typename T::Var::Target...>, Tuple<T...>> {
 public:
  /// The elements of the tuple.
  std::tuple<T...> elements;
  /// The separator used when converting to string.
  Separator sep;

  /**
   * Constructor.
   *
   * @param elements The elements of the tuple.
   */
  explicit Tuple(std::tuple<T...> elements);

  /**
   * Constructor with separator.
   *
   * @param elements The elements of the tuple.
   * @param sep The separator used when converting to string.
   */
  explicit Tuple(std::tuple<T...> elements, Separator sep);

  /**
   * Constructor with name.
   *
   * @param name The name of the tuple.
   * @param elements The elements of the tuple.
   */
  explicit Tuple(std::string name, std::tuple<T...> elements);

  /**
   * Constructor with separator and name.
   *
   * @param name The name of the tuple.
   * @param elements The elements of the tuple.
   * @param sep The separator used when converting to string.
   */
  explicit Tuple(std::string name, std::tuple<T...> elements, Separator sep);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The tuple of elements.
   */
  auto read_from(Reader& in) const -> std::tuple<typename T::Var::Target...> override;

 private:
  /**
   * @brief Implementation detail for reading a tuple with separators.
   *
   * This helper function uses a fold expression over an index sequence to read each
   * element of the tuple. It correctly inserts a separator read operation
   * before each element except the first one.
   *
   * @tparam Is The compile-time sequence of indices for the tuple elements.
   * @param in The reader object.
   * @param seq An empty `std::index_sequence` object to deduce the indices `Is`.
   * @return The tuple of elements read from the stream.
   */
  template <std::size_t... Is>
  auto read_from_impl(Reader& in, std::index_sequence<Is...>) const
      -> std::tuple<typename T::Var::Target...>;
};

/**
 * `FnVar` is used to wrap a function into a variable reading template.
 *
 * The type of the first parameter of the wrapped function must be `var::Reader`, and subsequent
 * parameters (which may not exist) are arbitrary.
 *
 * @tparam F The type of the function.
 */
template <class F>
struct FnVar : Var<typename std::function<F>::result_type, FnVar<F>> {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the variable to be read.
   * @param f The function object.
   * @param args The second to last arguments to the function, the count and type must match the
   * parameters of `f`.
   */
  template <class... Args>
  FnVar(std::string name, std::function<F> f, Args... args);

  /**
   * Read from reader.
   *
   * Use `in` as the first argument, together with subsequent arguments passed in the constructor,
   * to call the wrapped function.
   *
   * @param in The reader object.
   * @return The result of the function.
   */
  auto read_from(Reader& in) const -> typename std::function<F>::result_type override;

 private:
  /// The inner function.
  std::function<typename std::function<F>::result_type(Reader& in)> inner_function_;
};

// Defines the requirements for a type T to be "readable"
// with a static 'read' method that takes a Reader and additional arguments.
template <typename T, typename... Args>
concept Readable = requires(Reader& reader, Args&&... args) {
  // T must have a static member function named 'read'.
  // It must be callable with a Reader& and the given Args.
  // Its return type must be convertible to T (or exactly T).
  { T::read(reader, std::forward<Args>(args)...) } -> std::same_as<T>;
};

/**
 * For a type `T` which has implemented `static auto T::read(var::Reader&, ...) -> T`,
 * `ExtVar<T>` provides a variable template for creating `T` by calling `T::read`.
 *
 * @tparam T The type of the variable, which must have a static `auto T::read(var::Reader&, ...)
 * -> T`
 */
template <class T>
struct ExtVar : Var<T, ExtVar<T>> {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the variable to be read.
   * @param args The second to last arguments to the function `T::read`.
   */
  template <class... Args>
  explicit ExtVar(std::string name, Args... args)
    requires Readable<T, Args...>;

  /**
   * Read from reader.
   *
   * Use `in` as the first argument, together with subsequent arguments passed in the constructor,
   * to call `T::read`.
   *
   * @param in The reader object.
   * @return The result of `T::read`.
   */
  auto read_from(Reader& in) const -> T override;

 private:
  /// The inner function that encapsulates the call to T::read.
  std::function<T(Reader&)> inner_function_;
};

using i8 = Int<std::int8_t>;
using u8 = Int<std::uint8_t>;
using i16 = Int<std::int16_t>;
using u16 = Int<std::uint16_t>;
using i32 = Int<std::int32_t>;
using u32 = Int<std::uint32_t>;
using i64 = Int<std::int64_t>;
using u64 = Int<std::uint64_t>;
using f32 = Float<float>;
using f64 = Float<double>;
using fext = Float<long double>;
using f32s = StrictFloat<float>;
using f64s = StrictFloat<double>;
using fexts = StrictFloat<long double>;

const auto space = Separator("space", ' ');
const auto tab = Separator("tab", '\t');
const auto eoln = Separator("eoln", '\n');
};  // namespace cplib::var

#include "var.i.hpp"  // IWYU pragma: export

#endif
