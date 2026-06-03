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

#ifndef CPLIB_GENERATOR_HPP_
#define CPLIB_GENERATOR_HPP_

#include <any>  // IWYU pragma: keep
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

/* cplib_embed_ignore start */
#include "random.hpp"
/* cplib_embed_ignore end */

namespace cplib::generator {
/**
 * `Report` represents a report showing when generator exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  struct Status {
   public:
    enum Value : std::uint8_t {
      /// Indicates an internal error occurred.
      INTERNAL_ERROR,
      /// Indicates the program runs normally.
      OK,
    };

    /**
     * Default constructor for Status.
     */
    Status() = default;

    /**
     * Constructor for Status with a given value.
     *
     * @param value The value of the status.
     */
    constexpr Status(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the status.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

    /**
     * Get the string representation of the status.
     *
     * @return The string representation of the status.
     */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  /// The status of the report.
  Status status;

  /// The message associated with the report.
  std::string message;

  /**
   * Constructor for Report.
   * @param status The status of the report.
   * @param message The message associated with the report.
   */
  Report(Status status, std::string message);
};

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State &state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string> &args) -> void = 0;

 protected:
  auto state() -> State &;

 private:
  State *state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report &report) -> int = 0;
};

struct State {
 public:
  /// The parser function of a flag type (`--flag`) command-line argument.
  using FlagParser = std::function<auto(std::set<std::string> flag_args)->void>;

  /// The parser function of a variable type (`--var=value`) command-line argument.
  using VarParser = std::function<auto(std::map<std::string, std::string> var_args)->void>;

  /// Random number generator.
  Random rnd;

  /// Initializer parses command-line arguments and initializes `generator::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `generator::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

  /// Names of the flag type (`--flag`) command-line arguments required by the generator.
  std::vector<std::string> required_flag_args;

  /// Names of the variable type (`--var=value`) command-line arguments required by the generator.
  std::vector<std::string> required_var_args;

  /// Functions to parse flag type command-line arguments.
  std::vector<FlagParser> flag_parsers;

  /// Functions to parse variable type command-line arguments.
  std::vector<VarParser> var_parsers;

  /**
   * Constructs a new `State` object with the given initializer.
   *
   * @param initializer The initializer.
   */
  explicit State(std::unique_ptr<Initializer> initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Quits the generator with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(const Report &report) -> void;

  /**
   * Quits the generator with the `report::Status::OK` status.
   */
  [[noreturn]] auto quit_ok() -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};
};

/**
 * Default initializer of generator.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string> &args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

struct ArgumentsContext {
  State *state{};
  std::map<std::string, std::any> value_map{};

  explicit ArgumentsContext(State &state);
};

struct ArgValueTag {};

auto set_active_arguments_context(ArgumentsContext &context) -> void;
auto active_arguments_context() -> ArgumentsContext &;

struct FlagArg {
  using ResultType = bool;

  std::string name;
  ArgumentsContext *context;

  explicit FlagArg(std::string name);
  [[nodiscard]] auto operator|(ArgValueTag) const -> const ResultType &;
};

template <class T>
struct VarArg {
  using ResultType = typename T::Target;

  T var;
  ArgumentsContext *context;

  template <class... Args>
  explicit VarArg(Args &&...args);

  [[nodiscard]] auto operator|(ArgValueTag) const -> const ResultType &;
};

using MainFunc = auto (*)() -> void;

auto run_generator(State &state, int argc, char **argv, MainFunc main_func) -> int;

#define CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_)                              \
  auto cplib_generator_args_context_ = ::cplib::generator::ArgumentsContext(state_var_name_); \
  namespace cplib_generator_args_ {                                                           \
  using ::cplib::generator::FlagArg;                                                          \
  using ::cplib::generator::ArgValueTag;                                                      \
  template <class T>                                                                          \
  using Var = ::cplib::generator::VarArg<T>;                                                  \
  using Flag = ::cplib::generator::FlagArg;                                                   \
  }

#define CPLIB_REGISTER_GENERATOR_ARG_(arg)             \
  namespace cplib_generator_args_ {                    \
  const auto &arg | ::cplib::generator::ArgValueTag{}; \
  }

#define CPLIB_REGISTER_GENERATOR_ARGS_DEFER_(id) id CPLIB_REGISTER_GENERATOR_ARGS_EMPTY_()
#define CPLIB_REGISTER_GENERATOR_ARGS_EMPTY_()
#define CPLIB_REGISTER_GENERATOR_ARGS_AGAIN_() CPLIB_REGISTER_GENERATOR_ARGS_FOR_EACH_
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(...) __VA_ARGS__
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(...)                                  \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(                                            \
      CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_( \
          CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(__VA_ARGS__))))
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(...)                                  \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(                                            \
      CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_( \
          CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(__VA_ARGS__))))
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND4_(...)                                  \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(                                            \
      CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_( \
          CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(__VA_ARGS__))))

#define CPLIB_REGISTER_GENERATOR_ARGS_FOR_EACH_(arg, ...) \
  CPLIB_REGISTER_GENERATOR_ARG_(arg)                      \
  __VA_OPT__(                                             \
      CPLIB_REGISTER_GENERATOR_ARGS_DEFER_(CPLIB_REGISTER_GENERATOR_ARGS_AGAIN_)()(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_(...)          \
  __VA_OPT__(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND4_( \
      CPLIB_REGISTER_GENERATOR_ARGS_FOR_EACH_(__VA_ARGS__)))

/**
 * Macro to register generator with custom initializer.
 *
 * @param state_var_name_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR_OPT(state_var_name_, initializer_, args_namespace_name_, ...) \
  auto generator_main() -> void;                                                               \
  auto state_var_name_ = ::cplib::generator::State(                                            \
      ::std::unique_ptr<::cplib::generator::Initializer>(new initializer_));                   \
  CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_);                                    \
  CPLIB_REGISTER_GENERATOR_ARGS_(__VA_ARGS__);                                                 \
  namespace args_namespace_name_ = ::cplib_generator_args_;                                    \
  auto main(int argc, char **argv) -> int {                                                    \
    return ::cplib::generator::run_generator(state_var_name_, argc, argv, generator_main);     \
  }

#ifndef CPLIB_GENERATOR_DEFAULT_INITIALIZER
#define CPLIB_GENERATOR_DEFAULT_INITIALIZER ::cplib::generator::DefaultInitializer()
#endif

/**
 * Macro to register generator with default initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR(var_name_, args_namespace_name_, ...)         \
  CPLIB_REGISTER_GENERATOR_OPT(var_name_, CPLIB_GENERATOR_DEFAULT_INITIALIZER, \
                               args_namespace_name_ __VA_OPT__(, ) __VA_ARGS__)
}  // namespace cplib::generator

#include "generator.i.hpp"  // IWYU pragma: export

#endif
