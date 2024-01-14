/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_CHECKER_HPP_
#define CPLIB_CHECKER_HPP_

#include <string>       // for basic_string, string
#include <string_view>  // for string_view

/* cplib_embed_ignore start */
#include "random.hpp"  // for Random
#include "utils.hpp"   // for UniqueFunction, UniqueFunction<>::UniqueFunct...
#include "var.hpp"     // for Reader
/* cplib_embed_ignore end */

namespace cplib::checker {
/**
 * `Report` represents a report showing when checker exits.
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
      /// Indicates the solution is accepted
      ACCEPTED,
      /// Indicates the solution is incorrect.
      WRONG_ANSWER,
      /// Indicates the solution is partially correct.
      PARTIALLY_CORRECT,
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

  /// The score of the report (full score is 1.0).
  double score;

  /// The message associated with the report.
  std::string message;

  /**
   * Constructor for Report.
   * @param status The status of the report.
   * @param score The score of the report.
   * @param message The message associated with the report.
   */
  Report(Status status, double score, std::string message);
};

/**
 * Represents the state of the checker.
 */
class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = UniqueFunction<auto(State& state, int argc, char** argv)->void>;

  /// The type of function used for reporting.
  using Reporter = UniqueFunction<auto(const Report& report)->void>;

  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Output file reader.
  var::Reader ouf;

  /// Answer file reader.
  var::Reader ans;

  /// Initializer is a function parsing command line arguments and initializing `checker::State`
  Initializer initializer;

  /// Reporter is a function that reports the given `checker::Report` and exits the program.
  Reporter reporter;

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
   * Disables the check for redundant content in the output file.
   */
  auto disable_check_dirt() -> void;

  /**
   * Quits the checker with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(Report report) -> void;

  /**
   * Quits the checker with the `report::Status::ACCEPTED` status.
   */
  [[noreturn]] auto quit_ac() -> void;

  /**
   * Quits the checker with the `report::Status::WRONG_ANSWER` status and the given message.
   *
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_wa(std::string_view message) -> void;

  /**
   * Quits the checker with the `report::Status::PARTIALLY_CORRECT` status, the given points, and
   * message.
   *
   * @param points The points to be awarded.
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_pc(double points, std::string_view message) -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};

  /// Whether to check for redundant content in the output file.
  bool check_dirt_{true};
};

/**
 * Default initializer of checker.
 */
struct DefaultInitializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param state The state object to be initialized.
   * @param argc The number of command line arguments.
   * @param argv The command line arguments.
   */
  auto operator()(State& state, int argc, char** argv) -> void;
};

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
 * Macro to register checker with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_CHECKER_OPT(var_, initializer_) \
  auto var_ = ::cplib::checker::State(initializer_);   \
  auto main(signed argc, char** argv)->signed {        \
    var_.initializer(var_, argc, argv);                \
    auto checker_main(void)->void;                     \
    checker_main();                                    \
    return 0;                                          \
  }

/**
 * Macro to register checker with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_CHECKER(var) \
  CPLIB_REGISTER_CHECKER_OPT(var, ::cplib::checker::DefaultInitializer())
}  // namespace cplib::checker

#include "checker.i.hpp"  // IWYU pragma: export

#endif
