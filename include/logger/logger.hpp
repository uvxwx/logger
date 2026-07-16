#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string_view>

#include "logger/expected.hpp"
#include "logger/sink.hpp"

namespace logger {

// Потокобезопасно фильтрует записи по уровню и передает их в sink.
class Logger {
 public:
  // Создает логгер с sink и минимальным уровнем.
  Logger(std::unique_ptr<Sink> sink, LogLevel min_level);
  ~Logger() = default;

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  // Логирует сообщение с заданным уровнем.
  Expected<void, Error> Log(std::string_view message, LogLevel level) noexcept;
  // Логирует сообщение с текущим минимальным уровнем.
  Expected<void, Error> Log(std::string_view message) noexcept;

  // Меняет минимальный уровень фильтрации.
  void SetMinLevel(LogLevel level) noexcept;
  // Возвращает текущий минимальный уровень.
  LogLevel MinLevel() const noexcept;

 private:
  std::unique_ptr<Sink> sink_;
  std::atomic<LogLevel> min_level_;
  std::mutex sink_mutex_;
};

}  // namespace logger
