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

#ifndef CPLIB_CHECKER_HPP_
#define CPLIB_CHECKER_HPP_

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/* cplib_embed_ignore start */
#include "evaluate.hpp"
#include "random.hpp"
#include "trace.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

namespace cplib::checker {
/**
 * `Report` represents a report showing when checker exits.
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
     * Constructor for Status from cplib::evaluate::Result::Status.
     *
     * @param status The evaluate result status.
     */
    constexpr explicit Status(evaluate::Result::Status status);

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
  auto set_inf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_ouf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_ans_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_inf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_ouf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_ans_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_evaluator(trace::Level trace_level) -> void;

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

  auto attach_reader_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;
  auto attach_evaluator_trace_stack(trace::TraceStack<evaluate::EvaluatorTrace> trace_stack)
      -> void;
  [[nodiscard]] auto get_evaluator_trace_stacks() const
      -> const std::vector<trace::TraceStack<evaluate::EvaluatorTrace>>&;

 protected:
  std::vector<trace::TraceStack<var::ReaderTrace>> reader_trace_stacks_{};
  std::vector<trace::TraceStack<evaluate::EvaluatorTrace>> evaluator_trace_stacks_{};
};

/**
 * Represents the state of the checker.
 */
struct State {
 public:
  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Output file reader.
  var::Reader ouf;

  /// Answer file reader.
  var::Reader ans;

  /// Evaluator
  evaluate::Evaluator evaluator;

  /// Initializer parses command-line arguments and initializes `checker::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `checker::Report` and exits the program.
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
 * Macro to register checker with custom initializer.
 *
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_CHECKER_OPT(input_struct_, output_struct_, initializer_)                    \
  static_assert(::cplib::var::Readable<input_struct_>, "`" #input_struct_ "` should be Readable"); \
  static_assert(::cplib::var::Readable<output_struct_, const input_struct_&>,                      \
                "`" #output_struct_ "` should be Readable");                                       \
  static_assert(::cplib::evaluate::Evaluatable<output_struct_, const input_struct_&>,              \
                "`" #output_struct_ "` should be Evaluatable");                                    \
  auto main(int argc, char** argv) -> int {                                                        \
    ::std::vector<::std::string> args;                                                             \
    args.reserve(argc);                                                                            \
    for (int i = 1; i < argc; ++i) {                                                               \
      args.emplace_back(argv[i]);                                                                  \
    }                                                                                              \
    /* std::exit only destroys static objects */                                                   \
    static auto state =                                                                            \
        ::cplib::checker::State(::std::unique_ptr<decltype(initializer_)>(new initializer_));      \
    state.initializer->init(argv[0], args);                                                        \
    input_struct_ input{state.inf.read(::cplib::var::ExtVar<input_struct_>("input"))};             \
    output_struct_ output{state.ouf.read(::cplib::var::ExtVar<output_struct_>("output", input))};  \
    output_struct_ answer{state.ans.read(::cplib::var::ExtVar<output_struct_>("answer", input))};  \
    ::cplib::evaluate::Result result = state.evaluator("output", output, answer, input);           \
    ::std::string report_message;                                                                  \
    auto evaluator_trace_stacks = state.reporter->get_evaluator_trace_stacks();                    \
    auto it = ::std::ranges::find_if(evaluator_trace_stacks, [](const auto& x) {                   \
      return !x.stack.empty() && x.stack.back().result.has_value() &&                              \
             !x.stack.back().result->message.empty();                                              \
    });                                                                                            \
    if (it != evaluator_trace_stacks.end()) {                                                      \
      report_message = it->stack.back().result->message;                                           \
    }                                                                                              \
    ::cplib::checker::Report report{::cplib::checker::Report::Status(result.status), result.score, \
                                    report_message};                                               \
    state.quit(report);                                                                            \
    return 0;                                                                                      \
  }

#ifndef CPLIB_CHECKER_DEFAULT_INITIALIZER
#define CPLIB_CHECKER_DEFAULT_INITIALIZER ::cplib::checker::DefaultInitializer()
#endif

/**
 * Macro to register checker with default initializer.
 */
#define CPLIB_REGISTER_CHECKER(input_struct_, output_struct_) \
  CPLIB_REGISTER_CHECKER_OPT(input_struct_, output_struct_, CPLIB_CHECKER_DEFAULT_INITIALIZER)
}  // namespace cplib::checker

#include "checker.i.hpp"  // IWYU pragma: export

#endif
