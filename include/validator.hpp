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

#ifndef CPLIB_VALIDATOR_HPP_
#define CPLIB_VALIDATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "random.hpp"
#include "trace.hpp"
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
    enum Value : std::uint8_t {
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

  /// The map of traits that this trait depends on.
  ///
  /// `dependencies[name] = value` means that the current trait will only be evaluated when `name`
  /// is evaluated and `name == value`.
  std::map<std::string, bool> dependencies;

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
   * @param dependencies The map of traits that this trait depends on.
   */
  Trait(std::string name, CheckFunc check_func, std::map<std::string, bool> dependencies);
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

  auto set_inf_fileno(int fileno, trace::Level level) -> void;
  auto set_inf_path(std::string_view path, trace::Level level) -> void;

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

  auto attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;

  auto attach_trace_tree(const trace::TraceTreeNode<var::ReaderTrace>* root) -> void;

  auto attach_trait_status(const std::map<std::string, bool>& trait_status) -> void;

 protected:
  std::vector<trace::TraceStack<var::ReaderTrace>> trace_stacks_{};
  const trace::TraceTreeNode<var::ReaderTrace>* trace_tree_{};
  std::map<std::string, bool> trait_status_{};
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
   * @warning Calling this function multiple times will overwrite the last trait list.
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
  std::vector<std::vector<std::pair<std::size_t, bool>>> trait_edges_;
};

/**
 * Default initializer of validator.
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
 * Macro to register validator with custom initializer.
 *
 * @param initializer_ The initializer function.
 * @param traits_func_ The function returns a list of traits.
 */
#define CPLIB_REGISTER_VALIDATOR_OPT(input_struct_, traits_func_, initializer_)                    \
  static_assert(::cplib::var::Readable<input_struct_>, "`" #input_struct_ "` should be Readable"); \
  auto main(int argc, char** argv) -> int {                                                        \
    ::std::vector<::std::string> args;                                                             \
    args.reserve(argc);                                                                            \
    for (int i = 1; i < argc; ++i) {                                                               \
      args.emplace_back(argv[i]);                                                                  \
    }                                                                                              \
    auto state =                                                                                   \
        ::cplib::validator::State(std::unique_ptr<decltype(initializer_)>(new initializer_));      \
    state.initializer->init(argv[0], args);                                                        \
    input_struct_ input{state.inf.read(::cplib::var::ExtVar<input_struct_>("input"))};             \
    std::function<auto(const input_struct_&)->::std::vector<::cplib::validator::Trait>>            \
        traits_func = traits_func_;                                                                \
    state.traits(traits_func(input));                                                              \
    state.quit_valid();                                                                            \
    return 0;                                                                                      \
  }

#ifndef CPLIB_VALIDATOR_DEFAULT_INITIALIZER
#define CPLIB_VALIDATOR_DEFAULT_INITIALIZER ::cplib::validator::DefaultInitializer()
#endif

/**
 * Macro to register validator with default initializer.
 */
#define CPLIB_REGISTER_VALIDATOR(input_struct_, traits_func_) \
  CPLIB_REGISTER_VALIDATOR_OPT(input_struct_, traits_func_, CPLIB_VALIDATOR_DEFAULT_INITIALIZER)
}  // namespace cplib::validator

#include "validator.i.hpp"  // IWYU pragma: export

#endif
