#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "logger/blocking_queue.hpp"
#include "test_framework.hpp"

// Проверяет сохранение FIFO-порядка.
TEST(BlockingQueuePreservesFifoOrder) {
  logger::BlockingQueue<int> queue;
  queue.Push(1);
  queue.Push(2);

  EXPECT_EQ(queue.WaitPop(), 1);
  EXPECT_EQ(queue.WaitPop(), 2);
}

// Проверяет блокировку потребителя до появления данных.
TEST(BlockingQueueConsumerWaitsForData) {
  logger::BlockingQueue<int> queue;
  std::atomic<bool> popped{false};
  int value = 0;

  std::thread consumer{[&queue, &popped, &value] {
    value = queue.WaitPop();
    popped.store(true);
  }};

  std::this_thread::sleep_for(std::chrono::milliseconds{50});
  EXPECT_FALSE(popped.load());
  queue.Push(42);
  consumer.join();

  EXPECT_TRUE(popped.load());
  EXPECT_EQ(value, 42);
}

// Проверяет отсутствие потерь при нескольких producers.
TEST(BlockingQueueHandlesMultipleProducersWithoutLoss) {
  logger::BlockingQueue<int> queue;
  constexpr int kProducerCount = 4;
  constexpr int kValuesPerProducer = 25;
  std::vector<std::thread> producers;

  for (int producer = 0; producer < kProducerCount; ++producer) {
    producers.emplace_back([producer, &queue] {
      for (int index = 0; index < kValuesPerProducer; ++index) {
        queue.Push(producer * kValuesPerProducer + index);
      }
    });
  }

  for (auto& producer : producers) {
    producer.join();
  }

  int count = 0;
  for (int index = 0; index < kProducerCount * kValuesPerProducer; ++index) {
    queue.WaitPop();
    ++count;
  }

  EXPECT_EQ(count, kProducerCount * kValuesPerProducer);
}
