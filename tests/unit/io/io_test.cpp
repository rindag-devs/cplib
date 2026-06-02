#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <ios>
#include <memory>
#include <sstream>
#include <string>

#include "cplib.hpp"
#include "gtest/gtest.h"
#include "test_utils.hpp"

using cplib::io::InStream;
using cplib_test::make_test_reader;
using cplib_test::TestExitException;

namespace {
int partial_write_call_count = 0;
int interrupted_write_call_count = 0;
int would_block_write_call_count = 0;
int failing_write_call_count = 0;
int interrupted_read_call_count = 0;
int failing_read_call_count = 0;

inline auto partial_write_once(int, const void *, std::size_t len) -> ssize_t {
  ++partial_write_call_count;
  if (partial_write_call_count == 1) {
    return static_cast<ssize_t>(len > 3 ? 3 : len);
  }
  return static_cast<ssize_t>(len);
}

inline auto interrupted_then_partial_write(int, const void *, std::size_t len) -> ssize_t {
  ++interrupted_write_call_count;
  if (interrupted_write_call_count == 1) {
    errno = EINTR;
    return -1;
  }
  return static_cast<ssize_t>(len > 2 ? 2 : len);
}

inline auto partial_then_would_block(int, const void *, std::size_t len) -> ssize_t {
  ++would_block_write_call_count;
  if (would_block_write_call_count == 1) {
    return static_cast<ssize_t>(len > 3 ? 3 : len);
  }
  errno = EAGAIN;
  return -1;
}

inline auto always_fail_write(int, const void *, std::size_t) -> ssize_t {
  ++failing_write_call_count;
  errno = EIO;
  return -1;
}

inline auto partial_then_fail_write(int, const void *, std::size_t len) -> ssize_t {
  ++failing_write_call_count;
  if (failing_write_call_count == 1) {
    return static_cast<ssize_t>(len > 3 ? 3 : len);
  }
  errno = EIO;
  return -1;
}

inline auto interrupted_then_read(int, void *data, std::size_t len) -> ssize_t {
  ++interrupted_read_call_count;
  if (interrupted_read_call_count == 1) {
    errno = EINTR;
    return -1;
  }
  if (len > 0) {
    static_cast<char *>(data)[0] = 'x';
  }
  return 1;
}

inline auto always_fail_read(int, void *, std::size_t) -> ssize_t {
  ++failing_read_call_count;
  errno = EIO;
  return -1;
}
}  // namespace

TEST(IoTest, StrictModeWhitespace) {
  std::string content = "   token";
  auto buf = std::make_unique<std::stringbuf>(content);
  // Strict = true
  InStream stream(std::make_unique<std::stringbuf>(content), "test", true);
  // In strict mode, if first char is space, returns empty string.
  EXPECT_EQ(stream.read_token(), "");
}

TEST(IoTest, NonStrictSkipping) {
  std::string content = "   token";
  InStream stream(std::make_unique<std::stringbuf>(content), "test", false);
  EXPECT_EQ(stream.read_token(), "token");
}

TEST(IoTest, PositionTracking) {
  // Line 0: 012\n
  // Line 1: 45
  std::string content = "abc\nde";
  InStream stream(std::make_unique<std::stringbuf>(content), "test", false);

  EXPECT_EQ(stream.read(), 'a');  // byte 1, col 1
  EXPECT_EQ(stream.pos().line, 0);
  EXPECT_EQ(stream.pos().col, 1);

  stream.read();  // b
  stream.read();  // c
  stream.read();  // \n

  EXPECT_EQ(stream.pos().line, 1);
  EXPECT_EQ(stream.pos().col, 0);

  EXPECT_EQ(stream.read(), 'd');
}

TEST(IoTest, EofCheck) {
  InStream stream(std::make_unique<std::stringbuf>(""), "test", false);
  EXPECT_TRUE(stream.eof());
  EXPECT_TRUE(stream.seek_eof());
}

