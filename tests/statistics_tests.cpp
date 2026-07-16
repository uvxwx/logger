#include <chrono>

#include "logger/statistics.hpp"
#include "test_framework.hpp"
#include "test_utils.hpp"

// Проверяет базовые счетчики и длины сообщений.
TEST(StatisticsTracksCountsAndLengths) {
  logger::Statistics statistics;
  auto now = std::chrono::system_clock::now();

  statistics.AddMessage(test::MakeRecord("abc", logger::LogLevel::kDebug), now);
  statistics.AddMessage(test::MakeRecord("abcdef", logger::LogLevel::kError),
                        now);

  EXPECT_TRUE(statistics.HasMessages());
  EXPECT_EQ(statistics.TotalMessages(), 2U);
  EXPECT_EQ(statistics.MessagesByLevel(logger::LogLevel::kDebug), 1U);
  EXPECT_EQ(statistics.MessagesByLevel(logger::LogLevel::kInfo), 0U);
  EXPECT_EQ(statistics.MessagesByLevel(logger::LogLevel::kError), 1U);
  EXPECT_EQ(statistics.MinMessageLength(), std::size_t{3});
  EXPECT_EQ(statistics.MaxMessageLength(), std::size_t{6});
  EXPECT_EQ(statistics.LastHourCount(), std::size_t{2});
  EXPECT_TRUE(statistics.AverageMessageLength() > 4.4);
  EXPECT_TRUE(statistics.AverageMessageLength() < 4.6);
}

// Проверяет удаление сообщений старше часа.
TEST(StatisticsExpiresOlderThanOneHour) {
  logger::Statistics statistics;
  auto now = std::chrono::system_clock::now();

  statistics.AddMessage(test::MakeRecord("old"), now - std::chrono::hours(2));
  statistics.AddMessage(test::MakeRecord("fresh"),
                        now - std::chrono::minutes(10));
  statistics.ClearDirty();

  EXPECT_TRUE(statistics.ExpireOld(now));
  EXPECT_EQ(statistics.LastHourCount(), std::size_t{1});
  EXPECT_TRUE(statistics.IsDirty());
}

// Проверяет сохранение сообщения на точной границе часа.
TEST(StatisticsKeepsExactOneHourBoundary) {
  logger::Statistics statistics;
  auto now = std::chrono::system_clock::now();
  auto boundary = now - std::chrono::hours(1);

  statistics.AddMessage(test::MakeRecord("boundary"), boundary);
  statistics.ClearDirty();

  EXPECT_FALSE(statistics.ExpireOld(now));
  EXPECT_EQ(statistics.LastHourCount(), std::size_t{1});
}

// Проверяет сброс флага измененности.
TEST(StatisticsClearDirtyResetsDirtyFlag) {
  logger::Statistics statistics;
  statistics.AddMessage(test::MakeRecord("x"),
                        std::chrono::system_clock::now());

  EXPECT_TRUE(statistics.IsDirty());
  statistics.ClearDirty();
  EXPECT_FALSE(statistics.IsDirty());
}

// Проверяет состав текстового вывода статистики.
TEST(FormatStatisticsIncludesAllFields) {
  logger::Statistics statistics;
  auto now = std::chrono::system_clock::now();
  statistics.AddMessage(test::MakeRecord("abc", logger::LogLevel::kInfo), now);

  const std::string text = logger::FormatStatistics(statistics);
  EXPECT_CONTAINS(text, "Statistics");
  EXPECT_CONTAINS(text, "Total messages: 1");
  EXPECT_CONTAINS(text, "Info: 1");
  EXPECT_CONTAINS(text, "Average message length:");
}
