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
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <random>
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

#if defined(ON_WINDOWS) && !defined(isatty)
#define CPLIB_ISATTY _isatty
#else
#define CPLIB_ISATTY isatty
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
};  // namespace var

enum class WorkMode {
  NONE,
  CHECKER,
};

// Get current work mode of cplib.
auto get_work_mode() -> WorkMode;

namespace checker {

struct Report {
  enum class Status {
    INTERNAL_ERROR,
    ACCEPTED,
    WRONG_ANSWER,
    PARTIALLY_CORRECT,
  };

  static auto status_to_string(Status status) -> std::string;

  Status status;
  double score;  // Full score is 1.0
  std::string message;

  Report(Status status, double score, std::string message);
};

class State {
 public:
  var::Reader inf;
  var::Reader ouf;
  var::Reader ans;

  // Initializer is a function parsing command line arguments and initializing [`checker::State`]
  std::function<auto(State& state, int argc, char** argv)->void> initializer;

  // Formatter is a function formating given [`checker::Report`] to string
  std::function<auto(const Report& report)->std::string> formatter;

  State(std::function<auto(State& state, int argc, char** argv)->void> initializer);

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

auto json_formatter(const Report& report) -> std::string;

auto plain_text_formatter(const Report& report) -> std::string;

auto colored_text_formatter(const Report& report) -> std::string;

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
};  // namespace cplib

#endif  // CPLIB_HPP_

// === Implementations ===

namespace cplib {

// Impl panic {{{
namespace detail {
inline std::function<auto(std::string_view)->void> panic_impl = [](std::string_view s) {
  std::cerr << "unrecoverable error: " << s << '\n';
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

// Impl compress {{{
namespace detail {
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

inline auto compress(std::string_view s) -> std::string {
  auto t = detail::hex_encode(s);
  if (t.size() <= 64)
    return std::string(t);
  else
    return std::string(t.substr(0, 30)) + "..." + std::string(t.substr(t.size() - 31, 31));
}
// /Impl compress }}}

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
inline Vec<T>::Vec(T element, size_t len) : Vec<T>(element, len, ' ') {}

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

inline auto Report::status_to_string(Report::Status status) -> std::string {
  switch (status) {
    case Status::INTERNAL_ERROR:
      return "internal_error";
    case Status::ACCEPTED:
      return "accepted";
    case Status::WRONG_ANSWER:
      return "wrong_answer";
    case Status::PARTIALLY_CORRECT:
      return "partially_correct";
    default:
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, double score, std::string message)
    : status(std::move(status)), score(std::move(score)), message(std::move(message)) {}

// Impl State {{{
inline State::State(std::function<auto(State& state, int argc, char** argv)->void> initializer)
    : inf(var::Reader(nullptr)),
      ouf(var::Reader(nullptr)),
      ans(var::Reader(nullptr)),
      initializer(std::move(initializer)),
      formatter(json_formatter),
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

  std::clog << formatter(report) << '\n';
  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
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
namespace detail {
inline auto has_colors() -> bool {
  // https://bixense.com/clicolors/
  if (std::getenv("NO_COLOR") != nullptr) return false;
  if (std::getenv("CLICOLOR_FORCE") != nullptr) return true;
  return CPLIB_ISATTY(2);
}
}  // namespace detail

inline auto default_initializer(State& state, int argc, char** argv) -> void {
  constexpr std::string_view ARGS_USAGE =
      "<input_file> <output_file> <answer_file> [--report-format={auto|json|text}]";

  if (argc > 1 && !std::strcmp("--help", argv[1])) {
    std::string msg =
        format("cplib (CPLib) " CPLIB_VERSION
               "\n"
               "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
               "\n"
               "Usage:\n"
               "  %s %s",
               argv[0], ARGS_USAGE.data());
    std::clog << msg << '\n';
    exit(0);
  }

  auto detect_formatter = [&]() {
    if (!CPLIB_ISATTY(2))
      state.formatter = json_formatter;
    else if (detail::has_colors())
      state.formatter = colored_text_formatter;
    else
      state.formatter = plain_text_formatter;
  };

  detect_formatter();

  if (argc < 4)
    panic("Program must be run with the following arguments:\n  " + std::string(ARGS_USAGE));

  bool need_detect_formatter = true;

  for (int i = 4; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (arg.rfind("--report-format=", 0) == 0) {
      arg.remove_prefix(std::string_view("--report-format=").size());
      if (arg == "auto") {
        need_detect_formatter = true;
      } else if (arg == "json") {
        state.formatter = json_formatter;
        need_detect_formatter = false;
      } else if (arg == "text") {
        if (detail::has_colors())
          state.formatter = colored_text_formatter;
        else
          state.formatter = plain_text_formatter;
        need_detect_formatter = false;
      } else {
        panic(format("Unknown --report-format option: %s", arg.data()));
      }
    } else {
      panic(format("Unknown option: %s", arg.data()));
    }
  }

  if (need_detect_formatter) detect_formatter();

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

// Impl formatters {{{
namespace detail {
inline auto json_string_encode(std::string_view s) -> std::string {
  std::string result;
  for (auto c : s) {
    if (c == '\"')
      result.push_back('\\'), result.push_back('\"');
    else if (c == '\\')
      result.push_back('\\'), result.push_back('\\');
    else if (c == '\b')
      result.push_back('\\'), result.push_back('b');
    else if (c == '\f')
      result.push_back('\\'), result.push_back('f');
    else if (c == '\n')
      result.push_back('\\'), result.push_back('n');
    else if (c == '\r')
      result.push_back('\\'), result.push_back('r');
    else if (c == '\t')
      result.push_back('\\'), result.push_back('t');
    else if (!isprint(c))
      result += format("\\u%04x", static_cast<int>(c));
    else
      result.push_back(c);
  }
  return result;
}

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

inline auto json_formatter(const Report& report) -> std::string {
  return format("{\"status\": \"%s\", \"score\": %.3f, \"message\": \"%s\"}",
                Report::status_to_string(report.status).c_str(), report.score,
                detail::json_string_encode(report.message).c_str());
}

inline auto plain_text_formatter(const Report& report) -> std::string {
  return format("%s, scores %.1f of 100.\n%s",
                detail::status_to_title_string(report.status).c_str(), report.score * 100.0,
                report.message.c_str());
}

inline auto colored_text_formatter(const Report& report) -> std::string {
  return format("%s, scores \x1b[0;33m%.1f\x1b[0m of 100.\n%s",
                detail::status_to_colored_title_string(report.status).c_str(), report.score * 100.0,
                report.message.c_str());
}
// /Impl formatters }}}

}  // namespace checker

}  // namespace cplib
