#ifndef CPLIB_HPP_
#define CPLIB_HPP_

#include <regex.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

#if __STDC_VERSION__ >= 201112L
#define NORETURN _Noreturn
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

// Format string using printf-like syntax.
CPLIB_PRINTF_LIKE(1, 2) auto format(const char* fmt, ...) -> std::string;

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
                    std::function<void(std::string_view)> fail_func);

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

  // Get the current line number
  auto line_num() const -> size_t;

  // Get the current column number
  auto col_num() const -> size_t;

  // Check if current position is EOF. If not it doesn't move stream pointer
  auto eof() -> bool;

  // Move pointer to the first non-white-space character and calls [`eof()`]
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
   * Read a new token. Ignores white-spaces into the non-strict mode
   * (strict mode is used in validators usually).
   */
  auto read_token() -> std::string;

  /**
   * Reads line from the current position to EOLN or EOF. Moves stream pointer to
   * the first character of the new line (if possible).
   *
   * The return value does not include the trailing \n.
   */
  auto read_line() -> std::string;

  // Quit program with an error
  CPLIB_NORETURN auto fail(std::string_view message) -> void;

 private:
  std::unique_ptr<std::streambuf> buf_;
  std::istream inner_;
  std::string name_;
  bool strict_;                                      // In strict mode, blank-chars are not ignored
  std::function<void(std::string_view)> fail_func_;  // Calls when fail
  size_t line_num_;
  size_t col_num_;
};

}  // namespace io

namespace var {

template <class T>
class Var;

// Represents a traced input stream with line and column information
class Reader {
 public:
  struct Trace {
    std::string var_name;
    std::string stream_name;
    size_t line_num;
    size_t col_num;
    std::shared_ptr<Trace> parent;
  };

  // Create a root reader of input stream
  explicit Reader(std::shared_ptr<io::InStream> inner);

  explicit Reader(Reader&&) = default;
  Reader& operator=(Reader&&) = default;

  // Get the inner wrapped input stream
  auto inner() -> std::shared_ptr<io::InStream>;

  // Call [`Instream::fail`] of the wrapped input stream
  CPLIB_NORETURN auto fail(std::string_view message) -> void;

  // Read a variable
  template <class T>
  auto read(const Var<T>& v) -> T;

  // Read multiple variables and put them into a tuple
  template <class... T>
  auto operator()(T... vars) -> std::tuple<typename T::Var::inner_type...>;

 private:
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  explicit Reader(std::shared_ptr<io::InStream> inner, std::shared_ptr<Trace> trace);

  std::shared_ptr<io::InStream> inner_;
  std::shared_ptr<Trace> trace_;
};

// Generic template class for variables
template <class T>
class Var {
 public:
  using inner_type = T;

  virtual ~Var() = 0;

  // Get the name of variable
  auto name() const -> std::string_view;

  friend auto Reader::read(const Var<T>& v) -> T;

  template <class T1>
  friend auto renamed(const T1& var, std::string_view name) -> T1;

 protected:
  explicit Var();
  explicit Var(std::string name);

  virtual auto read_from(Reader& in) const -> T = 0;

 private:
  std::string name_;
};

// Create a variable based on self with the given name
template <class T>
auto renamed(const T& var, std::string_view name) -> T;

// Integer
template <class T>
class Int : public Var<T> {
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
class Float : public Var<T> {
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
class StrictFloat : public Var<T> {
 public:
  T min, max;
  size_t min_n_digit, max_n_digit;

  explicit StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit);
  explicit StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit, std::string name);

 protected:
  auto read_from(Reader& in) const -> T override;
};

// Blank-char separated string
class String : public Var<std::string> {
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
 * Strict string
 *
 * - In strict mode, read exact pat.size() characters and determine whether they are equal to pat.
 * - Otherwise, a minimum number of characters will be matched as follows:
 *   - Match zero or more blank-chars (' ', '\n', '\t', ...) at the beginning of the string.
 *   - Non-blank-chars matches exactly one corresponding character.
 *   - Longest continuously string of blank-chars match one or more number of blank-chars.
 */
class StrictString : public Var<std::string> {
 public:
  std::string pat;

  explicit StrictString(std::string pat);
  explicit StrictString(std::string pat, std::string name);

