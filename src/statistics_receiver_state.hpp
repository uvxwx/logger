#pragma once

#include <cstdint>

namespace logger {

// Отслеживает количество сообщений с момента последнего вывода статистики.
class StatisticsReportState {
 public:
  // Увеличивает счетчик после приема очередного сообщения.
  void OnMessage() noexcept { ++messages_since_output_; }

  // Проверяет, пора ли печатать статистику по правилу каждого N-го сообщения.
  bool ShouldReportByCount(std::uint64_t report_every_n) const noexcept {
    return messages_since_output_ >= report_every_n;
  }

  // Сбрасывает счетчик после любого фактического вывода статистики.
  void OnReport() noexcept { messages_since_output_ = 0; }

  // Возвращает текущее число сообщений с момента прошлого вывода.
  std::uint64_t MessagesSinceLastReport() const noexcept {
    return messages_since_output_;
  }

 private:
  std::uint64_t messages_since_output_ = 0;
};

}  // namespace logger
