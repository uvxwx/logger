#include "logger/statistics.hpp"

#include <iomanip>
#include <sstream>

namespace logger {

// Добавляет сообщение в счетчики и часовое окно.
void Statistics::AddMessage(
    const LogRecord& record,
    const std::chrono::system_clock::time_point receipt_time) {
  const std::size_t length = record.message.size();
  ++total_messages_;
  ++messages_by_level_[LevelIndex(record.level)];
  total_length_ += length;
  recent_messages_.push_back(receipt_time);

  if (!has_messages_) {
    min_length_ = length;
    max_length_ = length;
    has_messages_ = true;
  } else {
    if (length < min_length_) {
      min_length_ = length;
    }
    if (length > max_length_) {
      max_length_ = length;
    }
  }

  dirty_ = true;
}

// Удаляет из окна сообщения старше одного часа.
bool Statistics::ExpireOld(const std::chrono::system_clock::time_point now) {
  const auto cutoff = now - std::chrono::hours(1);
  const std::size_t original_size = recent_messages_.size();
  // В deque времена лежат по возрастанию, поэтому устаревшие элементы снимаются
  // только с головы.
  while (!recent_messages_.empty() && recent_messages_.front() < cutoff) {
    recent_messages_.pop_front();
  }

  if (recent_messages_.size() != original_size) {
    dirty_ = true;
    return true;
  }

  return false;
}

// Сбрасывает флаг измененности.
void Statistics::ClearDirty() noexcept { dirty_ = false; }

// Возвращает текущий флаг измененности.
bool Statistics::IsDirty() const noexcept { return dirty_; }

// Проверяет, были ли сообщения.
bool Statistics::HasMessages() const noexcept { return has_messages_; }

// Возвращает общее число сообщений.
std::uint64_t Statistics::TotalMessages() const noexcept {
  return total_messages_;
}

// Возвращает число сообщений по уровню.
std::uint64_t Statistics::MessagesByLevel(const LogLevel level) const noexcept {
  return messages_by_level_[LevelIndex(level)];
}

// Возвращает минимальную длину сообщения.
std::size_t Statistics::MinMessageLength() const noexcept {
  return min_length_;
}

// Возвращает максимальную длину сообщения.
std::size_t Statistics::MaxMessageLength() const noexcept {
  return max_length_;
}

// Вычисляет среднюю длину сообщения.
double Statistics::AverageMessageLength() const noexcept {
  if (total_messages_ == 0U) {
    return 0.0;
  }

  return static_cast<double>(total_length_) /
         static_cast<double>(total_messages_);
}

// Возвращает размер окна последнего часа.
std::size_t Statistics::LastHourCount() const noexcept {
  return recent_messages_.size();
}

// Преобразует уровень в индекс массива счетчиков.
std::size_t Statistics::LevelIndex(const LogLevel level) noexcept {
  return static_cast<std::size_t>(level);
}

// Формирует человекочитаемый текст статистики.
std::string FormatStatistics(const Statistics& statistics) {
  std::ostringstream stream{};
  stream << "Statistics\n"
         << "  Total messages: " << statistics.TotalMessages() << '\n'
         << "  Debug: " << statistics.MessagesByLevel(LogLevel::kDebug) << '\n'
         << "  Info: " << statistics.MessagesByLevel(LogLevel::kInfo) << '\n'
         << "  Error: " << statistics.MessagesByLevel(LogLevel::kError) << '\n'
         << "  Last hour: " << statistics.LastHourCount() << '\n'
         << "  Minimum message length: " << statistics.MinMessageLength()
         << '\n'
         << "  Maximum message length: " << statistics.MaxMessageLength()
         << '\n'
         << "  Average message length: " << std::fixed << std::setprecision(2)
         << statistics.AverageMessageLength() << '\n';
  return stream.str();
}

}  // namespace logger