 protected:
  auto read_from(Reader& in) const -> std::string override;
};

// Eoln separated string
class Line : public Var<std::string> {
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
class Vec : public Var<std::vector<typename T::Var::inner_type>> {
 public:
  T element;
  size_t len;
  StrictString spec;

  explicit Vec(T element, size_t len);
  explicit Vec(T element, size_t len, std::string spec);

 protected:
  auto read_from(Reader& in) const -> std::vector<typename T::Var::inner_type> override;
};

// Matrix (2D vector) of variables
template <class T>
class Mat : public Var<std::vector<std::vector<typename T::Var::inner_type>>> {
 public:
  T element;
  size_t len0, len1;
  StrictString spec0, spec1;

  explicit Mat(T element, size_t len0, size_t len1);
  explicit Mat(T element, size_t len0, size_t len1, std::string spec0, std::string spec1);

 protected:
  auto read_from(Reader& in) const
      -> std::vector<std::vector<typename T::Var::inner_type>> override;
};

// Pair of variables
template <class F, class S>
class Pair : public Var<std::pair<typename F::Var::inner_type, typename S::Var::inner_type>> {
 public:
  F first;
  S second;
  StrictString spec;

  explicit Pair(F first, S second);
  explicit Pair(F first, S second, std::string spec);
  explicit Pair(F first, S second, std::string spec, std::string name);
  explicit Pair(std::pair<F, S> pr);
  explicit Pair(std::pair<F, S> pr, std::string spec);
  explicit Pair(std::pair<F, S> pr, std::string spec, std::string name);

 protected:
  auto read_from(Reader& in) const
      -> std::pair<typename F::Var::inner_type, typename S::Var::inner_type> override;
};

// Tuple of variables
template <class... T>
class Tuple : public Var<std::tuple<typename T::Var::inner_type...>> {
 public:
  std::tuple<T...> elements;
  StrictString spec;

  explicit Tuple(std::tuple<T...> elements);
  explicit Tuple(std::tuple<T...> elements, std::string spec);
  explicit Tuple(std::tuple<T...> elements, std::string spec, std::string name);

 protected:
  auto read_from(Reader& in) const -> std::tuple<typename T::Var::inner_type...> override;
};

template <class F>
class FnVar : public Var<typename std::function<F>::result_type> {
 public:
  std::function<typename std::function<F>::result_type(Reader& in)> inner;

  template <class... Args>
  FnVar(std::function<F> f, Args... args);

  template <class... Args>
  FnVar(std::string name, std::function<F> f, Args... args);

 protected:
  auto read_from(Reader& in) const -> typename std::function<F>::result_type override;
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

  auto to_json() const -> std::string;
};

enum class CompatibleConf : uint64_t {
  NONE,
};

class State {
 public:
  var::Reader inf;
  var::Reader ouf;
  var::Reader ans;

  State(CompatibleConf conf);

  // Set command line arguments
  auto set_args(int argc, char** argv) -> void;

  // Quit checker with status
  CPLIB_NORETURN auto quit(Report report) -> void;

  // Quit checker with [`report::status::ACCEPTED`]
  CPLIB_NORETURN auto quit_ac() -> void;

  // Quit checker with [`report::status::WRONG_ANSWER`]
  CPLIB_NORETURN auto quit_wa(std::string_view message) -> void;

