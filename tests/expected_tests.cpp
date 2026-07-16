#include <string>

#include "logger/error.hpp"
#include "logger/expected.hpp"
#include "test_framework.hpp"

// Проверяет успешное хранение значения в Expected.
TEST(ExpectedHoldsValue) {
  logger::Expected<int, logger::Error> value(42);

  EXPECT_TRUE(value);
  EXPECT_EQ(value.Value(), 42);
}

// Проверяет хранение ошибки в Expected.
TEST(ExpectedHoldsError) {
  logger::Expected<int, logger::Error> value(
      logger::Error{logger::LogError::kInvalidArgument, "bad"});

  EXPECT_FALSE(value);
  EXPECT_EQ(value.Error().code, logger::LogError::kInvalidArgument);
  EXPECT_EQ(value.Error().message.value(), std::string("bad"));
}

// Проверяет специализацию Expected<void, E>.
TEST(ExpectedVoidSupportsSuccessAndFailure) {
  logger::Expected<void, logger::Error> success;
  EXPECT_TRUE(success);

  logger::Expected<void, logger::Error> failure(
      logger::Error{logger::LogError::kWriteFailed});
  EXPECT_FALSE(failure);
  EXPECT_EQ(failure.Error().code, logger::LogError::kWriteFailed);
}
