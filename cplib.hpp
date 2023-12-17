/**
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_HPP_
#define CPLIB_HPP_

#include <regex.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#define CPLIB_VERSION "0.0.1-SNAPSHOT"

#if (_WIN32 || __WIN32__ || __WIN32 || _WIN64 || __WIN64__ || __WIN64 || WINNT || __WINNT || \
     __WINNT__ || __CYGWIN__)
#if !defined(_MSC_VER) || _MSC_VER > 1400
#define NOMINMAX 1
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <io.h>
#define ON_WINDOWS
#if defined(_MSC_VER) && _MSC_VER > 1400
#pragma warning(disable : 4127)
#pragma warning(disable : 4146)
#pragma warning(disable : 4458)
#endif
#else
#include <unistd.h>
#endif

#if __STDC_VERSION__ >= 201112L
#define CPLIB_NORETURN _Noreturn
#elif defined _MSC_VER
#define CPLIB_NORETURN __declspec(noreturn)
#elif defined __GNUC__
#define CPLIB_NORETURN __attribute__((noreturn))
#else
#define CPLIB_NORETURN
#endif

#if defined(__GNUC__)
#define CPLIB_PRINTF_LIKE(n, m) __attribute__((format(printf, n, m)))
#else
#define CPLIB_PRINTF_LIKE(n, m) /* If only */
#endif                          /* __GNUC__ */

namespace cplib {

CPLIB_NORETURN auto panic(std::string_view message) -> void;

// Format string using printf-like syntax
CPLIB_PRINTF_LIKE(1, 2) auto format(const char* fmt, ...) -> std::string;

// Determine whether the two floating-point values are equals within the accuracy range
template <class T>
auto float_equals(T expected, T result, T max_err) -> bool;

// Calculate the minimum relative and absolute error between two floating-point values
template <class T>
auto float_delta(T expected, T result) -> T;

// Compress string to at most 64 bytes
auto compress(std::string_view s) -> std::string;

// Trim spaces at beginning and end of string
auto trim(std::string_view s) -> std::string;

// Concatenate the values between [first,last) into a string through separator
template <class It>
auto join(It first, It last, char separator) -> std::string;

// Splits string s by character separators returning exactly k+1 items, where k is the number of
// separator occurences. Split the string into a list of strings using separator
auto split(std::string_view s, char separator) -> std::vector<std::string>;

// Similar to `split`, but ignores the empty string
auto tokenize(std::string_view s, char separator) -> std::vector<std::string>;

/**
 * Regex pattern in POSIX-Extended style. Used for matching strings.
 *
 * Format see
 * `https://en.wikibooks.org/wiki/Regular_Expressions/POSIX-Extended_Regular_Expressions`.
 */
class Pattern {
 public:
  // Create pattern instance by string
  explicit Pattern(std::string src);

  // Checks if given string match the pattern
  auto match(std::string_view s) const -> bool;

  // Returns source string of the pattern
  auto src() const -> std::string_view;

 private:
  std::string src_;

  // `re->second` represents whether regex is compiled successfully
  std::shared_ptr<std::pair<regex_t, bool>> re_;
};

namespace io {

class InStream {
 public:
  explicit InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict,
                    std::function<auto(std::string_view)->void> fail_func);

  // Get the name of stream
  auto name() const -> std::string_view;

  // Move stream pointer to the first non-black character or EOF
  auto skip_blanks() -> void;

  // Return the current character in the stream, or EOF if reached, without removing it from the
  // stream
  auto seek() -> int;

  // Return current character and moves pointer one character forward, or EOF if reached
  auto read() -> int;

  // Read at most n characters
  auto read_n(size_t n) -> std::string;

  // Check if stream is in strict mode
  auto is_strict() const -> bool;

  // Set strict mode of stream
  auto set_strict(bool b) -> void;

  // Get the current line number
  auto line_num() const -> size_t;

  // Get the current column number
  auto col_num() const -> size_t;

  // Check if current position is EOF. If not it doesn't move stream pointer
  auto eof() -> bool;

  // Move pointer to the first non-whitespace character and calls [`eof()`]
  auto seek_eof() -> bool;

  /**
   * Checks that current position contains '\n'. If not it doesn't move stream pointer
   *
   * The differences between OSs are not distinguished, on Windows "\r\n" is not treated as
   * end-of-line
   */
  auto eoln() -> bool;

  // Move pointer to the first non-space and non-tab character and calls [`eoln()`]
  auto seek_eoln() -> bool;

  // Move stream pointer to the first character of the next line (if exists)
  auto next_line() -> void;

  /**
   * Read a new token. Ignores whitespaces into the non-strict mode
   * (strict mode is used in validators usually).
   */
  auto read_token() -> std::string;

  /**
   * If current position contains EOF, do nothing and returns `std::nullopt`.
   *
   * Otherwise, reads line from the current position to EOLN or EOF. Moves stream pointer to
   * the first character of the new line (if possible).
   *
   * The return value does not include the trailing \n.
   */
  auto read_line() -> std::optional<std::string>;

  // Quit program with an error
  CPLIB_NORETURN auto fail(std::string_view message) -> void;

 private:
  std::unique_ptr<std::streambuf> buf_;
  std::string name_;
  bool strict_;  // In strict mode, whitespace characters are not ignored
  std::function<auto(std::string_view)->void> fail_func_;  // Calls when fail
  size_t line_num_;
  size_t col_num_;
};

}  // namespace io

namespace var {

template <class T, class D>
class Var;

// Represents a traced input stream with line and column information
class Reader {
 public:
  struct Trace {
    std::string var_name;
    size_t line_num;
    size_t col_num;
  };

  // Create a root reader of input stream
  explicit Reader(std::unique_ptr<io::InStream> inner);

  explicit Reader(Reader&&) = default;
  Reader& operator=(Reader&&) = default;

  // Get the inner wrapped input stream
  auto inner() -> io::InStream&;

  // Call [`Instream::fail`] of the wrapped input stream
  CPLIB_NORETURN auto fail(std::string_view message) -> void;

  // Read a variable
  template <class T, class D>
  auto read(const Var<T, D>& v) -> T;

  // Read multiple variables and put them into a tuple
  template <class... T>
  auto operator()(T... vars) -> std::tuple<typename T::Var::result_type...>;

 private:
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  std::unique_ptr<io::InStream> inner_;
  std::vector<Trace> traces_;
};

template <class T>
class Vec;

// Generic template class for variables
template <class T, class D>
class Var {
 public:
  using result_type = T;
  using derived_type = D;

  virtual ~Var() = 0;

  // Get the name of variable
  auto name() const -> std::string_view;

  // Create a variable based on self with the given name
  auto renamed(std::string_view name) const -> D;

  // Parse string into variable
  auto parse(std::string_view s) const -> T;

  // Creates a `var::Vec<D>` containing self of size `len`
  auto operator*(size_t len) const -> Vec<D>;

  friend auto Reader::read(const Var<T, D>& v) -> T;

 protected:
  explicit Var();
  explicit Var(std::string name);

  virtual auto read_from(Reader& in) const -> T = 0;

 private:
  std::string name_;
};

// Integer
template <class T>
class Int : public Var<T, Int<T>> {
 public:
  std::optional<T> min, max;

  explicit Int();
  explicit Int(std::string name);
  explicit Int(std::optional<T> min, std::optional<T> max);
  explicit Int(std::optional<T> min, std::optional<T> max, std::string name);

