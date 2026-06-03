#include <sstream>
#include <string>

#include "cplib.hpp"
#include "gtest/gtest.h"

TEST(ValidatorReporterTest, JsonSkipsHiddenRootTree) {
  cplib::trace::TraceTreeNode<cplib::var::ReaderTrace> root(
      cplib::var::ReaderTrace("root", cplib::io::Position(0, 0, 0)));
  root.tags.emplace("#hidden", cplib::json::Value(true));

  cplib::validator::JsonReporter reporter;
  reporter.attach_trace_tree(&root);

  std::ostringstream captured;
  auto *old_buf = std::clog.rdbuf(captured.rdbuf());
  const auto restore = [old_buf]() -> void { std::clog.rdbuf(old_buf); };

  const auto exit_code =
      reporter.report(cplib::validator::Report(cplib::validator::Report::Status::VALID, "ok"));
  restore();

  EXPECT_EQ(exit_code, EXIT_SUCCESS);
  EXPECT_EQ(captured.str(), "{\"message\":\"ok\",\"status\":\"valid\"}\n");
}
