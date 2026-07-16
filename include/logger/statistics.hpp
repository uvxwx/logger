#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

#include "logger/log_record.hpp"

namespace logger {

// Накопляет общую статистику и скользящее окно за последний час.
class Statistics {
 public:
  // Добавляет сообщение в агрегированную статистику.
  void AddMessage(const LogRecord& record,
                  std::chrono::system_clock::time_point receipt_time);
  // Удаляет из часового окна устаревшие сообщения.
  bool ExpireOld(std::chrono::system_clock::time_point now);

  // Сбрасывает флаг измененности.
  void ClearDirty() noexcept;
  // Показывает, менялась ли статистика.
  bool IsDirty() const noexcept;
  // Проверяет, были ли сообщения вообще.
  bool HasMessages() const noexcept;

  // Возвращает общее число сообщений.
  std::uint64_t TotalMessages() const noexcept;
  // Возвращает число сообщений заданного уровня.
  std::uint64_t MessagesByLevel(LogLevel level) const noexcept;
  // Возвращает минимальную длину сообщения.
  std::size_t MinMessageLength() const noexcept;
  // Возвращает максимальную длину сообщения.
  std::size_t MaxMessageLength() const noexcept;
  // Возвращает среднюю длину сообщения.
  double AverageMessageLength() const noexcept;
  // Возвращает число сообщений за последний час.
  std::size_t LastHourCount() const noexcept;

 private:
  // Преобразует уровень в индекс массива счетчиков.
  static std::size_t LevelIndex(LogLevel level) noexcept;

  std::uint64_t total_messages_ = 0;
  std::array<std::uint64_t, 3> messages_by_level_{};
  std::size_t min_length_ = 0;
  std::size_t max_length_ = 0;
  std::uint64_t total_length_ = 0;
  // Время прихода сообщений хранится по порядку, чтобы быстро чистить хвост
  // окна.
  std::deque<std::chrono::system_clock::time_point> recent_messages_;
  bool has_messages_ = false;
  bool dirty_ = false;
};

// Форматирует статистику в текстовый блок.
std::string FormatStatistics(const Statistics& statistics);

}  // namespace logger