 protected:
  auto read_from(Reader& in) const -> T override;
};

// Floating-point number
template <class T>
class Float : public Var<T, Float<T>> {
 public:
  std::optional<T> min, max;

  explicit Float();
  explicit Float(std::string name);
  explicit Float(std::optional<T> min, std::optional<T> max);
  explicit Float(std::optional<T> min, std::optional<T> max, std::string name);

 protected:
  auto read_from(Reader& in) const -> T override;
};

// Floating-point number with digit count restrictions
template <class T>
class StrictFloat : public Var<T, StrictFloat<T>> {
 public:
  T min, max;
  size_t min_n_digit, max_n_digit;

  explicit StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit);
  explicit StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit, std::string name);

 protected:
  auto read_from(Reader& in) const -> T override;
};

// Whitespace separated string
class String : public Var<std::string, String> {
 public:
  std::optional<Pattern> pat;

  explicit String();
  explicit String(Pattern pat);
  explicit String(std::string name);
  explicit String(Pattern pat, std::string name);

 protected:
  auto read_from(Reader& in) const -> std::string override;
};

/**
 * Separator
 *
 * - If it is in strict mode, read exact one character determine whether it is same as `sep`.
 * - Otherwise, if `std::isspace(sep)`, read the next consecutive whitespaces.
 * - Otherwise, try skipping blanks and read exact one character `sep`.
 */
class Separator : public Var<std::nullopt_t, Separator> {
 public:
  char sep;

  Separator(char sep);
  explicit Separator(char sep, std::string name);

 protected:
  auto read_from(Reader& in) const -> std::nullopt_t override;
};

// Eoln separated string
class Line : public Var<std::string, Line> {
 public:
  std::optional<Pattern> pat;

  explicit Line();
  explicit Line(Pattern pat);
  explicit Line(std::string name);
  explicit Line(Pattern pat, std::string name);

 protected:
  auto read_from(Reader& in) const -> std::string override;
};

// Vector of variables
template <class T>
class Vec : public Var<std::vector<typename T::Var::result_type>, Vec<T>> {
 public:
  T element;
  size_t len;
  Separator sep;

  explicit Vec(T element, size_t len);
  explicit Vec(T element, size_t len, Separator sep);

 protected:
  auto read_from(Reader& in) const -> std::vector<typename T::Var::result_type> override;
};

// Matrix (2D vector) of variables
template <class T>
class Mat : public Var<std::vector<std::vector<typename T::Var::result_type>>, Mat<T>> {
 public:
  T element;
  size_t len0, len1;
  Separator sep0, sep1;

  explicit Mat(T element, size_t len0, size_t len1);
  explicit Mat(T element, size_t len0, size_t len1, Separator sep0, Separator sep1);

 protected:
  auto read_from(Reader& in) const
      -> std::vector<std::vector<typename T::Var::result_type>> override;
};

// Pair of variables
template <class F, class S>
class Pair : public Var<std::pair<typename F::Var::result_type, typename S::Var::result_type>,
                        Pair<F, S>> {
 public:
  F first;
  S second;
  Separator sep;

  explicit Pair(F first, S second);
  explicit Pair(F first, S second, Separator sep);
  explicit Pair(F first, S second, Separator sep, std::string name);
  explicit Pair(std::pair<F, S> pr);
  explicit Pair(std::pair<F, S> pr, Separator sep);
  explicit Pair(std::pair<F, S> pr, Separator sep, std::string name);

 protected:
  auto read_from(Reader& in) const
      -> std::pair<typename F::Var::result_type, typename S::Var::result_type> override;
};

// Tuple of variables
template <class... T>
class Tuple : public Var<std::tuple<typename T::Var::result_type...>, Tuple<T...>> {
 public:
  std::tuple<T...> elements;
  Separator sep;

  explicit Tuple(std::tuple<T...> elements);
  explicit Tuple(std::tuple<T...> elements, Separator sep);
  explicit Tuple(std::tuple<T...> elements, Separator sep, std::string name);

 protected:
  auto read_from(Reader& in) const -> std::tuple<typename T::Var::result_type...> override;
};

// Wrapped function
template <class F>
class FnVar : public Var<typename std::function<F>::result_type, FnVar<F>> {
 public:
  std::function<auto(Reader& in)->typename std::function<F>::result_type> inner;

  template <class... Args>
  FnVar(std::string name, std::function<F> f, Args... args);

 protected:
  auto read_from(Reader& in) const -> typename std::function<F>::result_type override;
};

/**
 * For a type `T` which has implemented `static auto T::read(var::Reader&, ...) -> T`,
 * `ExtVar<T>` provides a variable template for creating `T` by calling `T::read`.
 */
template <class T>
class ExtVar : public Var<T, ExtVar<T>> {
 public:
  std::function<auto(Reader& in)->T> inner;

  template <class... Args>
  ExtVar(std::string name, Args... args);

 protected:
  auto read_from(Reader& in) const -> T override;
};

using i8 = Int<int8_t>;
using u8 = Int<uint8_t>;
using i16 = Int<int16_t>;
using u16 = Int<uint16_t>;
using i32 = Int<int32_t>;
using u32 = Int<uint32_t>;
using i64 = Int<int64_t>;
using u64 = Int<uint64_t>;
using f32 = Float<float>;
using f64 = Float<double>;
using fext = Float<long double>;
using f32s = StrictFloat<float>;
using f64s = StrictFloat<double>;
using fexts = StrictFloat<long double>;

const auto space = Separator(' ', "space");
const auto tab = Separator('\t', "tab");
const auto eoln = Separator('\n', "eoln");
};  // namespace var

enum class WorkMode {
  NONE,
  CHECKER,
  VALIDATOR,
};

// Get current work mode of cplib.
auto get_work_mode() -> WorkMode;

namespace checker {

struct Report {
  class Status {
   public:
    enum Value {
      INTERNAL_ERROR,
      ACCEPTED,
      WRONG_ANSWER,
      PARTIALLY_CORRECT,
    };

    Status() = default;
    constexpr Status(Value value);

    constexpr operator Value() const;

    explicit operator bool() const = delete;

    constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  Status status;
  double score;  // Full score is 1.0
  std::string message;

  Report(Status status, double score, std::string message);
};

class State {
 public:
  using initializer_type = std::function<auto(State& state, int argc, char** argv)->void>;
  using reporter_type = std::function<auto(const Report& report)->void>;

  var::Reader inf;
  var::Reader ouf;
  var::Reader ans;

  // Initializer is a function parsing command line arguments and initializing [`checker::State`]
  initializer_type initializer;

  // Reporter is a function that reports the given [`checker::Report`] and exits the program.
  reporter_type reporter;

  State(initializer_type initializer);

  ~State();

  // Disable the check for redundant content in the output file
  auto disable_check_dirt() -> void;

  // Quit checker with status
  CPLIB_NORETURN auto quit(Report report) -> void;

  // Quit checker with [`report::status::ACCEPTED`]
  CPLIB_NORETURN auto quit_ac() -> void;

  // Quit checker with [`report::status::WRONG_ANSWER`]
  CPLIB_NORETURN auto quit_wa(std::string_view message) -> void;

  // Quit checker with [`report::status::PARTIALLY_CORRECT`]
  CPLIB_NORETURN auto quit_pc(double points, std::string_view message) -> void;

 private:
  bool exited_;

