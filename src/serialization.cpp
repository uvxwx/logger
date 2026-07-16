#include "logger/serialization.hpp"

#include <chrono>

namespace logger {
namespace {

// Формат пакета: 1 байт уровня, 8 байт времени, 4 байта длины, затем payload.
constexpr std::size_t kHeaderSize = 13U;

// Дописывает 32-битное число в сетевом порядке байт.
void AppendUint32(std::string& output, const std::uint32_t value) {
  output.push_back(static_cast<char>((value >> 24U) & 0xFFU));
  output.push_back(static_cast<char>((value >> 16U) & 0xFFU));
  output.push_back(static_cast<char>((value >> 8U) & 0xFFU));
  output.push_back(static_cast<char>(value & 0xFFU));
}

// Дописывает 64-битное число в сетевом порядке байт.
void AppendUint64(std::string& output, const std::uint64_t value) {
  for (int shift = 56; shift >= 0; shift -= 8) {
    output.push_back(static_cast<char>((value >> shift) & 0xFFU));
  }
}

// Читает 32-битное число из буфера по смещению.
bool ReadUint32(std::string_view input, const std::size_t offset,
                std::uint32_t& value) {
  if (offset + sizeof(std::uint32_t) > input.size()) {
    return false;
  }

  value = (static_cast<std::uint32_t>(static_cast<unsigned char>(input[offset]))
           << 24U) |
          (static_cast<std::uint32_t>(
               static_cast<unsigned char>(input[offset + 1U]))
           << 16U) |
          (static_cast<std::uint32_t>(
               static_cast<unsigned char>(input[offset + 2U]))
           << 8U) |
          static_cast<std::uint32_t>(
              static_cast<unsigned char>(input[offset + 3U]));
  return true;
}

// Читает 64-битное число из буфера по смещению.
bool ReadUint64(std::string_view input, const std::size_t offset,
                std::uint64_t& value) {
  if (offset + sizeof(std::uint64_t) > input.size()) {
    return false;
  }

  value = 0;
  for (std::size_t index = 0; index < sizeof(std::uint64_t); ++index) {
    value =
        (value << 8U) | static_cast<std::uint64_t>(
                            static_cast<unsigned char>(input[offset + index]));
  }

  return true;
}

// Проверяет и восстанавливает уровень логирования из байта.
bool ReadLevel(const unsigned char code, LogLevel& level) {
  if (code > static_cast<unsigned char>(LogLevel::kError)) {
    return false;
  }

  level = static_cast<LogLevel>(code);
  return true;
}

}  // namespace

// Преобразует запись в бинарный формат датаграммы.
Expected<std::string, Error> SerializeRecord(const LogRecord& record) noexcept {
  if (record.message.size() > kMaxUdpMessageSize) {
    return Error{LogError::kSerializationFailed};
  }

  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                          record.timestamp.time_since_epoch())
                          .count();
  if (milliseconds < 0) {
    return Error{LogError::kSerializationFailed};
  }

  std::string serialized{};
  // Буфер собирается вручную, чтобы сетевой формат был фиксированным и
  // платформенно-независимым.
  serialized.reserve(kHeaderSize + record.message.size());
  serialized.push_back(static_cast<char>(record.level));
  AppendUint64(serialized, static_cast<std::uint64_t>(milliseconds));
  AppendUint32(serialized, static_cast<std::uint32_t>(record.message.size()));
  serialized.append(record.message);
  return serialized;
}

// Восстанавливает запись из бинарной датаграммы.
Expected<LogRecord, Error> DeserializeRecord(
    const std::string_view payload) noexcept {
  if (payload.size() < kHeaderSize) {
    return Error{LogError::kSerializationFailed};
  }

  LogLevel level = LogLevel::kInfo;
  if (!ReadLevel(static_cast<unsigned char>(payload[0]), level)) {
    return Error{LogError::kSerializationFailed};
  }

  std::uint64_t timestamp_millis = 0;
  std::uint32_t message_length = 0;
  if (!ReadUint64(payload, 1U, timestamp_millis) ||
      !ReadUint32(payload, 9U, message_length)) {
    return Error{LogError::kSerializationFailed};
  }

  if (message_length > kMaxUdpMessageSize) {
    return Error{LogError::kSerializationFailed};
  }

  // Принимаем только пакет точного размера, чтобы не маскировать обрезанные и
  // лишние байты.
  if (payload.size() != kHeaderSize + message_length) {
    return Error{LogError::kSerializationFailed};
  }

  return LogRecord{
      std::chrono::system_clock::time_point(
          std::chrono::milliseconds{timestamp_millis}),
      level,
      std::string{payload.substr(kHeaderSize, message_length)},
  };
}

}  // namespace logger
