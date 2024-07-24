/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_INTERACTOR_HPP_
#define CPLIB_INTERACTOR_HPP_

#include <memory>       // for unique_ptr
#include <ostream>      // for basic_ostream, ostream, streambuf
#include <string>       // for basic_string, string
#include <string_view>  // for string_view

/* cplib_embed_ignore start */
#include "random.hpp"  // for Random
#include "utils.hpp"   // for UniqueFunction, UniqueFunction<>::UniqueFunct...
#include "var.hpp"     // for Reader
/* cplib_embed_ignore end */

namespace cplib::interactor {
/**
 * `Report` represents a report showing when interactor exits.
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
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[noreturn]] virtual auto report(const Report& report) -> void = 0;

  auto attach_trace_stack(const cplib::var::Reader::TraceStack& trace_stack) -> void;

 protected:
  std::optional<cplib::var::Reader::TraceStack> trace_stack_;
};

/**
 * Represents the state of the validator.
 */
class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = UniqueFunction<auto(State& state, int argc, char** argv)->void>;

  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Reader used to read user program stdout.
  var::Reader from_user;

  /// Output stream used to send information to user program stdin.
  std::ostream to_user;

  /// Initializer is a function parsing command line arguments and initializing `interactor::State`
  Initializer initializer;

  /// Reporter is a function that reports the given `interactor::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

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
   * Disables the check for redundant content of `from_user` stream.
   */
  auto disable_check_dirt() -> void;

  /**
   * Quits the interactor with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(Report report) -> void;

  /**
   * Quits the interactor with the `report::Status::ACCEPTED` status.
   */
  [[noreturn]] auto quit_ac() -> void;

  /**
   * Quits the interactor with the `report::Status::WRONG_ANSWER` status and the given message.
   *
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_wa(std::string_view message) -> void;

  /**
   * Quits the interactor with the `report::Status::PARTIALLY_CORRECT` status, the given points, and
   * message.
   *
   * @param points The points to be awarded.
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_pc(double points, std::string_view message) -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};

  /// Whether to check for redundant content in the `from_user` stream.
  bool check_dirt_{true};
};

/**
 * Default initializer of interactor.
 */
struct DefaultInitializer {
  std::unique_ptr<std::streambuf> to_user_buf;

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
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[noreturn]] auto report(const Report& report) -> void override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[noreturn]] auto report(const Report& report) -> void override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[noreturn]] auto report(const Report& report) -> void override;
};

/**
 * Macro to register interactor with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_INTERACTOR_OPT(var_, initializer_) \
  auto var_ = ::cplib::interactor::State(initializer_);   \
  auto main(signed argc, char** argv) -> signed {         \
    var_.initializer(var_, argc, argv);                   \
    auto interactor_main(void) -> void;                   \
    interactor_main();                                    \
    return 0;                                             \
  }

/**
 * Macro to register interactor with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_INTERACTOR(var) \
  CPLIB_REGISTER_INTERACTOR_OPT(var, ::cplib::interactor::DefaultInitializer())
}  // namespace cplib::interactor

#include "interactor.i.hpp"  // IWYU pragma: export

#endif