  // Check if the output file has redundant content when reporting
  bool check_dirt_;
};

auto default_initializer(State& state, int argc, char** argv) -> void;

auto json_reporter(const Report& report) -> void;

auto plain_text_reporter(const Report& report) -> void;

auto colored_text_reporter(const Report& report) -> void;

// Macro to register checker
#define CPLIB_REGISTER_CHECKER_OPT(var_, initializer_) \
  ::cplib::checker::State var_(initializer_);          \
  signed main(signed argc, char** argv) {              \
    var_.initializer(var_, argc, argv);                \
    auto checker_main(void)->void;                     \
    checker_main();                                    \
    return 0;                                          \
  }

#define CPLIB_REGISTER_CHECKER(var) \
  CPLIB_REGISTER_CHECKER_OPT(var, ::cplib::checker::default_initializer)

}  // namespace checker

namespace validator {
struct Report {
  class Status {
   public:
    enum Value {
      INTERNAL_ERROR,
      VALID,
      INVALID,
    };

    Status() = default;
    constexpr Status(Value value);

    constexpr operator Value() const;

    explicit operator bool() const = delete;

    constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  Status status;
  std::string message;

  Report(Status status, std::string message);
};

struct Trait {
  using check_func_type = std::function<auto()->bool>;

  std::string name;
  check_func_type check_func;
  std::vector<std::string> dependencies;

  Trait(std::string name, check_func_type check_func);
  Trait(std::string name, check_func_type check_func, std::vector<std::string> dependencies);
};

class State {
 public:
  using initializer_type = std::function<auto(State& state, int argc, char** argv)->void>;
  using reporter_type = std::function<
      auto(const Report& report, const std::map<std::string, bool>& trait_status)->void>;

  var::Reader inf;

  // Initializer is a function parsing command line arguments and initializing [`validator::State`]
  initializer_type initializer;

  // Reporter is a function that reports the given [`validator::Report`] and exits the program.
  reporter_type reporter;

  State(initializer_type initializer);

  ~State();

  // Set traits
  auto traits(std::vector<Trait> traits) -> void;

  // Quit validator with status
  CPLIB_NORETURN auto quit(Report report) -> void;

  // Quit validator with [`report::status::VALID`]
  CPLIB_NORETURN auto quit_valid() -> void;

  // Quit validator with [`report::status::INVALID`]
  CPLIB_NORETURN auto quit_invalid(std::string_view message) -> void;

 private:
  bool exited_;
  std::vector<Trait> traits_;
  std::vector<std::vector<size_t>> trait_edges_;
};

auto default_initializer(State& state, int argc, char** argv) -> void;

auto json_reporter(const Report& report, const std::map<std::string, bool>& trait_status) -> void;

auto plain_text_reporter(const Report& report, const std::map<std::string, bool>& trait_status)
    -> void;

auto colored_text_reporter(const Report& report, const std::map<std::string, bool>& trait_status)
    -> void;

// Macro to register validator
#define CPLIB_REGISTER_VALIDATOR_OPT(var_, initializer_) \
  ::cplib::validator::State var_(initializer_);          \
  signed main(signed argc, char** argv) {                \
    var_.initializer(var_, argc, argv);                  \
    auto validator_main(void)->void;                     \
    validator_main();                                    \
    return 0;                                            \
  }

#define CPLIB_REGISTER_VALIDATOR(var) \
  CPLIB_REGISTER_VALIDATOR_OPT(var, ::cplib::validator::default_initializer)

}  // namespace validator

}  // namespace cplib

#endif  // CPLIB_HPP_

// === Implementations ===

namespace cplib {

namespace detail {
inline auto has_colors() -> bool {
  // https://bixense.com/clicolors/
  if (std::getenv("NO_COLOR") != nullptr) return false;
  if (std::getenv("CLICOLOR_FORCE") != nullptr) return true;
  return isatty(fileno(stderr));
}

inline auto json_string_encode(char c) -> std::string {
  if (c == '\\')
    return "\\\\";
  else if (c == '\b')
    return "\\b";
  else if (c == '\f')
    return "\\f";
  else if (c == '\n')
    return "\\n";
  else if (c == '\r')
    return "\\r";
  else if (c == '\t')
    return "\\t";
  else if (!isprint(c))
    return format("\\u%04x", static_cast<int>(c));
  else
    return std::string{c};
}

inline auto json_string_encode(std::string_view s) -> std::string {
  std::string result;
  for (auto c : s) result += json_string_encode(c);
  return result;
}

inline auto hex_encode(char c) -> std::string {
  if (c == '\\')
    return "\\\\";
  else if (c == '\b')
    return "\\b";
  else if (c == '\f')
    return "\\f";
  else if (c == '\n')
    return "\\n";
  else if (c == '\r')
    return "\\r";
  else if (c == '\t')
    return "\\t";
  else if (!isprint(c))
    return format("\\x%02x", static_cast<int>(c));
  else
    return std::string{c};
}

inline auto hex_encode(std::string_view s) -> std::string {
  std::string result;
  for (auto c : s) result += hex_encode(c);
  return result;
}
}  // namespace detail

// Impl panic {{{
namespace detail {
inline std::function<auto(std::string_view)->void> panic_impl = [](std::string_view s) {
  std::cerr << "Unrecoverable error: " << s << '\n';
  exit(EXIT_FAILURE);
};
}  // namespace detail

CPLIB_NORETURN inline auto panic(std::string_view message) -> void {
  detail::panic_impl(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in special cases to prevent problems.
}
// /Impl panic }}}

// Impl format {{{
namespace detail {
inline auto string_vsprintf(const char* format, std::va_list args) -> std::string {
  va_list tmp_args;         // unfortunately you cannot consume a va_list twice
  va_copy(tmp_args, args);  // so we have to copy it
  const int required_len = vsnprintf(nullptr, 0, format, tmp_args);
  va_end(tmp_args);

  std::string buf(required_len, '\0');
  if (std::vsnprintf(&buf[0], required_len + 1, format, args) < 0) {
    panic("string_vsprintf encoding error");
    return "";
  }
  return buf;
}
}  // namespace detail

CPLIB_PRINTF_LIKE(1, 2) inline auto format(const char* fmt, ...) -> std::string {
  std::va_list args;
  va_start(args, fmt);
  std::string str{detail::string_vsprintf(fmt, args)};
  va_end(args);
  return str;
}
// /Impl format }}}

template <class T>
inline auto float_equals(T expected, T result, T max_err) -> bool {
  if (bool x_nan = std::isnan(expected), y_nan = std::isnan(result); x_nan || y_nan)
    return x_nan && y_nan;
  if (bool x_inf = std::isinf(expected), y_inf = std::isinf(result); x_inf || y_inf)
    return x_inf && y_inf && (expected > 0) == (result > 0);

  max_err += 1e-15;

  if (std::abs(expected - result) <= max_err) return true;

  T min_v = std::min<T>(expected * (1.0 - max_err), expected * (1.0 + max_err));
  T max_v = std::max<T>(expected * (1.0 - max_err), expected * (1.0 + max_err));
  return result >= min_v && result <= max_v;
}

template <class T>
inline auto float_delta(T expected, T result) -> T {
  T absolute = std::abs(expected - result);

  if (std::abs(expected) > 1E-9) {
    double relative = std::abs(absolute / expected);
    return std::min(absolute, relative);
  }

  return absolute;
}

inline auto compress(std::string_view s) -> std::string {
  auto t = detail::hex_encode(s);
  if (t.size() <= 64)
    return std::string(t);
  else
    return std::string(t.substr(0, 30)) + "..." + std::string(t.substr(t.size() - 31, 31));
}

inline auto trim(std::string_view s) -> std::string {
  if (s.empty()) return std::string(s);

  ssize_t left = 0;
  while (left < ssize_t(s.size()) && std::isspace(s[left])) left++;
  if (left >= ssize_t(s.size())) return "";

  ssize_t right = ssize_t(s.size()) - 1;
  while (right >= 0 && std::isspace(s[right])) right--;
  if (right < 0) return "";

  return std::string(s.substr(left, right - left + 1));
}

template <typename It>
inline auto join(It first, It last, char separator) -> std::string {
  std::string result;
  bool repeated = false;
  for (It i = first; i != last; ++i) {
    if (repeated)
      result.push_back(separator);
    else
      repeated = true;
    result += *i;
  }
  return result;
}

inline auto split(std::string_view s, char separator) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string item;
  for (size_t i = 0; i < s.size(); i++)
    if (s[i] == separator) {
      result.push_back(item);
      item.clear();
    } else
      item.push_back(s[i]);
  result.push_back(item);
  return result;
}

inline auto tokenize(std::string_view s, char separator) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string item;
  for (size_t i = 0; i < s.size(); i++)
    if (s[i] == separator) {
      if (!item.empty()) result.push_back(item);
      item.clear();
    } else
      item.push_back(s[i]);
  if (!item.empty()) result.push_back(item);
  return result;
}

// Impl pattern {{{
namespace detail {
inline auto get_regex_err_msg(int err_code, regex_t* re) -> std::string {
  size_t len = regerror(err_code, re, NULL, 0);
  std::string buf(len, 0);
  regerror(err_code, re, buf.data(), len);
  return buf;
}
}  // namespace detail

inline Pattern::Pattern(std::string src)
    : src_(std::move(src)), re_(new std::pair<regex_t, bool>, [](std::pair<regex_t, bool>* p) {
        if (p->second) regfree(&p->first);
        delete p;
      }) {
  using namespace std::string_literals;
  // In function `regexec`, a match anywhere within the string is considered successful unless the
  // regular expression contains `^` or `$`.
  if (int err = regcomp(&re_->first, ("^"s + src_ + "$"s).c_str(), REG_EXTENDED | REG_NOSUB); err) {
    auto err_msg = detail::get_regex_err_msg(err, &re_->first);
    panic("pattern constructor failed: "s + err_msg);
  }
  re_->second = true;
}

inline auto Pattern::match(std::string_view s) const -> bool {
  using namespace std::string_literals;

  int result = regexec(&re_->first, s.data(), 0, nullptr, 0);

  if (!result) return true;

  if (result == REG_NOMATCH) return false;

  auto err_msg = detail::get_regex_err_msg(result, &re_->first);
  panic("pattern match failed: "s + err_msg);
  return false;
}

inline auto Pattern::src() const -> std::string_view { return src_; }
// Impl pattern }}}

namespace io {

inline InStream::InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict,
                          std::function<auto(std::string_view)->void> fail_func)
    : buf_(std::move(buf)),
      name_(std::move(name)),
      strict_(std::move(strict)),
      fail_func_(std::move(fail_func)),
      line_num_(1),
      col_num_(1) {}

inline auto InStream::name() const -> std::string_view { return name_; }

inline auto InStream::skip_blanks() -> void {
  while (true) {
    if (int c = seek(); c == EOF || !std::isspace(c)) break;
    read();
  }
}

inline auto InStream::seek() -> int { return buf_->sgetc(); }

inline auto InStream::read() -> int {
  int c = buf_->sbumpc();
  if (c == EOF) return EOF;
  if (c == '\n')
    ++line_num_, col_num_ = 1;
  else
    ++col_num_;
  return c;
}

inline auto InStream::read_n(size_t n) -> std::string {
  std::string buf;
  buf.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    if (eof()) break;
    buf.push_back(static_cast<char>(read()));
  }
  return buf;
}

