#pragma once

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>

#include "logger/blocking_queue.hpp"
#include "logger/logger.hpp"
#include "logger/sink.hpp"

namespace logger {

struct LogCommand {
  std::string message;
  std::optional<LogLevel> level;
};

enum class QueuedCommandKind {
  kLogMessage,
  kSetLevel,
  kShutdown,
};

struct QueuedCommand {
  QueuedCommandKind kind = QueuedCommandKind::kLogMessage;
  LogCommand log_command;
  LogLevel new_level = LogLevel::kInfo;
};

enum class InputActionKind {
  kLogMessage,
  kSetLevel,
  kHelp,
  kQuit,
  kInvalid,
};

struct InputAction {
  InputActionKind kind = InputActionKind::kInvalid;
  LogCommand log_command;
  LogLevel new_level = LogLevel::kInfo;
  std::string error_message;
};

// Разбирает строку ввода в действие консольной программы.
InputAction ParseInputLine(std::string_view line);
// Печатает краткую справку по командам.
void PrintHelp(std::ostream& output);
// Запускает консольный цикл логирования поверх sink.
int RunConsoleLogger(std::istream& input, std::ostream& output,
                     std::ostream& error, std::unique_ptr<Sink> sink,
                     LogLevel min_level);

}  // namespace logger
