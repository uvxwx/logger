#include <memory>
#include <optional>
#include <vector>

#include "logger/logger.hpp"
#include "test_framework.hpp"

namespace {

// Тестовый sink, который либо запоминает записи, либо возвращает заранее
// заданную ошибку.
class MemorySink final : public logger::Sink {
 public:
  // Либо запоминает запись, либо возвращает заранее заданную ошибку.
  logger::Expected<void, logger::Error> Write(
      const logger::LogRecord& record) noexcept override {
    if (forced_error_.has_value()) {
      return *forced_error_;
    }

    records.push_back(record);
    return {};
  }

  std::vector<logger::LogRecord> records;
  std::optional<logger::Error> forced_error_;
};

}  // namespace

// Проверяет отбрасывание сообщений ниже минимального уровня.
TEST(LoggerDiscardsMessagesBelowMinimumLevel) {
  auto sink = std::make_unique<MemorySink>();
  MemorySink& sink_ref = *sink;
  logger::Logger logger{std::move(sink), logger::LogLevel::kInfo};

  auto result = logger.Log("hidden", logger::LogLevel::kDebug);

  EXPECT_TRUE(result);
  EXPECT_TRUE(sink_ref.records.empty());
}

// Проверяет запись сообщений подходящего уровня.
TEST(LoggerWritesMessagesAtOrAboveMinimumLevel) {
  auto sink = std::make_unique<MemorySink>();
  MemorySink& sink_ref = *sink;
  logger::Logger logger{std::move(sink), logger::LogLevel::kInfo};

  EXPECT_TRUE(logger.Log("kept", logger::LogLevel::kInfo));
  EXPECT_TRUE(logger.Log("also kept", logger::LogLevel::kError));

  EXPECT_EQ(sink_ref.records.size(), std::size_t{2});
  EXPECT_EQ(sink_ref.records[0].message, std::string{"kept"});
  EXPECT_EQ(sink_ref.records[1].level, logger::LogLevel::kError);
}

// Проверяет немедленное применение нового минимального уровня.
TEST(LoggerSetMinLevelTakesEffectImmediately) {
  auto sink = std::make_unique<MemorySink>();
  MemorySink& sink_ref = *sink;
  logger::Logger logger{std::move(sink), logger::LogLevel::kError};

  EXPECT_TRUE(logger.Log("discarded", logger::LogLevel::kInfo));
  logger.SetMinLevel(logger::LogLevel::kInfo);
  EXPECT_TRUE(logger.Log("accepted", logger::LogLevel::kInfo));

  EXPECT_EQ(sink_ref.records.size(), std::size_t{1});
  EXPECT_EQ(sink_ref.records.front().message, std::string{"accepted"});
}

// Проверяет использование текущего минимального уровня в перегрузке по
// умолчанию.
TEST(LoggerUsesCurrentMinimumLevelForDefaultCalls) {
  auto sink = std::make_unique<MemorySink>();
  MemorySink& sink_ref = *sink;
  logger::Logger logger{std::move(sink), logger::LogLevel::kInfo};

  EXPECT_TRUE(logger.Log("first"));
  logger.SetMinLevel(logger::LogLevel::kError);
  EXPECT_TRUE(logger.Log("second"));

  EXPECT_EQ(sink_ref.records.size(), std::size_t{2});
  EXPECT_EQ(sink_ref.records[0].level, logger::LogLevel::kInfo);
  EXPECT_EQ(sink_ref.records[1].level, logger::LogLevel::kError);
}

// Проверяет отклонение пустого сообщения.
TEST(LoggerRejectsEmptyMessages) {
  auto sink = std::make_unique<MemorySink>();
  MemorySink& sink_ref = *sink;
  logger::Logger logger{std::move(sink), logger::LogLevel::kDebug};

  auto result = logger.Log("", logger::LogLevel::kDebug);

  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kInvalidMessage);
  EXPECT_TRUE(sink_ref.records.empty());
}

// Проверяет распространение ошибки sink наружу.
TEST(LoggerPropagatesSinkErrors) {
  auto sink = std::make_unique<MemorySink>();
  sink->forced_error_ = logger::Error{logger::LogError::kWriteFailed};
  logger::Logger logger{std::move(sink), logger::LogLevel::kDebug};

  auto result = logger.Log("boom", logger::LogLevel::kError);

  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kWriteFailed);
}

// Проверяет ошибку при попытке логировать без sink.
TEST(LoggerRejectsNullSink) {
  logger::Logger logger{nullptr, logger::LogLevel::kDebug};

  auto result = logger.Log("boom", logger::LogLevel::kError);

  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kInvalidArgument);
  EXPECT_EQ(result.Error().message.value(), std::string{"logger sink is null"});
}

// Проверяет, что отсутствие sink не маскируется фильтрацией по уровню.
TEST(LoggerRejectsNullSinkEvenForFilteredMessage) {
  logger::Logger logger{nullptr, logger::LogLevel::kError};

  auto result = logger.Log("hidden", logger::LogLevel::kDebug);

  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kInvalidArgument);
  EXPECT_EQ(result.Error().message.value(), std::string{"logger sink is null"});
}