inline auto InStream::is_strict() const -> bool { return strict_; }

inline auto InStream::set_strict(bool b) -> void {
  if (line_num() != 1 || col_num() != 1)
    panic(format("Can't set strict mode of `%s` when not at the beginning of the file",
                 name().data()));
  strict_ = b;
}

inline auto InStream::line_num() const -> size_t { return line_num_; }

inline auto InStream::col_num() const -> size_t { return col_num_; }

inline auto InStream::eof() -> bool { return seek() == EOF; }

inline auto InStream::seek_eof() -> bool {
  if (!strict_) skip_blanks();
  return eof();
}

inline auto InStream::eoln() -> bool { return seek() == '\n'; }

inline auto InStream::seek_eoln() -> bool {
  if (!strict_) skip_blanks();
  return eoln();
}

inline auto InStream::next_line() -> void {
  int c;
  do c = read();
  while (c != EOF && c != '\n');
}

inline auto InStream::read_token() -> std::string {
  if (!strict_) skip_blanks();

  std::string token;
  while (true) {
    if (int c = seek(); c == EOF || std::isspace(c)) break;
    token.push_back(read());
  }
  return token;
}

inline auto InStream::read_line() -> std::optional<std::string> {
  std::string line;
  if (eof()) return std::nullopt;
  while (true) {
    int c = read();
    if (c == EOF || c == '\n') break;
    line.push_back(static_cast<char>(c));
  }
  return line;
}

CPLIB_NORETURN inline auto InStream::fail(std::string_view message) -> void {
  fail_func_(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in sepial cases to prevent problems.
}

}  // namespace io

