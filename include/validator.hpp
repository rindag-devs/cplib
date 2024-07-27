/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_VALIDATOR_HPP_
#define CPLIB_VALIDATOR_HPP_

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/* cplib_embed_ignore start */
#include "random.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

namespace cplib::validator {
/**
 * `Report` represents a report showing when validator exits.
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
      /// Indicates the input file is valid.
      VALID,
      /// Indicates the input file is invalid.
      INVALID,
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

struct Trait {
  /// Type of function which checks the trait is satisfied.
  using CheckFunc = std::function<auto()->bool>;

  /// The name of the trait.
  std::string name;

  /// The function used to check if the trait is satisfied.
  CheckFunc check_func;

  /// The list of names of traits that this trait depends on.
  std::vector<std::string> dependencies;

  /**
   * Constructor that takes the name and check function.
   *
   * @param name The name of the trait.
   * @param check_func The function used to check if the trait is satisfied.
   */
  Trait(std::string name, CheckFunc check_func);

  /**
   * Constructor that takes the name, check function, and dependencies.
   *
   * @param name The name of the trait.
   * @param check_func The function used to check if the trait is satisfied.
   * @param dependencies The list of names of traits that this trait depends on.
   */
  Trait(std::string name, CheckFunc check_func, std::vector<std::string> dependencies);
};

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State& state) -> void;

  virtual auto init(std::string_view argv0, const std::vector<std::string>& args) -> void = 0;

 protected:
  State* state_;

  auto set_inf_fileno(int fileno) -> void;
  auto set_inf_path(std::string_view path) -> void;
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[noreturn]] virtual auto report(const Report& report) -> void = 0;

  auto attach_trace_stack(const cplib::var::Reader::TraceStack& trace_stack) -> void;

  auto attach_trait_status(const std::map<std::string, bool>& trait_status) -> void;

 protected:
  std::optional<cplib::var::Reader::TraceStack> trace_stack_;
  std::map<std::string, bool> trait_status_;
};

struct State {
 public:
  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Initializer parses command-line arguments and initializes `validator::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `validator::Report` and exits the program.
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
   * Sets the traits of the validator.
   *
   * **WARNING**: Calling this function multiple times will overwrite the last trait list.
   *
   * @param traits The list of traits.
   */
  auto traits(std::vector<Trait> traits) -> void;

  /**
   * Quits the validator with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(Report report) -> void;

  /**
   * Quits the validator with the `report::Status::VALID` status.
   */
  [[noreturn]] auto quit_valid() -> void;

  /**
   * Quits the validator with the `report::Status::INVALID` status and given message.
   *
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_invalid(std::string_view message) -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};

  /// The list of traits of the validator.
  std::vector<Trait> traits_;

  /// The edge set of a directed graph established based on the dependencies between traits.
  std::vector<std::vector<std::size_t>> trait_edges_;
};

/**
 * Default initializer of validator.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param argv0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view argv0, const std::vector<std::string>& args) -> void override;
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
 * Macro to register validator with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_VALIDATOR_OPT(var_, initializer_)                                    \
  auto var_ =                                                                               \
      ::cplib::validator::State(std::unique_ptr<decltype(initializer_)>(new initializer_)); \
  auto main(int argc, char** argv) -> int {                                                 \
    std::vector<std::string> args;                                                          \
    for (int i = 1; i < argc; ++i) {                                                        \
      args.emplace_back(argv[i]);                                                           \
    }                                                                                       \
    var_.initializer->init(argv[0], args);                                                  \
    auto validator_main(void) -> void;                                                      \
    validator_main();                                                                       \
    return 0;                                                                               \
  }

/**
 * Macro to register validator with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_VALIDATOR(var) \
  CPLIB_REGISTER_VALIDATOR_OPT(var, ::cplib::validator::DefaultInitializer())
}  // namespace cplib::validator

#include "validator.i.hpp"  // IWYU pragma: export

#endif
