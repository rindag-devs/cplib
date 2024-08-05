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

#ifndef CPLIB_INTERACTOR_HPP_
#define CPLIB_INTERACTOR_HPP_

#include <memory>
#include <optional>
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>
#include <vector>

/* cplib_embed_ignore start */
#include "random.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

namespace cplib::interactor {
/**
 * `Report` represents a report showing when interactor exits.
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

  auto set_inf_fileno(int fileno, var::Reader::TraceLevel trace_level) -> void;
  auto set_from_user_fileno(int fileno, var::Reader::TraceLevel trace_level) -> void;
  auto set_to_user_fileno(int fileno) -> void;
  auto set_inf_path(std::string_view path, var::Reader::TraceLevel trace_level) -> void;
  auto set_from_user_path(std::string_view path, var::Reader::TraceLevel trace_level) -> void;
  auto set_to_user_path(std::string_view path) -> void;

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

  auto attach_trace_stack(const var::Reader::TraceStack& trace_stack) -> void;

 protected:
  std::optional<var::Reader::TraceStack> trace_stack_{};
};

/**
 * Represents the state of the validator.
 */
struct State {
 public:
  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Reader used to read user program stdout.
  var::Reader from_user;

  /// Output stream used to send information to user program stdin.
  std::ostream to_user;

  /// Stream buffer of `to_user`.
  std::unique_ptr<std::streambuf> to_user_buf;

  /// Initializer parses command-line arguments and initializes `interactor::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `interactor::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

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
   * Quits the interactor with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(const Report& report) -> void;

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
};

/**
 * Default initializer of interactor.
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

/**
 * Macro to register interactor with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_INTERACTOR_OPT(var_, initializer_)                                    \
  auto var_ =                                                                                \
      ::cplib::interactor::State(std::unique_ptr<decltype(initializer_)>(new initializer_)); \
  auto main(int argc, char** argv) -> int {                                                  \
    std::vector<std::string> args;                                                           \
    for (int i = 1; i < argc; ++i) {                                                         \
      args.emplace_back(argv[i]);                                                            \
    }                                                                                        \
    var_.initializer->init(argv[0], args);                                                   \
    auto interactor_main(void) -> void;                                                      \
    interactor_main();                                                                       \
    return 0;                                                                                \
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