  // Quit checker with [`report::status::PARTIALLY_CORRECT`]
  CPLIB_NORETURN auto quit_pc(double points, std::string_view message) -> void;
};

// Macro to register checker
#define CPLIB_REGISTER_CHECKER(var, compatible_conf) \
  cplib::checker::State var(compatible_conf);        \
  signed main(signed argc, char** argv) {            \
    var.set_args(argc, argv);                        \
    auto checker_main(void)->void;                   \
    checker_main();                                  \
    return 0;                                        \
  }

}  // namespace checker
};  // namespace cplib

#endif  // CPLIB_HPP_

// === Implementations ===

namespace cplib {

// Impl panic {{{
namespace detail {
inline std::function<void(std::string_view)> panic_impl = [](std::string_view s) {
  std::cerr << "unrecoverable error: " << s << '\n';
  exit(EXIT_FAILURE);
};
}  // namespace detail

CPLIB_NORETURN inline auto panic(std::string_view message) -> void {
  detail::panic_impl(message);
  exit(-1);  // Usually unnecessary, but in special cases to prevent problems.
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

// Impl pattern {{{

namespace detail {
inline auto get_regex_err_msg(int err_code, regex_t* re) -> std::string {
  size_t length = regerror(err_code, re, NULL, 0);
  std::string buf(length, 0);
  regerror(err_code, re, buf.data(), length);
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
                          std::function<void(std::string_view)> fail_func)
    : buf_(std::move(buf)),
      inner_(std::istream(buf_.get())),
      name_(std::move(name)),
      strict_(std::move(strict)),
      fail_func_(std::move(fail_func)),
      line_num_(1),
      col_num_(1) {}

inline auto InStream::name() const -> std::string_view { return name_; }

inline auto InStream::skip_blanks() -> void {
  while (!eof() && std::isspace(seek())) read();
}

inline auto InStream::seek() -> int { return inner_.peek(); }

inline auto InStream::read() -> int {
  int c = inner_.get();
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

inline auto InStream::line_num() const -> size_t { return line_num_; }

inline auto InStream::col_num() const -> size_t { return col_num_; }

inline auto InStream::eof() -> bool { return seek() == EOF; }

inline auto InStream::seek_eof() -> bool {
  skip_blanks();
  return eof();
}

inline auto InStream::eoln() -> bool { return seek() == '\n'; }

inline auto InStream::seek_eoln() -> bool {
  skip_blanks();
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
  while (!eof() && !std::isspace(seek())) token.push_back(read());
  return token;
}

inline auto InStream::read_line() -> std::string {
  std::string line;
  while (!eof()) {
    int c = read();
    if (c == EOF || c == '\n') break;
    line.push_back(static_cast<char>(c));
  }
  return line;
}

CPLIB_NORETURN inline auto InStream::fail(std::string_view message) -> void {
  fail_func_(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in special cases to prevent problems.
}

}  // namespace io

namespace var {

inline Reader::Reader(std::shared_ptr<io::InStream> inner) : Reader(inner, nullptr) {}

inline Reader::Reader(std::shared_ptr<io::InStream> inner, std::shared_ptr<Trace> trace)
    : inner_(inner), trace_(trace) {}

inline auto Reader::inner() -> std::shared_ptr<io::InStream> { return inner_; }

CPLIB_NORETURN inline auto Reader::fail(std::string_view message) -> void {
  using namespace std::string_literals;
  std::string result = "read error: "s + std::string(message);
  auto p = trace_;
  size_t depth = 0;
  while (p != nullptr) {
    result += format("\n  #%zu: %s @ %s:%zu:%zu", depth, p->var_name.c_str(),
                     p->stream_name.c_str(), p->line_num, p->col_num);
    p = p->parent;
    ++depth;
  }
  inner_->fail(result);
}

template <class T>
inline auto Reader::read(const Var<T>& v) -> T {
  auto trace = std::make_shared<Trace>(Trace{std::string(v.name()), std::string(inner_->name()),
                                             inner_->line_num(), inner_->col_num(), trace_});
  auto child = Reader(inner_, std::move(trace));
  return v.read_from(child);
}

template <class... T>
inline auto Reader::operator()(T... vars) -> std::tuple<typename T::Var::inner_type...> {
  return {read(vars)...};
}

namespace detail {
inline constexpr std::string_view VAR_DEFAULT_NAME("<unnamed>");
}

template <class T>
inline Var<T>::~Var() {}

template <class T>
inline auto Var<T>::name() const -> std::string_view {
  return name_;
}

template <class T>
inline Var<T>::Var() : name_(std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline auto renamed(const T& var, std::string_view name) -> T {
  auto cloned = var;
  cloned.name_ = std::string(name);
  return cloned;
}

template <class T>
inline Var<T>::Var(std::string name) : name_(std::move(name)) {}

template <class T>
inline Int<T>::Int() : Int<T>(std::nullopt, std::nullopt, std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline Int<T>::Int(std::string name) : Int<T>(std::nullopt, std::nullopt, std::move(name)) {}

template <class T>
inline Int<T>::Int(std::optional<T> min, std::optional<T> max)
    : Int<T>(std::move(min), std::move(max), std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline Int<T>::Int(std::optional<T> min, std::optional<T> max, std::string name)
    : Var<T>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
inline auto Int<T>::read_from(Reader& in) const -> T {
  auto token = in.inner()->read_token();

  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result);

  if (ec != std::errc() || ptr != token.c_str() + token.size()) {
    in.fail(format("Expected an integer, got `%s`", token.c_str()));
  }

  if (min.has_value() && result < min.value()) {
    in.fail(format("Expected an integer >= %s, got `%s`", std::to_string(min.value()).c_str(),
                   token.c_str()));
  }

  if (max.has_value() && result > max.value()) {
    in.fail(format("Expected an integer <= %s, got `%s`", std::to_string(max.value()).c_str(),
                   token.c_str()));
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
    : Var<T>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
inline auto Float<T>::read_from(Reader& in) const -> T {
  auto token = in.inner()->read_token();

  // `Float<T>` usually uses with non-strict streams, so it should support both fixed format and
  // scientific.
  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result,
                                   std::chars_format::general);

  if (ec != std::errc() || ptr != token.c_str() + token.size()) {
    in.fail(format("Expected a float, got `%s`", token.c_str()));
  }

  if (min.has_value() && result < min.value()) {
    in.fail(format("Expected a float >= %s, got `%s`", std::to_string(min.value()).c_str(),
                   token.c_str()));
  }

  if (max.has_value() && result > max.value()) {
    in.fail(format("Expected a float <= %s, got `%s`", std::to_string(max.value()).c_str(),
                   token.c_str()));
  }

  return result;
}

template <class T>
inline StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit)
    : StrictFloat<T>(min, max, min_n_digit, max_n_digit, std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T>
inline StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit,
                                   std::string name)
    : Var<T>(std::move(name)),
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
  auto token = in.inner()->read_token();

  auto pat = Pattern(min_n_digit == 0
                         ? format("-?([1-9][0-9]*|0)(\\.[0-9]{,%zu})?", max_n_digit)
                         : format("-?([1-9][0-9]*|0)\\.[0-9]{%zu,%zu}", min_n_digit, max_n_digit));

  if (!pat.match(token)) {
    in.fail(format("Expected a strict float, got `%s`", token.c_str()));
  }

  // Different from `Float<T>`, only fixed format should be allowed.
  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result,
                                   std::chars_format::fixed);

  if (ec != std::errc() || ptr != token.c_str() + token.size()) {
    in.fail(format("Expected a strict float, got `%s`", token.c_str()));
  }

  if (result < min) {
    in.fail(format("Expected a strict float >= %s, got `%s`", std::to_string(min).c_str(),
                   token.c_str()));
  }

  if (result > max) {
    in.fail(format("Expected a strict float <= %s, got `%s`", std::to_string(max).c_str(),
                   token.c_str()));
  }

  return result;
}

inline String::String() : String(std::string(detail::VAR_DEFAULT_NAME)) {}

inline String::String(Pattern pat)
    : String(std::move(pat), std::string(detail::VAR_DEFAULT_NAME)) {}

inline String::String(std::string name) : Var<std::string>(std::move(name)), pat(std::nullopt) {}

inline String::String(Pattern pat, std::string name)
    : Var<std::string>(std::move(name)), pat(std::move(pat)) {}

inline auto String::read_from(Reader& in) const -> std::string {
  auto token = in.inner()->read_token();

  if (pat.has_value() && !pat->match(token)) {
    in.fail(format("Expected a string matching `%s`, got `%s`", pat->src().data(), token.c_str()));
  }

  return token;
}

// Impl StrictString {{{
namespace detail {
inline auto read_spaces(Reader& in, std::string& result) -> bool {
  if (in.inner()->eof() || !std::isspace(in.inner()->seek())) return false;
  while (!in.inner()->eof() && std::isspace(in.inner()->seek())) {
    result.push_back(static_cast<char>(in.inner()->read()));
  }
  return true;
}
}  // namespace detail

inline StrictString::StrictString(std::string pat)
    : StrictString(std::move(pat), std::string(detail::VAR_DEFAULT_NAME)) {}

inline StrictString::StrictString(std::string pat, std::string name)
    : Var<std::string>(std::move(name)), pat(std::move(pat)) {}

inline auto StrictString::read_from(Reader& in) const -> std::string {
  if (pat.empty()) return "";

  if (in.inner()->is_strict()) {
    auto size = pat.size();
    auto str = in.inner()->read_n(size);
    if (str != pat)
      in.fail(format("Expected a strict string matching `%s`, got `%s`", pat.data(), str.c_str()));
    return str;
  }

  std::string result;

  if (!std::isspace(pat.front())) detail::read_spaces(in, result);

  bool pat_last_blank = false;
  for (auto c : pat) {
    if (std::isspace(c) && pat_last_blank) continue;

    if (std::isspace(c)) {
      auto has_space = detail::read_spaces(in, result);
      if (!has_space) in.fail("Expected whitespace while reading strict string");
      pat_last_blank = true;
      continue;
    }

    if (in.inner()->eof()) in.fail("Unexpected EOF while reading strict string");

    int got = in.inner()->read();
    if (got != c)
      in.fail(format("Expected charector `%c` while reading strict string, got `%c`", c, got));
    result.push_back(got);
    pat_last_blank = false;
  }

  return result;
}
// /Impl StrictString }}}

inline Line::Line() : Line(std::string(detail::VAR_DEFAULT_NAME)) {}

inline Line::Line(Pattern pat) : Line(std::move(pat), std::string(detail::VAR_DEFAULT_NAME)) {}

inline Line::Line(std::string name) : Var<std::string>(std::move(name)), pat(std::nullopt) {}

inline Line::Line(Pattern pat, std::string name)
    : Var<std::string>(std::move(name)), pat(std::move(pat)) {}

inline auto Line::read_from(Reader& in) const -> std::string {
  auto token = in.inner()->read_line();

  if (pat.has_value() && pat->match(token)) {
    in.fail(format("Expected a string matching `%s`, got `%s`", pat->src().data(), token.c_str()));
  }

  return token;
}

template <class T>
inline Vec<T>::Vec(T element, size_t len) : Vec<T>(element, len, " ") {}

template <class T>
inline Vec<T>::Vec(T element, size_t len, std::string spec)
    : Var<std::vector<typename T::Var::inner_type>>(std::string(element.name())),
      element(std::move(element)),
      len(std::move(len)),
      spec(StrictString(spec, "spec")) {}

template <class T>
inline auto Vec<T>::read_from(Reader& in) const -> std::vector<typename T::Var::inner_type> {
  std::vector<typename T::Var::inner_type> result(len);
  for (size_t i = 0; i < len; ++i) {
    if (i > 0) in.read(renamed(spec, format("spec_%zu", i)));
    result[i] = in.read(renamed(element, std::to_string(i)));
  }
  return result;
}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1) : Mat<T>(element, len0, len1, " ", "\n") {}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1, std::string spec0, std::string spec1)
    : Var<std::vector<std::vector<typename T::Var::inner_type>>>(std::string(element.name())),
      element(std::move(element)),
      len0(std::move(len0)),
      len1(std::move(len1)),
      spec0(StrictString(spec0, "spec0")),
      spec1(StrictString(spec1, "spec1")) {}

template <class T>
inline auto Mat<T>::read_from(Reader& in) const
    -> std::vector<std::vector<typename T::Var::inner_type>> {
  std::vector<std::vector<typename T::Var::inner_type>> result(
      len0, std::vector<typename T::Var::inner_type>(len1));
  for (size_t i = 0; i < len0; ++i) {
    if (i > 1) in.read(renamed(spec0, format("spec_%zu", i)));
    for (size_t j = 0; j < len1; ++i) {
      if (j > 1) in.read(renamed(spec1, format("spec_%zu_%zu", i, j)));
      result[i][j] = in.read(renamed(element, format("%zu_%zu", i, j)));
    }
  }
  return result;
}

template <class F, class S>
inline Pair<F, S>::Pair(F first, S second)
    : Pair<F, S>(std::move(first), std::move(second), " ", std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(F first, S second, std::string spec)
    : Pair<F, S>(std::move(first), std::move(second), std::move(StrictString(spec, "spec")),
                 std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(F first, S second, std::string spec, std::string name)
    : Var<std::pair<typename F::Var::inner_type, typename S::Var::inner_type>>(std::move(name)),
      first(std::move(first)),
      second(std::move(second)),
      spec(std::move(spec)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), " ",
                 std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr, std::string spec)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), std::move(spec),
                 std::string(detail::VAR_DEFAULT_NAME)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr, std::string spec, std::string name)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), std::move(spec), std::move(name)) {}

template <class F, class S>
inline auto Pair<F, S>::read_from(Reader& in) const
    -> std::pair<typename F::Var::inner_type, typename S::Var::inner_type> {
  auto result_first = in.read(renamed(first, "first"));
  in.read(spec);
  auto result_second = in.read(renamed(second, "second"));
  return {result_first, result_second};
}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements)
    : Tuple<T...>(std::move(elements), " ", std::string(detail::VAR_DEFAULT_NAME)) {}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements, std::string spec)
    : Tuple<T...>(std::move(elements), std::move(spec), std::string(detail::VAR_DEFAULT_NAME)) {}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements, std::string spec, std::string name)
    : Var<std::tuple<typename T::Var::inner_type...>>(std::move(name)),
      elements(std::move(elements)),
      spec(std::move(spec)) {}

