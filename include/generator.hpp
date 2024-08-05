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

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State& state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string>& args) -> void = 0;

 protected:
  auto state() -> State&;

 private:
  State* state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report& report) -> int = 0;
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
 * Default initializer of generator.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string>& args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

#define CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_)                                 \
  namespace cplib_generator_args_detail_ {                                                       \
  struct AsResultTag_ {};                                                                        \
                                                                                                 \
  std::map<std::string, std::any> value_map_;                                                    \
                                                                                                 \
  struct Flag {                                                                                  \
    using ResultType = bool;                                                                     \
    std::string name;                                                                            \
    explicit Flag(std::string name_) : name(std::move(name_)) {                                  \
      state_var_name_.required_flag_args.emplace_back(name);                                     \
      auto name = this->name;                                                                    \
      state_var_name_.flag_parsers.emplace_back([name](const std::set<std::string>& flag_args) { \
        value_map_[name] = static_cast<ResultType>(flag_args.count(name));                       \
      });                                                                                        \
    }                                                                                            \
    inline auto operator|(AsResultTag_) const -> const ResultType& {                             \
      return *std::any_cast<ResultType>(&(value_map_[name] = ResultType{}));                     \
    }                                                                                            \
  };                                                                                             \
                                                                                                 \
  template <class T>                                                                             \
  struct Var {                                                                                   \
    using ResultType = typename T::Target;                                                       \
    T var;                                                                                       \
    template <class... Args>                                                                     \
    explicit Var(Args... args) : var(std::forward<Args>(args)...) {                              \
      state_var_name_.required_var_args.emplace_back(var.name());                                \
      auto var = this->var;                                                                      \
      state_var_name_.var_parsers.emplace_back(                                                  \
          [var](const std::map<std::string, std::string>& var_args) {                            \
            auto name = std::string(var.name());                                                 \
            value_map_[name] = var.parse(var_args.at(name));                                     \
          });                                                                                    \
    }                                                                                            \
    inline auto operator|(AsResultTag_) const -> const ResultType& {                             \
      return *std::any_cast<ResultType>(&(value_map_[std::string(var.name())] = ResultType{}));  \
    }                                                                                            \
  };                                                                                             \
  }                                                                                              \
  namespace cplib_generator_args_ {                                                              \
  using ::cplib_generator_args_detail_::Flag;                                                    \
  using ::cplib_generator_args_detail_::Var;                                                     \
  }

#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(x) x

#define CPLIB_REGISTER_GENERATOR_ARGS_0_()

#define CPLIB_REGISTER_GENERATOR_ARGS_1_(arg)                       \
  namespace cplib_generator_args_ {                                 \
  const auto& arg | ::cplib_generator_args_detail_::AsResultTag_{}; \
  }

#define CPLIB_REGISTER_GENERATOR_ARGS_2_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_1_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_3_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_2_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_4_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_3_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_5_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_4_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_6_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_5_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_7_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_6_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_8_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_7_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_9_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_8_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_10_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_9_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_11_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_10_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_12_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_11_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_13_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_12_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_14_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_13_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_15_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_14_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_16_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_15_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_17_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_16_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_18_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_17_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_19_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_18_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_20_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_19_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_21_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_20_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_22_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_21_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_23_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_22_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_24_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_23_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_25_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_24_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_26_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_25_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_27_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_26_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_28_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_27_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_29_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_28_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_30_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_29_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_31_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_30_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_32_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_31_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_33_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_32_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_34_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_33_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_35_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_34_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_36_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_35_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_37_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_36_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_38_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_37_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_39_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_38_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_40_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_39_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_41_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_40_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_42_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_41_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_43_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_42_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_44_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_43_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_45_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_44_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_46_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_45_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_47_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_46_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_48_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_47_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_49_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_48_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_50_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_49_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_51_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_50_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_52_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_51_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_53_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_52_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_54_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_53_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_55_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_54_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_56_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_55_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_57_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_56_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_58_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_57_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_59_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_58_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_60_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_59_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_61_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_60_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_62_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_61_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_63_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_62_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_GET_NTH_ARG_(                                                \
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, \
    _59, _60, _61, _62, _63, N, ...)                                                               \
  N

