/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

/* cplib_embed_ignore start */
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
#include "io.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_IO_HPP_
#error "Must be included from io.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <memory>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>

/* cplib_embed_ignore start */
#include "macros.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::io {
namespace detail {
// A stream buffer that writes on a file descriptor
//
// https://www.josuttis.com/cppcode/fdstream.html
struct FdOutBuf : std::streambuf {
 public:
  explicit FdOutBuf(int fd) : fd_(fd) {
    /*
      We recommend using binary mode on Windows. However, Codeforces Polygon doesn’t think so.
      Since the only Online Judge that uses Windows seems to be Codeforces, make it happy.
    */
#if defined(ON_WINDOWS) && !defined(ONLINE_JUDGE)
    _setmode(fd_, _O_BINARY);
#endif
  }

 protected:
  // Write one character
  auto overflow(int_type c) -> int_type override {
    if (c != EOF) {
      auto z = static_cast<char>(c);
      if (write(fd_, &z, 1) != 1) {
        return EOF;
      }
    }
    return c;
  }
  // Write multiple characters
  auto xsputn(const char* s, std::streamsize num) -> std::streamsize override {
    return write(fd_, s, num);
  }

  int fd_;  // File descriptor
};

// A stream buffer that reads on a file descriptor
//
// https://www.josuttis.com/cppcode/fdstream.html
struct FdInBuf : std::streambuf {
 public:
  /**
   * Constructor
   * - Initialize file descriptor
   * - Initialize empty data buffer
   * - No putback area
   * => Force underflow()
   */
  explicit FdInBuf(int fd) : fd_(fd) {
    /*
      We recommend using binary mode on Windows. However, Codeforces Polygon doesn’t think so.
      Since the only Online Judge that uses Windows seems to be Codeforces, make it happy.
    */
#if defined(ON_WINDOWS) && !defined(ONLINE_JUDGE)
    _setmode(fd_, _O_BINARY);
#endif
    setg(buf_.data() + PB_SIZE,   // Beginning of putback area
         buf_.data() + PB_SIZE,   // Read position
         buf_.data() + PB_SIZE);  // End position
  }

 protected:
  // Insert new characters into the buffer
  auto underflow() -> int_type override {
    // Is read position before end of buffer?
    if (gptr() < egptr()) {
      return traits_type::to_int_type(*gptr());
    }

    /*
     * Process size of putback area
     * - Use number of characters read
     * - But at most size of putback area
     */
    std::ptrdiff_t num_putback = gptr() - eback();
    if (num_putback > PB_SIZE) {
      num_putback = PB_SIZE;
    }

    // Copy up to PB_SIZE characters previously read into the putback area
    std::memmove(buf_.data() + (PB_SIZE - num_putback), gptr() - num_putback, num_putback);

    // Read at most bufSize new characters
    std::ptrdiff_t num = read(fd_, buf_.data() + PB_SIZE, BUF_SIZE);
    if (num <= 0) {
      // Error or EOF
      return EOF;
    }

    // Reset buffer pointers
    setg(buf_.data() + (PB_SIZE - num_putback),  // Beginning of putback area
         buf_.data() + PB_SIZE,                  // Read position
         buf_.data() + PB_SIZE + num);           // End of buffer

    // Return next character
    return traits_type::to_int_type(*gptr());
  }

  int fd_;
  /**
   * Data buffer:
   * - At most, pbSize characters in putback area plus
   * - At most, bufSize characters in ordinary read buffer
   */
  static constexpr int PB_SIZE = 4;           // Size of putback area
  static constexpr int BUF_SIZE = 65536;      // Size of the data buffer
  std::array<char, BUF_SIZE + PB_SIZE> buf_;  // Data buffer
};
}  // namespace detail

inline InStream::InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict)
    : buf_(std::move(buf)), name_(std::move(name)), strict_(strict) {}

inline auto InStream::name() const -> std::string_view { return name_; }

inline auto InStream::skip_blanks() -> void {
  while (true) {
    if (int c = seek(); c == EOF || !std::isspace(c)) break;
    read();
  }
}

inline auto InStream::seek() -> int { return buf_->sgetc(); }

inline auto InStream::read() -> int {
  int c = buf_->sbumpc();
  if (c == EOF) return EOF;
  ++byte_num_;
  if (c == '\n') {
    ++line_num_, col_num_ = 0;
  } else {
    ++col_num_;
  }
  return c;
}

inline auto InStream::read_n(std::size_t n) -> std::string {
  std::string buf;
  buf.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    if (eof()) break;
    buf.push_back(static_cast<char>(read()));
  }
  return buf;
}

inline auto InStream::is_strict() const -> bool { return strict_; }

inline auto InStream::set_strict(bool b) -> void {
  if (byte_num() > 0) {
    panic(format("Can't set strict mode of input stream `%s` when not at the beginning of the file",
                 name().data()));
  }
  strict_ = b;
}

inline auto InStream::line_num() const -> std::size_t { return line_num_; }

inline auto InStream::col_num() const -> std::size_t { return col_num_; }

inline auto InStream::byte_num() const -> std::size_t { return byte_num_; }

inline auto InStream::eof() -> bool { return seek() == EOF; }

inline auto InStream::seek_eof() -> bool {
  if (!strict_) skip_blanks();
  return eof();
}

inline auto InStream::eoln() -> bool { return seek() == '\n'; }

inline auto InStream::seek_eoln() -> bool {
  if (!strict_) skip_blanks();
  return eoln();
}

inline auto InStream::next_line() -> void {
  int c;
  do {
    c = read();
  } while (c != EOF && c != '\n');
}

inline auto InStream::read_token() -> std::string {
  if (!strict_) skip_blanks();

  std::string token;
  while (true) {
    if (int c = seek(); c == EOF || std::isspace(c)) break;
    token.push_back(static_cast<char>(read()));
  }
  return token;
}

inline auto InStream::read_line() -> std::optional<std::string> {
  std::string line;
  if (eof()) return std::nullopt;
  while (true) {
    int c = read();
    if (c == EOF || c == '\n') break;
    line.push_back(static_cast<char>(c));
  }
  return line;
}
}  // namespace cplib::io