template <class... T>
inline auto Tuple<T...>::read_from(Reader& in) const -> std::tuple<typename T::Var::inner_type...> {
  return std::apply(
      [&in](const auto&... args) {
        size_t cnt = 0;
        auto renamed_inc = [&cnt](auto var) { return renamed(var, std::to_string(cnt++)); };
        return std::tuple{in.read(renamed_inc(args))...};
      },
      elements);
}

template <class F>
template <class... Args>
inline FnVar<F>::FnVar(std::function<F> f, Args... args)
    : FnVar<F>(std::string(detail::VAR_DEFAULT_NAME), f, args...) {}

template <class F>
template <class... Args>
inline FnVar<F>::FnVar(std::string name, std::function<F> f, Args... args)
    : Var<typename std::function<F>::result_type>(std::move(name)),
      inner([f, args...](Reader& in) { return f(in, args...); }) {}

template <class F>
inline auto FnVar<F>::read_from(Reader& in) const -> typename std::function<F>::result_type {
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

// Impl Report {{{

namespace detail {
inline auto encode_json_string(std::string_view s) -> std::string {
  std::string result;
  for (auto c : s) {
    if (c == '\'')
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
}  // namespace detail

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

inline auto Report::to_json() const -> std::string {
  return format("{\"status\": \"%s\", \"score\": %.3f, \"message\": \"%s\"}",
                status_to_string(status).c_str(), score,
                detail::encode_json_string(message).c_str());
}

// /Impl Report }}}

// Impl State {{{

inline State::State(CompatibleConf conf)
    : inf(var::Reader(nullptr)), ouf(var::Reader(nullptr)), ans(var::Reader(nullptr)) {
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::CHECKER;
}

inline auto State::set_args(int argc, char** argv) -> void {
  if (argc != 4)
    panic(
        "Program must be run with the following arguments:\n"
        "<input_file> <output_file> <answer_file>");

  auto inf_buf = std::make_unique<std::filebuf>();
  if (!inf_buf->open(argv[1], std::ios::binary | std::ios::in)) panic("Can't open input file");
  inf = var::Reader(std::make_shared<io::InStream>(std::move(inf_buf), "inf", false,
                                                   [](std::string_view msg) { panic(msg); }));

  auto ouf_buf = std::make_unique<std::filebuf>();
  if (!ouf_buf->open(argv[1], std::ios::binary | std::ios::in)) panic("Can't open output file");
  ouf = var::Reader(std::make_shared<io::InStream>(std::move(ouf_buf), "ouf", false,
                                                   [this](std::string_view msg) { quit_wa(msg); }));

  auto ans_buf = std::make_unique<std::filebuf>();
  if (!ans_buf->open(argv[1], std::ios::binary | std::ios::in)) panic("Can't open answer file");
  ans = var::Reader(std::make_shared<io::InStream>(std::move(ans_buf), "ans", false,
                                                   [](std::string_view msg) { panic(msg); }));
}

CPLIB_NORETURN inline auto State::quit(Report report) -> void {
  std::clog << report.to_json() << '\n';
  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}

CPLIB_NORETURN inline auto State::quit_ac() -> void {
  quit(Report(Report::Status::ACCEPTED, 1.0, ""));
}

CPLIB_NORETURN inline auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 1.0, std::string(message)));
}

CPLIB_NORETURN inline auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}

// /Impl State }}}

}  // namespace checker

}  // namespace cplib
