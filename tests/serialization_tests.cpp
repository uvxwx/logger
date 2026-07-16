#include <string>

#include "logger/serialization.hpp"
#include "test_framework.hpp"
#include "test_utils.hpp"

// Проверяет round-trip сериализации и десериализации.
TEST(SerializeRoundTripPreservesRecord) {
  const logger::LogRecord input =
      test::MakeRecord("payload", logger::LogLevel::kError);

  auto serialized = logger::SerializeRecord(input);
  EXPECT_TRUE(serialized);

  auto output = logger::DeserializeRecord(serialized.Value());
  EXPECT_TRUE(output);
  EXPECT_EQ(output.Value().level, logger::LogLevel::kError);
  EXPECT_EQ(output.Value().message, std::string("payload"));
  EXPECT_EQ(output.Value().timestamp, input.timestamp);
}

// Проверяет поддержку всех уровней логирования.
TEST(SerializeRoundTripSupportsEveryLevel) {
  for (auto level : {logger::LogLevel::kDebug, logger::LogLevel::kInfo,
                     logger::LogLevel::kError}) {
    auto serialized = logger::SerializeRecord(test::MakeRecord("x", level));
    EXPECT_TRUE(serialized);

    auto output = logger::DeserializeRecord(serialized.Value());
    EXPECT_TRUE(output);
    EXPECT_EQ(output.Value().level, level);
  }
}

// Проверяет поддержку пустого сообщения.
TEST(SerializeRoundTripSupportsEmptyPayload) {
  auto serialized =
      logger::SerializeRecord(test::MakeRecord("", logger::LogLevel::kInfo));
  EXPECT_TRUE(serialized);

  auto output = logger::DeserializeRecord(serialized.Value());
  EXPECT_TRUE(output);
  EXPECT_TRUE(output.Value().message.empty());
}

// Проверяет отказ для слишком большого сообщения.
TEST(SerializeRejectsOversizedPayload) {
  std::string oversized(logger::kMaxUdpMessageSize + 1U, 'x');
  auto result = logger::SerializeRecord(test::MakeRecord(oversized));

  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kSerializationFailed);
}

// Проверяет отказ для неизвестного кода уровня.
TEST(DeserializeRejectsUnknownLevel) {
  std::string payload(13, '\0');
  payload[0] = static_cast<char>(9);

  auto result = logger::DeserializeRecord(payload);
  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kSerializationFailed);
}

// Проверяет отказ при несовпадении объявленной длины.
TEST(DeserializeRejectsIncorrectDeclaredLength) {
  auto serialized = logger::SerializeRecord(test::MakeRecord("hello"));
  EXPECT_TRUE(serialized);
  serialized.Value()[12] = static_cast<char>(9);

  auto result = logger::DeserializeRecord(serialized.Value());
  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kSerializationFailed);
}

// Проверяет отказ для усеченного заголовка.
TEST(DeserializeRejectsTruncatedHeader) {
  auto result = logger::DeserializeRecord("LOG");
  EXPECT_FALSE(result);
  EXPECT_EQ(result.Error().code, logger::LogError::kSerializationFailed);
}