namespace var {

inline Reader::Reader(std::unique_ptr<io::InStream> inner)
    : inner_(std::move(inner)), traces_({}) {}

inline auto Reader::inner() -> io::InStream& { return *inner_; }

CPLIB_NORETURN inline auto Reader::fail(std::string_view message) -> void {
  using namespace std::string_literals;
  std::string result = "read error: "s + std::string(message);
  size_t depth = 0;
  for (auto it = traces_.rbegin(); it != traces_.rend(); ++it) {
    result += format("\n  #%zu: %s @ %s:%zu:%zu", depth, it->var_name.c_str(),
                     inner().name().data(), it->line_num, it->col_num);
    ++depth;
  }
  inner_->fail(result);
}

template <class T, class D>
inline auto Reader::read(const Var<T, D>& v) -> T {
  traces_.emplace_back(Trace{std::string(v.name()), inner_->line_num(), inner_->col_num()});
  auto result = v.read_from(*this);
  traces_.pop_back();
  return result;
}

template <class... T>
inline auto Reader::operator()(T... vars) -> std::tuple<typename T::Var::result_type...> {
  return {read(vars)...};
}

namespace detail {
inline constexpr std::string_view VAR_DEFAULT_NAME("<unnamed>");
}

template <class T, class D>
inline Var<T, D>::~Var() {}

template <class T, class D>
inline auto Var<T, D>::name() const -> std::string_view {
  return name_;
}

template <class T, class D>
inline auto Var<T, D>::renamed(std::string_view name) const -> D {
  D clone = *static_cast<const D*>(this);
  clone.name_ = name;
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::parse(std::string_view s) const -> T {
  auto buf = std::make_unique<std::stringbuf>(std::string(s), std::ios_base::in);
  auto reader = Reader(std::make_unique<io::InStream>(std::move(buf), "str", true,
                                                      [&](std::string_view s) { panic(s); }));
  T result = reader.read(*this);
  if (!reader.inner().eof()) {
    panic("Var::parse failed, extra characters in string");
  }
  return result;
}

template <class T, class D>
inline auto Var<T, D>::operator*(size_t len) const -> Vec<D> {
  return Vec<D>(*static_cast<const D*>(this), len);
}

template <class T, class D>
inline Var<T, D>::Var() : name_(std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T, class D>
inline Var<T, D>::Var(std::string name) : name_(std::move(name)) {}

template <class T>
inline Int<T>::Int() : Int<T>(std::nullopt, std::nullopt, std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline Int<T>::Int(std::string name) : Int<T>(std::nullopt, std::nullopt, std::move(name)) {}

template <class T>
inline Int<T>::Int(std::optional<T> min, std::optional<T> max)
    : Int<T>(std::move(min), std::move(max), std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline Int<T>::Int(std::optional<T> min, std::optional<T> max, std::string name)
    : Var<T, Int<T>>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
inline auto Int<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof())
      in.fail("Expected an integer, got EOF");
    else
      in.fail(format("Expected an integer, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
  }

  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result);

  if (ec != std::errc() || ptr != token.c_str() + token.size()) {
    in.fail(format("Expected an integer, got `%s`", compress(token).c_str()));
  }

  if (min.has_value() && result < *min) {
    in.fail(format("Expected an integer >= %s, got `%s`", std::to_string(*min).c_str(),
                   compress(token).c_str()));
  }

  if (max.has_value() && result > *max) {
    in.fail(format("Expected an integer <= %s, got `%s`", std::to_string(*max).c_str(),
                   compress(token).c_str()));
  }

  return result;
}

template <class T>
inline Float<T>::Float()
    : Float<T>(std::nullopt, std::nullopt, std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline Float<T>::Float(std::string name) : Float<T>(std::nullopt, std::nullopt, std::move(name)) {}

template <class T>
inline Float<T>::Float(std::optional<T> min, std::optional<T> max)
    : Float<T>(std::move(min), std::move(max), std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline Float<T>::Float(std::optional<T> min, std::optional<T> max, std::string name)
    : Var<T, Float<T>>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
inline auto Float<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof())
      in.fail("Expected a float, got EOF");
    else
      in.fail(format("Expected a float, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
  }

  // `Float<T>` usually uses with non-strict streams, so it should support both fixed format and
  // scientific.
  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result,
                                   std::chars_format::general);

  if (ec != std::errc() || ptr != token.c_str() + token.size() || !std::isfinite(result)) {
    in.fail(format("Expected a float, got `%s`", compress(token).c_str()));
  }

  if (min.has_value() && result < *min) {
    in.fail(format("Expected a float >= %s, got `%s`", std::to_string(*min).c_str(),
                   compress(token).c_str()));
  }

  if (max.has_value() && result > *max) {
    in.fail(format("Expected a float <= %s, got `%s`", std::to_string(*max).c_str(),
                   compress(token).c_str()));
  }

  return result;
}

template <class T>
inline StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit)
    : StrictFloat<T>(min, max, min_n_digit, max_n_digit, std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit,
                                   std::string name)
    : Var<T, StrictFloat<T>>(std::move(name)),
      min(std::move(min)),
      max(std::move(max)),
      min_n_digit(std::move(min_n_digit)),
      max_n_digit(std::move(max_n_digit)) {
  if (min > max) panic("StrictFloat constructor failed: min must be <= max");
  if (min_n_digit > max_n_digit)
    panic("StrictFloat constructor failed: min_n_digit must be <= max_n_digit");
}

template <class T>
inline auto StrictFloat<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof())
      in.fail("Expected a strict float, got EOF");
    else
      in.fail(format("Expected a strict float, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
  }

  auto pat = Pattern(min_n_digit == 0
                         ? format("-?([1-9][0-9]*|0)(\\.[0-9]{,%zu})?", max_n_digit)
                         : format("-?([1-9][0-9]*|0)\\.[0-9]{%zu,%zu}", min_n_digit, max_n_digit));

  if (!pat.match(token)) {
    in.fail(format("Expected a strict float, got `%s`", compress(token).c_str()));
  }

  // Different from `Float<T>`, only fixed format should be allowed.
  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result,
                                   std::chars_format::fixed);

  if (ec != std::errc() || ptr != token.c_str() + token.size()) {
    in.fail(format("Expected a strict float, got `%s`", compress(token).c_str()));
  }

  if (result < min) {
    in.fail(format("Expected a strict float >= %s, got `%s`", std::to_string(min).c_str(),
                   compress(token).c_str()));
  }

  if (result > max) {
    in.fail(format("Expected a strict float <= %s, got `%s`", std::to_string(max).c_str(),
                   compress(token).c_str()));
  }

  return result;
}

inline String::String() : String(std::string(detail::VAR_DEFAULT_NAME)) {}

inline String::String(Pattern pat)
    : String(std::move(pat), std::string(detail::VAR_DEFAULT_NAME)) {}

inline String::String(std::string name)
    : Var<std::string, String>(std::move(name)), pat(std::nullopt) {}

inline String::String(Pattern pat, std::string name)
    : Var<std::string, String>(std::move(name)), pat(std::move(pat)) {}

inline auto String::read_from(Reader& in) const -> std::string {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof())
      in.fail("Expected a string, got EOF");
    else
      in.fail(format("Expected a string, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
  }

  if (pat.has_value() && !pat->match(token)) {
    in.fail(format("Expected a string matching `%s`, got `%s`", compress(pat->src()).data(),
                   compress(token).c_str()));
  }

  return token;
}

// Impl Separator {{{
inline Separator::Separator(char sep)
    : Separator(std::move(sep), std::string(detail::VAR_DEFAULT_NAME)) {}

inline Separator::Separator(char sep, std::string name)
    : Var<std::nullopt_t, Separator>(std::move(name)), sep(std::move(sep)) {}

inline auto Separator::read_from(Reader& in) const -> std::nullopt_t {
  if (in.inner().eof())
    in.fail(format("Expected a separator `%s`, got EOF", cplib::detail::hex_encode(sep).c_str()));

  if (in.inner().is_strict()) {
    auto got = in.inner().read();
    if (got != sep)
      in.fail(format("Expected a separator `%s`, got `%s`", cplib::detail::hex_encode(sep).c_str(),
                     cplib::detail::hex_encode(got).c_str()));
  } else if (std::isspace(sep)) {
    auto got = in.inner().read();
    if (!std::isspace(got))
      in.fail(format("Expected a separator `%s`, got `%s`", cplib::detail::hex_encode(sep).c_str(),
                     cplib::detail::hex_encode(got).c_str()));
  } else {
    in.inner().skip_blanks();
    auto got = in.inner().read();
    if (got != sep)
      in.fail(format("Expected a separator `%s`, got `%s`", cplib::detail::hex_encode(sep).c_str(),
                     cplib::detail::hex_encode(got).c_str()));
  }

  return std::nullopt;
}
// /Impl Separator }}}

inline Line::Line() : Line(std::string(detail::VAR_DEFAULT_NAME)) {}

inline Line::Line(Pattern pat) : Line(std::move(pat), std::string(detail::VAR_DEFAULT_NAME)) {}

inline Line::Line(std::string name) : Var<std::string, Line>(std::move(name)), pat(std::nullopt) {}

inline Line::Line(Pattern pat, std::string name)
    : Var<std::string, Line>(std::move(name)), pat(std::move(pat)) {}

inline auto Line::read_from(Reader& in) const -> std::string {
  auto line = in.inner().read_line();

  if (!line.has_value()) {
    in.fail("Expected a line, got EOF");
  }

  if (pat.has_value() && pat->match(*line)) {
    in.fail(format("Expected a line matching `%s`, got `%s`", compress(pat->src()).data(),
                   compress(*line).c_str()));
  }

  return *line;
}

