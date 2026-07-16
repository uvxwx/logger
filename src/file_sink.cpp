#include "logger/file_sink.hpp"

#include "logger/record_formatter.hpp"

namespace logger {

// Инициализирует sink уже открытым потоком.
FileSink::FileSink(std::filesystem::path path, std::ofstream stream)
    : path_(std::move(path)), stream_(std::move(stream)) {}

// Создает файловый sink и проверяет корректность пути.
Expected<std::unique_ptr<Sink>, Error> FileSink::Make(
    const std::filesystem::path& path) {
  std::error_code error_code;
  if (std::filesystem::is_directory(path, error_code)) {
    return Error{
        LogError::kInvalidArgument,
        "log path points to a directory",
    };
  }

  std::ofstream stream(path, std::ios::app);
  if (!stream.is_open()) {
    return Error{
        LogError::kSystemError,
        "failed to open log file '" + path.string() + "'",
    };
  }

  return std::unique_ptr<Sink>(new FileSink(path, std::move(stream)));
}

// Форматирует и записывает одну строку в файл.
Expected<void, Error> FileSink::Write(const LogRecord& record) noexcept {
  stream_ << FormatLogRecordLine(record) << '\n';
  stream_.flush();

  if (!stream_) {
    return Error{
        LogError::kWriteFailed,
        "failed to write log record to '" + path_.string() + "'",
    };
  }

  return {};
}

}  // namespace logger