TEST(IoTest, WriteAllRetriesAfterPartialWrites) {
  partial_write_call_count = 0;
  constexpr char data[] = "abcdef";
  const auto written = cplib::io::detail::write_all(-1, data, 6, partial_write_once);
  EXPECT_EQ(written, 6);
}

TEST(IoTest, WriteAllRetriesAfterEintr) {
  interrupted_write_call_count = 0;
  constexpr char data[] = "abcdef";
  const auto written = cplib::io::detail::write_all(-1, data, 6, interrupted_then_partial_write);
  EXPECT_EQ(written, 6);
}

TEST(IoTest, WriteAllReturnsPartialCountOnWouldBlock) {
  would_block_write_call_count = 0;
  constexpr char data[] = "abcdef";
  const auto written = cplib::io::detail::write_all(-1, data, 6, partial_then_would_block);
  EXPECT_EQ(written, 3);
}

TEST(IoTest, WriteAllPanicsOnHardFailureBeforeAnyByteIsWritten) {
  failing_write_call_count = 0;
  auto old_handler = std::move(cplib::detail::panic_impl);
  cplib::detail::panic_impl = [](std::string_view message) -> void {
    throw std::runtime_error(std::string(message));
  };

  constexpr char data[] = "abcdef";
  EXPECT_THROW(static_cast<void>(cplib::io::detail::write_all(-1, data, 6, always_fail_write)),
               std::runtime_error);
  EXPECT_EQ(failing_write_call_count, 1);

  cplib::detail::panic_impl = std::move(old_handler);
}

TEST(IoTest, WriteAllPanicsOnHardFailureAfterPartialWrite) {
  failing_write_call_count = 0;
  auto old_handler = std::move(cplib::detail::panic_impl);
  cplib::detail::panic_impl = [](std::string_view message) -> void {
    throw std::runtime_error(std::string(message));
  };

  constexpr char data[] = "abcdef";
  EXPECT_THROW(
      static_cast<void>(cplib::io::detail::write_all(-1, data, 6, partial_then_fail_write)),
      std::runtime_error);
  EXPECT_EQ(failing_write_call_count, 2);

  cplib::detail::panic_impl = std::move(old_handler);
}

TEST(IoTest, ReadAvailableRetriesAfterEintr) {
  interrupted_read_call_count = 0;
  char buffer[1] = {'\0'};
  const auto num_read = cplib::io::detail::read_available(-1, buffer, 1, interrupted_then_read);
  EXPECT_EQ(num_read, 1);
  EXPECT_EQ(buffer[0], 'x');
}

TEST(IoTest, ReadAvailablePanicsOnHardFailure) {
  failing_read_call_count = 0;
  auto old_handler = std::move(cplib::detail::panic_impl);
  cplib::detail::panic_impl = [](std::string_view message) -> void {
    throw std::runtime_error(std::string(message));
  };

  char buffer[1] = {'\0'};
  EXPECT_THROW(
      static_cast<void>(cplib::io::detail::read_available(-1, buffer, 1, always_fail_read)),
      std::runtime_error);
  EXPECT_EQ(failing_read_call_count, 1);

  cplib::detail::panic_impl = std::move(old_handler);
}

TEST(IoTest, OutBufWritesCompletePayload) {
  int pipe_fds[2];
  ASSERT_EQ(pipe(pipe_fds), 0);

  {
    cplib::io::OutBuf out(pipe_fds[1]);
    const std::string payload = "hello through outbuf";
    EXPECT_EQ(out.sputn(payload.data(), static_cast<std::streamsize>(payload.size())),
              static_cast<std::streamsize>(payload.size()));
  }

  std::string result(20, '\0');
  const auto bytes_read = read(pipe_fds[0], result.data(), result.size());
  ASSERT_EQ(bytes_read, static_cast<ssize_t>(result.size()));
  EXPECT_EQ(result, "hello through outbuf");

  close(pipe_fds[0]);
  close(pipe_fds[1]);
}