template <class T>
inline Vec<T>::Vec(T element, size_t len) : Vec<T>(element, len, var::space) {}

template <class T>
inline Vec<T>::Vec(T element, size_t len, Separator sep)
    : Var<std::vector<typename T::Var::result_type>, Vec<T>>(std::string(element.name())),
      element(std::move(element)),
      len(std::move(len)),
      sep(std::move(sep)) {}

template <class T>
inline auto Vec<T>::read_from(Reader& in) const -> std::vector<typename T::Var::result_type> {
  std::vector<typename T::Var::result_type> result(len);
  for (size_t i = 0; i < len; ++i) {
    if (i > 0) in.read(sep);
    result[i] = in.read(element.renamed(std::to_string(i)));
  }
  return result;
}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1) : Mat<T>(element, len0, len1, ' ', '\n') {}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1, Separator sep0, Separator sep1)
    : Var<std::vector<std::vector<typename T::Var::result_type>>, Mat<T>>(
          std::string(element.name())),
      element(std::move(element)),
      len0(std::move(len0)),
      len1(std::move(len1)),
      sep0(std::move(sep0)),
      sep1(std::move(sep1)) {}

template <class T>
inline auto Mat<T>::read_from(Reader& in) const
    -> std::vector<std::vector<typename T::Var::result_type>> {
  std::vector<std::vector<typename T::Var::result_type>> result(
      len0, std::vector<typename T::Var::result_type>(len1));
  for (size_t i = 0; i < len0; ++i) {
    if (i > 0) in.read(sep0);
    auto name_prefix = std::to_string(i) + "_";
    for (size_t j = 0; j < len1; ++i) {
      if (j > 0) in.read(sep1);
      result[i][j] = in.read(element.renamed(name_prefix + std::to_string(j)));
    }
  }
  return result;
}

template <class F, class S>
inline Pair<F, S>::Pair(F first, S second)
    : Pair<F, S>(std::move(first), std::move(second), " ", std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(F first, S second, Separator sep)
    : Pair<F, S>(std::move(first), std::move(second), std::move(sep),
                 std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(F first, S second, Separator sep, std::string name)
    : Var<std::pair<typename F::Var::result_type, typename S::Var::result_type>, Pair<F, S>>(
          std::move(name)),
      first(std::move(first)),
      second(std::move(second)),
      sep(std::move(sep)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), " ",
                 std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr, Separator sep)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), std::move(sep),
                 std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr, Separator sep, std::string name)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), std::move(sep), std::move(name)) {}

template <class F, class S>
inline auto Pair<F, S>::read_from(Reader& in) const
    -> std::pair<typename F::Var::result_type, typename S::Var::result_type> {
  auto result_first = in.read(first.renamed("first"));
  in.read(sep);
  auto result_second = in.read(second.renamed("second"));
  return {result_first, result_second};
}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements)
    : Tuple<T...>(std::move(elements), " ", std::string(detail::VAR_DEFAULT_NAME)) {}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements, Separator sep)
    : Tuple<T...>(std::move(elements), std::move(sep), std::string(detail::VAR_DEFAULT_NAME)) {}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements, Separator sep, std::string name)
    : Var<std::tuple<typename T::Var::result_type...>, Tuple<T...>>(std::move(name)),
      elements(std::move(elements)),
      sep(std::move(sep)) {}

template <class... T>
inline auto Tuple<T...>::read_from(Reader& in) const
    -> std::tuple<typename T::Var::result_type...> {
  return std::apply(
      [&in](const auto&... args) {
        size_t cnt = 0;
        auto renamed_inc = [&cnt](auto var) { return var.renamed(std::to_string(cnt++)); };
        return std::tuple{in.read(renamed_inc(args))...};
      },
      elements);
}

template <class F>
template <class... Args>
inline FnVar<F>::FnVar(std::string name, std::function<F> f, Args... args)
    : Var<typename std::function<F>::result_type, FnVar<F>>(std::move(name)),
      inner([f, args...](Reader& in) { return f(in, args...); }) {}

template <class F>
inline auto FnVar<F>::read_from(Reader& in) const -> typename std::function<F>::result_type {
  return inner(in);
}

template <class T>
template <class... Args>
inline ExtVar<T>::ExtVar(std::string name, Args... args)
    : Var<T, ExtVar<T>>(std::move(name)),
      inner([args...](Reader& in) { return T::read(in, args...); }) {}

template <class T>
inline auto ExtVar<T>::read_from(Reader& in) const -> T {
  return inner(in);
}
}  // namespace var

// Impl get_work_mode {{{
namespace detail {
inline WorkMode work_mode = WorkMode::NONE;
}

inline auto get_work_mode() -> WorkMode { return detail::work_mode; }
// /Impl get_work_mode }}}

namespace checker {

inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case ACCEPTED:
      return "accepted";
    case WRONG_ANSWER:
      return "wrong_answer";
    case PARTIALLY_CORRECT:
      return "partially_correct";
    default:
      panic(format("Unknown checker report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, double score, std::string message)
    : status(std::move(status)), score(std::move(score)), message(std::move(message)) {}

// Impl State {{{
inline State::State(initializer_type initializer)
    : inf(var::Reader(nullptr)),
      ouf(var::Reader(nullptr)),
      ans(var::Reader(nullptr)),
      initializer(std::move(initializer)),
      reporter(json_reporter),
      exited_(false),
      check_dirt_(true) {
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::CHECKER;
}

inline State::~State() {
  if (!exited_) panic("Checker must exit by calling method `State::quit*`");
}

inline auto State::disable_check_dirt() -> void { check_dirt_ = true; }

CPLIB_NORETURN inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (check_dirt_ &&
      (report.status == Report::Status::ACCEPTED ||
       report.status == Report::Status::PARTIALLY_CORRECT) &&
      !ouf.inner().seek_eof()) {
    report = Report(Report::Status::WRONG_ANSWER, 0.0, "Extra content in the output file");
  }

  reporter(report);

  std::cerr << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

CPLIB_NORETURN inline auto State::quit_ac() -> void {
  quit(Report(Report::Status::ACCEPTED, 1.0, ""));
}

CPLIB_NORETURN inline auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 0.0, std::string(message)));
}

CPLIB_NORETURN inline auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}
// /Impl State }}}

