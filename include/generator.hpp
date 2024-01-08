/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_GENERATOR_HPP_
#define CPLIB_GENERATOR_HPP_

#include <functional>   // for function
#include <map>          // for map
#include <set>          // for set
#include <string>       // for string, basic_string
#include <string_view>  // for string_view
#include <vector>       // for vector

/* cplib_embed_ignore start */
#include "random.hpp"  // for Random
/* cplib_embed_ignore end */

namespace cplib::generator {
/**
 * `Report` represents a report showing when generator exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  class Status {
   public:
    enum Value {
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

class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = std::function<auto(State& state, int argc, char** argv)->void>;

  /// The type of function used for reporting.
  using Reporter = std::function<auto(const Report& report)->void>;

  /// The parser function of a flag type (`--flag`) command line argument.
  using FlagParser = std::function<auto(std::set<std::string> flag_args)->void>;

  /// The parser function of a variable type (`--var=value`) command line argument.
  using VarParser = std::function<auto(std::map<std::string, std::string> var_args)->void>;

  /// Random number generator.
  Random rnd;

  /// Initializer is a function parsing command line arguments and initializing `generator::State`.
  Initializer initializer;

  /// Reporter is a function that reports the given `generator::Report` and exits the program.
  Reporter reporter;

  /// Names of the flag type (`--flag`) command line arguments required by the generator.
  std::vector<std::string> required_flag_args;

  /// Names of the variable type (`--var=value`) command line arguments required by the generator.
  std::vector<std::string> required_var_args;

  /// Functions to parse flag type command line arguments.
  std::vector<FlagParser> flag_parsers;

  /// Functions to parse variable type command line arguments.
  std::vector<VarParser> var_parsers;

  /**
   * Constructs a new `State` object with the given initializer function.
   *
   * @param initializer The initializer function.
   */
  explicit State(Initializer initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Quits the generator with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(const Report& report) -> void;

  /**
   * Quits the generator with the `report::Status::OK` status.
   */
  [[noreturn]] auto quit_ok() -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};
};

/**
 * Initialize state according to default behavior.
 *
 * @param state The state object to be initialized.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 */
auto default_initializer(State& state, int argc, char** argv) -> void;

/**
 * Report the given report in JSON format.
 *
 * @param report The report to be reported.
 */
auto json_reporter(const Report& report) -> void;

/**
 * Report the given report in plain text format for human reading.
 *
 * @param report The report to be reported.
 */
auto plain_text_reporter(const Report& report) -> void;

/**
 * Report the given report in colored text format for human reading.
 *
 * @param report The report to be reported.
 */
auto colored_text_reporter(const Report& report) -> void;

/**
 * Macro to register generator with custom initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 * @param args_struct_name_ The name of the command line arguments struct.
 * @param args_struct_impl_ The implementation of the command line arguments struct.
 */
#define CPLIB_REGISTER_GENERATOR_OPT(state_var_name_, initializer_, args_struct_name_,     \
                                     args_struct_impl_)                                    \
  ::cplib::generator::State state_var_name_(initializer_);                                 \
  namespace args_detail_ {                                                                 \
  struct Flag {                                                                            \
    std::string name;                                                                      \
    bool value{false};                                                                     \
    Flag(std::string name_) : name(name_) {                                                \
      state_var_name_.required_flag_args.emplace_back(name);                               \
      state_var_name_.flag_parsers.emplace_back(                                           \
          [&](const std::set<std::string>& flag_args) { value = flag_args.count(name); }); \
    }                                                                                      \
  };                                                                                       \
                                                                                           \
  template <class T>                                                                       \
  struct Var {                                                                             \
    T var;                                                                                 \
    typename T::Target value;                                                              \
    Var(T var_) : var(std::move(var_)), value(typename T::Target()) {                      \
      state_var_name_.required_var_args.emplace_back(var.name());                          \
      state_var_name_.var_parsers.emplace_back(                                            \
          [this](const std::map<std::string, std::string>& var_args) {                     \
            value = var.parse(var_args.at(std::string(var.name())));                       \
          });                                                                              \
    }                                                                                      \
  };                                                                                       \
                                                                                           \
  struct BaseArgs_ args_struct_impl_;                                                      \
                                                                                           \
  struct Args_ : BaseArgs_ {                                                               \
   public:                                                                                 \
    static Args_ instance;                                                                 \
                                                                                           \
   private:                                                                                \
    Args_() : BaseArgs_(){};                                                               \
  };                                                                                       \
  Args_ Args_::instance;                                                                   \
  }                                                                                        \
                                                                                           \
  using args_struct_name_ = args_detail_::Args_;                                           \
                                                                                           \
  auto main(signed argc, char** argv)->signed {                                            \
    state_var_name_.initializer(state_var_name_, argc, argv);                              \
    auto generator_main(const args_struct_name_& args)->void;                              \
    generator_main(args_detail_::Args_::instance);                                         \
    return 0;                                                                              \
  }

/**
 * Macro to register generator with default initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param args_struct_name_ The name of the command line arguments struct.
 * @param args_struct_impl_ The implementation of the command line arguments struct.
 */
#define CPLIB_REGISTER_GENERATOR(var, args_struct_name, args_struct_impl)                      \
  CPLIB_REGISTER_GENERATOR_OPT(var, ::cplib::generator::default_initializer, args_struct_name, \
                               args_struct_impl)
}  // namespace cplib::generator

#include "generator.i.hpp"  // IWYU pragma: export

#endif
