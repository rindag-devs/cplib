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

#ifndef CPLIB_EVALUATE_HPP_
#define CPLIB_EVALUATE_HPP_

#include <compare>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

/* cplib_embed_ignore start */
#include "json.hpp"
#include "trace.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::evaluate {
/// Represents the outcome of a test case or a set of test cases.
struct Result {
  /// Defines the status of a solution.
  struct Status {
   public:
    /// Enum values for different solution statuses.
    /// The order of these values is important for comparison:
    /// WRONG_ANSWER < PARTIALLY_CORRECT < ACCEPTED.
    enum Value : std::uint8_t {
      /// Indicates the solution is incorrect.
      WRONG_ANSWER,
      /// Indicates the solution is partially correct.
      PARTIALLY_CORRECT,
      /// Indicates the solution is accepted.
      ACCEPTED,
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

  Status status;
  double score;
  std::string message;

  /// Creates a Result representing zero score and ACCEPTED status, for additive accumulation.
  /// @return A Result object with ACCEPTED status and 0.0 score.
  static auto zero() -> Result;

  /// Creates an ACCEPTED Result with a full score (1.0).
  /// @return A Result object with ACCEPTED status and 1.0 score.
  static auto ac() -> Result;

  /// Creates an ACCEPTED Result with a full score (1.0) and a message.
  /// @param message An optional message for the result.
  /// @return A Result object with ACCEPTED status, 1.0 score, and the given message.
  static auto ac(std::string message) -> Result;

  /// Creates a WRONG_ANSWER Result with zero score (0.0) and a message.
  /// @param message A message explaining the wrong answer.
  /// @return A Result object with WRONG_ANSWER status, 0.0 score, and the given message.
  static auto wa(std::string message) -> Result;

  /// Creates a PARTIALLY_CORRECT Result with a specified score and message.
  /// @param score The partial score (will be clamped between 0.0 and 1.0).
  /// @param message A message explaining the partial correctness.
  /// @return A Result object with PARTIALLY_CORRECT status, clamped score, and the given message.
  static auto pc(double score, std::string message) -> Result;

  /// Creates a Result with a specified status, score and message.
  Result(Status status, double score, std::string message);

  /// C++ 20 three-way comparison operator.
  /// Compares Results primarily by status (lower enum value is "worse"),
  /// then by score (lower score is "worse"). Message is not part of comparison logic.
  /// @param other The Result to compare with.
  /// @return A std::strong_ordering indicating the relationship.
  constexpr auto operator<=>(const Result& other) const -> std::strong_ordering;

  /// Scales the score of the Result by a given factor.
  /// @param scale The scaling factor.
  /// @return A new Result object with the scaled score. Status and messages remain unchanged.
  [[nodiscard]] auto operator*(double scale) const -> Result;

  /// Scales the score of this Result by a given factor in-place. The message remains unchanged.
  /// @param scale The scaling factor.
  /// @return A reference to this Result object.
  auto operator*=(double scale) -> Result&;

  /// Combines two Results by adding their scores. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// @param other The Result to add.
  /// @return A new Result object representing the sum.
  [[nodiscard]] auto operator+(const Result& other) const -> Result;

  /// Combines this Result with another by adding scores in-place. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// @param other The Result to add.
  /// @return A reference to this Result object.
  auto operator+=(const Result& other) -> Result&;

  /// Combines two Results by taking the minimum of their scores. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// This is typically used for "all or nothing" scenarios or combining results where
  /// the lowest score dictates the overall outcome.
  /// @param other The Result to combine with (min operation).
  /// @return A new Result object representing the minimum.
  [[nodiscard]] auto operator&(const Result& other) const -> Result;

  /// Combines this Result with another by taking the minimum of their scores in-place. The message
  /// remains unchanged. The resulting status is the "worst" (minimum enum value) of the two
  /// statuses.
  /// @param other The Result to combine with (min operation).
  /// @return A reference to this Result object.
  auto operator&=(const Result& other) -> Result&;

  /**
   * Convert to json value.
   */
  [[nodiscard]] auto to_json() const -> json::Map;

 private:
  Result() = default;
};

#ifndef CPLIB_EVALUATOR_TRACE_LEVEL_MAX
#define CPLIB_EVALUATOR_TRACE_LEVEL_MAX static_cast<int>(::cplib::trace::Level::FULL)
#endif

struct Evaluator;

/**
 * Concept for types that can be evaluated for problem solutions.
 *
 * A type `T` satisfies `Evaluatable` if it provides a static member function named `evaluate` that
 * takes two arguments of type `const T&` (representing the participant's answer and the jury's
 * answer/correct output) and additional arguments and returns a `cplib::Result`.
 */
template <typename T, typename... Args>
concept Evaluatable = requires(Evaluator& ev, const T& pans, const T& jans, Args&&... args) {
  { T::evaluate(ev, pans, jans, std::forward<Args>(args)...) } -> std::same_as<Result>;
};

struct EvaluatorTrace {
  std::string var_name;
  std::optional<Result> result;

  explicit EvaluatorTrace(std::string var_name);

  /// Get name of node.
  [[nodiscard]] auto node_name() const -> std::string;

  /// Convert trace to plain text.
  [[nodiscard]] auto to_plain_text() const -> std::string;

  /// Convert trace to colored text.
  [[nodiscard]] auto to_colored_text() const -> std::string;

  /// Convert incomplete trace to JSON map.
  [[nodiscard]] auto to_stack_json() const -> json::Value;

  /// Convert complete trace to JSON map.
  [[nodiscard]] auto to_tree_json() const -> json::Value;
};

struct Evaluator : trace::Traced<EvaluatorTrace> {
  using FailFunc = UniqueFunction<auto(const Evaluator&, std::string_view)->void>;
  using EvaluationHook = UniqueFunction<auto(const Evaluator&, const Result&)->void>;

  /**
   * Create an evaluator.
   */
  explicit Evaluator(trace::Level trace_level, FailFunc fail_func, EvaluationHook evaluation_hook);

  /// Copy constructor (deleted to prevent copying).
  Evaluator(const Evaluator&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Evaluator&) -> Evaluator& = delete;

  /// Move constructor.
  Evaluator(Evaluator&&) noexcept = default;

  /// Move assignment operator.
  auto operator=(Evaluator&&) noexcept -> Evaluator& = default;

  /**
   * Call fail func with a message.
   *
   * @param message The error message.
   */
  [[noreturn]] auto fail(std::string_view message) -> void;

  /**
   * Evaluate answers from participant and jury with additional arguments.
   *
   * @param var_name Variable name.
   * @param pans Participant's answer.
   * @param jans Jury's answer.
   * @param args Additional arguments.
   * @return Result of the evaluation.
   */
  template <typename T, class... Args>
  auto operator()(std::string_view var_name, const T& pans, const T& jans, Args... args) -> Result
    requires Evaluatable<T, Args...>;

  template <std::equality_comparable T>
  auto eq(std::string_view var_name, const T& pans, const T& jans) -> Result;

  template <std::floating_point T>
  auto approx(std::string_view var_name, const T& pans, const T& jans, const T& max_err) -> Result;

  template <class T>
  auto approx_abs(std::string_view var_name, const T& pans, const T& jans, const T& abs_err)
      -> Result
    requires std::is_arithmetic_v<T>;

 private:
  FailFunc fail_func_;
  EvaluationHook evaluation_hook_;

  auto pre_evaluate(std::string_view var_name) -> void;
  auto post_evaluate(const Result& result) -> void;
};

}  // namespace cplib::evaluate

#include "evaluate.i.hpp"  // IWYU pragma: export

#endif