// Impl default_initializer {{{
inline auto default_initializer(State& state, int argc, char** argv) -> void {
  constexpr std::string_view ARGS_USAGE =
      "<input_file> <output_file> <answer_file> [--report-format={auto|json|text}]";

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    std::string msg =
        format("cplib (CPLib) " CPLIB_VERSION
               "\n"
               "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
               "\n"
               "Usage:\n"
               "  %s %s\n"
               "\n"
               "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
               "enable colors\n",
               argv[0], ARGS_USAGE.data());
    std::clog << msg << '\n';
    exit(0);
  }

  auto detect_reporter = [&]() {
    if (!isatty(fileno(stderr)))
      state.reporter = json_reporter;
    else if (cplib::detail::has_colors())
      state.reporter = colored_text_reporter;
    else
      state.reporter = plain_text_reporter;
  };

  detect_reporter();

  if (argc < 4)
    panic("Program must be run with the following arguments:\n  " + std::string(ARGS_USAGE));

  bool need_detect_reporter = true;

  for (int i = 4; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (arg.rfind("--report-format=", 0) == 0) {
      arg.remove_prefix(std::string_view("--report-format=").size());
      if (arg == "auto") {
        need_detect_reporter = true;
      } else if (arg == "json") {
        state.reporter = json_reporter;
        need_detect_reporter = false;
      } else if (arg == "text") {
        if (cplib::detail::has_colors())
          state.reporter = colored_text_reporter;
        else
          state.reporter = plain_text_reporter;
        need_detect_reporter = false;
      } else {
        panic(format("Unknown --report-format option: %s", arg.data()));
      }
    } else {
      panic(format("Unknown option: %s", arg.data()));
    }
  }

  if (need_detect_reporter) detect_reporter();

  auto inf_buf = std::make_unique<std::filebuf>();
  if (!inf_buf->open(argv[1], std::ios::binary | std::ios::in)) panic("Can't open input file");
  state.inf = var::Reader(std::make_unique<io::InStream>(std::move(inf_buf), "inf", false,
                                                         [](std::string_view msg) { panic(msg); }));

  auto ouf_buf = std::make_unique<std::filebuf>();
  if (!ouf_buf->open(argv[2], std::ios::binary | std::ios::in)) panic("Can't open output file");
  state.ouf = var::Reader(std::make_unique<io::InStream>(
      std::move(ouf_buf), "ouf", false, [&state](std::string_view msg) { state.quit_wa(msg); }));

  auto ans_buf = std::make_unique<std::filebuf>();
  if (!ans_buf->open(argv[3], std::ios::binary | std::ios::in)) panic("Can't open answer file");
  state.ans = var::Reader(std::make_unique<io::InStream>(std::move(ans_buf), "ans", false,
                                                         [](std::string_view msg) { panic(msg); }));
}
// /Impl default_initializer }}}

// Impl reporters {{{
namespace detail {
inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::ACCEPTED:
      return "Accepted";
    case Report::Status::WRONG_ANSWER:
      return "Wrong Answer";
    case Report::Status::PARTIALLY_CORRECT:
      return "Partially Correct";
    default:
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::ACCEPTED:
      return "\x1b[0;32mAccepted\x1b[0m";
    case Report::Status::WRONG_ANSWER:
      return "\x1b[0;31mWrong Answer\x1b[0m";
    case Report::Status::PARTIALLY_CORRECT:
      return "\x1b[0;36mPartially Correct\x1b[0m";
    default:
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report) -> void {
  auto msg = format("{\"status\": \"%s\", \"score\": %.3f, \"message\": \"%s\"}",
                    report.status.to_string().data(), report.score,
                    cplib::detail::json_string_encode(report.message).c_str());
  std::clog << msg << '\n';
  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}

inline auto plain_text_reporter(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_title_string(report.status).c_str() << ", scores "
            << report.score * 100.0 << " of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }
  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}

inline auto colored_text_reporter(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_colored_title_string(report.status).c_str()
            << ", scores \x1b[0;33m" << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }
  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}
// /Impl reporters }}}

}  // namespace checker

