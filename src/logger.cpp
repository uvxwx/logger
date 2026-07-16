#include "logger/logger.hpp"

#include <chrono>
#include <utility>

namespace logger {
namespace {

// Преобразует уровень в числовой приоритет для сравнения.
constexpr int SeverityRank(const LogLevel level) noexcept {
  return static_cast<int>(level);
}

}  // namespace

// Сохраняет sink и начальный уровень фильтрации.
Logger::Logger(std::unique_ptr<Sink> sink, const LogLevel min_level)
    : sink_(std::move(sink)), min_level_(min_level) {}

// Отбрасывает пустые или слишком низкие по уровню сообщения и пишет остальные.
Expected<void, Error> Logger::Log(const std::string_view message,
                                  const LogLevel level) noexcept {
  if (message.empty()) {
    return Error{LogError::kInvalidMessage};
  }

  if (!sink_) {
    return Error{
        LogError::kInvalidArgument,
        "logger sink is null",
    };
  }

  LogLevel current_min_level = min_level_.load();
  if (SeverityRank(level) < SeverityRank(current_min_level)) {
    return {};
  }

  LogRecord record{
      std::chrono::system_clock::now(),
      level,
      std::string{message},
  };

  // Сам sink защищаем отдельным mutex, чтобы записи не перемешивались при
  // параллельной записи.
  std::lock_guard<std::mutex> lock{sink_mutex_};
  return sink_->Write(record);
}

// Логирует сообщение с текущим минимальным уровнем.
Expected<void, Error> Logger::Log(const std::string_view message) noexcept {
  return Log(message, MinLevel());
}

// Обновляет минимальный уровень фильтрации.
void Logger::SetMinLevel(const LogLevel level) noexcept {
  min_level_.store(level);
}

// Возвращает текущий минимальный уровень фильтрации.
LogLevel Logger::MinLevel() const noexcept { return min_level_.load(); }

}  // namespace logger
