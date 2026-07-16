#include <filesystem>
#include <memory>
#include <sstream>

#include "demo_support.hpp"
#include "logger/file_sink.hpp"
#include "test_framework.hpp"
#include "test_utils.hpp"

// Проверяет обработку ввода и смену уровня во время работы.
TEST(LoggerDemoProcessesInputAndRuntimeLevelChanges) {
  auto path = test::MakeTempPath("logger_demo");
  std::istringstream input{
      "debug hidden\n"
      "info visible\n"
      ":level debug\n"
      "debug shown\n"
      ":quit\n"};
  std::ostringstream output{};
  std::ostringstream error{};

  auto sink = logger::FileSink::Make(path);
  EXPECT_TRUE(sink);

  const int exit_code = logger::RunConsoleLogger(
      input, output, error, std::move(sink.Value()), logger::LogLevel::kInfo);

  const std::string content = test::ReadFile(path);
  std::filesystem::remove(path);

  EXPECT_EQ(exit_code, 0);
  EXPECT_TRUE(error.str().empty());
  EXPECT_CONTAINS(output.str(), "minimum level set to DEBUG");
  EXPECT_FALSE(content.find("hidden") != std::string::npos);
  EXPECT_CONTAINS(content, "visible");
  EXPECT_CONTAINS(content, "shown");
}

// Проверяет разбор команд и сообщений с уровнем.
TEST(ParseInputLineRecognizesCommandsAndLevels) {
  const logger::InputAction action = logger::ParseInputLine(":level error");
  EXPECT_EQ(action.kind, logger::InputActionKind::kSetLevel);
  EXPECT_EQ(action.new_level, logger::LogLevel::kError);

  const logger::InputAction log_action =
      logger::ParseInputLine("debug hello world");
  EXPECT_EQ(log_action.kind, logger::InputActionKind::kLogMessage);
  EXPECT_EQ(log_action.log_command.level.value(), logger::LogLevel::kDebug);
  EXPECT_EQ(log_action.log_command.message, std::string{"hello world"});
}

// Проверяет ошибку запуска консольного логгера без sink.
TEST(LoggerDemoRejectsNullSink) {
  std::istringstream input{};
  std::ostringstream output{};
  std::ostringstream error{};

  const int exit_code = logger::RunConsoleLogger(input, output, error, nullptr,
                                                 logger::LogLevel::kInfo);

  EXPECT_EQ(exit_code, 1);
  EXPECT_TRUE(output.str().empty());
  EXPECT_CONTAINS(error.str(), "failed to initialize log sink");
}
