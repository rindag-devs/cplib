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

/* cplib_embed_ignore start */
#include <vector>
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
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <map>
#include <memory>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>

/* cplib_embed_ignore start */
#include "json.hpp"
#include "macros.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::io {

inline Position::Position() : line(0), col(0), byte(0) {}

inline Position::Position(std::size_t line, std::size_t col, std::size_t byte)
    : line(line), col(col), byte(byte) {}

inline auto Position::to_json() const -> std::unique_ptr<json::Map> {
  std::map<std::string, std::unique_ptr<json::Value>> map;

  map.emplace("line", std::make_unique<json::Int>(line));
  map.emplace("col", std::make_unique<json::Int>(col));
  map.emplace("byte", std::make_unique<json::Int>(byte));

  return std::make_unique<json::Map>(std::move(map));
}

InBuf::InBuf(int fd) : fd_(fd), need_close_(false) {
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

inline InBuf::InBuf(std::string_view path) : need_close_(true) {
  int flags = 0;
#ifdef ON_WINDOWS
  flags |= _O_RDONLY;
#ifndef ONLINE_JUDGE
  flags |= _O_BINARY;
#endif
#else
  flags |= O_RDONLY;
#endif
  fd_ = open(path.data(), flags);
  if (fd_ < 0) {
    panic("Failed to open file: " + std::string(path));
  }
  setg(buf_.data() + PB_SIZE,   // Beginning of putback area
       buf_.data() + PB_SIZE,   // Read position
       buf_.data() + PB_SIZE);  // End position
}

inline InBuf::~InBuf() {
  if (need_close_) {
    close(fd_);
  }
}

inline auto InBuf::underflow() -> int_type {
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
  ++pos_.byte;
  if (c == '\n') {
    ++pos_.line, pos_.col = 0;
  } else {
    ++pos_.col;
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
  if (pos().byte > 0) {
    panic(format("Can't set strict mode of input stream `%s` when not at the beginning of the file",
                 name().data()));
  }
  strict_ = b;
}

inline auto InStream::pos() const -> Position { return pos_; }

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

// https://www.josuttis.com/cppcode/fdstream.html
inline OutBuf::OutBuf(int fd) : fd_(fd), need_close_(false) {
  /*
    We recommend using binary mode on Windows. However, Codeforces Polygon
    doesn’t think so. Since the only Online Judge that uses Windows seems to
    be Codeforces, make it happy.
  */
#if defined(ON_WINDOWS) && !defined(ONLINE_JUDGE)
  _setmode(fd_, _O_BINARY);
#endif
}

inline OutBuf::OutBuf(std::string_view path) : need_close_(true) {
  int flags = 0;
#ifdef ON_WINDOWS
  flags |= _O_WRONLY | _O_CREAT | _O_TRUNC;
#ifndef ONLINE_JUDGE
  flags |= _O_BINARY;
#endif
#else
  flags |= O_WRONLY | O_CREAT | O_TRUNC;
#endif
  fd_ = open(path.data(), flags, 0666);
  if (fd_ < 0) {
    panic("Failed to open file: " + std::string(path));
  }
}

inline OutBuf::~OutBuf() {
  if (need_close_) {
    close(fd_);
  }
}

inline auto OutBuf::overflow(int_type c) -> int_type {
  if (c != EOF) {
    auto z = static_cast<char>(c);
    if (write(fd_, &z, 1) != 1) {
      return EOF;
    }
  }
  return c;
}

inline auto OutBuf::xsputn(const char* s, std::streamsize num) -> std::streamsize {
  return write(fd_, s, num);
}

namespace detail {
// Open the givin file and create a `std::::ostream`
inline auto make_ostream_by_path(std::string_view path, std::unique_ptr<std::streambuf>& buf,
                                 std::ostream& stream) -> void {
  buf = std::make_unique<io::OutBuf>(path);
  stream.rdbuf(buf.get());
}

// Use file with givin fileno as output stream and create a `std::::ostream`
inline auto make_ostream_by_fileno(int fileno, std::unique_ptr<std::streambuf>& buf,
                                   std::ostream& stream) -> void {
  buf = std::make_unique<io::OutBuf>(fileno);
  stream.rdbuf(buf.get());
}
}  // namespace detail
}  // namespace cplib::io
