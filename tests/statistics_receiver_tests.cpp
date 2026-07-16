#include "statistics_receiver_state.hpp"
#include "test_framework.hpp"

// Проверяет сброс счетчика после любого вывода статистики.
TEST(StatisticsReportStateResetsCountAfterTimeoutReport) {
  logger::StatisticsReportState state;

  state.OnMessage();
  state.OnMessage();
  EXPECT_EQ(state.MessagesSinceLastReport(), 2U);
  EXPECT_FALSE(state.ShouldReportByCount(3U));

  state.OnReport();
  EXPECT_EQ(state.MessagesSinceLastReport(), 0U);

  state.OnMessage();
  EXPECT_EQ(state.MessagesSinceLastReport(), 1U);
  EXPECT_FALSE(state.ShouldReportByCount(3U));
}

// Проверяет печать ровно на каждом N-м сообщении после последнего вывода.
TEST(StatisticsReportStateTriggersExactlyOnNthMessage) {
  logger::StatisticsReportState state;

  state.OnMessage();
  EXPECT_FALSE(state.ShouldReportByCount(2U));

  state.OnMessage();
  EXPECT_TRUE(state.ShouldReportByCount(2U));

  state.OnReport();
  EXPECT_FALSE(state.ShouldReportByCount(2U));

  state.OnMessage();
  EXPECT_FALSE(state.ShouldReportByCount(2U));

  state.OnMessage();
  EXPECT_TRUE(state.ShouldReportByCount(2U));
}
