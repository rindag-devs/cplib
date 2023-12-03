#ifndef CPLIB_HPP_
#define CPLIB_HPP_

#include <regex.h>

#include <cstdint>
#include <functional>
#include <istream>
#include <memory>
#include <optional>
#include <random>
#include <streambuf>
#include <string>
#include <string_view>
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

  explicit Mat(T element, size_t len1, size_t len2);
  explicit Mat(T element, size_t len1, size_t len2, std::string spec1, std::string spec2);

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
using i64 = Int<int32_t>;
using u64 = Int<uint32_t>;
#ifdef __SIZEOF_INT128__
using i128 = Int<__int128>;
using u128 = Int<unsigned __int128>;
#endif
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
