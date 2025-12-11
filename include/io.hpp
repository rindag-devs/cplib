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

#ifndef CPLIB_IO_HPP_
#define CPLIB_IO_HPP_

#include <array>
#include <cstddef>
#include <cstdio>
#include <ios>
#include <memory>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>

/* cplib_embed_ignore start */
#include "json.hpp"
/* cplib_embed_ignore end */

namespace cplib::io {

/// Represents a position in a file.
struct Position {
  /// Line number, starting from 0.
  std::size_t line;

  /// Column number, starting from 0.
  std::size_t col;

  /// Byte number, starting from 0.
  std::size_t byte;

  explicit Position();
  explicit Position(std::size_t line, std::size_t col, std::size_t byte);

  [[nodiscard]] auto to_json() const -> json::Map;
};

/// Buffer for input stream.
struct InBuf : std::streambuf {
 public:
  static constexpr int PB_SIZE = 1024;    // Size of putback area
  static constexpr int BUF_SIZE = 65536;  // Size of the data buffer

  /**
   * Constructor
   * - Initialize file descriptor
   * - Initialize empty data buffer
   * - No putback area
   * => Force underflow()
   */
  explicit InBuf(int fd);

  explicit InBuf(std::string_view path);

  ~InBuf() override;

 protected:
  // Insert new characters into the buffer
  auto underflow() -> int_type override;

  int fd_;
  bool need_close_;
  std::array<char, BUF_SIZE + PB_SIZE> buf_;  // Data buffer
};

/**
 * An input stream struct that provides various functionalities for reading and manipulating
 * streams.
 */
struct InStream {
 public:
  /**
   * Constructs an InStream object.
   *
   * @param buf A unique pointer to a stream buffer.
   * @param name The name of the stream.
   * @param strict Indicates if the stream is in strict mode.
   * @param fail_func A function that will be called when a failure occurs.
   */
  explicit InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict);

  /**
   * Returns the name of the stream.
   *
   * @return The name of the stream as a string view.
   */
  [[nodiscard]] auto name() const -> std::string_view;

  /**
   * Moves the stream pointer to the first non-blank character or EOF.
   */
  auto skip_blanks() -> void;

  /**
   * Returns the current character in the stream, or EOF if reached, without removing it from the
   * stream.
   *
   * @return The current character in the stream as an integer, or EOF if reached.
   */
  auto seek() -> int;

  /**
   * Returns the current character and moves the pointer one character forward, or EOF if reached.
   *
   * @return The current character in the stream as an integer, or EOF if reached.
   */
  auto read() -> int;

  /**
   * Reads at most n characters from the stream.
   *
   * @param n The maximum number of characters to read.
   * @return The read characters as a string.
   */
  auto read_n(std::size_t n) -> std::string;

  /**
   * Checks if the stream is in strict mode.
   *
   * @return True if the stream is in strict mode, false otherwise.
   */
  [[nodiscard]] auto is_strict() const -> bool;

  /**
   * Sets the strict mode of the stream.
   *
   * @param b The value to set strict mode to.
   */
  auto set_strict(bool b) -> void;

  /**
   * Returns the current position.
   *
   * @return The current line position.
   */
  [[nodiscard]] auto pos() const -> Position;

  /**
   * Checks if the current position is EOF.
   *
   * @return True if the current position is EOF, false otherwise.
   */
  auto eof() -> bool;

  /**
   * Moves the pointer to the first non-whitespace character and calls [`eof()`].
   *
   * @return True if the pointer is at EOF, false otherwise.
   */
  auto seek_eof() -> bool;

  /**
   * Checks if the current position contains '\n'.
   *
   * @return True if the current position contains '\n', false otherwise.
   */
  auto eoln() -> bool;

  /**
   * Moves the pointer to the first non-whitespace character and calls [`eoln()`].
   *
   * @return True if the pointer is at '\n', false otherwise.
   */
  auto seek_eoln() -> bool;

  /**
   * Moves the stream pointer to the first character of the next line (if it exists).
   */
  auto next_line() -> void;

  /**
   * Reads a new token from the stream.
   * Skip the previous blanks in non-strict mode.
   *
   * @return The read token as a string.
   */
  auto read_token() -> std::string;

  /**
   * Reads a new "word" from the stream.
   * Skip the previous blanks in non-strict mode.
   *
   * A "word" is defined as a string consisting only of characters from [A-Za-z0-9+\-_\.].
   *
   * @return The read word as a string.
   */
  auto read_word() -> std::string;

  /**
   * If the current position contains EOF, do nothing and return `std::nullopt`.
   *
   * Otherwise, reads a line from the current position to EOLN or EOF. Moves the stream pointer to
   * the first character of the new line (if possible).
   *
   * @return An optional string containing the line read, or `std::nullopt` if the current position
   *         contains EOF.
   */
  auto read_line() -> std::optional<std::string>;

 private:
  std::unique_ptr<std::streambuf> buf_;
  std::string name_;
  bool strict_;  // In strict mode, whitespace characters are not ignored
  Position pos_{};
};

/// Output stream buffer.
struct OutBuf : std::streambuf {
 public:
  explicit OutBuf(int fd);

  explicit OutBuf(std::string_view path);

  ~OutBuf() override;

 protected:
  /// Write one character
  auto overflow(int_type c) -> int_type override;
  /// Write multiple characters
  auto xsputn(const char* s, std::streamsize num) -> std::streamsize override;

  int fd_;  // File descriptor
  bool need_close_;
};
}  // namespace cplib::io

#include "io.i.hpp"  // IWYU pragma: export

#endif
