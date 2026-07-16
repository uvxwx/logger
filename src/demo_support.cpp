#include "demo_support.hpp"

#include <atomic>
#include <iostream>
#include <thread>
#include <utility>

namespace logger {
namespace {

// Создает действие с ошибкой разбора ввода.
InputAction MakeInvalidAction(std::string message) {
  InputAction action;
  action.kind = InputActionKind::kInvalid;
  action.error_message = std::move(message);
  return action;
}

}  // namespace

// Разбирает строку пользователя в команду или сообщение.
InputAction ParseInputLine(const std::string_view line) {
  if (line == ":quit") {
    return {InputActionKind::kQuit, {}, LogLevel::kInfo, {}};
  }

  if (line == ":help") {
    return {InputActionKind::kHelp, {}, LogLevel::kInfo, {}};
  }

  if (line.rfind(":level ", 0) == 0) {
    const std::string_view level_name = line.substr(7);
    auto level = ParseLogLevel(level_name);
    if (!level.has_value()) {
      return MakeInvalidAction("invalid level: " + std::string{level_name});
    }

    InputAction action;
    action.kind = InputActionKind::kSetLevel;
    action.new_level = *level;
    return action;
  }

  if (!line.empty() && line.front() == ':') {
    return MakeInvalidAction("unknown command: " + std::string{line});
  }

  const std::size_t first_space = line.find(' ');
  const std::string_view first_word = first_space == std::string_view::npos
                                          ? line
                                          : line.substr(0, first_space);
  auto parsed_level = ParseLogLevel(first_word);
  if (parsed_level.has_value()) {
    InputAction action;
    action.kind = InputActionKind::kLogMessage;
    action.log_command.level = *parsed_level;
    action.log_command.message =
        first_space == std::string_view::npos
            ? ""
            : std::string{line.substr(first_space + 1)};
    return action;
  }

  InputAction action;
  action.kind = InputActionKind::kLogMessage;
  action.log_command.message = std::string{line};
  return action;
}

// Печатает подсказку по допустимому вводу.
void PrintHelp(std::ostream& output) {
  output << "Input format:\n"
         << "  debug <message>\n"
         << "  info <message>\n"
         << "  error <message>\n"
         << "  <message without a level>\n"
         << "Commands:\n"
         << "  :help\n"
         << "  :level <debug|info|error>\n"
         << "  :quit\n";
}

// Запускает фонового писателя и обрабатывает консольные команды.
int RunConsoleLogger(std::istream& input, std::ostream& output,
                     std::ostream& error, std::unique_ptr<Sink> sink,
                     const LogLevel min_level) {
  if (!sink) {
    error << "failed to initialize log sink\n";
    return 1;
  }

  Logger logger{std::move(sink), min_level};
  BlockingQueue<QueuedCommand> queue;
  std::atomic<bool> write_failed{false};

  std::thread writer{[&logger, &queue, &error, &write_failed] {
    while (true) {
      QueuedCommand command = queue.WaitPop();
      switch (command.kind) {
        case QueuedCommandKind::kLogMessage: {
          auto result = command.log_command.level.has_value()
                            ? logger.Log(command.log_command.message,
                                         *command.log_command.level)
                            : logger.Log(command.log_command.message);
          if (!result) {
            write_failed.store(true);
            error << "log write failed: " << DescribeError(result.Error())
                  << '\n';
          }
          break;
        }
        case QueuedCommandKind::kSetLevel:
          logger.SetMinLevel(command.new_level);
          break;
        case QueuedCommandKind::kShutdown:
          return;
      }
    }
  }};

  auto stop_writer = [&queue, &writer] {
    // Завершение по явной команде, а не через специальное
    // состояние очереди.
    queue.Push({QueuedCommandKind::kShutdown, {}, LogLevel::kInfo});
    writer.join();
  };

  std::string line{};
  while (std::getline(input, line)) {
    InputAction action = ParseInputLine(line);
    switch (action.kind) {
      case InputActionKind::kLogMessage:
        queue.Push({QueuedCommandKind::kLogMessage, action.log_command,
                    LogLevel::kInfo});
        break;
      case InputActionKind::kSetLevel:
        queue.Push({QueuedCommandKind::kSetLevel, {}, action.new_level});
        output << "minimum level set to " << ToString(action.new_level) << '\n';
        break;
      case InputActionKind::kHelp:
        PrintHelp(output);
        break;
      case InputActionKind::kQuit:
        stop_writer();
        return write_failed.load() ? 1 : 0;
      case InputActionKind::kInvalid:
        error << action.error_message << '\n';
        break;
    }
  }

  if (!input.eof() && input.fail()) {
    error << "input error\n";
  }

  stop_writer();
  return write_failed.load() ? 1 : 0;
}

}  // namespace logger
