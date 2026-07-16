#pragma once

#include "logger/error.hpp"
#include "logger/expected.hpp"
#include "logger/log_record.hpp"

namespace logger {

// Абстрактный получатель готовых записей лога.
class Sink {
 public:
  // Освобождает ресурсы конкретной реализации sink.
  virtual ~Sink() = default;
  // Принимает одну готовую запись лога.
  virtual Expected<void, Error> Write(const LogRecord& record) noexcept = 0;
};

}  // namespace logger