#define CPLIB_REGISTER_GENERATOR_ARGS_(...)                                                       \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_GET_NTH_ARG_(               \
      dummy, ##__VA_ARGS__, CPLIB_REGISTER_GENERATOR_ARGS_63_, CPLIB_REGISTER_GENERATOR_ARGS_62_, \
      CPLIB_REGISTER_GENERATOR_ARGS_61_, CPLIB_REGISTER_GENERATOR_ARGS_60_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_59_, CPLIB_REGISTER_GENERATOR_ARGS_58_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_57_, CPLIB_REGISTER_GENERATOR_ARGS_56_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_55_, CPLIB_REGISTER_GENERATOR_ARGS_54_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_53_, CPLIB_REGISTER_GENERATOR_ARGS_52_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_51_, CPLIB_REGISTER_GENERATOR_ARGS_50_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_49_, CPLIB_REGISTER_GENERATOR_ARGS_48_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_47_, CPLIB_REGISTER_GENERATOR_ARGS_46_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_45_, CPLIB_REGISTER_GENERATOR_ARGS_44_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_43_, CPLIB_REGISTER_GENERATOR_ARGS_42_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_41_, CPLIB_REGISTER_GENERATOR_ARGS_40_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_39_, CPLIB_REGISTER_GENERATOR_ARGS_38_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_37_, CPLIB_REGISTER_GENERATOR_ARGS_36_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_35_, CPLIB_REGISTER_GENERATOR_ARGS_34_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_33_, CPLIB_REGISTER_GENERATOR_ARGS_32_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_31_, CPLIB_REGISTER_GENERATOR_ARGS_30_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_29_, CPLIB_REGISTER_GENERATOR_ARGS_28_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_27_, CPLIB_REGISTER_GENERATOR_ARGS_26_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_25_, CPLIB_REGISTER_GENERATOR_ARGS_24_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_23_, CPLIB_REGISTER_GENERATOR_ARGS_22_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_21_, CPLIB_REGISTER_GENERATOR_ARGS_20_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_19_, CPLIB_REGISTER_GENERATOR_ARGS_18_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_17_, CPLIB_REGISTER_GENERATOR_ARGS_16_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_15_, CPLIB_REGISTER_GENERATOR_ARGS_14_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_13_, CPLIB_REGISTER_GENERATOR_ARGS_12_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_11_, CPLIB_REGISTER_GENERATOR_ARGS_10_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_9_, CPLIB_REGISTER_GENERATOR_ARGS_8_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_7_, CPLIB_REGISTER_GENERATOR_ARGS_6_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_5_, CPLIB_REGISTER_GENERATOR_ARGS_4_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_3_, CPLIB_REGISTER_GENERATOR_ARGS_2_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_1_, CPLIB_REGISTER_GENERATOR_ARGS_0_)(__VA_ARGS__))

/**
 * Macro to register generator with custom initializer.
 *
 * @param state_var_name_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR_OPT(state_var_name_, initializer_, args_namespace_name_, ...) \
  auto state_var_name_ =                                                                       \
      ::cplib::generator::State(std::unique_ptr<decltype(initializer_)>(new initializer_));    \
  CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_);                                    \
  CPLIB_REGISTER_GENERATOR_ARGS_(__VA_ARGS__);                                                 \
  namespace args_namespace_name_ = ::cplib_generator_args_;                                    \
  auto main(int argc, char** argv) -> int {                                                    \
    std::vector<std::string> args;                                                             \
    for (int i = 1; i < argc; ++i) {                                                           \
      args.emplace_back(argv[i]);                                                              \
    }                                                                                          \
    state_var_name_.initializer->init(argv[0], args);                                          \
    auto generator_main(void) -> void;                                                         \
    generator_main();                                                                          \
    return 0;                                                                                  \
  }

/**
 * Macro to register generator with default initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR(var_name_, args_namespace_name_, ...)              \
  CPLIB_REGISTER_GENERATOR_OPT(var_name_, ::cplib::generator::DefaultInitializer(), \
                               args_namespace_name_, __VA_ARGS__)
}  // namespace cplib::generator

#include "generator.i.hpp"  // IWYU pragma: export

#endif
