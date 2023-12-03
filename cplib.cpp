#include "cplib.hpp"

#include <regex.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>
#include <tuple>

#define todo() panic("unimplemented")

namespace cplib {

// Impl panic {{{
namespace {
std::function<void(std::string_view)> panic_impl = [](std::string_view s) {
  std::cerr << "unrecoverable error: " << s << '\n';
  exit(EXIT_FAILURE);
};
}  // namespace

CPLIB_NORETURN auto panic(std::string_view message) -> void {
  panic_impl(message);
  exit(-1);  // Usually unnecessary, but in special cases to prevent problems.
}
// /Impl panic }}}

// Impl format {{{
namespace {
auto string_vsprintf(const char* format, std::va_list args) -> std::string {
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
}  // namespace

CPLIB_PRINTF_LIKE(1, 2) auto format(const char* fmt, ...) -> std::string {
  std::va_list args;
  va_start(args, fmt);
  std::string str{string_vsprintf(fmt, args)};
  va_end(args);
  return str;
}
// /Impl format }}}

// Impl pattern {{{

namespace {
auto get_regex_err_msg(int err_code, regex_t* re) -> std::string {
  size_t length = regerror(err_code, re, NULL, 0);
  std::string buf(length, 0);
  regerror(err_code, re, buf.data(), length);
  return buf;
}
}  // namespace

Pattern::Pattern(std::string src)
    : src_(std::move(src)), re_(new std::pair<regex_t, bool>, [](std::pair<regex_t, bool>* p) {
        if (p->second) regfree(&p->first);
        delete p;
      }) {
  using namespace std::string_literals;
  // In function `regexec`, a match anywhere within the string is considered successful unless the
  // regular expression contains `^` or `$`.
  if (int err = regcomp(&re_->first, ("^"s + src_ + "$"s).c_str(), REG_EXTENDED | REG_NOSUB); err) {
    auto err_msg = get_regex_err_msg(err, &re_->first);
    panic("pattern constructor failed: "s + err_msg);
  }
  re_->second = true;
}

auto Pattern::match(std::string_view s) const -> bool {
  using namespace std::string_literals;

  int result = regexec(&re_->first, s.data(), 0, nullptr, 0);

  if (!result) return true;

  if (result == REG_NOMATCH) return false;

  auto err_msg = get_regex_err_msg(result, &re_->first);
  panic("pattern match failed: "s + err_msg);
  return false;
}

auto Pattern::src() const -> std::string_view { return src_; }

// Impl pattern }}}

namespace io {

InStream::InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict,
                   std::function<void(std::string_view)> fail_func)
    : buf_(std::move(buf)),
      inner_(std::istream(buf_.get())),
      name_(std::move(name)),
      strict_(std::move(strict)),
      fail_func_(std::move(fail_func)),
      line_num_(1),
      col_num_(1) {}

auto InStream::name() const -> std::string_view { return name_; }

auto InStream::skip_blanks() -> void {
  while (!eof() && std::isspace(seek())) read();
}

auto InStream::seek() -> int { return inner_.peek(); }

auto InStream::read() -> int {
  int c = inner_.get();
  if (c == EOF) return EOF;
  if (c == '\n')
    ++line_num_, col_num_ = 1;
  else
    ++col_num_;
  return c;
}

auto InStream::read_n(size_t n) -> std::string {
  std::string buf;
  buf.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    if (eof()) break;
    buf.push_back(static_cast<char>(read()));
  }
  return buf;
}

auto InStream::is_strict() const -> bool { return strict_; }

auto InStream::line_num() const -> size_t { return line_num_; }

auto InStream::col_num() const -> size_t { return col_num_; }

auto InStream::eof() -> bool { return seek() == EOF; }

auto InStream::seek_eof() -> bool {
  skip_blanks();
  return eof();
}

auto InStream::eoln() -> bool { return seek() == '\n'; }

auto InStream::seek_eoln() -> bool {
  skip_blanks();
  return eoln();
}

auto InStream::next_line() -> void {
  int c;
  do c = read();
  while (c != EOF && c != '\n');
}

auto InStream::read_token() -> std::string {
  if (!strict_) skip_blanks();

  std::string token;
  while (!eof() && !std::isspace(seek())) token.push_back(read());
  return token;
}

auto InStream::read_line() -> std::string {
  std::string line;
  while (!eof()) {
    int c = read();
    if (c == EOF || c == '\n') break;
    line.push_back(static_cast<char>(c));
  }
  return line;
}

CPLIB_NORETURN auto InStream::fail(std::string_view message) -> void {
  fail_func_(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in special cases to prevent problems.
}

}  // namespace io

namespace var {

Reader::Reader(std::shared_ptr<io::InStream> inner) : Reader(inner, nullptr) {}

Reader::Reader(std::shared_ptr<io::InStream> inner, std::shared_ptr<Trace> trace)
    : inner_(inner), trace_(trace) {}

auto Reader::inner() -> std::shared_ptr<io::InStream> { return inner_; }

CPLIB_NORETURN auto Reader::fail(std::string_view message) -> void {
  inner_->fail(format("%s:%zu:%zu: %s: %s", trace_->stream_name.c_str(), trace_->line_num,
                      trace_->col_num, trace_->var_name.c_str(), message.data()));
}

template <class T>
auto Reader::read(const Var<T>& v) -> T {
  auto trace = std::make_shared<Trace>(Trace{std::string(v.name()), std::string(inner_->name()),
                                             inner_->line_num(), inner_->col_num(), trace_});
  auto child = Reader(inner_, std::move(trace));
  return v.read_from(child);
}

template <class... T>
auto Reader::operator()(T... vars) -> std::tuple<typename T::Var::inner_type...> {
  return {read(vars)...};
}

namespace {
constexpr std::string_view VAR_DEFAULT_NAME("<unnamed>");
}

template <class T>
Var<T>::~Var() {}

template <class T>
auto Var<T>::name() const -> std::string_view {
  return name_;
}

template <class T>
Var<T>::Var() : name_(std::string(VAR_DEFAULT_NAME)) {}

template <class T>
auto renamed(const T& var, std::string_view name) -> T {
  auto cloned = var;
  cloned.name_ = std::string(name);
  return cloned;
}

template <class T>
Var<T>::Var(std::string name) : name_(std::move(name)) {}

template <class T>
Int<T>::Int() : Int<T>(std::nullopt, std::nullopt, std::string(VAR_DEFAULT_NAME)) {}

template <class T>
Int<T>::Int(std::string name) : Int<T>(std::nullopt, std::nullopt, std::move(name)) {}

template <class T>
Int<T>::Int(std::optional<T> min, std::optional<T> max)
    : Int<T>(std::move(min), std::move(max), std::string(VAR_DEFAULT_NAME)) {}

template <class T>
Int<T>::Int(std::optional<T> min, std::optional<T> max, std::string name)
    : Var<T>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
auto Int<T>::read_from(Reader& in) const -> T {
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
Float<T>::Float() : Float<T>(std::nullopt, std::nullopt, std::string(VAR_DEFAULT_NAME)) {}

template <class T>
Float<T>::Float(std::string name) : Float<T>(std::nullopt, std::nullopt, std::move(name)) {}

template <class T>
Float<T>::Float(std::optional<T> min, std::optional<T> max)
    : Float<T>(std::move(min), std::move(max), std::string(VAR_DEFAULT_NAME)) {}

template <class T>
Float<T>::Float(std::optional<T> min, std::optional<T> max, std::string name)
    : Var<T>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
auto Float<T>::read_from(Reader& in) const -> T {
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
StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit)
    : StrictFloat<T>(min, max, min_n_digit, max_n_digit, std::string(VAR_DEFAULT_NAME)) {}

template <class T>
StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit, std::string name)
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
auto StrictFloat<T>::read_from(Reader& in) const -> T {
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

String::String() : String(std::string(VAR_DEFAULT_NAME)) {}

String::String(Pattern pat) : String(std::move(pat), std::string(VAR_DEFAULT_NAME)) {}

String::String(std::string name) : Var<std::string>(std::move(name)), pat(std::nullopt) {}

String::String(Pattern pat, std::string name)
    : Var<std::string>(std::move(name)), pat(std::move(pat)) {}

auto String::read_from(Reader& in) const -> std::string {
  auto token = in.inner()->read_token();

  if (pat.has_value() && !pat->match(token)) {
    in.fail(format("Expected a string matching `%s`, got `%s`", pat->src().data(), token.c_str()));
  }

  return token;
}

// Impl StrictString {{{
namespace {
auto read_spaces(Reader& in, std::string& result) -> bool {
  if (in.inner()->eof() || !std::isspace(in.inner()->seek())) return false;
  while (!in.inner()->eof() && std::isspace(in.inner()->seek())) {
    result.push_back(static_cast<char>(in.inner()->read()));
  }
  return true;
}
}  // namespace

StrictString::StrictString(std::string pat)
    : StrictString(std::move(pat), std::string(VAR_DEFAULT_NAME)) {}

StrictString::StrictString(std::string pat, std::string name)
    : Var<std::string>(std::move(name)), pat(std::move(pat)) {}

auto StrictString::read_from(Reader& in) const -> std::string {
  if (pat.empty()) return "";

  if (in.inner()->is_strict()) {
    auto size = pat.size();
    auto str = in.inner()->read_n(size);
    if (str != pat)
      in.fail(format("Expected a strict string matching `%s`, got `%s`", pat.data(), str.c_str()));
    return str;
  }

  std::string result;

  if (!std::isspace(pat.front())) read_spaces(in, result);

  bool pat_last_blank = false;
  for (auto c : pat) {
    if (std::isspace(c) && pat_last_blank) continue;

    if (std::isspace(c)) {
      auto has_space = read_spaces(in, result);
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

Line::Line() : Line(std::string(VAR_DEFAULT_NAME)) {}

Line::Line(Pattern pat) : Line(std::move(pat), std::string(VAR_DEFAULT_NAME)) {}

Line::Line(std::string name) : Var<std::string>(std::move(name)), pat(std::nullopt) {}

Line::Line(Pattern pat, std::string name)
    : Var<std::string>(std::move(name)), pat(std::move(pat)) {}

auto Line::read_from(Reader& in) const -> std::string {
  auto token = in.inner()->read_line();

  if (pat.has_value() && pat->match(token)) {
    in.fail(format("Expected a string matching `%s`, got `%s`", pat->src().data(), token.c_str()));
  }

  return token;
}

template <class T>
Vec<T>::Vec(T element, size_t len) : Vec<T>(element, len, " ") {}

template <class T>
Vec<T>::Vec(T element, size_t len, std::string spec)
    : Var<std::vector<typename T::Var::inner_type>>(std::string(element.name())),
      element(std::move(element)),
      len(std::move(len)),
      spec(StrictString(spec, "spec")) {}

template <class T>
auto Vec<T>::read_from(Reader& in) const -> std::vector<typename T::Var::inner_type> {
  std::vector<typename T::Var::inner_type> result(len);
  for (size_t i = 0; i < len; ++i) {
    if (i > 0) in.read(renamed(spec, format("spec_%zu", i)));
    result[i] = in.read(renamed(element, std::to_string(i)));
  }
  return result;
}

template <class T>
Mat<T>::Mat(T element, size_t len0, size_t len1) : Mat<T>(element, len0, len1, " ", "\n") {}

template <class T>
Mat<T>::Mat(T element, size_t len0, size_t len1, std::string spec0, std::string spec1)
    : Var<std::vector<std::vector<typename T::Var::inner_type>>>(std::string(element.name())),
      element(std::move(element)),
      len0(std::move(len0)),
      len1(std::move(len1)),
      spec0(StrictString(spec0, "spec0")),
      spec1(StrictString(spec1, "spec1")) {}

template <class T>
auto Mat<T>::read_from(Reader& in) const -> std::vector<std::vector<typename T::Var::inner_type>> {
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
Pair<F, S>::Pair(F first, S second)
    : Pair<F, S>(std::move(first), std::move(second), " ", std::string(VAR_DEFAULT_NAME)) {}

template <class F, class S>
Pair<F, S>::Pair(F first, S second, std::string spec)
    : Pair<F, S>(std::move(first), std::move(second), std::move(StrictString(spec, "spec")),
                 std::string(VAR_DEFAULT_NAME)) {}

template <class F, class S>
Pair<F, S>::Pair(F first, S second, std::string spec, std::string name)
    : Var<std::pair<typename F::Var::inner_type, typename S::Var::inner_type>>(std::move(name)),
      first(std::move(first)),
      second(std::move(second)),
      spec(std::move(spec)) {}

template <class F, class S>
Pair<F, S>::Pair(std::pair<F, S> pr)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), " ", std::string(VAR_DEFAULT_NAME)) {}

template <class F, class S>
Pair<F, S>::Pair(std::pair<F, S> pr, std::string spec)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), std::move(spec),
                 std::string(VAR_DEFAULT_NAME)) {}

template <class F, class S>
Pair<F, S>::Pair(std::pair<F, S> pr, std::string spec, std::string name)
    : Pair<F, S>(std::move(pr.first), std::move(pr.second), std::move(spec), std::move(name)) {}

template <class F, class S>
auto Pair<F, S>::read_from(Reader& in) const
    -> std::pair<typename F::Var::inner_type, typename S::Var::inner_type> {
  auto result_first = in.read(renamed(first, "first"));
  in.read(spec);
  auto result_second = in.read(renamed(second, "second"));
  return {result_first, result_second};
}

template <class... T>
Tuple<T...>::Tuple(std::tuple<T...> elements)
    : Tuple<T...>(std::move(elements), " ", std::string(VAR_DEFAULT_NAME)) {}

template <class... T>
Tuple<T...>::Tuple(std::tuple<T...> elements, std::string spec)
    : Tuple<T...>(std::move(elements), std::move(spec), std::string(VAR_DEFAULT_NAME)) {}

template <class... T>
Tuple<T...>::Tuple(std::tuple<T...> elements, std::string spec, std::string name)
    : Var<std::tuple<typename T::Var::inner_type...>>(std::move(name)),
      elements(std::move(elements)),
      spec(std::move(spec)) {}

template <class... T>
auto Tuple<T...>::read_from(Reader& in) const -> std::tuple<typename T::Var::inner_type...> {
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
FnVar<F>::FnVar(std::function<F> f, Args... args)
    : FnVar<F>(std::string(VAR_DEFAULT_NAME), f, args...) {}

template <class F>
template <class... Args>
FnVar<F>::FnVar(std::string name, std::function<F> f, Args... args)
    : Var<typename std::function<F>::result_type>(std::move(name)),
      inner([f, args...](Reader& in) { return f(in, args...); }) {}

template <class F>
auto FnVar<F>::read_from(Reader& in) const -> typename std::function<F>::result_type {
  return inner(in);
}

}  // namespace var

// Impl get_work_mode {{{
namespace {
WorkMode work_mode = WorkMode::NONE;
}

auto get_work_mode() -> WorkMode { return work_mode; }

// /Impl get_work_mode }}}

namespace checker {

// Impl Report {{{

namespace {
auto encode_json_string(std::string_view s) -> std::string {
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
}  // namespace

auto Report::status_to_string(Report::Status status) -> std::string {
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

Report::Report(Report::Status status, double score, std::string message)
    : status(std::move(status)), score(std::move(score)), message(std::move(message)) {}

auto Report::to_json() const -> std::string {
  return format("{\"status\": \"%s\", \"score\": %.3f, \"message\": \"%s\"}",
                status_to_string(status).c_str(), score, encode_json_string(message).c_str());
}

// /Impl Report }}}

// Impl State {{{

State::State(CompatibleConf conf)
    : inf(var::Reader(nullptr)), ouf(var::Reader(nullptr)), ans(var::Reader(nullptr)) {
  panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  work_mode = WorkMode::CHECKER;
}

auto State::set_args(int argc, char** argv) -> void {
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

CPLIB_NORETURN auto State::quit(Report report) -> void {
  std::clog << report.to_json() << '\n';
  std::exit(report.status == Report::Status::INTERNAL_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}

CPLIB_NORETURN auto State::quit_ac() -> void { quit(Report(Report::Status::ACCEPTED, 1.0, "")); }

CPLIB_NORETURN auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 1.0, std::string(message)));
}

CPLIB_NORETURN auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}

// /Impl State }}}

}  // namespace checker

}  // namespace cplib