namespace validator {
inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case VALID:
      return "valid";
    case INVALID:
      return "invalid";
    default:
      panic(format("Unknown validator report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(std::move(status)), message(std::move(message)) {}

inline Trait::Trait(std::string name, check_func_type check_func)
    : Trait(std::move(name), std::move(check_func), {}) {}

inline Trait::Trait(std::string name, check_func_type check_func,
                    std::vector<std::string> dependencies)
    : name(std::move(name)),
      check_func(std::move(check_func)),
      dependencies(std::move(dependencies)) {}

// Impl State {{{
namespace detail {
/**
 * In topological sorting, `callback` is called every time a new node is reached.
 * If `fn` returns false, nodes reachable by the current node will no longer be visited.
 */
inline auto topo_sort(const std::vector<std::vector<size_t>>& edges,
                      std::function<auto(size_t)->bool> callback) -> void {
  std::vector<size_t> degree(edges.size(), 0);

  for (size_t i = 0; i < edges.size(); ++i) {
    for (auto to : edges[i]) ++degree[to];
  }

  std::queue<size_t> queue;

  for (size_t i = 0; i < edges.size(); ++i) {
    if (degree[i] == 0) queue.push(i);
  }

  while (!queue.empty()) {
    auto front = queue.front();
    queue.pop();
    if (!callback(front)) continue;
    for (auto to : edges[front]) {
      --degree[to];
      if (!degree[to]) queue.push(to);
    }
  }
}

inline auto build_edges(std::vector<Trait>& traits)
    -> std::optional<std::vector<std::vector<size_t>>> {
  // Check duplicate name
  std::sort(traits.begin(), traits.end(),
            [](const Trait& x, const Trait& y) { return x.name < y.name; });
  if (std::unique(traits.begin(), traits.end(), [](const Trait& x, const Trait& y) {
        return x.name == y.name;
      }) != traits.end()) {
    // Found duplicate name
    return std::nullopt;
  }

  std::vector<std::vector<size_t>> edges(traits.size());

  for (size_t i = 0; i < traits.size(); ++i) {
    auto& trait = traits[i];
    // Check duplicate dependencies
    std::sort(trait.dependencies.begin(), trait.dependencies.end());
    if (std::unique(trait.dependencies.begin(), trait.dependencies.end()) !=
        trait.dependencies.end()) {
      // Found duplicate dependencies
      return std::nullopt;
    }

    for (const auto& dep : trait.dependencies) {
      auto dep_id =
          std::lower_bound(traits.begin(), traits.end(), dep,
                           [](const Trait& x, const std::string& y) { return x.name < y; }) -
          traits.begin();
      edges[dep_id].push_back(i);
    }
  }

  return edges;
}

inline auto have_loop(const std::vector<std::vector<size_t>>& edges) -> bool {
  std::vector<uint8_t> visited(edges.size(), 0);  // Never use std::vector<bool>

  topo_sort(edges, [&](size_t node) {
    visited[node] = 1;
    return true;
  });

  for (auto v : visited) {
    if (!v) return true;
  }
  return false;
}

inline auto validate_traits(const std::vector<Trait>& traits,
                            const std::vector<std::vector<size_t>>& edges)
    -> std::map<std::string, bool> {
  std::map<std::string, bool> results;
  for (const auto& trait : traits) results[trait.name] = false;

  topo_sort(edges, [&](size_t id) {
    auto& node = traits[id];
    auto result = node.check_func();
    results.at(node.name) = result;
    return result;
  });

  return results;
}
}  // namespace detail

inline State::State(initializer_type initializer)
    : inf(var::Reader(nullptr)),
      initializer(std::move(initializer)),
      reporter(json_reporter),
      exited_(false),
      traits_(),
      trait_edges_() {
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::VALIDATOR;
}

inline State::~State() {
  if (!exited_) panic("Validator must exit by calling method `State::quit*`");
}

inline auto State::traits(std::vector<Trait> traits) -> void {
  traits_ = std::move(traits);

  auto edges = detail::build_edges(traits_);
  if (!edges.has_value()) panic("Traits do not form a simple graph");

  if (detail::have_loop(*edges)) panic("Traits do not form a DAG");

  trait_edges_ = *edges;
}

CPLIB_NORETURN inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (report.status == Report::Status::VALID && !inf.inner().eof()) {
    report = Report(Report::Status::INVALID, "Extra content in the input file");
  }

  auto trait_status = detail::validate_traits(traits_, trait_edges_);
  reporter(report, trait_status);

  std::cerr << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

CPLIB_NORETURN inline auto State::quit_valid() -> void { quit(Report(Report::Status::VALID, "")); }

CPLIB_NORETURN inline auto State::quit_invalid(std::string_view message) -> void {
  quit(Report(Report::Status::INVALID, std::string(message)));
}
// /Impl State }}}

// Impl default_initializer {{{
// https://www.josuttis.com/cppcode/fdstream.html
class FStdinBuf : public std::streambuf {
 public:
  /**
   * Constructor
   * - Initialize file descriptor
   * - Initialize empty data buffer
   * - No putback area
   * => Force underflow()
   */
  explicit FStdinBuf() : fd_(fileno(stdin)) {
#ifdef ON_WINDOWS
    _setmode(_fileno(stdin), O_BINARY);  // Sets stdin mode to binary
#endif
    setg(buf_ + PB_SIZE,   // Beginning of putback area
         buf_ + PB_SIZE,   // Read position
         buf_ + PB_SIZE);  // End position
  }

 protected:
  // Insert new characters into the buffer
  virtual int_type underflow() {
    // Is read position before end of buffer?
    if (gptr() < egptr()) {
      return traits_type::to_int_type(*gptr());
    }

    /*
     * Process size of putback area
     * - Use number of characters read
     * - But at most size of putback area
     */
    int num_putback = gptr() - eback();
    if (num_putback > PB_SIZE) {
      num_putback = PB_SIZE;
    }

    // Copy up to PB_SIZE characters previously read into the putback area
    std::memmove(buf_ + (PB_SIZE - num_putback), gptr() - num_putback, num_putback);

    // Read at most bufSize new characters
    int num = read(fd_, buf_ + PB_SIZE, BUF_SIZE);
    if (num <= 0) {
      // ERROR or EOF
      return EOF;
    }

    // Reset buffer pointers
    setg(buf_ + (PB_SIZE - num_putback),  // Beginning of putback area
         buf_ + PB_SIZE,                  // Read position
         buf_ + PB_SIZE + num);           // End of buffer

    // Return next character
    return traits_type::to_int_type(*gptr());
  }

  int fd_;
  /* Data buffer:
   * - At most, pbSize characters in putback area plus
   * - At most, bufSize characters in ordinary read buffer
   */
  static constexpr int PB_SIZE = 4;       // Size of putback area
  static constexpr int BUF_SIZE = 65536;  // Size of the data buffer
  char buf_[BUF_SIZE + PB_SIZE];          // Data buffer
};

inline auto default_initializer(State& state, int argc, char** argv) -> void {
  constexpr std::string_view ARGS_USAGE = "[<input_file>] [--report-format={auto|json|text}]";

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    std::string msg =
        format("cplib (CPLib) " CPLIB_VERSION
               "\n"
               "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
               "\n"
               "Usage:\n"
               "  %s %s\n"
               "\n"
               "If <input_file> does not exist, stdin will be used as input\n"
               "\n"
               "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
               "enable colors\n",
               argv[0], ARGS_USAGE.data());
    std::clog << msg << '\n';
    exit(0);
  }

  auto detect_reporter = [&]() {
    if (!isatty(fileno(stderr)))
      state.reporter = json_reporter;
    else if (cplib::detail::has_colors())
      state.reporter = colored_text_reporter;
    else
      state.reporter = plain_text_reporter;
  };

  detect_reporter();

  bool need_detect_reporter = true;

  bool use_stdin = false;
  int opts_args_start = 2;
  if (argc < 2 || argv[1][0] == '\0' || argv[1][0] == '-') {
    use_stdin = true;
    opts_args_start = 1;
  }

  for (int i = opts_args_start; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (arg.rfind("--report-format=", 0) == 0) {
      arg.remove_prefix(std::string_view("--report-format=").size());
      if (arg == "auto") {
        need_detect_reporter = true;
      } else if (arg == "json") {
        state.reporter = json_reporter;
        need_detect_reporter = false;
      } else if (arg == "text") {
        if (cplib::detail::has_colors())
          state.reporter = colored_text_reporter;
        else
          state.reporter = plain_text_reporter;
        need_detect_reporter = false;
      } else {
        panic(format("Unknown --report-format option: %s", arg.data()));
      }
    } else {
      panic(format("Unknown option: %s", arg.data()));
    }
  }

  if (need_detect_reporter) detect_reporter();

  std::unique_ptr<std::streambuf> inf_buf = nullptr;

  if (use_stdin) {
    inf_buf = std::make_unique<FStdinBuf>();
  } else {
    auto tmp_inf_buf = std::make_unique<std::filebuf>();
    if (!tmp_inf_buf->open(argv[1], std::ios::binary | std::ios::in))
      panic("Can't open input file");
    inf_buf = std::move(tmp_inf_buf);
  }

  state.inf = var::Reader(
      std::make_unique<io::InStream>(std::move(inf_buf), "inf", true,
                                     [&state](std::string_view msg) { state.quit_invalid(msg); }));
}
// /Impl default_initializer }}}

// Impl reporters {{{
namespace detail {

inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::VALID:
      return "valid";
    case Report::Status::INVALID:
      return "invalid";
    default:
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::VALID:
      return "\x1b[0;32mValid\x1b[0m";
    case Report::Status::INVALID:
      return "\x1b[0;31mInvalid\x1b[0m";
    default:
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report, const std::map<std::string, bool>& trait_status)
    -> void {
  std::clog << std::boolalpha;

  std::clog << "{\"status\": \"" << report.status.to_string() << "\", \"message\": \""
            << cplib::detail::json_string_encode(report.message) << "\"";

  if (report.status == Report::Status::VALID) {
    std::clog << ", \"traits\": {";
    bool is_first = true;
    for (auto [name, satisfaction] : trait_status) {
      if (is_first) {
        is_first = false;
      } else {
        std::clog << ", ";
      }
      std::clog << "\"" << cplib::detail::json_string_encode(name) << "\": " << satisfaction;
    }
    std::clog << '}';
  }

  std::clog << "}\n";

  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}

inline auto plain_text_reporter(const Report& report,
                                const std::map<std::string, bool>& trait_status) -> void {
  std::clog << detail::status_to_title_string(report.status).c_str() << ".\n";

  if (report.status != Report::Status::VALID || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (report.status == Report::Status::VALID && !trait_status.empty()) {
    std::clog << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (auto [name, satisfaction] : trait_status) {
      if (satisfaction)
        satisfied.push_back(name);
      else
        dissatisfied.push_back(name);
    }

    for (const auto& name : satisfied) {
      std::clog << "+ " << cplib::detail::hex_encode(name) << '\n';
    }
    for (const auto& name : dissatisfied) {
      std::clog << "- " << cplib::detail::hex_encode(name) << '\n';
    }
  }

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto colored_text_reporter(const Report& report,
                                  const std::map<std::string, bool>& trait_status) -> void {
  std::clog << detail::status_to_colored_title_string(report.status).c_str() << ".\n";

  if (report.status != Report::Status::VALID || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (report.status == Report::Status::VALID && !trait_status.empty()) {
    std::clog << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (auto [name, satisfaction] : trait_status) {
      if (satisfaction)
        satisfied.push_back(name);
      else
        dissatisfied.push_back(name);
    }

    for (const auto& name : satisfied) {
      std::clog << "\x1b[0;32m+\x1b[0m " << name << '\n';
    }
    for (const auto& name : dissatisfied) {
      std::clog << "\x1b[0;31m-\x1b[0m " << name << '\n';
    }
  }

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}

}  // namespace validator

}  // namespace cplib
